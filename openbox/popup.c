/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   popup.c for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
   Copyright (c) 2003-2007   Dana Jansens

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   See the COPYING file for a copy of the GNU General Public License.
*/

#include "popup.h"

#include "openbox.h"
#include "frame.h"
#include "client.h"
#include "stacking.h"
#include "event.h"
#include "screen.h"
#include "mainloop.h"
#include "render/render.h"
#include "render/theme.h"

static gboolean popup_show_timeout(gpointer data)
{
    ObPopup *self = data;
    
    XMapWindow(ob_display, self->bg);
    stacking_raise(INTERNAL_AS_WINDOW(self));
    self->mapped = TRUE;
    self->delay_mapped = FALSE;

    return FALSE; /* don't repeat */
}

ObPopup *popup_new(gboolean hasicon)
{
    XSetWindowAttributes attrib;
    ObPopup *self = g_new0(ObPopup, 1);

    self->obwin.type = Window_Internal;
    self->hasicon = hasicon;
    self->gravity = NorthWestGravity;
    self->x = self->y = self->w = self->h = 0;
    self->a_bg = RrAppearanceCopy(ob_rr_theme->osd_hilite_bg);
    self->a_text = RrAppearanceCopy(ob_rr_theme->osd_hilite_label);

    attrib.override_redirect = True;
    self->bg = XCreateWindow(ob_display, RootWindow(ob_display, ob_screen),
                             0, 0, 1, 1, 0, RrDepth(ob_rr_inst),
                             InputOutput, RrVisual(ob_rr_inst),
                             CWOverrideRedirect, &attrib);
    
    self->text = XCreateWindow(ob_display, self->bg,
                               0, 0, 1, 1, 0, RrDepth(ob_rr_inst),
                               InputOutput, RrVisual(ob_rr_inst), 0, NULL);

    XMapWindow(ob_display, self->text);

    stacking_add(INTERNAL_AS_WINDOW(self));
    return self;
}

void popup_free(ObPopup *self)
{
    if (self) {
        XDestroyWindow(ob_display, self->bg);
        XDestroyWindow(ob_display, self->text);
        RrAppearanceFree(self->a_bg);
        RrAppearanceFree(self->a_text);
        stacking_remove(self);
        g_free(self);
    }
}

void popup_position(ObPopup *self, gint gravity, gint x, gint y)
{
    self->gravity = gravity;
    self->x = x;
    self->y = y;
}

void popup_width(ObPopup *self, gint w)
{
    self->w = w;
    self->maxw = w;
}

void popup_height(ObPopup *self, gint h)
{
    gint texth;

    /* don't let the height be smaller than the text */
    texth = RrMinHeight(self->a_text) + ob_rr_theme->paddingy * 2;
    self->h = MAX(h, texth);
}

void popup_width_to_string(ObPopup *self, gchar *text, gint max)
{
    self->a_text->texture[0].data.text.string = text;
    self->w = RrMinWidth(self->a_text);
    self->maxw = max;
}

void popup_height_to_string(ObPopup *self, gchar *text)
{
    self->h = RrMinHeight(self->a_text) + ob_rr_theme->paddingy * 2;
}

void popup_width_to_strings(ObPopup *self, gchar **strings, gint num, gint max)
{
    gint i, maxw;

    maxw = 0;
    for (i = 0; i < num; ++i) {
        popup_width_to_string(self, strings[i], max);
        maxw = MAX(maxw, self->w);
    }
    self->w = maxw;
}

void popup_set_text_align(ObPopup *self, RrJustify align)
{
    self->a_text->texture[0].data.text.justify = align;
}

