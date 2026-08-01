/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   snap.c for the Openbox window manager

   The "snap layouts" popup.  Hovering the maximize button of a window pops up
   a small grid of layout templates; each template is drawn as a miniature of
   the monitor divided into zones.  Clicking a zone moves and resizes the
   window to fill that part of the monitor.

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

#include "snap.h"
#include "openbox.h"
#include "client.h"
#include "frame.h"
#include "screen.h"
#include "stacking.h"
#include "window.h"
#include "event.h"
#include "config.h"
#include "geom.h"
#include "obrender/render.h"
#include "obrender/theme.h"

#include <X11/Xlib.h>

/*! How long to wait after the pointer leaves the maximize button (or the
  popup) before the popup is taken down.  This gives the user time to move
  the pointer across the gap between the button and the popup. */
#define SNAP_HIDE_DELAY 250

typedef struct _ObSnapZoneWin {
    Window win;
    /*! The zone's position within the monitor's usable area, in percent */
    Rect frac;
    /*! Where the zone is drawn inside the popup */
    Rect draw;
} ObSnapZoneWin;

static struct {
    /*! Must be first, the popup is an internal window */
    ObWindow obwin;
    Window bg;

    RrAppearance *a_bg;
    RrAppearance *a_zone;
    RrAppearance *a_zone_hilite;

    /*! A list of ObSnapZoneWin, one per zone of every layout */
    GSList *zones;
    /*! The zone the pointer is inside of, or NULL */
    ObSnapZoneWin *hover;

    /*! The client the popup is (or is about to be) shown for */
    ObClient *client;

    gboolean mapped;
    guint show_timer;
    guint hide_timer;
} popup;

static gboolean started = FALSE;

static void snap_destroy_zones(void);
static void snap_client_dest(ObClient *client, gpointer data);
static gboolean snap_show_timeout(gpointer data);
static gboolean snap_hide_timeout(gpointer data);
static void snap_stop_timers(void);
static void snap_paint_zone(ObSnapZoneWin *z, gboolean hilite);
static void snap_apply(ObClient *client, const Rect *frac);
static ObSnapZoneWin* snap_find_zone(Window win);

void snap_startup(gboolean reconfig)
{
    XSetWindowAttributes attrib;

    popup.obwin.type = OB_WINDOW_CLASS_INTERNAL;
    popup.zones = NULL;
    popup.hover = NULL;
    popup.client = NULL;
    popup.mapped = FALSE;
    popup.show_timer = popup.hide_timer = 0;

    popup.a_bg = RrAppearanceCopy(ob_rr_theme->osd_bg);
    popup.a_zone = RrAppearanceCopy(ob_rr_theme->osd_unhilite_bg);
    popup.a_zone_hilite = RrAppearanceCopy(ob_rr_theme->osd_hilite_bg);

    attrib.override_redirect = True;
    attrib.event_mask = EnterWindowMask | LeaveWindowMask |
        ButtonPressMask | ButtonReleaseMask;
    popup.bg = XCreateWindow(obt_display, obt_root(ob_screen),
                             0, 0, 1, 1, 0, RrDepth(ob_rr_inst),
                             InputOutput, RrVisual(ob_rr_inst),
                             CWOverrideRedirect | CWEventMask, &attrib);
    XSetWindowBorderWidth(obt_display, popup.bg, ob_rr_theme->obwidth);
    XSetWindowBorder(obt_display, popup.bg,
                     RrColorPixel(ob_rr_theme->osd_border_color));

    stacking_add(INTERNAL_AS_WINDOW(&popup));
    window_add(&popup.bg, INTERNAL_AS_WINDOW(&popup));

    if (!reconfig)
        client_add_destroy_notify(snap_client_dest, NULL);

    started = TRUE;
}

