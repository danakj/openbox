/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-
   
   propwin.c for the Openbox window manager
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

#include "propwin.h"
#include "openbox.h"
#include "client.h"
#include "debug.h"

typedef struct _ObPropWin     ObPropWin;
typedef struct _ObPropWinData ObPropWinData;

struct _ObPropWinData
{
    GSList *clients;
};

struct _ObPropWin
{
    Window win;
    ObPropWinData data[OB_NUM_PROPWIN_TYPES];
};

/*! A hash table that maps a window to an ObPropWin */
static GHashTable *propwin_map;

static guint window_hash(Window *w) { return *w; }
static gboolean window_comp(Window *w1, Window *w2) { return *w1 == *w2; }

void propwin_startup(gboolean reconfig)
{
    if (!reconfig)
        propwin_map = g_hash_table_new_full((GHashFunc)window_hash,
                                             (GEqualFunc)window_comp,
                                             NULL,
                                             g_free);
}

void propwin_shutdown(gboolean reconfig)
{
    if (!reconfig)
        g_hash_table_destroy(propwin_map);
    else
        g_assert(g_hash_table_size(propwin_map) == 0);
}

void propwin_add(Window win, ObPropWinType type, ObClient *client)
{
    ObPropWin *p;

    if (!win) return;

    g_assert(client);
    g_assert(type < OB_NUM_PROPWIN_TYPES);

    p = g_hash_table_lookup(propwin_map, &win);
    if (!p) {
        p = g_new0(ObPropWin, 1);
        p->win = win;
        g_hash_table_insert(propwin_map, &p->win, p);
        /* get property changes on this window */
        XSelectInput(ob_display, win, PropertyChangeMask);
    } else
        g_assert(g_slist_find(p->data[type].clients, client) == NULL);

    if (p->data[type].clients != NULL)
        ob_debug("Client %s is using a property window 0x%x that is already "
                 "in use\n", client->title, win);

    /* add it to the clients list */
    p->data[type].clients = g_slist_prepend(p->data[type].clients, client);
}

void propwin_remove(Window win, ObPropWinType type, ObClient *client)
{
    ObPropWin *p;

    if (!win) return;

    p = g_hash_table_lookup(propwin_map, &win);
    g_assert(p);

    /* remove it to the clients list */
    g_assert(g_slist_find(p->data[type].clients, client) != NULL);
    p->data[type].clients = g_slist_remove(p->data[type].clients, client);

    /* no more clients left for this type */
    if (p->data[type].clients == NULL) {
        guint i;
        gboolean none = TRUE;

        for (i = 0; i < OB_NUM_PROPWIN_TYPES; ++i)
            if (p->data[i].clients != NULL)
                none = FALSE; /* another type still has a client for this
                                 window */

        if (none) {
            /* don't get events for this window any more */
            XSelectInput(ob_display, win, NoEventMask);
            g_hash_table_remove(propwin_map, &win);
        }
    }
}

GSList* propwin_get_clients(Window win, ObPropWinType type)
{
    ObPropWin *p = g_hash_table_lookup(propwin_map, &win);
    if (p)
        return p->data[type].clients;
    else
        return NULL;
}
