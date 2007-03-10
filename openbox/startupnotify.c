/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   startupnotify.c for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
   Copyright (c) 2003        Ben Jansens

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

#include "startupnotify.h"

#ifndef USE_LIBSN

void sn_startup(gboolean reconfig) {}
void sn_shutdown(gboolean reconfig) {}
gboolean sn_app_starting() { return FALSE; }
void sn_app_started(gchar *wmclass) {}
gboolean sn_get_desktop(gchar *id, guint *desktop) { return FALSE; }

#else

#include "openbox.h"
#include "mainloop.h"
#include "screen.h"

#define SN_API_NOT_YET_FROZEN
#include <libsn/sn.h>

typedef struct {
    SnStartupSequence *seq;
    gboolean feedback;
} ObWaitData;

static SnDisplay *sn_display;
static SnMonitorContext *sn_context;
static GSList *sn_waits; /* list of ObWaitDatas */

static ObWaitData* wait_data_new(SnStartupSequence *seq);
static void wait_data_free(ObWaitData *d);
static ObWaitData* wait_find(const gchar *id);

static void sn_handler(const XEvent *e, gpointer data);
static void sn_event_func(SnMonitorEvent *event, gpointer data);

void sn_startup(gboolean reconfig)
{
    if (reconfig) return;

    sn_display = sn_display_new(ob_display, NULL, NULL);
    sn_context = sn_monitor_context_new(sn_display, ob_screen,
                                        sn_event_func, NULL, NULL);

    ob_main_loop_x_add(ob_main_loop, sn_handler, NULL, NULL);
}

void sn_shutdown(gboolean reconfig)
{
    GSList *it;

    if (reconfig) return;

    ob_main_loop_x_remove(ob_main_loop, sn_handler);

    for (it = sn_waits; it; it = g_slist_next(it))
        wait_data_free(it->data);
    g_slist_free(sn_waits);
    sn_waits = NULL;

    screen_set_root_cursor();

    sn_monitor_context_unref(sn_context);
    sn_display_unref(sn_display);
}

static ObWaitData* wait_data_new(SnStartupSequence *seq)
{
    ObWaitData *d = g_new(ObWaitData, 1);
    d->seq = seq;
    d->feedback = TRUE;

    sn_startup_sequence_ref(d->seq);

    return d;
}

static void wait_data_free(ObWaitData *d)
{
    if (d) {
        sn_startup_sequence_unref(d->seq);

        g_free(d);
    }
}

static ObWaitData* wait_find(const gchar *id)
{
    ObWaitData *ret = NULL;
    GSList *it;

    for (it = sn_waits; it; it = g_slist_next(it)) {
        ObWaitData *d = it->data;
        if (!strcmp(id, sn_startup_sequence_get_id(d->seq))) {
            ret = d;
            break;
        }
    }
    return ret;
}

gboolean sn_app_starting()
{
    GSList *it;

    for (it = sn_waits; it; it = g_slist_next(it)) {
        ObWaitData *d = it->data;
        if (d->feedback)
            return TRUE;
    }
    return FALSE;
}

static gboolean sn_wait_timeout(gpointer data)
{
    ObWaitData *d = data;
    d->feedback = FALSE;
    screen_set_root_cursor();
    return FALSE; /* don't repeat */
}

static void sn_wait_destroy(gpointer data)
{
    ObWaitData *d = data;
    sn_waits = g_slist_remove(sn_waits, d);
    wait_data_free(d);
}

static void sn_handler(const XEvent *e, gpointer data)
{
    XEvent ec;
    ec = *e;
    sn_display_process_event(sn_display, &ec);
}

static void sn_event_func(SnMonitorEvent *ev, gpointer data)
{
    SnStartupSequence *seq;
    gboolean change = FALSE;
    ObWaitData *d;

    if (!(seq = sn_monitor_event_get_startup_sequence(ev)))
        return;

    switch (sn_monitor_event_get_type(ev)) {
    case SN_MONITOR_EVENT_INITIATED:
        d = wait_data_new(seq);
        sn_waits = g_slist_prepend(sn_waits, d);
        /* 15 second timeout for apps to start */
        ob_main_loop_timeout_add(ob_main_loop, 15 * G_USEC_PER_SEC,
                                 sn_wait_timeout, d, sn_wait_destroy);
        change = TRUE;
        break;
    case SN_MONITOR_EVENT_CHANGED:
        /* XXX feedback changed? */
        change = TRUE;
        break;
    case SN_MONITOR_EVENT_COMPLETED:
    case SN_MONITOR_EVENT_CANCELED:
        if ((d = wait_find(sn_startup_sequence_get_id(seq)))) {
            d->feedback = FALSE;
            ob_main_loop_timeout_remove_data(ob_main_loop, sn_wait_timeout,
                                             d, FALSE);
            change = TRUE;
        }
        break;
    };

    if (change)
        screen_set_root_cursor();
}

void sn_app_started(const gchar *id, const gchar *wmclass)
{
    GSList *it;

    for (it = sn_waits; it; it = g_slist_next(it)) {
        ObWaitData *d = it->data;
        const gchar *seqid, *seqclass;
        seqid = sn_startup_sequence_get_id(d->seq);
        seqclass = sn_startup_sequence_get_wmclass(d->seq);
        if ((seqid && id && !strcmp(seqid, id)) ||
            (seqclass && wmclass && !strcmp(seqclass, wmclass)))
        {
            sn_startup_sequence_complete(d->seq);
            break;
        }
    }
}

gboolean sn_get_desktop(gchar *id, guint *desktop)
{
    ObWaitData *d;

    if (id && (d = wait_find(id))) {
        gint desk = sn_startup_sequence_get_workspace(d->seq);
        if (desk != -1) {
            *desktop = desk;
            return TRUE;
        }
    }
    return FALSE;
}

#endif
