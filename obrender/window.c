/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   window.c for the Openbox window manager
   Copyright (c) 2010        Dana Jansens

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

#include "window.h"
#include "instance.h"
#include "render.h"
#include <cairo.h>
#include <cairo-xlib.h>

struct _RrWindow {
    guint ref;
    cairo_surface_t *sur;
    cairo_t *cr;
};


RrWindow* RrWindowNew(RrInstance *inst)
{
    RrWindow *win;

    win = g_slice_new(RrWindow);
    win->sur = cairo_xlib_surface_create(inst->display, None, inst->visual,
                                         1, 1);
    win->cr = cairo_create(win->sur);

    return win;
}
void RrWindowRef(RrWindow *win)
{
    ++win->ref;
}

void RrWindowUnref(RrWindow *win)
{
    if (win && --win->ref < 1) {
        cairo_destroy(win->cr);
        cairo_surface_destroy(win->sur);
        g_slice_free(RrWindow, win);
    }
}