void popup_delay_show(ObPopup *self, gulong usec, gchar *text)
{
    gint l, t, r, b;
    gint x, y, w, h;
    gint textw, texth;
    gint iconw;
    Rect *area; /* won't go outside this */

    area = screen_physical_area();          /* XXX this should work quite
                                               good, someone with xinerama,
                                               and different resolutions on
                                               screens? */

    RrMargins(self->a_bg, &l, &t, &r, &b);

    XSetWindowBorderWidth(ob_display, self->bg, ob_rr_theme->fbwidth);
    XSetWindowBorder(ob_display, self->bg, ob_rr_theme->frame_b_color->pixel);

    /* set up the textures */
    self->a_text->texture[0].data.text.string = text;

    /* measure the text out */
    RrMinSize(self->a_text, &textw, &texth);
    texth += ob_rr_theme->paddingy * 2;

    /* set the sizes up and reget the text sizes from the calculated
       outer sizes */
    if (self->h) {
        h = self->h;
        texth = h - (t+b + ob_rr_theme->paddingy * 2);
    } else
        h = t+b + texth + ob_rr_theme->paddingy * 2;
    iconw = (self->hasicon ? texth : 0);
    if (self->w)
        textw = self->w;
    w = l+r + textw + iconw + ob_rr_theme->paddingx *
        (self->hasicon ? 3 : 2);
    /* cap it at "maxw" */
    if (self->maxw)
        w = MIN(w, self->maxw);

    /* sanity checks to avoid crashes! */
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    if (textw < 1) textw = 1;
    if (texth < 1) texth = 1;

    /* set up the x coord */
    x = self->x;
    switch (self->gravity) {
    case NorthGravity:
    case CenterGravity:
    case SouthGravity:
        x -= w / 2;
        break;
    case NorthEastGravity:
    case EastGravity:
    case SouthEastGravity:
        x -= w;
        break;
    }

    /* set up the y coord */
    y = self->y;
    switch (self->gravity) {
    case WestGravity:
    case CenterGravity:
    case EastGravity:
        y -= h / 2;
        break;
    case SouthWestGravity:
    case SouthGravity:
    case SouthEastGravity:
        y -= h;
        break;
    }

    x=MAX(MIN(x, area->width-w),0);
    y=MAX(MIN(y, area->height-h),0);

    /* set the windows/appearances up */
    XMoveResizeWindow(ob_display, self->bg, x, y, w, h);

    self->a_text->surface.parent = self->a_bg;
    self->a_text->surface.parentx = l + iconw +
        ob_rr_theme->paddingx * (self->hasicon ? 2 : 1);
    self->a_text->surface.parenty = t + ob_rr_theme->paddingy;
    XMoveResizeWindow(ob_display, self->text,
                      l + iconw + ob_rr_theme->paddingx *
                      (self->hasicon ? 2 : 1),
                      t + ob_rr_theme->paddingy, textw, texth);

    RrPaint(self->a_bg, self->bg, w, h);
    RrPaint(self->a_text, self->text, textw, texth);

    if (self->hasicon) {
        if (iconw < 1) iconw = 1; /* sanity check for crashes */
        if (self->draw_icon)
            self->draw_icon(l + ob_rr_theme->paddingx,
                            t + ob_rr_theme->paddingy,
                            iconw, texth, self->draw_icon_data);
    }

    /* do the actual showing */
    if (!self->mapped) {
        if (usec) {
            /* don't kill previous show timers */
            if (!self->delay_mapped) {
                ob_main_loop_timeout_add(ob_main_loop, usec,
                                         popup_show_timeout, self,
                                         g_direct_equal, NULL);
                self->delay_mapped = TRUE;
            }
        } else {
            popup_show_timeout(self);
        }
    }
}

void popup_hide(ObPopup *self)
{
    if (self->mapped) {
        XUnmapWindow(ob_display, self->bg);
        self->mapped = FALSE;

        /* kill enter events cause by this unmapping */
        event_ignore_queued_enters();
    } else if (self->delay_mapped) {
        ob_main_loop_timeout_remove(ob_main_loop, popup_show_timeout);
        self->delay_mapped = FALSE;
    }
}

