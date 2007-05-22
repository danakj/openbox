/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   focus_cycle_popup.c for the Openbox window manager
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

#include "focus_cycle_popup.h"
#include "popup.h"
#include "client.h"
#include "screen.h"
#include "focus.h"
#include "focus_cycle.h"
#include "openbox.h"
#include "window.h"
#include "event.h"
#include "render/render.h"

#include <X11/Xlib.h>
#include <glib.h>

#define ICON_SIZE 40
#define ICON_HILITE_WIDTH 2
#define ICON_HILITE_MARGIN 1
#define OUTSIDE_BORDER 3

typedef struct _ObFocusCyclePopup       ObFocusCyclePopup;
typedef struct _ObFocusCyclePopupTarget ObFocusCyclePopupTarget;

struct _ObFocusCyclePopupTarget
{
    ObClient *client;
    gchar *text;
    Window win;
};

struct _ObFocusCyclePopup
{
    ObWindow obwin;
    Window bg;

    Window text;

    GList *targets;
    gint n_targets;

    const ObFocusCyclePopupTarget *last_target;

    gint maxtextw;

    RrAppearance *a_bg;
    RrAppearance *a_text;
    RrAppearance *a_icon;

    RrPixel32 *hilite_rgba;

    gboolean mapped;
};

/*! This popup shows all possible windows */
static ObFocusCyclePopup popup;
/*! This popup shows a single window */
static ObIconPopup *single_popup;

static gchar *popup_get_name (ObClient *c);
static void   popup_setup    (ObFocusCyclePopup *p,
                              gboolean create_targets,
                              gboolean iconic_windows,
                              gboolean all_desktops,
                              gboolean dock_windows,
                              gboolean desktop_windows);
static void   popup_render   (ObFocusCyclePopup *p,
                              const ObClient *c);

static Window create_window(Window parent, guint bwidth, gulong mask,
                            XSetWindowAttributes *attr)
{
    return XCreateWindow(ob_display, parent, 0, 0, 1, 1, bwidth,
                         RrDepth(ob_rr_inst), InputOutput,
                         RrVisual(ob_rr_inst), mask, attr);
}

void focus_cycle_popup_startup(gboolean reconfig)
{
    XSetWindowAttributes attrib;

    single_popup = icon_popup_new(TRUE);

    popup.obwin.type = Window_Internal;
    popup.a_bg = RrAppearanceCopy(ob_rr_theme->osd_hilite_bg);
    popup.a_text = RrAppearanceCopy(ob_rr_theme->osd_hilite_label);
    popup.a_icon = RrAppearanceCopy(ob_rr_theme->a_clear_tex);

    popup.a_text->surface.parent = popup.a_bg;
    popup.a_icon->surface.parent = popup.a_bg;

    popup.a_icon->texture[0].type = RR_TEXTURE_RGBA;

    RrAppearanceAddTextures(popup.a_bg, 1);
    popup.a_bg->texture[0].type = RR_TEXTURE_RGBA;

    attrib.override_redirect = True;
    attrib.border_pixel=RrColorPixel(ob_rr_theme->frame_focused_border_color);
    popup.bg = create_window(RootWindow(ob_display, ob_screen),
                             ob_rr_theme->fbwidth,
                             CWOverrideRedirect | CWBorderPixel, &attrib);

    popup.text = create_window(popup.bg, 0, 0, NULL);

    popup.targets = NULL;
    popup.n_targets = 0;
    popup.last_target = NULL;

    popup.hilite_rgba = NULL;

    XMapWindow(ob_display, popup.text);

    stacking_add(INTERNAL_AS_WINDOW(&popup));
}

void focus_cycle_popup_shutdown(gboolean reconfig)
{
    icon_popup_free(single_popup);

    stacking_remove(INTERNAL_AS_WINDOW(&popup));

    while(popup.targets) {
        ObFocusCyclePopupTarget *t = popup.targets->data;

        g_free(t->text);
        XDestroyWindow(ob_display, t->win);

        popup.targets = g_list_delete_link(popup.targets, popup.targets);
    }

    g_free(popup.hilite_rgba);

    XDestroyWindow(ob_display, popup.text);
    XDestroyWindow(ob_display, popup.bg);

    RrAppearanceFree(popup.a_icon);
    RrAppearanceFree(popup.a_text);
    RrAppearanceFree(popup.a_bg);
}

