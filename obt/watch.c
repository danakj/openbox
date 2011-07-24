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
#include "obt/watch_interface.h"

#ifdef HAVE_STRING_H
#  include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h>
#endif

#include <glib.h>

/* Interface for system-specific stuff (e.g. inotify). the functions are
   defined in in watch_<system>.c
*/

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
    gboolean watch_hidden;
    ObtWatchFile *file;
    gpointer watch_sys_data;
};

struct _ObtWatchFile {
    ObtWatchFile *parent;
    gchar *name;
    time_t modified;
    /* If this is a directory, then the hash table contains pointers to other
       ObtWatchFile objects that are contained in this one. */
    GHashTable *children_by_name;

    gboolean seen;
    gpointer watch_sys_data;
};

static void target_free(ObtWatchTarget *t);

static ObtWatchFile* tree_create(ObtWatchTarget *t,
                                 const gchar *path, const gchar *last,
                                 ObtWatchFile *parent);
static void tree_destroy(ObtWatchFile *file);

static void notify(ObtWatchTarget *target, ObtWatchFile *file,
                   ObtWatchNotifyType type);

ObtWatch* obt_watch_new()
{
    ObtWatch *w;
    GSource *source;

    w = NULL;
    source = watch_sys_create_source();
    if (source) {
        w = g_slice_new(ObtWatch);
        w->ref = 1;
        w->targets_by_path = g_hash_table_new_full(
            g_str_hash, g_str_equal, NULL, (GDestroyNotify)target_free);
        w->source = source;

        g_source_attach(source, NULL);
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
    if (t->file) tree_destroy(t->file);
    g_free(t->base_path);
    g_slice_free(ObtWatchTarget, t);
}

/*! Scans through the filesystem under the target and reports each file/dir
  that it finds to the watch subsystem.
  @path Absolute path to the file to scan for.
  @last The last component (filename) of the file to scan for.
  @parent The parent directory (if it exists) of the file we are scanning for.
 */
static ObtWatchFile* tree_create(ObtWatchTarget *target,
                                 const gchar *path, const gchar *last,
                                 ObtWatchFile *parent)
{
    ObtWatchFile *file;
    struct stat buf;

    if (stat(path, &buf) < 0)
        return NULL;

    file = g_slice_new(ObtWatchFile);
    file->name = g_strdup(last);
    file->modified = 0;
    file->parent = parent;
    file->children_by_name = NULL;

    if (S_ISDIR(buf.st_mode))
        file->children_by_name =
            g_hash_table_new_full(g_str_hash, g_str_equal,
                                  NULL, (GDestroyNotify)tree_destroy);

    if (!parent) {
        g_assert(target->file == NULL);
        target->file = file;
    }
    else
        g_hash_table_replace(parent->children_by_name,
                             file->name,
                             file);

    if (!S_ISDIR(buf.st_mode))
        notify(target, file, OBT_WATCH_ADDED);

    file->watch_sys_data =
        watch_sys_add_file(target->w->source,
                           target, file, S_ISDIR(buf.st_mode));

    /* recurse on the contents if it's a directory */
    if (file && watch_main_file_is_dir(file)) {
        GDir *dir;

        dir = g_dir_open(path, 0, NULL);
        if (dir) {
            const gchar *name;

            while ((name = g_dir_read_name(dir))) {
                if (name[0] != '.' || target->watch_hidden) {
                    gchar *subpath;
                    ObtWatchFile *child;

                    subpath = g_build_filename(path, name, NULL);
                    child = tree_create(target, subpath, name, file);
                    g_free(subpath);
                }
            }
        }
        g_dir_close(dir);
    }

    if (file)
        file->modified = buf.st_mtime;

    return file;
}

static void tree_destroy(ObtWatchFile *file)
{
    g_free(file->name);
    if (file->children_by_name)
        g_hash_table_unref(file->children_by_name);
    g_slice_free(ObtWatchFile, file);
}

gboolean obt_watch_add(ObtWatch *w, const gchar *path, gboolean watch_hidden,
                       ObtWatchFunc func, gpointer data)
{
    ObtWatchTarget *t;

    g_return_val_if_fail(w != NULL, FALSE);
    g_return_val_if_fail(path != NULL, FALSE);
    g_return_val_if_fail(func != NULL, FALSE);
    g_return_val_if_fail(path[0] == G_DIR_SEPARATOR, FALSE);

    t = g_slice_new(ObtWatchTarget);
    t->w = w;
    t->base_path = g_strdup(path);
    t->watch_hidden = watch_hidden;
    t->func = func;
    t->data = data;
    t->file = NULL;
    g_hash_table_replace(w->targets_by_path, t->base_path, t);

    watch_main_notify_add(t, NULL, NULL);

    return TRUE;
}

void obt_watch_remove(ObtWatch *w, const gchar *path)
{
    g_return_if_fail(w != NULL);
    g_return_if_fail(path != NULL);
    g_return_if_fail(path[0] == G_DIR_SEPARATOR);

    /* this also calls target_free which does notifies */
    g_hash_table_remove(w->targets_by_path, path);
}

static ObtWatchFile* refresh_file(ObtWatchTarget *target, ObtWatchFile *file,
                                  ObtWatchFile *parent, const gchar *path)
{
    struct stat buf;

    g_assert(file != NULL);

    if (stat(path, &buf) < 0) {
        watch_main_notify_remove(target, file);
        file = NULL;
    }

    else if (S_ISDIR(buf.st_mode) != watch_main_file_is_dir(file)) {
        gchar *name = g_strdup(file->name);

        watch_main_notify_remove(target, file);
        watch_main_notify_add(target, parent, name);
        g_free(name);
        file = NULL;
    }

    /* recurse on the contents if it's a directory */
    else if (watch_main_file_is_dir(file)) {
        GDir *dir;
        GList *children, *it;

        children = watch_main_file_children(file);
        for (it = children; it; it = g_list_next(it))
            ((ObtWatchFile*)it->data)->seen = FALSE;
        g_list_free(children);

        dir = g_dir_open(path, 0, NULL);
        if (dir) {
            const gchar *name;

            while ((name = g_dir_read_name(dir))) {
                if (name[0] != '.' || target->watch_hidden) {
                    ObtWatchFile *child = watch_main_file_child(file, name);

                    if (!child)
                        watch_main_notify_add(target, file, name);
                    else {
                        gchar *subpath;
                        ObtWatchFile *newchild;

                        subpath = g_build_filename(path, name, NULL);
                        newchild = refresh_file(target, child, file, subpath);
                        g_free(subpath);

                        if (newchild)
                            child->seen = TRUE;
                    }
                }
            }
        }
        g_dir_close(dir);

        children = watch_main_file_children(file);
        for (it = children; it; it = g_list_next(it))
            if (((ObtWatchFile*)it->data)->seen == FALSE)
                watch_main_notify_remove(target, it->data);
        g_list_free(children);
    }

    /* check for modifications if it's a file */
    else {
        if (file->modified >= 0 && buf.st_mtime > file->modified)
            watch_main_notify_modify(target, file);
    }

    if (file)
        file->modified = buf.st_mtime;

    return file;
}

static void foreach_refresh_target(gpointer k, gpointer v, gpointer u)
{
    ObtWatchTarget *target = v;

    if (!target->file)
        /* we don't have any files being watched, try look for
           the target's root again */
        watch_main_notify_add(target, NULL, NULL);
    else
        refresh_file(target, target->file, NULL, target->base_path);
}

void obt_watch_refresh(ObtWatch *w)
{
    g_return_if_fail(w != NULL);

    g_hash_table_foreach(w->targets_by_path, foreach_refresh_target, NULL);
}

static void notify(ObtWatchTarget *target, ObtWatchFile *file,
                   ObtWatchNotifyType type)
{
    gchar *sub_path = watch_main_file_sub_path(file);
    target->func(target->w, target->base_path, sub_path, type,
                 target->data);
    g_free(sub_path);
}

void watch_main_notify_add(ObtWatchTarget *target,
                           ObtWatchFile *parent, const gchar *name)
{
    gchar *path;

    if (parent && name[0] == '.' && !target->watch_hidden)
        return;

    path = g_build_filename(target->base_path,
                            watch_main_file_sub_path(parent),
                            name,
                            NULL);
    tree_create(target, path, name, parent);
    g_free(path);
}

static void foreach_child_notify_removed(gpointer k, gpointer v, gpointer u)
{
    ObtWatchTarget *target = u;
    ObtWatchFile *file = v;

    if (watch_main_file_is_dir(file)) {
        g_hash_table_foreach(file->children_by_name,
                             foreach_child_notify_removed,
                             target);
        g_hash_table_remove_all(file->children_by_name);
    }
    else if (!file->parent)
        notify(target, file, OBT_WATCH_SELF_REMOVED);
    else
        notify(target, file, OBT_WATCH_REMOVED);

    watch_sys_remove_file(target->w->source,
                          target, file, file->watch_sys_data);
}

void watch_main_notify_remove(ObtWatchTarget *target, ObtWatchFile *file)
{
    foreach_child_notify_removed(NULL, file, target);

    tree_destroy(file);
    if (!file->parent)
        target->file = NULL;
}

void watch_main_notify_modify(ObtWatchTarget *target, ObtWatchFile *file)
{
    if (!watch_main_file_is_dir(file)) {
        notify(target, file, OBT_WATCH_MODIFIED);
    }
    file->modified = -1;
}

void build_sub_path(GString *str, ObtWatchFile *file)
{
    if (file->parent) {
        build_sub_path(str, file->parent);
        g_string_append(str, G_DIR_SEPARATOR_S);
        g_string_append(str, file->name);
    }
}

gboolean watch_main_target_watch_hidden(ObtWatchTarget *target)
{
    return target->watch_hidden;
}

ObtWatchFile* watch_main_target_root(ObtWatchTarget *target)
{
    return target->file;
}

gchar* watch_main_file_sub_path(ObtWatchFile *file)
{
    GString *str;
    gchar *ret = NULL;

    if (file) {
        str = g_string_new("");
        build_sub_path(str, file);
        ret = str->str;
        g_string_free(str, FALSE);
    }

    return ret;
}

gchar* watch_main_target_file_full_path(ObtWatchTarget *target,
                                        ObtWatchFile *file)
{
    GString *str;
    gchar *ret = NULL;

    if (file) {
        str = g_string_new(target->base_path);
        build_sub_path(str, file);
        ret = str->str;
        g_string_free(str, FALSE);
    }

    return ret;
}

gboolean watch_main_file_is_dir(ObtWatchFile *file)
{
    return file->children_by_name != NULL;
}

ObtWatchFile* watch_main_file_child(ObtWatchFile *file,
                                    const gchar *name)
{
    return g_hash_table_lookup(file->children_by_name, name);
}

GList* watch_main_file_children(ObtWatchFile *file)
{
    return g_hash_table_get_values(file->children_by_name);
}
