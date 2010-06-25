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
#include "unmanaged.h"
#include "composite.h"
#include "frame.h"
#include "openbox.h"
#include "prompt.h"
#include "debug.h"
#include "grab.h"
#include "obt/xqueue.h"

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

ObWindow* window_new_size(ObWindowClass type, gsize size)
{
    ObWindow *self;

    g_assert(size >= sizeof(ObWindow));
    self = g_slice_alloc0(size);
    self->bytes = size;
    self->type = type;
    return self;
}

void window_set_abstract(ObWindow *self,
                         const Window *top,
                         const Window *redir,
                         const ObStackingLayer *layer,
                         const int *depth,
                         const guint32 *alpha)
{
    g_assert(!self->top && !self->redir && !self->layer && !self->depth &&
             !self->alpha);
#ifdef USE_COMPOSITING
    g_assert(self->area.width > 0 && self->area.height > 0);
#endif

    self->top = top;
    self->redir = redir;
    self->layer = layer;
    self->depth = depth;
    self->alpha = alpha;

    /* set up any things in ObWindow that require use of the abstract pointers
       now */

#ifdef SHAPE
#ifdef USE_COMPOSITING
    {
        gint foo;
        guint ufoo;
        gint s;

        XShapeQueryExtents(obt_display, window_redir(self), &s, &foo,
                           &foo, &ufoo, &ufoo, &foo, &foo, &foo, &ufoo,
                           &ufoo);
        if (s) window_adjust_redir_shape(self);
    }
#endif
#endif

    composite_window_setup(self);
}

void window_set_top_area(ObWindow *self, const Rect *r, gint border)
{
    g_assert(!self->top);

#ifdef USE_COMPOSITING
    self->toparea = *r;
    self->topborder = border;
    RECT_SET(self->area, -border, -border,
             self->toparea.width + border * 2,
             self->toparea.height + border * 2);
#endif
}

void window_cleanup(ObWindow *self)
{
    composite_window_cleanup(self);
}

void window_free(ObWindow *self)
{
    /* The abstract pointers must not be used here, they are likely invalid
       by now ! */

    if (self->rects) XFree(self->rects);
    g_slice_free1(self->bytes, self);
}

ObWindow* window_find(Window xwin)
{
    return g_hash_table_lookup(window_map, &xwin);
}

void wfe(gpointer k, gpointer v, gpointer data)
{
    ((ObWindowForeachFunc)data)(v);
}

void window_foreach(ObWindowForeachFunc func)
{
    g_hash_table_foreach(window_map, wfe, func);
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

void window_adjust_redir_shape(ObWindow *self)
{
#ifdef USE_COMPOSITING
#ifdef SHAPE
    gint ord;
    if (self->rects)
        XFree(self->rects);
    self->rects = XShapeGetRectangles(obt_display, window_redir(self),
                                      ShapeBounding, &self->n_rects, &ord);
#endif
#endif
}

ObInternalWindow* window_internal_new(Window window, const Rect *area,
                                      gint border, gint depth)
{
    ObInternalWindow *self;

    self = window_new(OB_WINDOW_CLASS_INTERNAL, ObInternalWindow);
    self->window = window;
    self->layer = OB_STACKING_LAYER_INTERNAL;
    self->depth = depth;
    window_set_top_area(INTERNAL_AS_WINDOW(self), area, border);
    window_set_abstract(INTERNAL_AS_WINDOW(self),
                        &self->window, /* top-most window */
                        &self->window, /* composite redir window */
                        &self->layer,  /* stacking layer */
                        &self->depth,  /* window depth */
                        NULL);         /* opacity */
    return self;
}

void window_manage_all(void)
{
    guint i, j, nchild;
    Window w, *children;
    XWMHints *wmhints;
    XWindowAttributes attrib;

    if (!XQueryTree(obt_display, RootWindow(obt_display, ob_screen),
                    &w, &w, &children, &nchild)) {
        ob_debug("XQueryTree failed in window_manage_all");
        nchild = 0;
    }

    /* remove all icon windows from the list */
    for (i = 0; i < nchild; i++) {
        if (children[i] == None) continue;
        wmhints = XGetWMHints(obt_display, children[i]);
        if (wmhints) {
            if ((wmhints->flags & IconWindowHint) &&
                (wmhints->icon_window != children[i]))
                for (j = 0; j < nchild; j++)
                    if (children[j] == wmhints->icon_window) {
                        /* XXX watch the window though */
                        children[j] = None;
                        break;
                    }
            XFree(wmhints);
        }
    }

    for (i = 0; i < nchild; ++i) {
        if (children[i] == None) continue;
        if (window_find(children[i])) continue; /* skip our own windows */
        if (XGetWindowAttributes(obt_display, children[i], &attrib)) {
            if (attrib.map_state == IsUnmapped)
                ;
            else
                window_manage(children[i]);
        }
    }

    if (children) XFree(children);
}

static gboolean check_unmap(XEvent *e, gpointer data)
{
    const Window win = *(Window*)data;
    return ((e->type == DestroyNotify && e->xdestroywindow.window == win) ||
            (e->type == UnmapNotify && e->xunmap.window == win));
}

void window_manage(Window win)
{
    XWindowAttributes attrib;
    gboolean no_manage = FALSE;
    gboolean is_dockapp = FALSE;
    Window icon_win = None;

    grab_server(TRUE);

    /* check if it has already been unmapped by the time we started
       mapping. the grab does a sync so we don't have to here */
    if (xqueue_exists_local(check_unmap, &win)) {
        ob_debug("Trying to manage unmapped window. Aborting that.");
        no_manage = TRUE;
    }
    else if (!XGetWindowAttributes(obt_display, win, &attrib))
        no_manage = TRUE;
    else {
        XWMHints *wmhints;

        /* is the window a docking app */
        is_dockapp = FALSE;
        if ((wmhints = XGetWMHints(obt_display, win))) {
            if ((wmhints->flags & StateHint) &&
                wmhints->initial_state == WithdrawnState)
            {
                if (wmhints->flags & IconWindowHint)
                    icon_win = wmhints->icon_window;
                is_dockapp = TRUE;
            }
            XFree(wmhints);
        }
    }

    if (!no_manage) {
        if (attrib.override_redirect) {
            ob_debug("not managing override redirect window 0x%x", win);
            grab_server(FALSE);
        }
        else if (is_dockapp) {
            if (!icon_win)
                icon_win = win;
            dock_manage(icon_win, win);
        }
        else
            client_manage(win, NULL);
    }
    else {
        grab_server(FALSE);
        ob_debug("FAILED to manage window 0x%x", win);
    }
}

void window_unmanage_all(void)
{
    dock_unmanage_all();
    client_unmanage_all();
    unmanaged_destroy_all();
}
