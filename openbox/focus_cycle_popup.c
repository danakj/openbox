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
#include "openbox.h"
#include "window.h"
#include "event.h"
#include "render/render.h"

#include <X11/Xlib.h>
#include <glib.h>

/* Size of the hilite box around a window's icon */
#define HILITE_SIZE 40
/* Width of the outer ring around the hilite box */
#define HILITE_WIDTH 2
/* Space between the outer ring around the hilite box and the icon inside it */
#define HILITE_MARGIN 1
/* Total distance from the edge of the hilite box to the icon inside it */
#define HILITE_OFFSET (HILITE_WIDTH + HILITE_MARGIN)
/* Size of the icons, which can appear inside or outside of a hilite box */
#define ICON_SIZE (HILITE_SIZE - 2*HILITE_OFFSET)
/* Margin area around the outside of the dialog */
#define OUTSIDE_BORDER 3
/* Margin area around the text */
#define TEXT_BORDER 2

typedef struct _ObFocusCyclePopup       ObFocusCyclePopup;
typedef struct _ObFocusCyclePopupTarget ObFocusCyclePopupTarget;

struct _ObFocusCyclePopupTarget
{
    ObClient *client;
    gchar *text;
    Window iconwin;
    /* This is used when the popup is in list mode */
    Window textwin;
};

struct _ObFocusCyclePopup
{
    ObWindow obwin;
    Window bg;

    /* This is used when the popup is in icon mode */
    Window icon_mode_text;

    GList *targets;
    gint n_targets;

    const ObFocusCyclePopupTarget *last_target;

    gint maxtextw;

    RrAppearance *a_bg;
    RrAppearance *a_text;
    RrAppearance *a_hilite_text;
    RrAppearance *a_icon;

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
                              const ObClient *c,
                              ObFocusCyclePopupMode mode);

static Window create_window(Window parent, guint bwidth, gulong mask,
                            XSetWindowAttributes *attr)
{
    return XCreateWindow(obt_display, parent, 0, 0, 1, 1, bwidth,
                         RrDepth(ob_rr_inst), InputOutput,
                         RrVisual(ob_rr_inst), mask, attr);
}

void focus_cycle_popup_startup(gboolean reconfig)
{
    XSetWindowAttributes attrib;
    RrPixel32 *p;

    single_popup = icon_popup_new();

    popup.obwin.type = OB_WINDOW_CLASS_INTERNAL;
    popup.a_bg = RrAppearanceCopy(ob_rr_theme->osd_hilite_bg);
    popup.a_hilite_text = RrAppearanceCopy(ob_rr_theme->osd_hilite_label);
    popup.a_text = RrAppearanceCopy(ob_rr_theme->a_unfocused_label);
    popup.a_icon = RrAppearanceCopy(ob_rr_theme->a_clear);

    popup.a_hilite_text->surface.parent = popup.a_bg;
    popup.a_text->surface.parent = popup.a_bg;
    popup.a_icon->surface.parent = popup.a_bg;

    popup.a_text->texture[0].data.text.justify = RR_JUSTIFY_LEFT;
    popup.a_hilite_text->texture[0].data.text.justify = RR_JUSTIFY_LEFT;

    /* 2 textures. texture[0] is the icon.  texture[1] is the hilight, and
       may or may not be used */
    RrAppearanceAddTextures(popup.a_icon, 2);

    popup.a_icon->texture[0].type = RR_TEXTURE_RGBA;

    attrib.override_redirect = True;
    attrib.border_pixel=RrColorPixel(ob_rr_theme->osd_border_color);
    popup.bg = create_window(obt_root(ob_screen), ob_rr_theme->obwidth,
                             CWOverrideRedirect | CWBorderPixel, &attrib);

    /* create the text window used for the icon-mode popup */
    popup.icon_mode_text = create_window(popup.bg, 0, 0, NULL);

    popup.targets = NULL;
    popup.n_targets = 0;
    popup.last_target = NULL;

    /* set up the hilite texture for the icon */
    popup.a_icon->texture[1].data.rgba.width = HILITE_SIZE;
    popup.a_icon->texture[1].data.rgba.height = HILITE_SIZE;
    popup.a_icon->texture[1].data.rgba.alpha = 0xff;
    p = g_new(RrPixel32, HILITE_SIZE * HILITE_SIZE);
    popup.a_icon->texture[1].data.rgba.data = p;

    /* create the hilite under the target icon */
    {
        RrPixel32 color;
        gint x, y, o;

        color = ((ob_rr_theme->osd_color->r & 0xff) << RrDefaultRedOffset) +
            ((ob_rr_theme->osd_color->g & 0xff) << RrDefaultGreenOffset) +
            ((ob_rr_theme->osd_color->b & 0xff) << RrDefaultBlueOffset);

        o = 0;
        for (x = 0; x < HILITE_SIZE; x++)
            for (y = 0; y < HILITE_SIZE; y++) {
                guchar a;

                if (x < HILITE_WIDTH ||
                    x >= HILITE_SIZE - HILITE_WIDTH ||
                    y < HILITE_WIDTH ||
                    y >= HILITE_SIZE - HILITE_WIDTH)
                {
                    /* the border of the target */
                    a = 0x88;
                }
                else {
                    /* the background of the target */
                    a = 0x22;
                }

                p[o++] = color + (a << RrDefaultAlphaOffset);
            }
    }

    stacking_add(INTERNAL_AS_WINDOW(&popup));
    window_add(&popup.bg, INTERNAL_AS_WINDOW(&popup));
}

