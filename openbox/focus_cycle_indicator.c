/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   focus_cycle_indicator.c for the Openbox window manager
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

#include "focus_cycle.h"
#include "client.h"
#include "openbox.h"
#include "frame.h"
#include "event.h"
#include "render/render.h"

#include <X11/Xlib.h>
#include <glib.h>

#define FOCUS_INDICATOR_WIDTH 6

struct
{
    InternalWindow top;
    InternalWindow left;
    InternalWindow right;
    InternalWindow bottom;
} focus_indicator;

static RrAppearance *a_focus_indicator;
static RrColor      *color_white;
static gboolean      visible;

static Window create_window(Window parent, gulong mask,
                            XSetWindowAttributes *attrib)
{
    return XCreateWindow(ob_display, parent, 0, 0, 1, 1, 0,
                         RrDepth(ob_rr_inst), InputOutput,
                         RrVisual(ob_rr_inst), mask, attrib);

}

void focus_cycle_indicator_startup(gboolean reconfig)
{
    XSetWindowAttributes attr;

    visible = FALSE;

    if (reconfig) return;

    focus_indicator.top.obwin.type = Window_Internal;
    focus_indicator.left.obwin.type = Window_Internal;
    focus_indicator.right.obwin.type = Window_Internal;
    focus_indicator.bottom.obwin.type = Window_Internal;

    attr.override_redirect = True;
    attr.background_pixel = BlackPixel(ob_display, ob_screen);
    focus_indicator.top.win =
        create_window(RootWindow(ob_display, ob_screen),
                      CWOverrideRedirect | CWBackPixel, &attr);
    focus_indicator.left.win =
        create_window(RootWindow(ob_display, ob_screen),
                      CWOverrideRedirect | CWBackPixel, &attr);
    focus_indicator.right.win =
        create_window(RootWindow(ob_display, ob_screen),
                      CWOverrideRedirect | CWBackPixel, &attr);
    focus_indicator.bottom.win =
        create_window(RootWindow(ob_display, ob_screen),
                      CWOverrideRedirect | CWBackPixel, &attr);

    stacking_add(INTERNAL_AS_WINDOW(&focus_indicator.top));
    stacking_add(INTERNAL_AS_WINDOW(&focus_indicator.left));
    stacking_add(INTERNAL_AS_WINDOW(&focus_indicator.right));
    stacking_add(INTERNAL_AS_WINDOW(&focus_indicator.bottom));
    g_hash_table_insert(window_map, &focus_indicator.top.win,
                        &focus_indicator.top);
    g_hash_table_insert(window_map, &focus_indicator.left.win,
                        &focus_indicator.left);
    g_hash_table_insert(window_map, &focus_indicator.right.win,
                        &focus_indicator.right);
    g_hash_table_insert(window_map, &focus_indicator.bottom.win,
                        &focus_indicator.bottom);

    color_white = RrColorNew(ob_rr_inst, 0xff, 0xff, 0xff);

    a_focus_indicator = RrAppearanceNew(ob_rr_inst, 4);
    a_focus_indicator->surface.grad = RR_SURFACE_SOLID;
    a_focus_indicator->surface.relief = RR_RELIEF_FLAT;
    a_focus_indicator->surface.primary = RrColorNew(ob_rr_inst,
                                                    0, 0, 0);
    a_focus_indicator->texture[0].type = RR_TEXTURE_LINE_ART;
    a_focus_indicator->texture[0].data.lineart.color = color_white;
    a_focus_indicator->texture[1].type = RR_TEXTURE_LINE_ART;
    a_focus_indicator->texture[1].data.lineart.color = color_white;
    a_focus_indicator->texture[2].type = RR_TEXTURE_LINE_ART;
    a_focus_indicator->texture[2].data.lineart.color = color_white;
    a_focus_indicator->texture[3].type = RR_TEXTURE_LINE_ART;
    a_focus_indicator->texture[3].data.lineart.color = color_white;
}