static void icon_popup_draw_icon(gint x, gint y, gint w, gint h, gpointer data)
{
    ObIconPopup *self = data;

    self->a_icon->surface.parent = self->popup->a_bg;
    self->a_icon->surface.parentx = x;
    self->a_icon->surface.parenty = y;
    XMoveResizeWindow(ob_display, self->icon, x, y, w, h);
    RrPaint(self->a_icon, self->icon, w, h);
}

ObIconPopup *icon_popup_new()
{
    ObIconPopup *self;

    self = g_new0(ObIconPopup, 1);
    self->popup = popup_new(TRUE);
    self->a_icon = RrAppearanceCopy(ob_rr_theme->a_clear_tex);
    self->icon = XCreateWindow(ob_display, self->popup->bg,
                               0, 0, 1, 1, 0,
                               RrDepth(ob_rr_inst), InputOutput,
                               RrVisual(ob_rr_inst), 0, NULL);
    XMapWindow(ob_display, self->icon);

    self->popup->draw_icon = icon_popup_draw_icon;
    self->popup->draw_icon_data = self;

    return self;
}

void icon_popup_free(ObIconPopup *self)
{
    if (self) {
        XDestroyWindow(ob_display, self->icon);
        RrAppearanceFree(self->a_icon);
        popup_free(self->popup);
        g_free(self);
    }
}

void icon_popup_delay_show(ObIconPopup *self, gulong usec,
                           gchar *text, const ObClientIcon *icon)
{
    if (icon) {
        self->a_icon->texture[0].type = RR_TEXTURE_RGBA;
        self->a_icon->texture[0].data.rgba.width = icon->width;
        self->a_icon->texture[0].data.rgba.height = icon->height;
        self->a_icon->texture[0].data.rgba.data = icon->data;
    } else
        self->a_icon->texture[0].type = RR_TEXTURE_NONE;

    popup_delay_show(self->popup, usec, text);
}

static void pager_popup_draw_icon(gint px, gint py, gint w, gint h,
                                  gpointer data)
{
    ObPagerPopup *self = data;
    gint x, y;
    guint rown, n;
    guint horz_inc;
    guint vert_inc;
    guint r, c;
    gint eachw, eachh;

    eachw = (w - ob_rr_theme->fbwidth -
             (screen_desktop_layout.columns * ob_rr_theme->fbwidth))
        / screen_desktop_layout.columns;
    eachh = (h - ob_rr_theme->fbwidth -
             (screen_desktop_layout.rows * ob_rr_theme->fbwidth))
        / screen_desktop_layout.rows;
    /* make them squares */
    eachw = eachh = MIN(eachw, eachh);

    /* center */
    px += (w - (screen_desktop_layout.columns * (eachw + ob_rr_theme->fbwidth) +
                ob_rr_theme->fbwidth)) / 2;
    py += (h - (screen_desktop_layout.rows * (eachh + ob_rr_theme->fbwidth) +
                ob_rr_theme->fbwidth)) / 2;

    if (eachw <= 0 || eachh <= 0)
        return;

    switch (screen_desktop_layout.orientation) {
    case OB_ORIENTATION_HORZ:
        switch (screen_desktop_layout.start_corner) {
        case OB_CORNER_TOPLEFT:
            n = 0;
            horz_inc = 1;
            vert_inc = screen_desktop_layout.columns;
            break;
        case OB_CORNER_TOPRIGHT:
            n = screen_desktop_layout.columns - 1;
            horz_inc = -1;
            vert_inc = screen_desktop_layout.columns;
            break;
        case OB_CORNER_BOTTOMRIGHT:
            n = screen_desktop_layout.rows * screen_desktop_layout.columns - 1;
            horz_inc = -1;
            vert_inc = -screen_desktop_layout.columns;
            break;
        case OB_CORNER_BOTTOMLEFT:
            n = (screen_desktop_layout.rows - 1)
                * screen_desktop_layout.columns;
            horz_inc = 1;
            vert_inc = -screen_desktop_layout.columns;
            break;
        default:
            g_assert_not_reached();
        }
        break;
    case OB_ORIENTATION_VERT:
        switch (screen_desktop_layout.start_corner) {
        case OB_CORNER_TOPLEFT:
            n = 0;
            horz_inc = screen_desktop_layout.rows;
            vert_inc = 1;
            break;
        case OB_CORNER_TOPRIGHT:
            n = screen_desktop_layout.rows
                * (screen_desktop_layout.columns - 1);
            horz_inc = -screen_desktop_layout.rows;
            vert_inc = 1;
            break;
        case OB_CORNER_BOTTOMRIGHT:
            n = screen_desktop_layout.rows * screen_desktop_layout.columns - 1;
            horz_inc = -screen_desktop_layout.rows;
            vert_inc = -1;
            break;
        case OB_CORNER_BOTTOMLEFT:
            n = screen_desktop_layout.rows - 1;
            horz_inc = screen_desktop_layout.rows;
            vert_inc = -1;
            break;
        default:
            g_assert_not_reached();
        }
        break;
    default:
        g_assert_not_reached();
    }

    rown = n;
    for (r = 0, y = 0; r < screen_desktop_layout.rows;
         ++r, y += eachh + ob_rr_theme->fbwidth)
    {
        for (c = 0, x = 0; c < screen_desktop_layout.columns;
             ++c, x += eachw + ob_rr_theme->fbwidth)
        {
            RrAppearance *a;

            if (n < self->desks) {
                a = (n == self->curdesk ? self->hilight : self->unhilight);

                a->surface.parent = self->popup->a_bg;
                a->surface.parentx = x + px;
                a->surface.parenty = y + py;
                XMoveResizeWindow(ob_display, self->wins[n],
                                  x + px, y + py, eachw, eachh);
                RrPaint(a, self->wins[n], eachw, eachh);
            }
            n += horz_inc;
        }
        n = rown += vert_inc;
    }
}

