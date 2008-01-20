/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   extensions.c for the Openbox window manager
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

#include "openbox.h"
#include "geom.h"
#include "extensions.h"
#include "screen.h"
#include "debug.h"

gboolean extensions_xkb       = FALSE;
gint     extensions_xkb_event_basep;
gboolean extensions_shape     = FALSE;
gint     extensions_shape_event_basep;
gboolean extensions_xinerama  = FALSE;
gint     extensions_xinerama_event_basep;
gboolean extensions_randr     = FALSE;
gint     extensions_randr_event_basep;
gboolean extensions_sync      = FALSE;
gint     extensions_sync_event_basep;

void extensions_query_all(void)
{
    gint junk;
    (void)junk;

#ifdef XKB
    extensions_xkb =
        XkbQueryExtension(ob_display, &junk, &extensions_xkb_event_basep,
                          &junk, NULL, NULL);
    if (!extensions_xkb)
        ob_debug("XKB extension is not present on the server\n");
#endif

#ifdef SHAPE
    extensions_shape =
        XShapeQueryExtension(ob_display, &extensions_shape_event_basep,
                             &junk);
    if (!extensions_shape)
        ob_debug("X Shape extension is not present on the server\n");
#endif

#ifdef XINERAMA
    extensions_xinerama =
        XineramaQueryExtension(ob_display, &extensions_xinerama_event_basep,
                               &junk) && XineramaIsActive(ob_display);
    if (!extensions_xinerama)
        ob_debug("Xinerama extension is not present on the server\n");
#endif

#ifdef XRANDR
    extensions_randr =
        XRRQueryExtension(ob_display, &extensions_randr_event_basep,
                          &junk);
    if (!extensions_randr)
        ob_debug("XRandR extension is not present on the server\n");
#endif

#ifdef SYNC
    extensions_sync =
        XSyncQueryExtension(ob_display, &extensions_sync_event_basep,
                            &junk) &&
        XSyncInitialize(ob_display, &junk, &junk);
    if (!extensions_sync)
        ob_debug("X Sync extension is not present on the server or is an "
                 "incompatible version\n");
#endif
}

void extensions_xinerama_screens(Rect **xin_areas, guint *nxin)
{
    guint i;
    gint l, r, t, b;
    if (ob_debug_xinerama) {
        g_print("Using fake xinerama !\n");
        gint w = WidthOfScreen(ScreenOfDisplay(ob_display, ob_screen));
        gint h = HeightOfScreen(ScreenOfDisplay(ob_display, ob_screen));
        *nxin = 2;
        *xin_areas = g_new(Rect, *nxin + 1);
        RECT_SET((*xin_areas)[0], 0, 0, w/2, h);
        RECT_SET((*xin_areas)[1], w/2, 0, w-(w/2), h);
    }
    else
#ifdef XINERAMA
    if (extensions_xinerama) {
        guint i;
        gint n;
        XineramaScreenInfo *info = XineramaQueryScreens(ob_display, &n);
        *nxin = n;
        *xin_areas = g_new(Rect, *nxin + 1);
        for (i = 0; i < *nxin; ++i)
            RECT_SET((*xin_areas)[i], info[i].x_org, info[i].y_org,
                     info[i].width, info[i].height);
        XFree(info);
    }
    else
#endif
    {
        *nxin = 1;
        *xin_areas = g_new(Rect, *nxin + 1);
        RECT_SET((*xin_areas)[0], 0, 0,
                 WidthOfScreen(ScreenOfDisplay(ob_display, ob_screen)),
                 HeightOfScreen(ScreenOfDisplay(ob_display, ob_screen)));
    }

    /* returns one extra with the total area in it */
    l = (*xin_areas)[0].x;
    t = (*xin_areas)[0].y;
    r = (*xin_areas)[0].x + (*xin_areas)[0].width - 1;
    b = (*xin_areas)[0].y + (*xin_areas)[0].height - 1;
    for (i = 1; i < *nxin; ++i) {
        l = MIN(l, (*xin_areas)[i].x);
        t = MIN(l, (*xin_areas)[i].y);
        r = MAX(r, (*xin_areas)[i].x + (*xin_areas)[i].width - 1);
        b = MAX(b, (*xin_areas)[i].y + (*xin_areas)[i].height - 1);
    }
    RECT_SET((*xin_areas)[*nxin], l, t, r - l + 1, b - t + 1);
}
