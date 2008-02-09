/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   loco.c for the Openbox window manager
   Copyright (c) 2008        Derek Foreman
   Copyright (c) 2008        Dana Jansens

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

#include "loco.h"
#include "screen.h"
#include "window.h"
#include "paint.h"

#include "obt/mainloop.h"
#include "obt/display.h"
#include "obt/prop.h"

#include <glib.h>

#define REFRESH_RATE (G_USEC_PER_SEC/60)

static LocoScreen **screens = NULL;
static ObtMainLoop *mainloop = NULL;

/* XXX stop removing windows from the stacking until they have no more refs */

void COMPOSTER_RAWR(const XEvent *e, gpointer data)
{
    LocoScreen *sc = data;
    LocoWindow *lw = NULL;

    /*g_print("COMPOSTER_RAWR() %d\n", e->type);*/

    switch (e->type) {
    case CreateNotify:
        if (e->xcreatewindow.parent == sc->root)
            loco_screen_add_window(sc, e->xcreatewindow.window);
        break;
    case DestroyNotify:
        lw = loco_screen_find_window(sc, e->xdestroywindow.window);
        if (lw) {
            loco_window_hide(lw, TRUE);
            loco_screen_remove_window(sc, lw);
        }
        else
            g_print("destroy notify for unknown window 0x%lx\n",
                    e->xdestroywindow.window);
        break;
    case ReparentNotify:
        if (e->xreparent.parent == sc->root)
            /* reparented to root */
            loco_screen_add_window(sc, e->xreparent.window);
        else {
            /* reparented away from root */
            lw = loco_screen_find_window(sc, e->xreparent.window);
            if (lw) {
                g_print("window 0x%lx reparented from root\n", lw->id);
                loco_window_hide(lw, FALSE);
                loco_screen_remove_window(sc, lw);
            }
            else
                g_print("reparent notify away from root for unknown window "
                        "0x%lx\n", e->xreparent.window);
        }
        break;
    case MapNotify:
        lw = loco_screen_find_window(sc, e->xmap.window);
        if (lw) loco_window_show(lw);
        else g_print("map notify for unknown window 0x%lx\n", e->xmap.window);
        break;
    case UnmapNotify:
        lw = loco_screen_find_window(sc, e->xunmap.window);
        if (lw) loco_window_hide(lw, FALSE);
        else g_print("unmap notify for unknown window 0x%lx\n",
                     e->xunmap.window);
        break;
    case ConfigureNotify:
        lw = loco_screen_find_window(sc, e->xconfigure.window);
        if (lw) loco_window_configure(lw, &e->xconfigure);
        break;
    default:
        if (e->type == obt_display_extension_damage_basep + XDamageNotify) {
            const XDamageNotifyEvent *de = (const XDamageNotifyEvent*)e;

            lw = loco_screen_find_window(sc, de->drawable);
            if (de->drawable == sc->root)
                loco_screen_redraw(sc);
            else if (lw)
                loco_screen_redraw(sc);

            /* mark the damage as fixed - we know about it now */
            obt_display_ignore_errors(TRUE);
            XDamageSubtract(obt_display, de->damage, None, None);
            obt_display_ignore_errors(FALSE);
        }
    }
}

static gboolean compositor_timeout(gpointer data)
{
    LocoScreen *sc = data;

    if (sc->redraw)
        paint_everything(sc);

    return TRUE; /* repeat */
}

void loco_startup(ObtMainLoop *loop)
{
    mainloop = loop;
    screens = g_new0(LocoScreen*, ScreenCount(obt_display));
}

void loco_shutdown()
{
    int i;
    for (i = 0; i < ScreenCount(obt_display); ++i)
        loco_screen_unref(screens[i]);

    g_free(screens);
    screens = NULL;
}

void loco_start_screen(gint number)
{
    if (!screens[number])
        screens[number] = loco_screen_new(number);
    if (screens[number]) {
        if (!paint_setup(screens[number]))
            loco_stop_screen(number);
        else {
            obt_main_loop_x_add(mainloop, COMPOSTER_RAWR,
                                screens[number], NULL);
            obt_main_loop_timeout_add(mainloop, REFRESH_RATE,
                                      (GSourceFunc)compositor_timeout,
                                      screens[number], g_direct_equal, NULL);
        }
    }
}

void loco_stop_screen(gint number)
{
    obt_main_loop_x_remove_data(mainloop, COMPOSTER_RAWR, screens[number]);
    obt_main_loop_timeout_remove_data(mainloop,
                                      compositor_timeout, screens[number],
                                      FALSE);

    loco_screen_unref(screens[number]);
    screens[number] = NULL;
}

void loco_reconfigure_screen(gint number)
{
    /* reload stuff.. */
}

gboolean loco_on_screen(gint number)
{
    return screens[number] != NULL;
}
