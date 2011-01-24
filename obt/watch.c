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

#ifdef HAVE_SYS_INOTIFY_H
#  include <sys/inotify.h>
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <errno.h>

struct _ObtWatch {
    guint ref;
    gint ino_fd;
    guint ino_watch;
    GHashTable *targets;

#ifdef HAVE_SYS_INOTIFY_H
    GHashTable *targets_by_wd;
#endif
};

typedef struct _ObtWatchTarget {
    ObtWatch *w;

#ifdef HAVE_SYS_INOTIFY_H
    gint wd;
#endif

    gchar *path;
    ObtWatchFunc func;
    gpointer data;
} ObtWatchTarget;

static void init_inot(ObtWatch *w);
static gboolean read_inot(GIOChannel *s, GIOCondition cond, gpointer data);
static gboolean add_inot(ObtWatch *w, ObtWatchTarget *t, const char *path,
                         gboolean dir);
static void rm_inot(ObtWatchTarget *t);
static ObtWatchTarget* target_new(ObtWatch *w, const gchar *path,
                                  ObtWatchFunc func, gpointer data);
static void target_free(ObtWatchTarget *t);

ObtWatch* obt_watch_new()
{
    ObtWatch *w;

    w = g_slice_new(ObtWatch);
    w->ref = 1;
    w->ino_fd = -1;
    w->targets = g_hash_table_new_full(g_str_hash, g_str_equal,
                                       NULL, (GDestroyNotify)target_free);
#ifdef HAVE_SYS_INOTIFY_H
    w->targets_by_wd = g_hash_table_new(g_int_hash, g_int_equal);
#endif

    init_inot(w);

    return w;
}
void obt_watch_ref(ObtWatch *w)
{
    ++w->ref;
}

void obt_watch_unref(ObtWatch *w)
{
    if (--w->ref < 1) {
        if (w->ino_fd >= 0 && w->ino_watch)
            g_source_remove(w->ino_watch);

        g_hash_table_destroy(w->targets);
        g_hash_table_destroy(w->targets_by_wd);

        g_slice_free(ObtWatch, w);
    }
}

static void init_inot(ObtWatch *w)
{
#ifdef HAVE_SYS_INOTIFY_H
    if (w->ino_fd >= 0) return;

    w->ino_fd = inotify_init();
    if (w->ino_fd >= 0) {
        GIOChannel *ch;

        ch = g_io_channel_unix_new(w->ino_fd);
        w->ino_watch = g_io_add_watch(ch, G_IO_IN | G_IO_HUP | G_IO_ERR,
                                      read_inot, w);
        g_io_channel_unref(ch);
    }
#endif
}

static gboolean read_inot(GIOChannel *src, GIOCondition cond, gpointer data)
{
#ifdef HAVE_SYS_INOTIFY_H
    ObtWatch *w = data;
    ObtWatchTarget *t;
    struct inotify_event s;
    gint len;
    guint ilen;
    char *name;
    
    /* read the event */
    for (ilen = 0; ilen < sizeof(s); ilen += len) {
        len = read(w->ino_fd, ((char*)&s)+ilen, sizeof(s)-ilen);
        if (len < 0 && errno != EINTR) return FALSE; /* error, don't repeat */
        if (!len) return TRUE; /* nothing there */
    }

    name = g_new(char, s.len);

    /* read the filename */
    for (ilen = 0; ilen < s.len; ilen += len) {
        len = read(w->ino_fd, name+ilen, s.len-ilen);
        if (len < 0 && errno != EINTR) return FALSE; /* error, don't repeat */
        if (!len) return TRUE; /* nothing there */
    }

    t = g_hash_table_lookup(w->targets, &s.wd);
    if (t) t->func(w, name, t->data);

    g_free(name);
#endif
    return TRUE; /* repeat */
}

static gboolean add_inot(ObtWatch *w, ObtWatchTarget *t, const char *path,
                         gboolean dir)
{
#ifndef HAVE_SYS_INOTIFY_H
    return FALSE;
#else
    gint mask;
    if (w->ino_fd < 0) return FALSE;
    if (dir) mask = IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVE;
    else g_assert_not_reached();
    t->wd = inotify_add_watch(w->ino_fd, path, mask);
    return TRUE;
#endif
}

static void rm_inot(ObtWatchTarget *t)
{
#ifdef HAVE_SYS_INOTIFY_H
    if (t->w->ino_fd < 0) return;
    if (t->wd < 0) return;
    inotify_rm_watch(t->w->ino_fd, t->wd);
#endif
}

static ObtWatchTarget* target_new(ObtWatch *w, const gchar *path,
                                  ObtWatchFunc func, gpointer data)
{
    ObtWatchTarget *t;

    t = g_slice_new0(ObtWatchTarget);
    t->w = w;
    t->wd = -1;
    t->path = g_strdup(path);
    t->func = func;
    t->data = data;

    if (!add_inot(w, t, path, TRUE)) {
        g_assert_not_reached(); /* XXX do something */
    }

#ifndef HAVE_SYS_INOTIFY_H
#error need inotify for now
#endif

    return t;
}

static void target_free(ObtWatchTarget *t)
{
    rm_inot(t);

    g_free(t->path);
    g_slice_free(ObtWatchTarget, t);
}

void obt_paths_watch_dir(ObtWatch *w, const gchar *path,
                         ObtWatchFunc func, gpointer data)
{
    ObtWatchTarget *t;

    g_return_if_fail(w != NULL);
    g_return_if_fail(path != NULL);
    g_return_if_fail(data != NULL);

    t = target_new(w, path, func, data);
    g_hash_table_insert(w->targets, t->path, t);
#ifdef HAVE_SYS_INOTIFY_H
    g_hash_table_insert(w->targets_by_wd, &t->wd, t);
#endif
}

void obt_paths_unwatch_dir(ObtWatch *w, const gchar *path)
{
    ObtWatchTarget *t;
    
    g_return_if_fail(w != NULL);
    g_return_if_fail(path != NULL);

    t = g_hash_table_lookup(w->targets, path);

    if (t) {
#ifdef HAVE_SYS_INOTIFY_H
        g_hash_table_remove(w->targets_by_wd, &t->wd);
#endif
        g_hash_table_remove(w->targets, path);
    }
}
