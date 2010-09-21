/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   obt/watch.c for the Openbox window manager
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

#include "obt/watch.h"

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include <glib.h>

typedef struct _ObtWatchTarget ObtWatchTarget;

/*! Callback function for the system-specific GSource to alert us to changes.
*/
typedef void (*ObtWatchNotifyFunc)(const gchar *sub_path,
                                   const gchar *full_path, gpointer target,
                                   ObtWatchNotifyType type);


/* Interface for system-specific stuff (e.g. inotify). the functions are
   defined in in watch_<system>.c
*/

/*! Initializes the watch subsystem, and returns a GSource for it.
  @param notify The GSource will call @notify when a watched file is changed.
  @return Returns a GSource* on success, and a NULL if an error occurred.
*/
GSource* watch_sys_create_source(ObtWatchNotifyFunc notify);
/*! Add a target to the watch subsystem.
  @return Returns an integer key that is used to uniquely identify the target
    within this subsystem.  A negative value indicates an error.
*/
gint watch_sys_add_target(GSource *source, const char *path,
                          gboolean watch_hidden, gpointer target);
/*! Remove a target from the watch system, by its key.
  Use the key returned from watch_sys_add_target() to remove the target.
*/
void watch_sys_remove_target(GSource *source, gint key);


/* General system which uses the watch_sys_* stuff
*/

struct _ObtWatch {
    guint ref;
    GHashTable *targets_by_path;
    GSource *source;
};

struct _ObtWatchTarget {
    ObtWatch *w;
    gchar *base_path;
    ObtWatchFunc func;
    gpointer data;
    gint key;
};

static void target_free(ObtWatchTarget *t);
static void target_notify(const gchar *sub_path, const gchar *full_path,
                          gpointer target, ObtWatchNotifyType type);

ObtWatch* obt_watch_new()
{
    ObtWatch *w;
    GSource *source;

    w = NULL;
    source = watch_sys_create_source(target_notify);
    if (source) {
        w = g_slice_new(ObtWatch);
        w->ref = 1;
        w->targets_by_path = g_hash_table_new_full(
            g_str_hash, g_str_equal, NULL, (GDestroyNotify)target_free);
        w->source = source;
    }
    return w;
}

void obt_watch_ref(ObtWatch *w)
{
    ++w->ref;
}

void obt_watch_unref(ObtWatch *w)
{
    if (--w->ref < 1) {
        g_hash_table_destroy(w->targets_by_path);
        g_source_destroy(w->source);
        g_slice_free(ObtWatch, w);
    }
}

static void target_free(ObtWatchTarget *t)
{
    if (t->key >= 0)
        watch_sys_remove_target(t->w->source, t->key);
    g_free(t->base_path);
    g_slice_free(ObtWatchTarget, t);
}

gboolean obt_watch_add(ObtWatch *w, const gchar *path,
                       gboolean watch_hidden,
                       ObtWatchFunc func, gpointer data)
{
    ObtWatchTarget *t;

    g_return_val_if_fail(w != NULL, FALSE);
    g_return_val_if_fail(path != NULL, FALSE);
    g_return_val_if_fail(func != NULL, FALSE);
    g_return_val_if_fail(path[0] == G_DIR_SEPARATOR, FALSE);

    t = g_slice_new0(ObtWatchTarget);
    t->w = w;
    t->base_path = g_strdup(path);
    t->func = func;
    t->data = data;
    g_hash_table_insert(w->targets_by_path, t->base_path, t);

    t->key = watch_sys_add_target(w->source, path, watch_hidden, t);
    if (t->key < 0) {
        g_hash_table_remove(w->targets_by_path, t->base_path);
        return FALSE;
    }

    return TRUE;
}

void obt_watch_remove(ObtWatch *w, const gchar *path)
{
    g_return_if_fail(w != NULL);
    g_return_if_fail(path != NULL);
    g_return_if_fail(path[0] == G_DIR_SEPARATOR);

    /* this also calls target_free */
    g_hash_table_remove(w->targets_by_path, path);
}

static void target_notify(const gchar *sub_path, const gchar *full_path,
                          gpointer target, ObtWatchNotifyType type)
{
    ObtWatchTarget *t = target;
    if (type == OBT_WATCH_SELF_REMOVED) {
        /* this also calls target_free */
        g_hash_table_remove(t->w->targets_by_path, t->base_path);
    }
    t->func(t->w, t->base_path, sub_path, full_path, type, t->data);
}