void snap_shutdown(gboolean reconfig)
{
    started = FALSE;

    snap_stop_timers();
    snap_destroy_zones();

    if (!reconfig)
        client_remove_destroy_notify(snap_client_dest);

    window_remove(popup.bg);
    stacking_remove(INTERNAL_AS_WINDOW(&popup));
    XDestroyWindow(obt_display, popup.bg);

    RrAppearanceFree(popup.a_bg);
    RrAppearanceFree(popup.a_zone);
    RrAppearanceFree(popup.a_zone_hilite);
    popup.a_bg = popup.a_zone = popup.a_zone_hilite = NULL;

    popup.client = NULL;
    popup.hover = NULL;
    popup.mapped = FALSE;
}

static void snap_destroy_zones(void)
{
    GSList *it;

    for (it = popup.zones; it; it = g_slist_next(it)) {
        ObSnapZoneWin *z = it->data;
        XDestroyWindow(obt_display, z->win);
        g_slice_free(ObSnapZoneWin, z);
    }
    g_slist_free(popup.zones);
    popup.zones = NULL;
    popup.hover = NULL;
}

static void snap_client_dest(ObClient *client, gpointer data)
{
    if (popup.client == client)
        snap_hide();
}

static void snap_stop_timers(void)
{
    if (popup.show_timer) {
        g_source_remove(popup.show_timer);
        popup.show_timer = 0;
    }
    if (popup.hide_timer) {
        g_source_remove(popup.hide_timer);
        popup.hide_timer = 0;
    }
}

gboolean snap_visible(void)
{
    return popup.mapped;
}

gboolean snap_is_popup_window(Window win)
{
    if (!started || win == None)
        return FALSE;
    if (win == popup.bg)
        return TRUE;
    return snap_find_zone(win) != NULL;
}

static ObSnapZoneWin* snap_find_zone(Window win)
{
    GSList *it;

    for (it = popup.zones; it; it = g_slist_next(it)) {
        ObSnapZoneWin *z = it->data;
        if (z->win == win)
            return z;
    }
    return NULL;
}

/*! Can the popup be used for this client at all? */
static gboolean snap_client_ok(ObClient *client)
{
    return client != NULL &&
        (client->functions & OB_CLIENT_FUNC_MOVE) &&
        (client->functions & OB_CLIENT_FUNC_RESIZE);
}

void snap_hover_begin(ObClient *client)
{
    if (!started || !config_snap_layouts || !config_snap_layouts_list)
        return;
    if (!snap_client_ok(client))
        return;

    /* the pointer came back before we managed to take the popup down */
    if (popup.hide_timer && popup.client == client) {
        g_source_remove(popup.hide_timer);
        popup.hide_timer = 0;
        return;
    }

    if (popup.mapped && popup.client == client)
        return;

    /* a different window's button, start over */
    if (popup.client != client)
        snap_hide();

    popup.client = client;
    if (!popup.show_timer)
        popup.show_timer = g_timeout_add(config_snap_layouts_delay,
                                         snap_show_timeout, NULL);
}

void snap_hover_end(void)
{
    if (!started)
        return;

    if (popup.show_timer) {
        g_source_remove(popup.show_timer);
        popup.show_timer = 0;
    }
    if (popup.mapped && !popup.hide_timer)
        popup.hide_timer = g_timeout_add(SNAP_HIDE_DELAY,
                                         snap_hide_timeout, NULL);
    if (!popup.mapped)
        popup.client = NULL;
}

static gboolean snap_show_timeout(gpointer data)
{
    popup.show_timer = 0;
    snap_show(popup.client);
    return FALSE; /* don't repeat */
}

static gboolean snap_hide_timeout(gpointer data)
{
    popup.hide_timer = 0;
    snap_hide();
    return FALSE; /* don't repeat */
}

/*! Scale a percentage of a length, so that neighbouring zones share an edge
  rather than overlapping or leaving a hole between them. */
static inline gint snap_scale(gint percent, gint len)
{
    return (percent * len + 50) / 100;
}

