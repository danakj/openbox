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
#include "prop.h"
#include "event.h"
#include "mainloop.h"
#include "openbox.h"

typedef struct _ObPingTarget
{
    ObClient *client;
    ObPingEventHandler h;
    Time sent;
    gint waiting;
} ObPingTarget;

static GSList *ping_targets = NULL;
static gboolean active = FALSE;

#define PING_TIMEOUT (G_USEC_PER_SEC * 1)
/*! Warn the user after this many PING_TIMEOUT intervals */
#define PING_TIMEOUT_WARN 3

static void ping_send(ObPingTarget *t);
static void ping_end(ObClient *client, gpointer data);
static gboolean ping_timeout(gpointer data);

void ping_start(struct _ObClient *client, ObPingEventHandler h)
{
    GSList *it;
    ObPingTarget *t;

    g_assert(client->ping == TRUE);

    /* make sure we're not already pinging it */
    for (it = ping_targets; it != NULL; it = g_slist_next(it)) {
        t = it->data;
        if (t->client == client) return;
    }

    t = g_new(ObPingTarget, 1);
    t->client = client;
    t->h = h;
    t->waiting = 1; /* first wait for a reply */

    ping_send(t);
    ping_targets = g_slist_prepend(ping_targets, t);
    ob_main_loop_timeout_add(ob_main_loop, PING_TIMEOUT, ping_timeout,
                             t, NULL, NULL);

    if (!active) {
        active = TRUE;
        /* listen for the client to disappear */
        client_add_destroy_notify(ping_end, NULL);
    }
}

void ping_stop(struct _ObClient *c)
{
    ping_end(c, NULL);
}

void ping_got_pong(Time timestamp)
{
    GSList *it;
    ObPingTarget *t;

    /* make sure we're not already pinging it */
    for (it = ping_targets; it != NULL; it = g_slist_next(it)) {
        t = it->data;
        if (t->sent == timestamp) {
            ob_debug("Got PONG with timestamp %lu\n", timestamp);
            if (t->waiting > PING_TIMEOUT_WARN) {
                /* we had notified that they weren't responding, so now we
                   need to notify that they are again */
                t->h(t->client, FALSE);
            }
            t->waiting = 0; /* not waiting for a reply anymore */
            break;
        }
    }

    if (it == NULL)
        ob_debug("Got PONG with timestamp %lu but not waiting for one\n",
                 timestamp);
}

static void ping_send(ObPingTarget *t)
{
    t->sent = event_get_server_time();
    ob_debug("PINGing client 0x%x at %lu\n", t->client->window, t->sent);
    PROP_MSG_TO(t->client->window, t->client->window, wm_protocols,
                prop_atoms.net_wm_ping, t->sent, t->client->window, 0, 0,
                NoEventMask);
}

static gboolean ping_timeout(gpointer data)
{
    ObPingTarget *t = data;

    if (t->waiting == 0) { /* got a reply already */
        /* send another ping to make sure it's still alive */
        ping_send(t);
    }

    if (t->waiting == PING_TIMEOUT_WARN)
        t->h(t->client, TRUE); /* notify that the client isn't responding */

    ++t->waiting;

    return TRUE; /* repeat */
}

static void ping_end(ObClient *client, gpointer data)
{
    GSList *it;
    ObPingTarget *t;

    for (it = ping_targets; it != NULL; it = g_slist_next(it)) {
        t = it->data;
        if (t->client == client) {
            ping_targets = g_slist_remove_link(ping_targets, it);
            ob_main_loop_timeout_remove_data(ob_main_loop, ping_timeout, t,
                                             FALSE);
            g_free(t);
            break;
        }
    }

    /* stop listening if we're not waiting for any more pings */
    if (!ping_targets) {
        active = TRUE;
        client_remove_destroy_notify(ping_end);
    }    
}
