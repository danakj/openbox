/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   window.c for the Openbox window manager
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

#include "window.h"
#include "menuframe.h"
#include "config.h"
#include "dock.h"
#include "client.h"
#include "frame.h"

static GHashTable *window_map;

static guint window_hash(Window *w) { return *w; }
static gboolean window_comp(Window *w1, Window *w2) { return *w1 == *w2; }

void window_startup(gboolean reconfig)
{
    if (reconfig) return;

    window_map = g_hash_table_new((GHashFunc)window_hash,
                                  (GEqualFunc)window_comp);
}

void window_shutdown(gboolean reconfig)
{
    if (reconfig) return;

    g_hash_table_destroy(window_map);
}

Window window_top(ObWindow *self)
{
    switch (self->type) {
    case OB_WINDOW_CLASS_MENUFRAME:
        return WINDOW_AS_MENUFRAME(self)->window;
    case OB_WINDOW_CLASS_DOCK:
        return WINDOW_AS_DOCK(self)->frame;
    case OB_WINDOW_CLASS_CLIENT:
        return WINDOW_AS_CLIENT(self)->frame->window;
    case OB_WINDOW_CLASS_INTERNALWINDOW:
        return WINDOW_AS_INTERNALWINDOW(self)->window;
    }
    g_assert_not_reached();
    return None;
}

ObStackingLayer window_layer(ObWindow *self)
{
    switch (self->type) {
    case OB_WINDOW_CLASS_DOCK:
        return config_dock_layer;
    case OB_WINDOW_CLASS_CLIENT:
        return ((ObClient*)self)->layer;
    case OB_WINDOW_CLASS_MENUFRAME:
    case OB_WINDOW_CLASS_INTERNALWINDOW:
        return OB_STACKING_LAYER_INTERNAL;
    }
    g_assert_not_reached();
    return None;
}

ObWindow* window_find(Window xwin)
{
    g_assert(xwin != None);
    return g_hash_table_lookup(window_map, &xwin);
}

void window_add(Window *xwin, ObWindow *win)
{
    g_assert(xwin != NULL);
    g_assert(win != NULL);
    g_hash_table_insert(window_map, xwin, win);
}

void window_remove(Window xwin)
{
    g_assert(xwin != None);
    g_hash_table_remove(window_map, &xwin);
}