void snap_show(ObClient *client)
{
    GSList *lit;
    const Rect *marea;
    Rect *uarea;
    guint mon;
    gint bw, pad, gap, seam;
    gint tw, th, cols, rows, n, nlayouts;
    gint x, y, w, h, rx, ry;
    Window child;

    if (!started || !config_snap_layouts || !config_snap_layouts_list)
        return;
    if (!snap_client_ok(client))
        return;
    /* the maximize button has to be on the titlebar for the popup to be
       anchored to it */
    if (!client->frame->max_on)
        return;

    snap_stop_timers();
    snap_destroy_zones();
    popup.client = client;

    nlayouts = g_slist_length(config_snap_layouts_list);
    cols = (nlayouts > 1 ? 2 : 1);
    rows = (nlayouts + cols - 1) / cols;

    bw = ob_rr_theme->obwidth;
    pad = MAX(4, ob_rr_theme->paddingx);
    gap = pad;
    seam = MAX(2, bw * 2);

    /* make each template a miniature of the area the zones are measured
       against, which is the monitor minus any panels on it */
    mon = client_monitor(client);
    marea = screen_physical_area_monitor(mon);
    uarea = screen_area(client->desktop, mon, NULL);
    tw = MAX(64, ob_rr_theme->button_size * 7);
    th = MAX(40, (tw * uarea->height) / MAX(1, uarea->width));
    g_slice_free(Rect, uarea);

    w = pad * 2 + cols * tw + (cols - 1) * gap;
    h = pad * 2 + rows * th + (rows - 1) * gap;

    /* anchor the popup below the middle of the maximize button */
    if (!XTranslateCoordinates(obt_display, client->frame->max,
                               obt_root(ob_screen), 0, 0, &rx, &ry, &child))
    {
        snap_hide();
        return;
    }
    x = rx + ob_rr_theme->button_size / 2 - w / 2;
    y = ry + ob_rr_theme->button_size + bw + 2;

    /* keep it on the monitor */
    x = MAX(MIN(x, marea->x + marea->width - w - bw * 2), marea->x);
    y = MAX(MIN(y, marea->y + marea->height - h - bw * 2), marea->y);

    XMoveResizeWindow(obt_display, popup.bg, x, y, w, h);
    RrPaint(popup.a_bg, popup.bg, w, h);

    /* build the templates */
    for (lit = config_snap_layouts_list, n = 0; lit;
         lit = g_slist_next(lit), ++n)
    {
        const ObSnapLayout *layout = lit->data;
        const gint tx = pad + (n % cols) * (tw + gap);
        const gint ty = pad + (n / cols) * (th + gap);
        GSList *zit;

        for (zit = layout->zones; zit; zit = g_slist_next(zit)) {
            const ObSnapZone *def = zit->data;
            ObSnapZoneWin *z;
            gint zx, zy, zw, zh;

            /* compute both edges from the percentages so that adjacent zones
               line up exactly */
            zx = snap_scale(def->x, tw);
            zy = snap_scale(def->y, th);
            zw = snap_scale(def->x + def->width, tw) - zx;
            zh = snap_scale(def->y + def->height, th) - zy;

            /* inset each zone so the background shows through as a divider */
            zx += seam / 2;
            zy += seam / 2;
            zw -= seam;
            zh -= seam;
            if (zw < 1 || zh < 1)
                continue;

            z = g_slice_new0(ObSnapZoneWin);
            RECT_SET(z->frac, def->x, def->y, def->width, def->height);
            RECT_SET(z->draw, tx + zx, ty + zy, zw, zh);
            z->win = XCreateWindow(obt_display, popup.bg,
                                   tx + zx, ty + zy, zw, zh, 0,
                                   RrDepth(ob_rr_inst), InputOutput,
                                   RrVisual(ob_rr_inst), 0, NULL);
            XSelectInput(obt_display, z->win,
                         EnterWindowMask | LeaveWindowMask |
                         ButtonPressMask | ButtonReleaseMask);
            popup.zones = g_slist_append(popup.zones, z);

            snap_paint_zone(z, FALSE);
            XMapWindow(obt_display, z->win);
        }
    }

    if (!popup.mapped) {
        XMapWindow(obt_display, popup.bg);
        stacking_raise(INTERNAL_AS_WINDOW(&popup));
        popup.mapped = TRUE;
    }
}

