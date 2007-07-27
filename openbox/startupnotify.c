/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   startupnotify.c for the Openbox window manager
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

#include "startupnotify.h"
#include "gettext.h"
#include "event.h"

#include <stdlib.h>

#ifndef USE_LIBSN

void sn_startup(gboolean reconfig) {}
void sn_shutdown(gboolean reconfig) {}
gboolean sn_app_starting() { return FALSE; }
Time sn_app_started(const gchar *id, const gchar *wmclass)
{
    return CurrentTime;
}
gboolean sn_get_desktop(gchar *id, guint *desktop) { return FALSE; }
void sn_setup_spawn_environment(gchar *program, gchar *name,
                                gchar *icon_name, gint desktop) {}
void sn_spawn_cancel() {}

#else

#include "openbox.h"
#include "screen.h"

#define SN_API_NOT_YET_FROZEN
#include <libsn/sn.h>

static SnDisplay *sn_display;
static SnMonitorContext *sn_context;
static SnLauncherContext *sn_launcher;
static GSList *sn_waits; /* list of SnStartupSequences we're waiting on */

static SnStartupSequence* sequence_find(const gchar *id);

static void sn_handler(const XEvent *e, gpointer data);
static void sn_event_func(SnMonitorEvent *event, gpointer data);

void sn_startup(gboolean reconfig)
{
    gchar *s;

    if (reconfig) return;

    /* unset this so we don't pass it on unknowingly */
    s = g_strdup("DESKTOP_STARTUP_ID");
    putenv(s);
    g_free(s);

    sn_display = sn_display_new(obt_display, NULL, NULL);
    sn_context = sn_monitor_context_new(sn_display, ob_screen,
                                        sn_event_func, NULL, NULL);
    sn_launcher = sn_launcher_context_new(sn_display, ob_screen);

    obt_main_loop_x_add(ob_main_loop, sn_handler, NULL, NULL);
}

void sn_shutdown(gboolean reconfig)
{
    GSList *it;

    if (reconfig) return;

    obt_main_loop_x_remove(ob_main_loop, sn_handler);

    for (it = sn_waits; it; it = g_slist_next(it))
        sn_startup_sequence_unref((SnStartupSequence*)it->data);
    g_slist_free(sn_waits);
    sn_waits = NULL;

    screen_set_root_cursor();

    sn_launcher_context_unref(sn_launcher);
    sn_monitor_context_unref(sn_context);
    sn_display_unref(sn_display);
}

static SnStartupSequence* sequence_find(const gchar *id)
{
    SnStartupSequence*ret = NULL;
    GSList *it;

    for (it = sn_waits; it; it = g_slist_next(it)) {
        SnStartupSequence *seq = it->data;
        if (!strcmp(id, sn_startup_sequence_get_id(seq))) {
            ret = seq;
            break;
        }
    }
    return ret;
}

gboolean sn_app_starting(void)
{
    return sn_waits != NULL;
}

static gboolean sn_wait_timeout(gpointer data)
{
    SnStartupSequence *seq = data;
    sn_waits = g_slist_remove(sn_waits, seq);
    screen_set_root_cursor();
    return FALSE; /* don't repeat */
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

    if (!(seq = sn_monitor_event_get_startup_sequence(ev)))
        return;

    switch (sn_monitor_event_get_type(ev)) {
    case SN_MONITOR_EVENT_INITIATED:
        sn_startup_sequence_ref(seq);
        sn_waits = g_slist_prepend(sn_waits, seq);
        /* 20 second timeout for apps to start if the launcher doesn't
           have a timeout */
        obt_main_loop_timeout_add(ob_main_loop, 20 * G_USEC_PER_SEC,
                                  sn_wait_timeout, seq,
                                  g_direct_equal,
                                  (GDestroyNotify)sn_startup_sequence_unref);
        change = TRUE;
        break;
    case SN_MONITOR_EVENT_CHANGED:
        /* XXX feedback changed? */
        change = TRUE;
        break;
    case SN_MONITOR_EVENT_COMPLETED:
    case SN_MONITOR_EVENT_CANCELED:
        if ((seq = sequence_find(sn_startup_sequence_get_id(seq)))) {
            sn_waits = g_slist_remove(sn_waits, seq);
            obt_main_loop_timeout_remove_data(ob_main_loop, sn_wait_timeout,
                                              seq, FALSE);
            change = TRUE;
        }
        break;
    };

    if (change)
        screen_set_root_cursor();
}