ObPagerPopup *pager_popup_new()
{
    ObPagerPopup *self;

    self = g_new(ObPagerPopup, 1);
    self->popup = popup_new(TRUE);

    self->desks = 0;
    self->wins = g_new(Window, self->desks);
    self->hilight = RrAppearanceCopy(ob_rr_theme->osd_hilite_fg);
    self->unhilight = RrAppearanceCopy(ob_rr_theme->osd_unhilite_fg);

    self->popup->hasicon = TRUE;
    self->popup->draw_icon = pager_popup_draw_icon;
    self->popup->draw_icon_data = self;

    return self;
}

void pager_popup_free(ObPagerPopup *self)
{
    if (self) {
        guint i;

        for (i = 0; i < self->desks; ++i)
            XDestroyWindow(ob_display, self->wins[i]);
        g_free(self->wins);
        RrAppearanceFree(self->hilight);
        RrAppearanceFree(self->unhilight);
        popup_free(self->popup);
        g_free(self);
    }
}

void pager_popup_delay_show(ObPagerPopup *self, gulong usec,
                            gchar *text, guint desk)
{
    guint i;

    if (screen_num_desktops < self->desks)
        for (i = screen_num_desktops; i < self->desks; ++i)
            XDestroyWindow(ob_display, self->wins[i]);

    if (screen_num_desktops != self->desks)
        self->wins = g_renew(Window, self->wins, screen_num_desktops);

    if (screen_num_desktops > self->desks)
        for (i = self->desks; i < screen_num_desktops; ++i) {
            XSetWindowAttributes attr;

            attr.border_pixel = RrColorPixel(ob_rr_theme->frame_b_color);
            self->wins[i] = XCreateWindow(ob_display, self->popup->bg,
                                          0, 0, 1, 1, ob_rr_theme->fbwidth,
                                          RrDepth(ob_rr_inst), InputOutput,
                                          RrVisual(ob_rr_inst), CWBorderPixel,
                                          &attr);
            XMapWindow(ob_display, self->wins[i]);
        }

    self->desks = screen_num_desktops;
    self->curdesk = desk;

    popup_delay_show(self->popup, usec, text);
}