void snap_hide(void)
{
    if (!started)
        return;

    snap_stop_timers();

    if (popup.mapped) {
        gulong ignore_start;

        /* kill enter events caused by this unmapping */
        ignore_start = event_start_ignore_all_enters();
        XUnmapWindow(obt_display, popup.bg);
        popup.mapped = FALSE;
        event_end_ignore_all_enters(ignore_start);
    }

    snap_destroy_zones();
    popup.client = NULL;
}

static void snap_paint_zone(ObSnapZoneWin *z, gboolean hilite)
{
    RrAppearance *a = (hilite ? popup.a_zone_hilite : popup.a_zone);

    a->surface.parent = popup.a_bg;
    a->surface.parentx = z->draw.x;
    a->surface.parenty = z->draw.y;
    RrPaint(a, z->win, z->draw.width, z->draw.height);
}

static void snap_set_hover(ObSnapZoneWin *z)
{
    if (popup.hover == z)
        return;
    if (popup.hover)
        snap_paint_zone(popup.hover, FALSE);
    popup.hover = z;
    if (popup.hover)
        snap_paint_zone(popup.hover, TRUE);
}

gboolean snap_event(XEvent *e)
{
    ObSnapZoneWin *z;

    if (!started)
        return FALSE;

    switch (e->type) {
    case EnterNotify:
        /* the pointer is inside the popup, so it isn't going anywhere */
        if (popup.hide_timer) {
            g_source_remove(popup.hide_timer);
            popup.hide_timer = 0;
        }
        if ((z = snap_find_zone(e->xcrossing.window)))
            snap_set_hover(z);
        else if (e->xcrossing.window == popup.bg &&
                 e->xcrossing.detail != NotifyInferior)
            snap_set_hover(NULL);
        return TRUE;
    case LeaveNotify:
        if ((z = snap_find_zone(e->xcrossing.window))) {
            if (popup.hover == z)
                snap_set_hover(NULL);
        }
        else if (e->xcrossing.window == popup.bg &&
                 e->xcrossing.detail != NotifyInferior)
        {
            /* the pointer left the popup entirely */
            snap_set_hover(NULL);
            if (popup.mapped && !popup.hide_timer)
                popup.hide_timer = g_timeout_add(SNAP_HIDE_DELAY,
                                                 snap_hide_timeout, NULL);
        }
        return TRUE;
    case ButtonPress:
        return TRUE; /* wait for the release, but don't let anyone else
                        use the press */
    case ButtonRelease:
        if (e->xbutton.button == 1 &&
            (z = snap_find_zone(e->xbutton.window)))
        {
            ObClient *c = popup.client;
            Rect frac = z->frac;

            snap_hide();
            snap_apply(c, &frac);
        }
        else if (e->xbutton.button == 3)
            snap_hide();
        return TRUE;
    }
    return FALSE;
}

static void snap_apply(ObClient *client, const Rect *frac)
{
    Rect *area;
    gint x, y, w, h;

    if (!snap_client_ok(client))
        return;

    if (client->fullscreen)
        client_fullscreen(client, FALSE);
    if (client->shaded)
        client_shade(client, FALSE);
    if (client->max_horz || client->max_vert)
        client_maximize(client, FALSE, 0);

    area = screen_area(client->desktop, client_monitor(client), NULL);

    /* where the frame should end up */
    x = area->x + snap_scale(frac->x, area->width);
    y = area->y + snap_scale(frac->y, area->height);
    w = snap_scale(frac->x + frac->width, area->width) -
        snap_scale(frac->x, area->width);
    h = snap_scale(frac->y + frac->height, area->height) -
        snap_scale(frac->y, area->height);

    g_slice_free(Rect, area);

    /* take the decorations out of the size, and turn the frame's position
       into the client's position */
    w -= client->frame->size.left + client->frame->size.right;
    h -= client->frame->size.top + client->frame->size.bottom;
    frame_frame_gravity(client->frame, &x, &y);

    client_move_resize(client, x, y, MAX(1, w), MAX(1, h));
    stacking_raise(CLIENT_AS_WINDOW(client));
    client_focus(client);
}