void focus_cycle_indicator_shutdown(gboolean reconfig)
{
    if (reconfig) return;

    RrColorFree(color_white);

    RrAppearanceFree(a_focus_indicator);

    g_hash_table_remove(window_map, &focus_indicator.top.win);
    g_hash_table_remove(window_map, &focus_indicator.left.win);
    g_hash_table_remove(window_map, &focus_indicator.right.win);
    g_hash_table_remove(window_map, &focus_indicator.bottom.win);

    stacking_remove(INTERNAL_AS_WINDOW(&focus_indicator.top));
    stacking_remove(INTERNAL_AS_WINDOW(&focus_indicator.left));
    stacking_remove(INTERNAL_AS_WINDOW(&focus_indicator.right));
    stacking_remove(INTERNAL_AS_WINDOW(&focus_indicator.bottom));

    XDestroyWindow(ob_display, focus_indicator.top.win);
    XDestroyWindow(ob_display, focus_indicator.left.win);
    XDestroyWindow(ob_display, focus_indicator.right.win);
    XDestroyWindow(ob_display, focus_indicator.bottom.win);
}

void focus_cycle_draw_indicator(ObClient *c)
{
    if (!c && visible) {
        gulong ignore_start;

        /* kill enter events cause by this unmapping */
        ignore_start = event_start_ignore_all_enters();

        XUnmapWindow(ob_display, focus_indicator.top.win);
        XUnmapWindow(ob_display, focus_indicator.left.win);
        XUnmapWindow(ob_display, focus_indicator.right.win);
        XUnmapWindow(ob_display, focus_indicator.bottom.win);

        event_end_ignore_all_enters(ignore_start);

        visible = FALSE;
    }
    else if (c) {
        /*
          if (c)
              frame_adjust_focus(c->frame, FALSE);
          frame_adjust_focus(c->frame, TRUE);
        */
        gint x, y, w, h;
        gint wt, wl, wr, wb;

        wt = wl = wr = wb = FOCUS_INDICATOR_WIDTH;

        x = c->frame->area.x;
        y = c->frame->area.y;
        w = c->frame->area.width;
        h = wt;

        XMoveResizeWindow(ob_display, focus_indicator.top.win,
                          x, y, w, h);
        a_focus_indicator->texture[0].data.lineart.x1 = 0;
        a_focus_indicator->texture[0].data.lineart.y1 = h-1;
        a_focus_indicator->texture[0].data.lineart.x2 = 0;
        a_focus_indicator->texture[0].data.lineart.y2 = 0;
        a_focus_indicator->texture[1].data.lineart.x1 = 0;
        a_focus_indicator->texture[1].data.lineart.y1 = 0;
        a_focus_indicator->texture[1].data.lineart.x2 = w-1;
        a_focus_indicator->texture[1].data.lineart.y2 = 0;
        a_focus_indicator->texture[2].data.lineart.x1 = w-1;
        a_focus_indicator->texture[2].data.lineart.y1 = 0;
        a_focus_indicator->texture[2].data.lineart.x2 = w-1;
        a_focus_indicator->texture[2].data.lineart.y2 = h-1;
        a_focus_indicator->texture[3].data.lineart.x1 = (wl-1);
        a_focus_indicator->texture[3].data.lineart.y1 = h-1;
        a_focus_indicator->texture[3].data.lineart.x2 = w - wr;
        a_focus_indicator->texture[3].data.lineart.y2 = h-1;
        RrPaint(a_focus_indicator, focus_indicator.top.win,
                w, h);

        x = c->frame->area.x;
        y = c->frame->area.y;
        w = wl;
        h = c->frame->area.height;

        XMoveResizeWindow(ob_display, focus_indicator.left.win,
                          x, y, w, h);
        a_focus_indicator->texture[0].data.lineart.x1 = w-1;
        a_focus_indicator->texture[0].data.lineart.y1 = 0;
        a_focus_indicator->texture[0].data.lineart.x2 = 0;
        a_focus_indicator->texture[0].data.lineart.y2 = 0;
        a_focus_indicator->texture[1].data.lineart.x1 = 0;
        a_focus_indicator->texture[1].data.lineart.y1 = 0;
        a_focus_indicator->texture[1].data.lineart.x2 = 0;
        a_focus_indicator->texture[1].data.lineart.y2 = h-1;
        a_focus_indicator->texture[2].data.lineart.x1 = 0;
        a_focus_indicator->texture[2].data.lineart.y1 = h-1;
        a_focus_indicator->texture[2].data.lineart.x2 = w-1;
        a_focus_indicator->texture[2].data.lineart.y2 = h-1;
        a_focus_indicator->texture[3].data.lineart.x1 = w-1;
        a_focus_indicator->texture[3].data.lineart.y1 = wt-1;
        a_focus_indicator->texture[3].data.lineart.x2 = w-1;
        a_focus_indicator->texture[3].data.lineart.y2 = h - wb;
        RrPaint(a_focus_indicator, focus_indicator.left.win,
                w, h);

        x = c->frame->area.x + c->frame->area.width - wr;
        y = c->frame->area.y;
        w = wr;
        h = c->frame->area.height ;

        XMoveResizeWindow(ob_display, focus_indicator.right.win,
                          x, y, w, h);
        a_focus_indicator->texture[0].data.lineart.x1 = 0;
        a_focus_indicator->texture[0].data.lineart.y1 = 0;
        a_focus_indicator->texture[0].data.lineart.x2 = w-1;
        a_focus_indicator->texture[0].data.lineart.y2 = 0;
        a_focus_indicator->texture[1].data.lineart.x1 = w-1;
        a_focus_indicator->texture[1].data.lineart.y1 = 0;
        a_focus_indicator->texture[1].data.lineart.x2 = w-1;
        a_focus_indicator->texture[1].data.lineart.y2 = h-1;
        a_focus_indicator->texture[2].data.lineart.x1 = w-1;
        a_focus_indicator->texture[2].data.lineart.y1 = h-1;
        a_focus_indicator->texture[2].data.lineart.x2 = 0;
        a_focus_indicator->texture[2].data.lineart.y2 = h-1;
        a_focus_indicator->texture[3].data.lineart.x1 = 0;
        a_focus_indicator->texture[3].data.lineart.y1 = wt-1;
        a_focus_indicator->texture[3].data.lineart.x2 = 0;
        a_focus_indicator->texture[3].data.lineart.y2 = h - wb;
        RrPaint(a_focus_indicator, focus_indicator.right.win,
                w, h);

        x = c->frame->area.x;
        y = c->frame->area.y + c->frame->area.height - wb;
        w = c->frame->area.width;
        h = wb;

        XMoveResizeWindow(ob_display, focus_indicator.bottom.win,
                          x, y, w, h);
        a_focus_indicator->texture[0].data.lineart.x1 = 0;
        a_focus_indicator->texture[0].data.lineart.y1 = 0;
        a_focus_indicator->texture[0].data.lineart.x2 = 0;
        a_focus_indicator->texture[0].data.lineart.y2 = h-1;
        a_focus_indicator->texture[1].data.lineart.x1 = 0;
        a_focus_indicator->texture[1].data.lineart.y1 = h-1;
        a_focus_indicator->texture[1].data.lineart.x2 = w-1;
        a_focus_indicator->texture[1].data.lineart.y2 = h-1;
        a_focus_indicator->texture[2].data.lineart.x1 = w-1;
        a_focus_indicator->texture[2].data.lineart.y1 = h-1;
        a_focus_indicator->texture[2].data.lineart.x2 = w-1;
        a_focus_indicator->texture[2].data.lineart.y2 = 0;
        a_focus_indicator->texture[3].data.lineart.x1 = wl-1;
        a_focus_indicator->texture[3].data.lineart.y1 = 0;
        a_focus_indicator->texture[3].data.lineart.x2 = w - wr;
        a_focus_indicator->texture[3].data.lineart.y2 = 0;
        RrPaint(a_focus_indicator, focus_indicator.bottom.win,
                w, h);

        XMapWindow(ob_display, focus_indicator.top.win);
        XMapWindow(ob_display, focus_indicator.left.win);
        XMapWindow(ob_display, focus_indicator.right.win);
        XMapWindow(ob_display, focus_indicator.bottom.win);

        visible = TRUE;
    }
}
