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

#define ICON_SIZE 40
#define ICON_HILITE_WIDTH 2
#define ICON_HILITE_MARGIN 1
#define OUTSIDE_BORDER 3
#define TEXT_BORDER 2

typedef struct _ObFocusCyclePopup       ObFocusCyclePopup;
typedef struct _ObFocusCyclePopupTarget ObFocusCyclePopupTarget;

struct _ObFocusCyclePopupTarget
{
    ObClient *client;
    gchar *text;
    Window iconwin;
    Window textwin;
};

struct _ObFocusCyclePopup
{
    ObWindow obwin;
    Window bg;

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
    return XCreateWindow(obt_display, parent, 0, 0, 1, 1, bwidth,
                         RrDepth(ob_rr_inst), InputOutput,
                         RrVisual(ob_rr_inst), mask, attr);
}

void focus_cycle_popup_startup(gboolean reconfig)
{
    XSetWindowAttributes attrib;

    single_popup = icon_popup_new();

    popup.obwin.type = OB_WINDOW_CLASS_INTERNAL;
    popup.a_bg = RrAppearanceCopy(ob_rr_theme->osd_hilite_bg);
    popup.a_text = RrAppearanceCopy(ob_rr_theme->osd_hilite_label);
    popup.a_icon = RrAppearanceCopy(ob_rr_theme->a_clear);

    popup.a_text->surface.parent = popup.a_bg;
    popup.a_icon->surface.parent = popup.a_bg;

    RrAppearanceAddTextures(popup.a_icon, 2);

    popup.a_icon->texture[1].type = RR_TEXTURE_RGBA;

    attrib.override_redirect = True;
    attrib.border_pixel=RrColorPixel(ob_rr_theme->osd_border_color);
    popup.bg = create_window(obt_root(ob_screen), ob_rr_theme->obwidth,
                             CWOverrideRedirect | CWBorderPixel, &attrib);

    popup.targets = NULL;
    popup.n_targets = 0;
    popup.last_target = NULL;

    popup.hilite_rgba = NULL;

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

    g_free(popup.hilite_rgba);
    popup.hilite_rgba = NULL;

    XDestroyWindow(obt_display, popup.bg);

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
                XMapWindow(obt_display, t->textwin);

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

static void popup_render(ObFocusCyclePopup *p, const ObClient *c)
{
    gint ml, mt, mr, mb;
    gint l, t, r, b;
    gint x, y, w, h;
    Rect *screen_area = NULL;
    gint rgbax, rgbay, rgbaw, rgbah;
    gint innerw, innerh;
    gint i;
    GList *it;
    const ObFocusCyclePopupTarget *newtarget;
    gint newtargetx, newtargety;

    screen_area = screen_physical_area_active();

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

    /* find the height of the dialog */
#warning limit the height and scroll entries somehow
    h = t + b + (p->n_targets * ICON_SIZE) + OUTSIDE_BORDER;

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
        /* position the background but don't draw it*/
        XMoveResizeWindow(obt_display, p->bg, x, y, w, h);

        /* set up the hilite texture for the icon */
        p->a_icon->texture[0].data.rgba.width = ICON_SIZE;
        p->a_icon->texture[0].data.rgba.height = ICON_SIZE;
        p->a_icon->texture[0].data.rgba.alpha = 0xff;
        p->hilite_rgba = g_new(RrPixel32, ICON_SIZE * ICON_SIZE);
        p->a_icon->texture[0].data.rgba.data = p->hilite_rgba;
    }

    /* find the focused target */
    for (i = 0, it = p->targets; it; ++i, it = g_list_next(it)) {
        const ObFocusCyclePopupTarget *target = it->data;

        if (target->client == c) {
            /* save the target */
            newtarget = target;
            newtargetx = l;
            newtargety = t + i * ICON_SIZE;

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
        for (x = 0; x < ICON_SIZE; x++)
            for (y = 0; y < ICON_SIZE; y++) {
                guchar a;

                if (x < ICON_HILITE_WIDTH ||
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
    if (!p->mapped)
        RrPaint(p->a_bg, p->bg, w, h);

    /* draw the icons and text */
    for (i = 0, it = p->targets; it; ++i, it = g_list_next(it)) {
        const ObFocusCyclePopupTarget *target = it->data;

        /* have to redraw the targetted icon and last targetted icon,
           they can pick up the hilite changes in the backgroud */
        if (!p->mapped || newtarget == target || p->last_target == target) {
            const ObClientIcon *icon;
            gint innerx, innery;

            /* find the dimensions of the icon inside it */
            innerx = l;
            innerx += ICON_HILITE_WIDTH + ICON_HILITE_MARGIN;
            innery = t + i * ICON_SIZE;
            innery += ICON_HILITE_WIDTH + ICON_HILITE_MARGIN;

            /* move the icon */
            XMoveResizeWindow(obt_display, target->iconwin,
                              innerx, innery, innerw, innerh);

            /* move the text */
            XMoveResizeWindow(obt_display, target->textwin,
                              innerx + ICON_SIZE, innery,
                              w - innerx - ICON_SIZE - OUTSIDE_BORDER, innerh);

            /* get the icon from the client */
            icon = client_icon(target->client, innerw, innerh);
            p->a_icon->texture[1].data.rgba.width = icon->width;
            p->a_icon->texture[1].data.rgba.height = icon->height;
            p->a_icon->texture[1].data.rgba.alpha =
                target->client->iconic ? OB_ICONIC_ALPHA : 0xff;
            p->a_icon->texture[1].data.rgba.data = icon->data;

            /* Draw the hilite? */
#warning do i have to add more obrender interface thingers to get it to draw the icon inside the hilight? sigh
            p->a_icon->texture[0].type = (target == newtarget) ?
                                         RR_TEXTURE_RGBA : RR_TEXTURE_NONE;

            /* draw the icon */
            p->a_icon->surface.parentx = innerx;
            p->a_icon->surface.parenty = innery;
            RrPaint(p->a_icon, target->iconwin, innerw, innerh);

            /* draw the text */
            p->a_text->texture[0].data.text.string = target->text;
            p->a_text->surface.parentx = innerx + ICON_SIZE;
            p->a_text->surface.parenty = innery;
            RrPaint(p->a_text, target->textwin, w - innerx - ICON_SIZE - OUTSIDE_BORDER, innerh);
        }
    }

    p->last_target = newtarget;

    g_free(screen_area);
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
    icon_popup_show(single_popup, text, client_icon(c, ICON_SIZE, ICON_SIZE));
    g_free(text);
    screen_hide_desktop_popup();
}

void focus_cycle_popup_single_hide(void)
{
    icon_popup_hide(single_popup);
}
