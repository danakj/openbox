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

GHashTable *window_map;

void window_startup(gboolean reconfig)
{
    if (reconfig) return;

    window_map = g_hash_table_new(g_int_hash, g_int_equal);
}

void window_shutdown(gboolean reconfig)
{
    if (reconfig) return;

    g_hash_table_destroy(window_map);
}

Window window_top(ObWindow *self)
{
    switch (self->type) {
    case Window_Menu:
        return ((ObMenuFrame*)self)->window;
    case Window_Dock:
        return ((ObDock*)self)->frame;
    case Window_DockApp:
        /* not to be used for stacking */
        g_assert_not_reached();
        break;
    case Window_Client:
        return ((ObClient*)self)->frame->window;
    case Window_Internal:
        return ((InternalWindow*)self)->win;
    }
    g_assert_not_reached();
    return None;
}

Window window_layer(ObWindow *self)
{
    switch (self->type) {
    case Window_Menu:
        return OB_STACKING_LAYER_INTERNAL;
    case Window_Dock:
        return config_dock_layer;
    case Window_DockApp:
        /* not to be used for stacking */
        g_assert_not_reached();
        break;
    case Window_Client:
        return ((ObClient*)self)->layer;
    case Window_Internal:
        return OB_STACKING_LAYER_INTERNAL;
    }
    g_assert_not_reached();
    return None;
}