Time sn_app_started(const gchar *id, const gchar *wmclass)
{
    GSList *it;
    Time t = CurrentTime;

    if (!id && !wmclass)
        return t;

    for (it = sn_waits; it; it = g_slist_next(it)) {
        SnStartupSequence *seq = it->data;
        gboolean found = FALSE;
        const gchar *seqid, *seqclass, *seqname, *seqbin;
        seqid = sn_startup_sequence_get_id(seq);
        seqclass = sn_startup_sequence_get_wmclass(seq);
        seqname = sn_startup_sequence_get_name(seq);
        seqbin = sn_startup_sequence_get_binary_name(seq);

        if (id && seqid) {
            /* if the app has a startup id, then look for that for highest
               accuracy */
            if (!strcmp(seqid, id))
                found = TRUE;
        } else {
            seqclass = sn_startup_sequence_get_wmclass(seq);
            seqname = sn_startup_sequence_get_name(seq);
            seqbin = sn_startup_sequence_get_binary_name(seq);

            if ((seqname && !g_ascii_strcasecmp(seqname, wmclass)) ||
                (seqbin && !g_ascii_strcasecmp(seqbin, wmclass)) ||
                (seqclass && !strcmp(seqclass, wmclass)))
                found = TRUE;
        }

        if (found) {
            sn_startup_sequence_complete(seq);
            t = sn_startup_sequence_get_timestamp(seq);
            break;
        }
    }
    return t;
}

gboolean sn_get_desktop(gchar *id, guint *desktop)
{
    SnStartupSequence *seq;

    if (id && (seq = sequence_find(id))) {
        gint desk = sn_startup_sequence_get_workspace(seq);
        if (desk != -1) {
            *desktop = desk;
            return TRUE;
        }
    }
    return FALSE;
}

static gboolean sn_launch_wait_timeout(gpointer data)
{
    SnLauncherContext *sn = data;
    sn_launcher_context_complete(sn);
    return FALSE; /* don't repeat */
}

void sn_setup_spawn_environment(gchar *program, gchar *name,
                                gchar *icon_name, gint desktop)
{
    gchar *desc;
    const char *id;

    desc = g_strdup_printf(_("Running %s\n"), program);

    if (sn_launcher_context_get_initiated(sn_launcher)) {
        sn_launcher_context_unref(sn_launcher);
        sn_launcher = sn_launcher_context_new(sn_display, ob_screen);
    }

    sn_launcher_context_set_name(sn_launcher, name ? name : program);
    sn_launcher_context_set_description(sn_launcher, desc);
    sn_launcher_context_set_icon_name(sn_launcher, icon_name ?
                                      icon_name : program);
    sn_launcher_context_set_binary_name(sn_launcher, program);
    if (desktop >= 0 && (unsigned) desktop < screen_num_desktops)
        sn_launcher_context_set_workspace(sn_launcher, (signed) desktop);
    sn_launcher_context_initiate(sn_launcher, "openbox", program,
                                 event_curtime);
    id = sn_launcher_context_get_startup_id(sn_launcher);

    /* 20 second timeout for apps to start */
    sn_launcher_context_ref(sn_launcher);
    obt_main_loop_timeout_add(ob_main_loop, 20 * G_USEC_PER_SEC,
                              sn_launch_wait_timeout, sn_launcher,
                              g_direct_equal,
                              (GDestroyNotify)sn_launcher_context_unref);

    putenv(g_strdup_printf("DESKTOP_STARTUP_ID=%s", id));

    g_free(desc);
}

void sn_spawn_cancel(void)
{
    sn_launcher_context_complete(sn_launcher);
}

#endif
