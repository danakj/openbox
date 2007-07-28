/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   obt/xevent.c for the Openbox window manager
   Copyright (c) 2007        Dana Jansens

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

#include "obt/xevent.h"
#include "obt/mainloop.h"

typedef struct _ObtXEventBinding ObtXEventBinding;

struct _ObtXEventHandler
{
    gint ref;
    ObtMainLoop *loop;

    /* A hash table where the key is the window, and the value is the
       ObtXEventBinding */
    GHashTable *bindings[LASTEvent]; /* LASTEvent comes from X.h */
};

struct _ObtXEventBinding
{
    Window win;
    ObtXEventCallback func;
    gpointer data;
};

static void xevent_handler(const XEvent *e, gpointer data);
static guint window_hash(Window *w) { return *w; }
static gboolean window_comp(Window *w1, Window *w2) { return *w1 == *w2; }

ObtXEventHandler* xevent_new()
{
    ObtXEventHandler *h;
    gint i;

    h = g_new(ObtXEventHandler, 1);
    h->ref = 1;
    for (i = 0; i < LASTEvent; ++i)
        h->bindings[i] = g_hash_table_new_full((GHashFunc)window_hash,
                                               (GEqualFunc)window_comp,
                                               NULL, g_free);
    return h;
}

void xevent_ref(ObtXEventHandler *h)
{
    ++h->ref;
}

void xevent_unref(ObtXEventHandler *h)
{
    if (h && --h->ref == 0) {
        if (h->loop)
            obt_main_loop_x_remove(h->loop, xevent_handler);
    }
}

void xevent_register(ObtXEventHandler *h, ObtMainLoop *loop)
{
    h->loop = loop;
    obt_main_loop_x_add(loop, xevent_handler, h, NULL);
}

void xevent_set_handler(ObtXEventHandler *h, gint type, Window win,
                        ObtXEventCallback func, gpointer data)
{
    ObtXEventBinding *b;

    g_assert(type < LASTEvent);
    g_assert(win);
    g_assert(func);

    b = g_new(ObtXEventBinding, 1);
    b->win = win;
    b->func = func;
    b->data = data;
    g_hash_table_replace(h->bindings[type], &b->win, b);
}

void xevent_remove_handler(ObtXEventHandler *h, gint type, Window win)
{
    g_assert(type < LASTEvent);
    g_assert(win);

    g_hash_table_remove(h->bindings[type], &win);
}

static void xevent_handler(const XEvent *e, gpointer data)
{
    ObtXEventHandler *h;
    ObtXEventBinding *b;

    h = data;
    b = g_hash_table_lookup(h->bindings[e->xany.type], &e->xany.window);
    if (b) b->func(e, b->data);
}
