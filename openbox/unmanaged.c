/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   unmanaged.c for the Openbox window manager
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

#include "unmanaged.h"
#include "composite.h"
#include "window.h"
#include "obt/display.h"
#include "obt/prop.h"

struct _ObUnmanaged {
    ObWindow super;
    Window window;
    ObStackingLayer layer;
    gint depth;
    guint32 alpha;
};

static GSList *unmanaged_list = NULL;

ObUnmanaged* unmanaged_new(Window w)
{
    XWindowAttributes at;
    ObUnmanaged *self;

    if (w == composite_overlay)
        return NULL;
    if (!XGetWindowAttributes(obt_display, w, &at))
        return NULL;        
    if (at.class == InputOnly)
        return NULL;

    self = window_new(OB_WINDOW_CLASS_UNMANAGED, ObUnmanaged);
    self->window = w;
    self->layer = OB_STACKING_LAYER_ALL;
    self->depth = at.depth;
    self->alpha = 0xffffffff;

    XSelectInput(obt_display, self->window, PropertyChangeMask);

    unmanaged_update_opacity(self);
    
    window_set_abstract(UNMANAGED_AS_WINDOW(self),
                        &self->window,
                        &self->layer,
                        &self->depth,
                        &self->alpha);

    window_add(&self->window, UNMANAGED_AS_WINDOW(self));
    stacking_add(UNMANAGED_AS_WINDOW(self));
    unmanaged_list = g_slist_prepend(unmanaged_list, self);

    return self;
}

void unmanaged_destroy(ObUnmanaged *self)
{
    XSelectInput(obt_display, self->window, NoEventMask);

    window_cleanup(UNMANAGED_AS_WINDOW(self));
    unmanaged_list = g_slist_remove(unmanaged_list, self);
    stacking_remove(UNMANAGED_AS_WINDOW(self));
    window_remove(self->window);
    window_free(UNMANAGED_AS_WINDOW(self));
}

void unmanaged_destroy_all(void)
{
    while (unmanaged_list)
        unmanaged_destroy(unmanaged_list->data);
}

void unmanaged_update_opacity(ObUnmanaged *self)
{
    if (!OBT_PROP_GET32(self->window, NET_WM_WINDOW_OPACITY, CARDINAL,
                        &self->alpha))
        self->alpha = 0xffffffff;
}
