/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   client.h for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
   Copyright (c) 2003-2008   Dana Jansens

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

#include "ping.h"
#include "client.h"
#include "event.h"
#include "debug.h"
#include "openbox.h"
#include "obt/mainloop.h"
#include "obt/prop.h"

typedef struct _ObPingTarget
{
    ObClient *client;
    ObPingEventHandler h;
    guint32 id;
    gint waiting;
} ObPingTarget;

static GHashTable *ping_ids     = NULL;
static guint32     ping_next_id = 1;

#define PING_TIMEOUT (G_USEC_PER_SEC * 3)
/*! Warn the user after this many PING_TIMEOUT intervals */
#define PING_TIMEOUT_WARN 3

static void     ping_send(ObPingTarget *t);
static void     ping_end(ObClient *client, gpointer data);
static gboolean ping_timeout(gpointer data);
static gboolean find_client(gpointer key, gpointer value, gpointer client);

void ping_startup(gboolean reconfigure)
{
    if (reconfigure) return;

    ping_ids = g_hash_table_new(g_int_hash, g_int_equal);

    /* listen for clients to disappear */
    client_add_destroy_notify(ping_end, NULL);
}

void ping_shutdown(gboolean reconfigure)
{
    if (reconfigure) return;

    g_hash_table_unref(ping_ids);
    ping_ids = NULL;

    client_remove_destroy_notify(ping_end);
}

void ping_start(struct _ObClient *client, ObPingEventHandler h)
{
    ObPingTarget *t;

    /* make sure we're not already pinging the client */
    g_assert(g_hash_table_find(ping_ids, find_client, client) == NULL);

    g_assert(client->ping == TRUE);

    t = g_new0(ObPingTarget, 1);
    t->client = client;
    t->h = h;

    obt_main_loop_timeout_add(ob_main_loop, PING_TIMEOUT, ping_timeout,
                              t, g_direct_equal, NULL);
    /* act like we just timed out immediately, to start the pinging process
       now instead of after the first delay.  this makes sure the client
       ends up in the ping_ids hash table now. */
    ping_timeout(t);

    /* make sure we can remove the client later */
    g_assert(g_hash_table_find(ping_ids, find_client, client) != NULL);
}

void ping_stop(struct _ObClient *c)
{
    ping_end(c, NULL);
}

void ping_got_pong(guint32 id)
{
    ObPingTarget *t;

    if ((t = g_hash_table_lookup(ping_ids, &id))) {
        /*ob_debug("-PONG: '%s' (id %u)", t->client->title, t->id);*/
        if (t->waiting > PING_TIMEOUT_WARN) {
            /* we had notified that they weren't responding, so now we
               need to notify that they are again */
            t->h(t->client, FALSE);
        }
        t->waiting = 0; /* not waiting for a reply anymore */
    }
    else
        ob_debug("Got PONG with id %u but not waiting for one", id);
}

static gboolean find_client(gpointer key, gpointer value, gpointer client)
{
    ObPingTarget *t = value;
    return t->client == client;
}

static void ping_send(ObPingTarget *t)
{
    /* t->id is 0 when it hasn't been assigned an id ever yet.
       we can reuse ids when t->waiting == 0, because we won't be getting a
       pong for that id in the future again.  that way for apps that aren't
       timing out we don't need to remove/add them from/to the hash table */
    if (t->id == 0 || t->waiting > 0) {
        /* pick an id, and reinsert in the hash table with the new id */
        if (t->id) g_hash_table_remove(ping_ids, &t->id);
        t->id = ping_next_id;
        if (++ping_next_id == 0) ++ping_next_id; /* skip 0 on wraparound */
        g_hash_table_insert(ping_ids, &t->id, t);
    }

    /*ob_debug("+PING: '%s' (id %u)", t->client->title, t->id);*/
    OBT_PROP_MSG_TO(t->client->window, t->client->window, WM_PROTOCOLS,
                    OBT_PROP_ATOM(NET_WM_PING), t->id, t->client->window, 0, 0,
                    NoEventMask);
}

static gboolean ping_timeout(gpointer data)
{
    ObPingTarget *t = data;

    ping_send(t);

    /* if the client hasn't been responding then do something about it */
    if (t->waiting == PING_TIMEOUT_WARN)
        t->h(t->client, TRUE); /* notify that the client isn't responding */

    ++t->waiting;

    return TRUE; /* repeat */
}

static void ping_end(ObClient *client, gpointer data)
{
    ObPingTarget *t;

    if ((t = g_hash_table_find(ping_ids, find_client, client))) {
        g_hash_table_remove(ping_ids, &t->id);

        obt_main_loop_timeout_remove_data(ob_main_loop, ping_timeout,
                                          t, FALSE);

        g_free(t);
    }
}
