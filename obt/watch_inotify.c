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

#ifdef HAVE_SYS_INOTIFY_H

#include "watch.h"

#include <sys/inotify.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#ifdef HAVE_STRING_H
#  include <string.h>
#endif
#ifdef HAVE_ERRNO_H
#  include <errno.h>
#endif

#include <glib.h>

typedef struct _InoSource InoSource;
typedef struct _InoTarget InoTarget;

/*! Callback function in the watch general system.
  Matches definition in watch.c
*/
typedef void (*ObtWatchNotifyFunc)(const gchar *path, gpointer target,
                                   ObtWatchNotifyType type);

struct _InoSource {
    GSource source;

    GPollFD pfd;
    ObtWatchNotifyFunc notify;
    GHashTable *targets_by_key;
    GHashTable *targets_by_path;
};

struct _InoTarget {
    gint key;
    gchar *path;
    gpointer watch_target;
    gboolean is_dir;
    gboolean watch_hidden;
};

static gboolean source_check(GSource *source);
static gboolean source_prepare(GSource *source, gint *timeout);
static gboolean source_read(GSource *source, GSourceFunc cb, gpointer data);
static void source_finalize(GSource *source);
static gint add_target(GSource *source, InoTarget *parent,
                       const gchar *path, gboolean watch_hidden,
                       gpointer target);
static void remove_target(GSource *source, InoTarget *target);
static void target_free(InoTarget *target);

static GSourceFuncs source_funcs = {
    source_prepare,
    source_check,
    source_read,
    source_finalize
};

gint watch_sys_add_target(GSource *source, const char *path,
                          gboolean watch_hidden, gpointer target)
{
    return add_target(source, NULL, path, watch_hidden, target);
}

void watch_sys_remove_target(GSource *source, gint key)
{
    InoSource *ino_source = (InoSource*)source;
    InoTarget *t;

    t = g_hash_table_lookup(ino_source->targets_by_key, &key);
    remove_target(source, t);
}

GSource* watch_sys_create_source(ObtWatchNotifyFunc notify)
{
    gint fd;
    GSource *source;
    InoSource *ino_source;

    g_return_val_if_fail(notify != NULL, NULL);

    source = NULL;
    fd = inotify_init();
    if (fd < 0) {
        gchar *s = strerror(errno);
        g_warning("Failed to initialize inotify: %s", s);
    }
    else {
        g_debug("initialized inotify on fd %d", fd);
        source = g_source_new(&source_funcs, sizeof(InoSource));
        ino_source = (InoSource*)source;
        ino_source->notify = notify;
        ino_source->targets_by_key = g_hash_table_new_full(
            g_int_hash, g_int_equal, NULL, NULL);
        ino_source->targets_by_path = g_hash_table_new_full(
            g_str_hash, g_str_equal, NULL, (GDestroyNotify)target_free);
        ino_source->pfd = (GPollFD){ fd, G_IO_IN, G_IO_IN };
        g_source_add_poll(source, &ino_source->pfd);
        g_source_attach(source, NULL);
    }
    return source;
}

static gboolean source_prepare(GSource *source, gint *timeout)
{
    *timeout = -1;
    return FALSE;
}

static gboolean source_check(GSource *source)
{
    return TRUE;
}

