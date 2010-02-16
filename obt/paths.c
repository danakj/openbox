/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   obt/paths.c for the Openbox window manager
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

#include "obt/paths.h"
#include "obt/util.h"

#ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#ifdef HAVE_STRING_H
#  include <string.h>
#endif

struct _ObtPaths
{
    gint   ref;
    gchar  *config_home;
    gchar  *data_home;
    gchar  *cache_home;
    GSList *config_dirs;
    GSList *data_dirs;
};

static gint slist_path_cmp(const gchar *a, const gchar *b)
{
    return strcmp(a, b);
}

typedef GSList* (*GSListFunc) (gpointer list, gconstpointer data);

static GSList* slist_path_add(GSList *list, gpointer data, GSListFunc func)
{
    g_assert(func);

    if (!data)
        return list;

    if (!g_slist_find_custom(list, data, (GCompareFunc) slist_path_cmp))
        list = func(list, data);
    else
        g_free(data);

    return list;
}

static GSList* split_paths(const gchar *paths)
{
    GSList *list = NULL;
    gchar **spl, **it;

    if (!paths)
        return NULL;
    spl = g_strsplit(paths, ":", -1);
    for (it = spl; *it; ++it)
        list = slist_path_add(list, *it, (GSListFunc) g_slist_append);
    g_free(spl);
    return list;
}

ObtPaths* obt_paths_new(void)
{
    ObtPaths *p;
    const gchar *path;

    p = g_slice_new0(ObtPaths);
    p->ref = 1;

    path = g_getenv("XDG_CONFIG_HOME");
    if (path && path[0] != '\0') /* not unset or empty */
        p->config_home = g_build_filename(path, NULL);
    else
        p->config_home = g_build_filename(g_get_home_dir(), ".config", NULL);

    path = g_getenv("XDG_DATA_HOME");
    if (path && path[0] != '\0') /* not unset or empty */
        p->data_home = g_build_filename(path, NULL);
    else
        p->data_home = g_build_filename(g_get_home_dir(), ".local",
                                        "share", NULL);

    path = g_getenv("XDG_CACHE_HOME");
    if (path && path[0] != '\0') /* not unset or empty */
        p->cache_home = g_build_filename(path, NULL);
    else
        p->cache_home = g_build_filename(g_get_home_dir(), ".cache", NULL);

    path = g_getenv("XDG_CONFIG_DIRS");
    if (path && path[0] != '\0') /* not unset or empty */
        p->config_dirs = split_paths(path);
    else {
        p->config_dirs = slist_path_add(p->config_dirs,
                                        g_strdup(CONFIGDIR),
                                        (GSListFunc) g_slist_append);
        p->config_dirs = slist_path_add(p->config_dirs,
                                        g_build_filename
                                        (G_DIR_SEPARATOR_S,
                                         "etc", "xdg", NULL),
                                        (GSListFunc) g_slist_append);
    }
    p->config_dirs = slist_path_add(p->config_dirs,
                                    g_strdup(p->config_home),
                                    (GSListFunc) g_slist_prepend);

    path = g_getenv("XDG_DATA_DIRS");
    if (path && path[0] != '\0') /* not unset or empty */
        p->data_dirs = split_paths(path);
    else {
        p->data_dirs = slist_path_add(p->data_dirs,
                                      g_strdup(DATADIR),
                                      (GSListFunc) g_slist_append);
        p->data_dirs = slist_path_add(p->data_dirs,
                                      g_build_filename
                                      (G_DIR_SEPARATOR_S,
                                       "usr", "local", "share", NULL),
                                      (GSListFunc) g_slist_append);
        p->data_dirs = slist_path_add(p->data_dirs,
                                      g_build_filename
                                      (G_DIR_SEPARATOR_S,
                                       "usr", "share", NULL),
                                      (GSListFunc) g_slist_append);
    }
    p->data_dirs = slist_path_add(p->data_dirs,
                                  g_strdup(p->data_home),
                                  (GSListFunc) g_slist_prepend);
    return p;
}

void obt_paths_ref(ObtPaths *p)
{
    ++p->ref;
}

void obt_paths_unref(ObtPaths *p)
{
    if (p && --p->ref == 0) {
        GSList *it;

        for (it = p->config_dirs; it; it = g_slist_next(it))
            g_free(it->data);
        g_slist_free(p->config_dirs);
        for (it = p->data_dirs; it; it = g_slist_next(it))
            g_free(it->data);
        g_slist_free(p->data_dirs);
        g_free(p->config_home);
        g_free(p->data_home);
        g_free(p->cache_home);

        g_slice_free(ObtPaths, p);
    }
}

gchar *obt_paths_expand_tilde(const gchar *f)
{
    gchar *ret;
    GRegex *regex;

    if (!f)
        return NULL;

    regex = g_regex_new("(?:^|(?<=[ \\t]))~(?=[/ \\t$])", G_REGEX_MULTILINE | G_REGEX_RAW, 0, NULL);
    ret = g_regex_replace_literal(regex, f, -1, 0, g_get_home_dir(), 0, NULL);
    g_regex_unref(regex);

    return ret;
}

gboolean obt_paths_mkdir(const gchar *path, gint mode)
{
    gboolean ret = TRUE;

    g_return_val_if_fail(path != NULL, FALSE);
    g_return_val_if_fail(path[0] != '\0', FALSE);

    if (!g_file_test(path, G_FILE_TEST_IS_DIR))
        if (mkdir(path, mode) == -1)
            ret = FALSE;

    return ret;
}

gboolean obt_paths_mkdir_path(const gchar *path, gint mode)
{
    gboolean ret = TRUE;

    g_return_val_if_fail(path != NULL, FALSE);
    g_return_val_if_fail(path[0] == '/', FALSE);

    if (!g_file_test(path, G_FILE_TEST_IS_DIR)) {
        gchar *c, *e;

        c = g_strdup(path);
        e = c;
        while ((e = strchr(e + 1, '/'))) {
            *e = '\0';
            if (!(ret = obt_paths_mkdir(c, mode)))
                goto parse_mkdir_path_end;
            *e = '/';
        }
        ret = obt_paths_mkdir(c, mode);

    parse_mkdir_path_end:
        g_free(c);
    }

    return ret;
}

const gchar* obt_paths_config_home(ObtPaths *p)
{
    return p->config_home;
}

const gchar* obt_paths_data_home(ObtPaths *p)
{
    return p->data_home;
}

const gchar* obt_paths_cache_home(ObtPaths *p)
{
    return p->cache_home;
}

GSList* obt_paths_config_dirs(ObtPaths *p)
{
    return p->config_dirs;
}

GSList* obt_paths_data_dirs(ObtPaths *p)
{
    return p->data_dirs;
}