static void popup_setup(ObFocusCyclePopup *p, gboolean create_targets,
                        gboolean iconic_windows, gboolean all_desktops,
                        gboolean dock_windows, gboolean desktop_windows)
{
    gint maxwidth, n;
    GList *it;

    g_assert(p->targets == NULL);
    g_assert(p->n_targets == 0);

    /* make its width to be the width of all the possible titles */

    /* build a list of all the valid focus targets and measure their strings,
       and count them */
    maxwidth = 0;
    n = 0;
    for (it = g_list_last(focus_order); it; it = g_list_previous(it)) {
        ObClient *ft = it->data;

        if (focus_cycle_target_valid(ft,
                                     iconic_windows,
                                     all_desktops,
                                     dock_windows,
                                     desktop_windows))
        {
            gchar *text = popup_get_name(ft);

            /* measure */
            p->a_text->texture[0].data.text.string = text;
            maxwidth = MAX(maxwidth, RrMinWidth(p->a_text));

            if (!create_targets)
                g_free(text);
            else {
                ObFocusCyclePopupTarget *t = g_new(ObFocusCyclePopupTarget, 1);

                t->client = ft;
                t->text = text;
                t->win = create_window(p->bg, 0, 0, NULL);

                XMapWindow(ob_display, t->win);

                p->targets = g_list_prepend(p->targets, t);
                ++n;
            }
        }
    }

    p->n_targets = n;
    p->maxtextw = maxwidth;
}

static gchar *popup_get_name(ObClient *c)
{
    ObClient *p;
    gchar *title = NULL;
    const gchar *desk = NULL;
    gchar *ret;

    /* find our highest direct parent, including non-normal windows */
    for (p = c; p->transient_for && p->transient_for != OB_TRAN_GROUP;
         p = p->transient_for);

    if (c->desktop != DESKTOP_ALL && c->desktop != screen_desktop)
        desk = screen_desktop_names[c->desktop];

    /* use the transient's parent's title/icon if we don't have one */
    if (p != c && !strcmp("", (c->iconic ? c->icon_title : c->title)))
        title = g_strdup(p->iconic ? p->icon_title : p->title);

    if (title == NULL)
        title = g_strdup(c->iconic ? c->icon_title : c->title);

    if (desk)
        ret = g_strdup_printf("%s [%s]", title, desk);
    else {
        ret = title;
        title = NULL;
    }
    g_free(title);

    return ret;
}