static gboolean source_read(GSource *source, GSourceFunc cb, gpointer data)
{
    const gint BUF_LEN = sizeof(struct inotify_event) + 1024;
    InoSource *ino_source = (InoSource*)source;
    gchar buffer[BUF_LEN];
    gchar *name;
    guint name_len; /* number of bytes read for the name */
    guint event_len; /* number of bytes read for the event */
    gint len; /* number of bytes in the buffer */
    gint pos; /* position in the buffer */
    enum {
        READING_EVENT,
        READING_NAME_BUFFER,
        READING_NAME_HEAP
    } state;
    struct inotify_event event;

    pos = BUF_LEN;
    state = READING_EVENT;
    len = event_len = name_len = 0;

    while (TRUE) {
        if (pos == len || !len || event_len) {
            /* refill the buffer */

            if (event_len)
                pos = event_len;
            else
                pos = 0;

            len = read(ino_source->pfd.fd, &buffer[pos], BUF_LEN-pos);

            if (len < 0 && errno != EINTR) {
                gchar *s = strerror(errno);
                g_warning("Failed to read from inotify: %s", s);
                return FALSE; /* won't read any more */
            }
            if (len == 0) {
                g_debug("Done reading from inotify");
                return TRUE;
            }

            g_debug("read %d bytes", len);
        }

        if (state == READING_EVENT) {
            const guint remain = len - pos;

            if (remain < sizeof(struct inotify_event)) {
                /* there is more of the event struct to read */
                guint i;
                for (i = 0; i < remain; ++i)
                    buffer[i] = buffer[pos+i];
                g_debug("leftover %d bytes of event struct", remain);
            }
            else {
                event = *(struct inotify_event*)&buffer[pos];
                pos += sizeof(struct inotify_event);

                g_debug("read event: wd %d mask %x len %d",
                        event.wd, event.mask, event.len);

                if (remain >= event.len) {
                    g_debug("name fits in buffer");
                    state = READING_NAME_BUFFER;
                    name = &buffer[pos];
                    name_len = event.len;
                    pos += event.len;
                }
                else { /* remain < event.len */
                    g_debug("name doesn't fit in buffer");
                    state = READING_NAME_HEAP;
                    name = g_new(gchar, event.len);
                    memcpy(name, &buffer[pos], remain);
                    name_len = remain;
                    pos += remain;
                }
            }
        }
        if (state == READING_NAME_HEAP && pos < len) {
            const guint buf_remain = len - pos;
            const guint name_remain = event.len - name_len;
            const guint copy_len = MIN(buf_remain, name_remain);
            memcpy(name+name_len, &buffer[pos], copy_len);
            name_len += copy_len;
            pos += copy_len;
        }
        if ((state == READING_NAME_BUFFER || state == READING_NAME_HEAP) &&
            name_len == event.len)
        {
            /* done reading the file name ! */
            InoTarget *t;
            gboolean report;
            ObtWatchNotifyType type;
            gchar *full_path;

            g_debug("read filename %s mask %x", name, event.mask);

            event.mask &= ~IN_IGNORED;  /* skip this one, we watch for things
                                           to get removed explicitly so this
                                           will just be double-reporting */
            if (event.mask) {

                t = g_hash_table_lookup(ino_source->targets_by_key, &event.wd);
                g_assert(t != NULL);

                full_path = g_build_filename(t->path, name, NULL);
                g_debug("full path to change: %s", full_path);

                /* don't report hidden stuff inside a directory watch */
                report = !t->is_dir || name[0] != '.' || t->watch_hidden;
                if (event.mask & IN_MOVE_SELF) {
                    g_warning("Watched target was moved away: %s", t->path);
                    type = OBT_WATCH_SELF_REMOVED;
                }
                else if (event.mask & IN_ISDIR) {
                    if (event.mask & IN_MOVED_TO ||
                        event.mask & IN_CREATE)
                    {
                        add_target(source, t, full_path, t->watch_hidden,
                                   t->watch_target);
                        g_debug("added %s", full_path);
                    }
                    else if (event.mask & IN_MOVED_FROM ||
                             event.mask & IN_DELETE)
                    {
                        InoTarget *subt;

                        subt = g_hash_table_lookup(ino_source->targets_by_path,
                                                   full_path);
                        g_assert(subt);
                        remove_target(source, subt);
                        g_debug("removed %s", full_path);
                    }
                    report = FALSE;
                }
                else {
                    if (event.mask & IN_MOVED_TO || event.mask & IN_CREATE)
                        type = OBT_WATCH_ADDED;
                    else if (event.mask & IN_MOVED_FROM ||
                             event.mask & IN_DELETE)
                        type = OBT_WATCH_REMOVED;
                    else if (event.mask & IN_MODIFY)
                        type = OBT_WATCH_MODIFIED;
                    else
                        g_assert_not_reached();
                }

                if (report) {
                    /* call the GSource callback if there is one */
                    if (cb) cb(data);

                    /* call the WatchNotify callback */
                    ino_source->notify(full_path, t->watch_target, type);
                }

                g_free(full_path);
            }

            if (state == READING_NAME_HEAP)
                g_free(name);
            state = READING_EVENT;
        }
    }
}

static void source_finalize(GSource *source)
{
    InoSource *ino_source = (InoSource*)source;
    g_debug("source_finalize");
    close(ino_source->pfd.fd);
    g_hash_table_destroy(ino_source->targets_by_key);
}

static gint add_target(GSource *source, InoTarget *parent,
                       const gchar *path, gboolean watch_hidden,
                       gpointer target)
{
    InoSource *ino_source;
    InoTarget *ino_target;
    guint32 mask;
    gint key;
    gboolean is_dir;

    ino_source = (InoSource*)source;

    is_dir = g_file_test(path, G_FILE_TEST_IS_DIR);
    if (is_dir)
        mask = IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVE;
    else
        mask = IN_MODIFY;
    /* only watch IN_MOVE_SELF on the top-most target of the watch */
    if (!parent)
        mask |= IN_MOVE_SELF;

    ino_target = NULL;
    key = inotify_add_watch(ino_source->pfd.fd, path, mask);
    g_debug("added watch descriptor %d for fd %d on path %s",
            key, ino_source->pfd.fd, path);
    if (key < 0) {
        gchar *s = strerror(errno);
        g_warning("Unable to watch path %s: %s", path, s);
    }
    else {
        ino_target = g_slice_new(InoTarget);
        ino_target->key = key;
        ino_target->path = g_strdup(path);
        ino_target->is_dir = is_dir;
        ino_target->watch_hidden = watch_hidden;
        ino_target->watch_target = target;
        g_hash_table_insert(ino_source->targets_by_key, &ino_target->key,
                            ino_target);
        g_hash_table_insert(ino_source->targets_by_path, ino_target->path,
                            ino_target);
    }

    if (key >= 0 && is_dir) {
        /* recurse */
        GDir *dir;

        dir = g_dir_open(path, 0, NULL);
        if (dir) {
            const gchar *name;

            while ((name = g_dir_read_name(dir))) {
                if (name[0] != '.' || watch_hidden) {
                    gchar *subpath;

                    subpath = g_build_filename(path, name, NULL);
                    if (g_file_test(subpath, G_FILE_TEST_IS_DIR))
                        add_target(source, ino_target, subpath, watch_hidden,
                                   target);
                    else
                        /* notify for each file in the directory on startup */
                        ino_source->notify(subpath, ino_target->watch_target,
                                           OBT_WATCH_ADDED);
                    g_free(subpath);
                }
            }
        }
        g_dir_close(dir);
    }

    return key;
}

static void remove_target(GSource *source, InoTarget *target)
{
    InoSource *ino_source = (InoSource*)source;
    g_debug("removing wd %d for fd %d", target->key, ino_source->pfd.fd);
    inotify_rm_watch(ino_source->pfd.fd, (guint32)target->key);
    g_hash_table_remove(ino_source->targets_by_key, &target->key);
    g_hash_table_remove(ino_source->targets_by_path, target->path);
}

static void target_free(InoTarget *target)
{
    g_free(target->path);
    g_slice_free(InoTarget, target);
}

#endif