void focus_cycle_popup_shutdown(gboolean reconfig)
{
    icon_popup_free(single_popup);

    window_remove(popup.bg);
    stacking_remove(INTERNAL_AS_WINDOW(&popup));

    while(popup.targets) {
        ObFocusCyclePopupTarget *t = popup.targets->data;

        g_free(t->text);
        XDestroyWindow(obt_display, t->iconwin);
        XDestroyWindow(obt_display, t->textwin);

        popup.targets = g_list_delete_link(popup.targets, popup.targets);
    }

    g_free(popup.a_icon->texture[1].data.rgba.data);
    popup.a_icon->texture[1].data.rgba.data = NULL;

    XDestroyWindow(obt_display, popup.icon_mode_text);
    XDestroyWindow(obt_display, popup.bg);

    RrAppearanceFree(popup.a_icon);
    RrAppearanceFree(popup.a_hilite_text);
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

        if (focus_valid_target(ft, TRUE,
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
                t->iconwin = create_window(p->bg, 0, 0, NULL);
                t->textwin = create_window(p->bg, 0, 0, NULL);

                XMapWindow(obt_display, t->iconwin);

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
    gchar *title;
    const gchar *desk = NULL;
    gchar *ret;

    /* find our highest direct parent */
    p = client_search_top_direct_parent(c);

    if (c->desktop != DESKTOP_ALL && c->desktop != screen_desktop)
        desk = screen_desktop_names[c->desktop];

    title = c->iconic ? c->icon_title : c->title;

    /* use the transient's parent's title/icon if we don't have one */
    if (p != c && title[0] == '\0')
        title = p->iconic ? p->icon_title : p->title;

    if (desk)
        ret = g_strdup_printf("%s [%s]", title, desk);
    else
        ret = g_strdup(title);

    return ret;
}

static void popup_render(ObFocusCyclePopup *p, const ObClient *c,
                         ObFocusCyclePopupMode mode)
{
    gint ml, mt, mr, mb;
    gint l, t, r, b;
    gint x, y, w, h;
    Rect *screen_area = NULL;
    gint rgbax, rgbay, rgbaw, rgbah;
    gint i;
    GList *it;
    const ObFocusCyclePopupTarget *newtarget;
    gint icons_per_row;
    gint icon_rows;
    gint textw, texth;

    /* vars for icon mode */
    gint icon_mode_textx;
    gint icon_mode_texty;
    gint icons_center_x;

    /* vars for list mode */
    gint list_mode_icon_column_w = HILITE_SIZE + OUTSIDE_BORDER;

    g_assert(mode == OB_FOCUS_CYCLE_POPUP_MODE_ICONS ||
             mode == OB_FOCUS_CYCLE_POPUP_MODE_LIST);

    screen_area = screen_physical_area_active();

    /* get the outside margins */
    RrMargins(p->a_bg, &ml, &mt, &mr, &mb);

    /* get our outside borders */
    l = ml + OUTSIDE_BORDER;
    r = mr + OUTSIDE_BORDER;
    t = mt + OUTSIDE_BORDER;
    b = mb + OUTSIDE_BORDER;

    /* get the width from the text and keep it within limits */
    w = l + r + p->maxtextw;
    if (mode == OB_FOCUS_CYCLE_POPUP_MODE_LIST)
        /* when in list mode, there are icons down the side */
        w += list_mode_icon_column_w;
    w = MIN(w, MAX(screen_area->width/3, POPUP_WIDTH)); /* max width */
    w = MAX(w, POPUP_WIDTH); /* min width */

    /* get the text height */
    texth = RrMinHeight(p->a_hilite_text);
    if (mode == OB_FOCUS_CYCLE_POPUP_MODE_LIST)
        texth = MAX(MAX(texth, RrMinHeight(p->a_text)), ICON_SIZE);
    else
        texth += TEXT_BORDER * 2;

    if (mode == OB_FOCUS_CYCLE_POPUP_MODE_ICONS) {
        /* how many icons will fit in that row? make the width fit that */
        w -= l + r;
        icons_per_row = (w + HILITE_SIZE - 1) / HILITE_SIZE;
        w = icons_per_row * HILITE_SIZE + l + r;

        /* how many rows do we need? */
        icon_rows = (p->n_targets-1) / icons_per_row + 1;

    }
    else {
        /* in list mode, there is one column of icons.. */
        icons_per_row = 1;
        /* maximum is 80% of the screen height */
        icon_rows = MIN(p->n_targets,
                        (4*screen_area->height/5) /* 80% of the screen */
                        /
                        MAX(HILITE_SIZE, texth)); /* height of each row */
    }

    /* get the text width */
    textw = w - l - r;
    if (mode == OB_FOCUS_CYCLE_POPUP_MODE_LIST)
        /* leave space on the side for the icons */
        textw -= list_mode_icon_column_w;

    /* find the height of the dialog */
#warning limit the height and scroll entries somehow
    h = t + b + (icon_rows * MAX(HILITE_SIZE, texth));
    if (mode == OB_FOCUS_CYCLE_POPUP_MODE_ICONS)
        /* in icon mode the text sits below the icons, so make some space */
        h += OUTSIDE_BORDER + texth;

    /* center the icons if there is less than one row */
    if (icon_rows == 1)
        icons_center_x = (w - p->n_targets * HILITE_SIZE) / 2;
    else
        icons_center_x = 0;

    if (mode == OB_FOCUS_CYCLE_POPUP_MODE_ICONS) {
        /* get the position of the text */
        icon_mode_textx = l;
        icon_mode_texty = h - texth - b;
    }

    /* find the position for the popup (include the outer borders) */
    x = screen_area->x + (screen_area->width -
                          (w + ob_rr_theme->obwidth * 2)) / 2;
    y = screen_area->y + (screen_area->height -
                          (h + ob_rr_theme->obwidth * 2)) / 2;

    /* get the dimensions of the target hilite texture */
    rgbax = ml;
    rgbay = mt;
    rgbaw = w - ml - mr;
    rgbah = h - mt - mb;

    if (!p->mapped) {
        /* position the background but don't draw it */
        XMoveResizeWindow(obt_display, p->bg, x, y, w, h);

        if (mode == OB_FOCUS_CYCLE_POPUP_MODE_ICONS) {
            /* position the text */
            XMoveResizeWindow(obt_display, p->icon_mode_text,
                              icon_mode_textx, icon_mode_texty, textw, texth);
            XMapWindow(obt_display, popup.icon_mode_text);
        }
        else
            XUnmapWindow(obt_display, popup.icon_mode_text);
    }

    /* find the focused target */
    newtarget = NULL;
    for (i = 0, it = p->targets; it; ++i, it = g_list_next(it)) {
        const ObFocusCyclePopupTarget *target = it->data;
        if (target->client == c) {
            /* save the target */
            newtarget = target;
            break;
        }
    }

    g_assert(newtarget != NULL);

    /* * * draw everything * * */

    /* draw the background */
    if (!p->mapped)
        RrPaint(p->a_bg, p->bg, w, h);

    /* draw the icons and text */
    for (i = 0, it = p->targets; it; ++i, it = g_list_next(it)) {
        const ObFocusCyclePopupTarget *target = it->data;

        /* have to redraw the targetted icon and last targetted icon
         * to update the hilite */
        if (!p->mapped || newtarget == target || p->last_target == target) {
            const gint row = i / icons_per_row; /* starting from 0 */
            const gint col = i % icons_per_row; /* starting from 0 */
            const ObClientIcon *icon;
            gint iconx, icony;
            gint list_mode_textx, list_mode_texty;
            RrAppearance *text;

            /* find the coordinates for the icon */
            iconx = icons_center_x + l + (col * HILITE_SIZE);
            icony = t + (row * MAX(texth, HILITE_SIZE))
                + MAX(texth - HILITE_SIZE, 0) / 2;

            /* find the dimensions of the text box */
            list_mode_textx = iconx + HILITE_SIZE + TEXT_BORDER;
            list_mode_texty = icony + HILITE_OFFSET;

            /* position the icon */
            XMoveResizeWindow(obt_display, target->iconwin,
                              iconx, icony, HILITE_SIZE, HILITE_SIZE);

            /* position the text */
            if (mode == OB_FOCUS_CYCLE_POPUP_MODE_LIST) {
                XMoveResizeWindow(obt_display, target->textwin,
                                  list_mode_textx, list_mode_texty,
                                  textw, texth);
                XMapWindow(obt_display, target->textwin);
            }
            else
                XUnmapWindow(obt_display, target->textwin);

            /* get the icon from the client */
            icon = client_icon(target->client, ICON_SIZE, ICON_SIZE);
            p->a_icon->texture[0].data.rgba.width = icon->width;
            p->a_icon->texture[0].data.rgba.height = icon->height;
            p->a_icon->texture[0].data.rgba.twidth = ICON_SIZE;
            p->a_icon->texture[0].data.rgba.theight = ICON_SIZE;
            p->a_icon->texture[0].data.rgba.tx = HILITE_OFFSET;
            p->a_icon->texture[0].data.rgba.ty = HILITE_OFFSET;
            p->a_icon->texture[0].data.rgba.alpha =
                target->client->iconic ? OB_ICONIC_ALPHA : 0xff;
            p->a_icon->texture[0].data.rgba.data = icon->data;

            /* Draw the hilite? */
            p->a_icon->texture[1].type = (target == newtarget) ?
                                         RR_TEXTURE_RGBA : RR_TEXTURE_NONE;

            /* draw the icon */
            p->a_icon->surface.parentx = iconx;
            p->a_icon->surface.parenty = icony;
            RrPaint(p->a_icon, target->iconwin, HILITE_SIZE, HILITE_SIZE);

            /* draw the text */
            if (mode == OB_FOCUS_CYCLE_POPUP_MODE_LIST || target == newtarget)
            {
                text = (target == newtarget) ? p->a_hilite_text : p->a_text;
                text->texture[0].data.text.string = target->text;
                text->surface.parentx =
                    mode == OB_FOCUS_CYCLE_POPUP_MODE_ICONS ?
                    icon_mode_textx : list_mode_textx;
                text->surface.parenty =
                    mode == OB_FOCUS_CYCLE_POPUP_MODE_ICONS ?
                    icon_mode_texty : list_mode_texty;
                RrPaint(text,
                        (mode == OB_FOCUS_CYCLE_POPUP_MODE_ICONS ?
                         p->icon_mode_text : target->textwin),
                        textw, texth);
            }
        }
    }

    p->last_target = newtarget;

    g_free(screen_area);
}

void focus_cycle_popup_show(ObClient *c, gboolean iconic_windows,
                            gboolean all_desktops, gboolean dock_windows,
                            gboolean desktop_windows,
                            ObFocusCyclePopupMode mode)
{
    g_assert(c != NULL);

    if (mode == OB_FOCUS_CYCLE_POPUP_MODE_NONE) {
        focus_cycle_popup_hide();
        return;
    }

    /* do this stuff only when the dialog is first showing */
    if (!popup.mapped)
        popup_setup(&popup, TRUE, iconic_windows, all_desktops,
                    dock_windows, desktop_windows);
    g_assert(popup.targets != NULL);

    popup_render(&popup, c, mode);

    if (!popup.mapped) {
        /* show the dialog */
        XMapWindow(obt_display, popup.bg);
        XFlush(obt_display);
        popup.mapped = TRUE;
        screen_hide_desktop_popup();
    }
}

void focus_cycle_popup_hide(void)
{
    gulong ignore_start;

    ignore_start = event_start_ignore_all_enters();

    XUnmapWindow(obt_display, popup.bg);
    XFlush(obt_display);

    event_end_ignore_all_enters(ignore_start);

    popup.mapped = FALSE;

    while(popup.targets) {
        ObFocusCyclePopupTarget *t = popup.targets->data;

        g_free(t->text);
        XDestroyWindow(obt_display, t->iconwin);
        XDestroyWindow(obt_display, t->textwin);
        g_free(t);

        popup.targets = g_list_delete_link(popup.targets, popup.targets);
    }
    popup.n_targets = 0;
    popup.last_target = NULL;
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
        a = screen_physical_area_active();
        icon_popup_position(single_popup, CenterGravity,
                            a->x + a->width / 2, a->y + a->height / 2);
        icon_popup_height(single_popup, POPUP_HEIGHT);
        icon_popup_min_width(single_popup, POPUP_WIDTH);
        icon_popup_max_width(single_popup, MAX(a->width/3, POPUP_WIDTH));
        icon_popup_text_width(single_popup, popup.maxtextw);
        g_free(a);
    }

    text = popup_get_name(c);
    icon_popup_show(single_popup, text, client_icon(c, HILITE_SIZE,
                                                    HILITE_SIZE));
    g_free(text);
    screen_hide_desktop_popup();
}

void focus_cycle_popup_single_hide(void)
{
    icon_popup_hide(single_popup);
}