static void popup_render(ObFocusCyclePopup *p, const ObClient *c)
{
    gint ml, mt, mr, mb;
    gint l, t, r, b;
    gint x, y, w, h;
    Rect *screen_area;
    gint icons_per_row;
    gint icon_rows;
    gint textx, texty, textw, texth;
    gint rgbax, rgbay, rgbaw, rgbah;
    gint icons_center_x;
    gint innerw, innerh;
    gint i;
    GList *it;
    const ObFocusCyclePopupTarget *newtarget;
    gint newtargetx, newtargety;

    /* XXX find the middle monitor? */
    screen_area = screen_physical_area_monitor(0);

    /* get the outside margins */
    RrMargins(p->a_bg, &ml, &mt, &mr, &mb);

    /* get our outside borders */
    l = ml + OUTSIDE_BORDER;
    r = mr + OUTSIDE_BORDER;
    t = mt + OUTSIDE_BORDER;
    b = mb + OUTSIDE_BORDER;

    /* get the icon pictures' sizes */
    innerw = ICON_SIZE - (ICON_HILITE_WIDTH + ICON_HILITE_MARGIN) * 2;
    innerh = ICON_SIZE - (ICON_HILITE_WIDTH + ICON_HILITE_MARGIN) * 2;

    /* get the width from the text and keep it within limits */
    w = l + r + p->maxtextw;
    w = MIN(w, MAX(screen_area->width/3, POPUP_WIDTH)); /* max width */
    w = MAX(w, POPUP_WIDTH); /* min width */

    /* how many icons will fit in that row? make the width fit that */
    w -= l + r;
    icons_per_row = (w + ICON_SIZE - 1) / ICON_SIZE;
    w = icons_per_row * ICON_SIZE + l + r;

    /* how many rows do we need? */
    icon_rows = (p->n_targets-1) / icons_per_row + 1;

    /* get the text dimensions */
    textw = w - l - r;
    texth = RrMinHeight(p->a_text);

    /* find the height of the dialog */
    h = t + b + (icon_rows * ICON_SIZE) + (OUTSIDE_BORDER*2 + texth);

    /* get the position of the text */
    textx = l;
    texty = h - texth - b;

    /* find the position for the popup (include the outer borders) */
    x = screen_area->x + (screen_area->width -
                          (w + ob_rr_theme->fbwidth * 2)) / 2;
    y = screen_area->y + (screen_area->height -
                          (h + ob_rr_theme->fbwidth * 2)) / 2;

    /* get the dimensions of the target hilite texture */
    rgbax = ml;
    rgbay = mt;
    rgbaw = w - ml - mr;
    rgbah = h - mt - mb;

    /* center the icons if there is less than one row */
    if (icon_rows == 1)
        icons_center_x = (w - p->n_targets * ICON_SIZE) / 2;
    else
        icons_center_x = 0;

    if (!p->mapped) {
        /* position the background but don't draw it*/
        XMoveResizeWindow(ob_display, p->bg, x, y, w, h);

        /* set up the hilite texture for the background */
        p->a_bg->texture[0].data.rgba.width = rgbaw;
        p->a_bg->texture[0].data.rgba.height = rgbah;
        p->hilite_rgba = g_new(RrPixel32, rgbaw * rgbah);
        p->a_bg->texture[0].data.rgba.data = p->hilite_rgba;

        /* position the text, but don't draw it */
        XMoveResizeWindow(ob_display, p->text, textx, texty, textw, texth);
        p->a_text->surface.parentx = textx;
        p->a_text->surface.parenty = texty;
    }

    /* find the focused target */
    for (i = 0, it = p->targets; it; ++i, it = g_list_next(it)) {
        const ObFocusCyclePopupTarget *target = it->data;
        const gint row = i / icons_per_row; /* starting from 0 */
        const gint col = i % icons_per_row; /* starting from 0 */

        if (target->client == c) {
            /* save the target */
            newtarget = target;
            newtargetx = icons_center_x + l + (col * ICON_SIZE);
            newtargety = t + (row * ICON_SIZE);

            if (!p->mapped)
                break; /* if we're not dimensioning, then we're done */
        }
    }

    g_assert(newtarget != NULL);

    /* create the hilite under the target icon */
    {
        RrPixel32 color;
        gint i, j, o;

        color = ((ob_rr_theme->osd_color->r & 0xff) << RrDefaultRedOffset) +
            ((ob_rr_theme->osd_color->g & 0xff) << RrDefaultGreenOffset) +
            ((ob_rr_theme->osd_color->b & 0xff) << RrDefaultBlueOffset);

        o = 0;
        for (i = 0; i < rgbah; ++i)
            for (j = 0; j < rgbaw; ++j) {
                guchar a;
                const gint x = j + rgbax - newtargetx;
                const gint y = i + rgbay - newtargety;

                if (x < 0 || x >= ICON_SIZE ||
                    y < 0 || y >= ICON_SIZE)
                {
                    /* outside the target */
                    a = 0x00;
                }
                else if (x < ICON_HILITE_WIDTH ||
                         x >= ICON_SIZE - ICON_HILITE_WIDTH ||
                         y < ICON_HILITE_WIDTH ||
                         y >= ICON_SIZE - ICON_HILITE_WIDTH)
                {
                    /* the border of the target */
                    a = 0x88;
                }
                else {
                    /* the background of the target */
                    a = 0x22;
                }

                p->hilite_rgba[o++] =
                    color + (a << RrDefaultAlphaOffset);
            }
    }

    /* * * draw everything * * */

    /* draw the background */
    RrPaint(p->a_bg, p->bg, w, h);

    /* draw the icons */
    for (i = 0, it = p->targets; it; ++i, it = g_list_next(it)) {
        const ObFocusCyclePopupTarget *target = it->data;

        /* have to redraw the targetted icon and last targetted icon,
           they can pick up the hilite changes in the backgroud */
        if (!p->mapped || newtarget == target || p->last_target == target) {
            const ObClientIcon *icon;
            const gint row = i / icons_per_row; /* starting from 0 */
            const gint col = i % icons_per_row; /* starting from 0 */
            gint innerx, innery;

            /* find the dimensions of the icon inside it */
            innerx = icons_center_x + l + (col * ICON_SIZE);
            innerx += ICON_HILITE_WIDTH + ICON_HILITE_MARGIN;
            innery = t + (row * ICON_SIZE);
            innery += ICON_HILITE_WIDTH + ICON_HILITE_MARGIN;

            /* move the icon */
            XMoveResizeWindow(ob_display, target->win,
                              innerx, innery, innerw, innerh);

            /* get the icon from the client */
            icon = client_icon(target->client, innerw, innerh);
            p->a_icon->texture[0].data.rgba.width = icon->width;
            p->a_icon->texture[0].data.rgba.height = icon->height;
            p->a_icon->texture[0].data.rgba.data = icon->data;

            /* draw the icon */
            p->a_icon->surface.parentx = innerx;
            p->a_icon->surface.parenty = innery;
            RrPaint(p->a_icon, target->win, innerw, innerh);
        }
    }

    /* draw the text */
    p->a_text->texture[0].data.text.string = newtarget->text;
    p->a_text->surface.parentx = textx;
    p->a_text->surface.parenty = texty;
    RrPaint(p->a_text, p->text, textw, texth);

    p->last_target = newtarget;
}

