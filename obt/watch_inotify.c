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
#include "watch_interface.h"

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
typedef struct _InoData InoData;

struct _InoSource {
    GSource source;

    GPollFD pfd;
    GHashTable *targets_by_key;
    GHashTable *files_by_key;
};

struct _InoData {
    gint key;
};

static gboolean source_check(GSource *source);
static gboolean source_prepare(GSource *source, gint *timeout);
static gboolean source_read(GSource *source, GSourceFunc cb, gpointer data);
static void source_finalize(GSource *source);

static GSourceFuncs source_funcs = {
    source_prepare,
    source_check,
    source_read,
    source_finalize
};

GSource* watch_sys_create_source(void)
{
    gint fd;
    GSource *source;
    InoSource *ino_source;

    source = NULL;
    fd = inotify_init();
    if (fd < 0) {
        gchar *s = strerror(errno);
        g_warning("Failed to initialize inotify: %s", s);
    }
    else {
        /*g_debug("initialized inotify on fd %u", fd);*/
        source = g_source_new(&source_funcs, sizeof(InoSource));
        ino_source = (InoSource*)source;
        ino_source->targets_by_key = g_hash_table_new_full(
            g_int_hash, g_int_equal, NULL, NULL);
        ino_source->files_by_key = g_hash_table_new_full(
            g_int_hash, g_int_equal, NULL, NULL);
        ino_source->pfd = (GPollFD){ fd, G_IO_IN, 0 };
        g_source_add_poll(source, &ino_source->pfd);
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
    InoSource *ino_source = (InoSource*)source;

    return ino_source->pfd.revents;
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

    /* sometimes we can end up here even tho no events have been reported by
       the kernel.  blame glib?  but we'll block if we read in that case. */
    while (ino_source->pfd.revents) {
        if (pos == len || !len || event_len) {
            /* refill the buffer */

            if (event_len)
                pos = event_len;
            else
                pos = 0;

            len = read(ino_source->pfd.fd, &buffer[pos], BUF_LEN-pos);

            if (len < 0) {
                if (errno != EINTR) {
                    gchar *s = strerror(errno);
                    g_warning("Failed to read from inotify: %s", s);
                    return FALSE; /* won't read any more */
                }
                else {
                    g_warning("Interrupted reading from inotify");
                    return TRUE;
                }
            }
            if (len == 0) {
                /*g_debug("Done reading from inotify");*/
                return TRUE;
            }

            /*g_debug("read %d bytes", len);*/
        }

        if (state == READING_EVENT) {
            const guint remain = len - pos;

            if (remain < sizeof(struct inotify_event)) {
                /* there is more of the event struct to read */
                guint i;
                for (i = 0; i < remain; ++i)
                    buffer[i] = buffer[pos+i];
                /*g_debug("leftover %d bytes of event struct", remain);*/
            }
            else {
                event = *(struct inotify_event*)&buffer[pos];
                pos += sizeof(struct inotify_event);

                /*g_debug("read event: wd %d mask %x len %d",
                          event.wd, event.mask, event.len);*/

                if (remain >= event.len) {
                    /*g_debug("name fits in buffer");*/
                    state = READING_NAME_BUFFER;
                    name = &buffer[pos];
                    name_len = event.len;
                    pos += event.len;
                }
                else { /* remain < event.len */
                    /*g_debug("name doesn't fit in buffer");*/
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

            /*
            if (event.len)
                g_debug("read filename %s mask %x", name, event.mask);
            else
                g_debug("read no filename mask %x", event.mask);
            */

            event.mask &= ~IN_IGNORED;  /* skip this one, we watch for things
                                           to get removed explicitly so this
                                           will just be double-reporting */
            if (event.mask) {
                ObtWatchTarget *t;
                ObtWatchFile *f;
                gchar **name_split;
                gchar **c;

                f = g_hash_table_lookup(ino_source->files_by_key, &event.wd);
                t = g_hash_table_lookup(ino_source->targets_by_key, &event.wd);
                g_assert(f != NULL);

                if (event.len)
                    name_split = g_strsplit(name, G_DIR_SEPARATOR_S, 0);
                else
                    name_split = g_strsplit("", G_DIR_SEPARATOR_S, 0);

                if (f) {
                    if (event.mask & IN_MOVED_TO || event.mask & IN_CREATE) {
                        ObtWatchFile *parent = f;
                        for (c = name_split; *(c+1); ++c) {
                            parent = watch_main_file_child(parent, *c);
                            g_assert(parent);
                        }
                        watch_main_notify_add(t, parent, *c);
                    }
                    else if (event.mask & IN_MOVED_FROM ||
                             event.mask & IN_MOVE_SELF ||
                             event.mask & IN_DELETE ||
                             event.mask & IN_DELETE_SELF)
                    {
                        ObtWatchFile *file = f;
                        for (c = name_split; *c; ++c)
                            file = watch_main_file_child(file, *c);
                        if (file) /* may not have been tracked */
                            watch_main_notify_remove(t, file);
                    }
                    else if (event.mask & IN_MODIFY) {
                        ObtWatchFile *file = f;
                        for (c = name_split; *c; ++c)
                            file = watch_main_file_child(file, *c);
                        if (file) /* may not have been tracked */
                            watch_main_notify_modify(t, file);
                    }
                    else
                        g_assert_not_reached();

                    /* call the GSource callback if there is one */
                    if (cb) cb(data);
                }

                g_strfreev(name_split);

                /* only read one event at a time, so poll can tell us if there
                   is another one ready, and we don't block on the read()
                   needlessly. */
                break;
            }

            if (state == READING_NAME_HEAP)
                g_free(name);
            state = READING_EVENT;
        }
    }
    return TRUE;
}

static void source_finalize(GSource *source)
{
    InoSource *ino_source = (InoSource*)source;
    /*g_debug("source_finalize");*/
    close(ino_source->pfd.fd);
    g_hash_table_destroy(ino_source->targets_by_key);
    g_hash_table_destroy(ino_source->files_by_key);
}


gpointer watch_sys_add_file(GSource *source,
                            ObtWatchTarget *target,
                            ObtWatchFile *file,
                            gboolean is_dir)
{
    InoSource *ino_source;
    InoData *ino_data;
    guint32 mask;
    gint key;
    gchar *path;

    ino_source = (InoSource*)source;

    if (is_dir)
        mask = IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVE;
    else
        mask = IN_MODIFY;
    /* only watch IN_MOVE_SELF on the top-most target of the watch */
    if (watch_main_target_root(target) == file)
        mask |= IN_MOVE_SELF | IN_DELETE_SELF;

    path = watch_main_target_file_full_path(target, file);

    ino_data = NULL;
    key = inotify_add_watch(ino_source->pfd.fd, path, mask);
    /*g_debug("added watch descriptor %d for fd %d on path %s",
              key, ino_source->pfd.fd, path);*/
    if (key < 0) {
        gchar *s = strerror(errno);
        g_warning("Unable to watch path %s: %s", path, s);
    }
    else {
        ino_data = g_slice_new(InoData);
        ino_data->key = key;
        g_hash_table_insert(ino_source->targets_by_key,
                            &ino_data->key,
                            target);
        g_hash_table_insert(ino_source->files_by_key,
                            &ino_data->key,
                            file);
    }

    g_free(path);

    return ino_data;
}

void watch_sys_remove_file(GSource *source,
                           ObtWatchTarget *target,
                           ObtWatchFile *file,
                           gpointer data)
{
    InoSource *ino_source;
    InoData *ino_data;

    if (data) {
        ino_source = (InoSource*)source;
        ino_data = data;

        /*g_debug("removing wd %d for fd %d", ino_data->key,
                  ino_source->pfd.fd); */
        inotify_rm_watch(ino_source->pfd.fd, (guint32)ino_data->key);

        g_hash_table_remove(ino_source->targets_by_key, &ino_data->key);
        g_hash_table_remove(ino_source->files_by_key, &ino_data->key);
        g_slice_free(InoData, ino_data);
    }
}


#endif