void focus_cycle_popup_show(ObClient *c, gboolean iconic_windows,
                            gboolean all_desktops, gboolean dock_windows,
                            gboolean desktop_windows)
{
    g_assert(c != NULL);

    /* do this stuff only when the dialog is first showing */
    if (!popup.mapped)
        popup_setup(&popup, TRUE, iconic_windows, all_desktops,
                    dock_windows, desktop_windows);
    g_assert(popup.targets != NULL);

    popup_render(&popup, c);

    if (!popup.mapped) {
        /* show the dialog */
        XMapWindow(ob_display, popup.bg);
        XFlush(ob_display);
        popup.mapped = TRUE;
    }
}

void focus_cycle_popup_hide()
{
    XUnmapWindow(ob_display, popup.bg);
    XFlush(ob_display);

    popup.mapped = FALSE;

    /* kill enter events cause by this unmapping */
    event_ignore_all_queued_enters();

    /* kill enter events cause by this unmapping */
    event_ignore_all_queued_enters();

    while(popup.targets) {
        ObFocusCyclePopupTarget *t = popup.targets->data;

        g_free(t->text);
        XDestroyWindow(ob_display, t->win);

        popup.targets = g_list_delete_link(popup.targets, popup.targets);
    }
    popup.n_targets = 0;
    popup.last_target = NULL;

    g_free(popup.hilite_rgba);
    popup.hilite_rgba = NULL;
}

void focus_cycle_popup_single_show(struct _ObClient *c,
                                   gboolean iconic_windows,
                                   gboolean all_desktops,
                                   gboolean dock_windows,
                                   gboolean desktop_windows)
{
    gchar *text;

    g_assert(c != NULL);

    /* do this stuff only when the dialog is first showing */
    if (!popup.mapped) {
        Rect *a;

        popup_setup(&popup, FALSE, iconic_windows, all_desktops,
                    dock_windows, desktop_windows);
        g_assert(popup.targets == NULL);

        /* position the popup */
        a = screen_physical_area_monitor(0);
        icon_popup_position(single_popup, CenterGravity,
                            a->x + a->width / 2, a->y + a->height / 2);
        icon_popup_height(single_popup, POPUP_HEIGHT);
        icon_popup_min_width(single_popup, POPUP_WIDTH);
        icon_popup_max_width(single_popup, MAX(a->width/3, POPUP_WIDTH));
        icon_popup_text_width(single_popup, popup.maxtextw);
    }

    text = popup_get_name(c);
    icon_popup_show(single_popup, text, client_icon(c, ICON_SIZE, ICON_SIZE));
    g_free(text);
}

void focus_cycle_popup_single_hide()
{
    icon_popup_hide(single_popup);
}
