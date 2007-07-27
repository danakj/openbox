/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   obt/parse.c for the Openbox window manager
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

#include "obt/parse.h"

#include <glib.h>

#ifdef HAVE_STRING_H
#  include <string.h>
#endif
#ifdef HAVE_ERRNO_H
#  include <errno.h>
#endif
#ifdef HAVE_SYS_STAT_H
#  include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

static gboolean xdg_start;
static gchar   *xdg_config_home_path;
static gchar   *xdg_data_home_path;
static GSList  *xdg_config_dir_paths;
static GSList  *xdg_data_dir_paths;

struct Callback {
    gchar *tag;
    ObtParseCallback func;
    gpointer data;
};

struct _ObtParseInst {
    gint ref;
    GHashTable *callbacks;
    xmlDocPtr doc;
    xmlNodePtr root;
    gchar *path;
};

static void destfunc(struct Callback *c)
{
    g_free(c->tag);
    g_free(c);
}

ObtParseInst* obt_parse_instance_new()
{
    ObtParseInst *i = g_new(ObtParseInst, 1);
    i->ref = 1;
    i->callbacks = g_hash_table_new_full(g_str_hash, g_str_equal, NULL,
                                         (GDestroyNotify)destfunc);
    i->doc = NULL;
    i->root = NULL;
    i->path = NULL;
    return i;
}

void obt_parse_instance_ref(ObtParseInst *i)
{
    ++i->ref;
}

void obt_parse_instance_unref(ObtParseInst *i)
{
    if (i && --i->ref == 0) {
        g_hash_table_destroy(i->callbacks);
        g_free(i);
    }
}

void parse_register(ObtParseInst *i, const gchar *tag,
                    ObtParseCallback func, gpointer data)
{
    struct Callback *c;

    if ((c = g_hash_table_lookup(i->callbacks, tag))) {
        g_error("Tag '%s' already registered", tag);
        return;
    }

    c = g_new(struct Callback, 1);
    c->tag = g_strdup(tag);
    c->func = func;
    c->data = data;
    g_hash_table_insert(i->callbacks, c->tag, c);
}

static gboolean load_file(ObtParseInst *i,
                          const gchar *domain,
                          const gchar *filename,
                          const gchar *root_node,
                          GSList *paths)
{
    GSList *it;
    gboolean r = FALSE;

    g_assert(i->doc == NULL); /* another doc isn't open already? */

    for (it = paths; !r && it; it = g_slist_next(it)) {
        gchar *path;
        struct stat s;

        path = g_build_filename(it->data, domain, filename, NULL);

        if (stat(path, &s) >= 0) {
            /* XML_PARSE_BLANKS is needed apparently, or the tree can end up
               with extra nodes in it. */
            i->doc = xmlReadFile(path, NULL, (XML_PARSE_NOBLANKS |
                                              XML_PARSE_RECOVER));
            if (i->doc) {
                i->root = xmlDocGetRootElement(i->doc);
                if (!i->root) {
                    xmlFreeDoc(i->doc);
                    i->doc = NULL;
                    g_message("%s is an empty XML document", path);
                }
                else if (xmlStrcmp(i->root->name,
                                   (const xmlChar*)root_node)) {
                    xmlFreeDoc(i->doc);
                    i->doc = NULL;
                    i->root = NULL;
                    g_message("XML document %s is of wrong type. Root "
                              "node is not '%s'", path, root_node);
                }
                else {
                    i->path = g_strdup(path);
                    r = TRUE; /* ok! */
                }
            }
        }

        g_free(path);
    }

    return r;
}

gboolean obt_parse_load_config_file(ObtParseInst *i,
                                    const gchar *domain,
                                    const gchar *filename,
                                    const gchar *root_node)
{
    GSList *it, *paths = NULL;
    gboolean r;

    for (it = xdg_config_dir_paths; it; it = g_slist_next(it))
        paths = g_slist_append(paths, g_strdup(it->data));

    r = load_file(i, domain, filename, root_node, paths);

    while (paths) {
        g_free(paths->data);
        paths = g_slist_delete_link(paths, paths);
    }
    return r;
}

gboolean obt_parse_load_data_file(ObtParseInst *i,
                                  const gchar *domain,
                                  const gchar *filename,
                                  const gchar *root_node)
{
    GSList *it, *paths = NULL;
    gboolean r;

    for (it = xdg_data_dir_paths; it; it = g_slist_next(it))
        paths = g_slist_append(paths, g_strdup(it->data));

    r = load_file(i, domain, filename, root_node, paths);

    while (paths) {
        g_free(paths->data);
        paths = g_slist_delete_link(paths, paths);
    }
    return r;
}

gboolean obt_parse_load_theme_file(ObtParseInst *i,
                                   const gchar *theme,
                                   const gchar *domain,
                                   const gchar *filename,
                                   const gchar *root_node)
{
    GSList *it, *paths = NULL;
    gboolean r;

    /* use ~/.themes for backwards compatibility */
    paths = g_slist_append
        (paths, g_build_filename(g_get_home_dir(), ".themes", theme, NULL));

    for (it = xdg_data_dir_paths; it; it = g_slist_next(it))
        paths = g_slist_append
            (paths, g_build_filename(it->data, "themes", theme, NULL));

    r = load_file(i, domain, filename, root_node, paths);

    while (paths) {
        g_free(paths->data);
        paths = g_slist_delete_link(paths, paths);
    }
    return r;
}


gboolean obt_parse_load_mem(ObtParseInst *i,
                            gpointer data, guint len, const gchar *root_node)
{
    gboolean r = FALSE;

    g_assert(i->doc == NULL); /* another doc isn't open already? */

    i->doc = xmlParseMemory(data, len);
    if (i) {
        i->root = xmlDocGetRootElement(i->doc);
        if (!i->root) {
            xmlFreeDoc(i->doc);
            i->doc = NULL;
            g_message("Given memory is an empty document");
        }
        else if (xmlStrcmp(i->root->name, (const xmlChar*)root_node)) {
            xmlFreeDoc(i->doc);
            i->doc = NULL;
            i->root = NULL;
            g_message("XML Document in given memory is of wrong "
                      "type. Root node is not '%s'\n", root_node);
        }
        else
            r = TRUE; /* ok ! */
    }
    return r;
}

void obt_parse_close(ObtParseInst *i)
{
    if (i && i->doc) {
        xmlFreeDoc(i->doc);
        g_free(i->path);
        i->doc = NULL;
        i->root = NULL;
        i->path = NULL;
    }
}

void obt_parse_tree(ObtParseInst *i, xmlNodePtr node)
{
    g_assert(i->doc); /* a doc is open? */

    while (node) {
        struct Callback *c = g_hash_table_lookup(i->callbacks, node->name);
        if (c) c->func(i, i->doc, node, c->data);
        node = node->next;
    }
}

gchar *obt_parse_node_string(xmlNodePtr node)
{
    xmlChar *c = xmlNodeGetContent(node);
    gchar *s = g_strdup(c ? (gchar*)c : "");
    xmlFree(c);
    return s;
}

gint obt_parse_node_int(xmlNodePtr node)
{
    xmlChar *c = xmlNodeGetContent(node);
    gint i = c ? atoi((gchar*)c) : 0;
    xmlFree(c);
    return i;
}

gboolean obt_parse_node_bool(xmlNodePtr node)
{
    xmlChar *c = xmlNodeGetContent(node);
    gboolean b = FALSE;
    if (c && !xmlStrcasecmp(c, (const xmlChar*) "true"))
        b = TRUE;
    else if (c && !xmlStrcasecmp(c, (const xmlChar*) "yes"))
        b = TRUE;
    else if (c && !xmlStrcasecmp(c, (const xmlChar*) "on"))
        b = TRUE;
    xmlFree(c);
    return b;
}

gboolean obt_parse_node_contains(xmlNodePtr node, const gchar *val)
{
    xmlChar *c = xmlNodeGetContent(node);
    gboolean r;
    r = !xmlStrcasecmp(c, (const xmlChar*) val);
    xmlFree(c);
    return r;
}

xmlNodePtr obt_parse_find_node(xmlNodePtr node, const gchar *tag)
{
    while (node) {
        if (!xmlStrcmp(node->name, (const xmlChar*) tag))
            return node;
        node = node->next;
    }
    return NULL;
}

gboolean obt_parse_attr_bool(xmlNodePtr node, const gchar *name,
                             gboolean *value)
{
    xmlChar *c = xmlGetProp(node, (const xmlChar*) name);
    gboolean r = FALSE;
    if (c) {
        if (!xmlStrcasecmp(c, (const xmlChar*) "true"))
            *value = TRUE, r = TRUE;
        else if (!xmlStrcasecmp(c, (const xmlChar*) "yes"))
            *value = TRUE, r = TRUE;
        else if (!xmlStrcasecmp(c, (const xmlChar*) "on"))
            *value = TRUE, r = TRUE;
        else if (!xmlStrcasecmp(c, (const xmlChar*) "false"))
            *value = FALSE, r = TRUE;
        else if (!xmlStrcasecmp(c, (const xmlChar*) "no"))
            *value = FALSE, r = TRUE;
        else if (!xmlStrcasecmp(c, (const xmlChar*) "off"))
            *value = FALSE, r = TRUE;
    }
    xmlFree(c);
    return r;
}

gboolean obt_parse_attr_int(xmlNodePtr node, const gchar *name, gint *value)
{
    xmlChar *c = xmlGetProp(node, (const xmlChar*) name);
    gboolean r = FALSE;
    if (c) {
        *value = atoi((gchar*)c);
        r = TRUE;
    }
    xmlFree(c);
    return r;
}

gboolean obt_parse_attr_string(xmlNodePtr node, const gchar *name,
                               gchar **value)
{
    xmlChar *c = xmlGetProp(node, (const xmlChar*) name);
    gboolean r = FALSE;
    if (c) {
        *value = g_strdup((gchar*)c);
        r = TRUE;
    }
    xmlFree(c);
    return r;
}

gboolean obt_parse_attr_contains(xmlNodePtr node, const gchar *name,
                                 const gchar *val)
{
    xmlChar *c = xmlGetProp(node, (const xmlChar*) name);
    gboolean r = FALSE;
    if (c)
        r = !xmlStrcasecmp(c, (const xmlChar*) val);
    xmlFree(c);
    return r;
}

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

void parse_paths_startup()
{
    const gchar *path;

    if (xdg_start)
        return;
    xdg_start = TRUE;

    path = g_getenv("XDG_CONFIG_HOME");
    if (path && path[0] != '\0') /* not unset or empty */
        xdg_config_home_path = g_build_filename(path, NULL);
    else
        xdg_config_home_path = g_build_filename(g_get_home_dir(), ".config",
                                                NULL);

    path = g_getenv("XDG_DATA_HOME");
    if (path && path[0] != '\0') /* not unset or empty */
        xdg_data_home_path = g_build_filename(path, NULL);
    else
        xdg_data_home_path = g_build_filename(g_get_home_dir(), ".local",
                                              "share", NULL);

    path = g_getenv("XDG_CONFIG_DIRS");
    if (path && path[0] != '\0') /* not unset or empty */
        xdg_config_dir_paths = split_paths(path);
    else {
        xdg_config_dir_paths = slist_path_add(xdg_config_dir_paths,
                                              g_strdup(CONFIGDIR),
                                              (GSListFunc) g_slist_append);
        xdg_config_dir_paths = slist_path_add(xdg_config_dir_paths,
                                              g_build_filename
                                              (G_DIR_SEPARATOR_S,
                                               "etc", "xdg", NULL),
                                              (GSListFunc) g_slist_append);
    }
    xdg_config_dir_paths = slist_path_add(xdg_config_dir_paths,
                                          g_strdup(xdg_config_home_path),
                                          (GSListFunc) g_slist_prepend);

    path = g_getenv("XDG_DATA_DIRS");
    if (path && path[0] != '\0') /* not unset or empty */
        xdg_data_dir_paths = split_paths(path);
    else {
        xdg_data_dir_paths = slist_path_add(xdg_data_dir_paths,
                                            g_strdup(DATADIR),
                                            (GSListFunc) g_slist_append);
        xdg_data_dir_paths = slist_path_add(xdg_data_dir_paths,
                                            g_build_filename
                                            (G_DIR_SEPARATOR_S,
                                             "usr", "local", "share", NULL),
                                            (GSListFunc) g_slist_append);
        xdg_data_dir_paths = slist_path_add(xdg_data_dir_paths,
                                            g_build_filename
                                            (G_DIR_SEPARATOR_S,
                                             "usr", "share", NULL),
                                            (GSListFunc) g_slist_append);
    }
    xdg_data_dir_paths = slist_path_add(xdg_data_dir_paths,
                                        g_strdup(xdg_data_home_path),
                                        (GSListFunc) g_slist_prepend);
}

void parse_paths_shutdown()
{
    GSList *it;

    if (!xdg_start)
        return;
    xdg_start = FALSE;

    for (it = xdg_config_dir_paths; it; it = g_slist_next(it))
        g_free(it->data);
    g_slist_free(xdg_config_dir_paths);
    xdg_config_dir_paths = NULL;
    for (it = xdg_data_dir_paths; it; it = g_slist_next(it))
        g_free(it->data);
    g_slist_free(xdg_data_dir_paths);
    xdg_data_dir_paths = NULL;
    g_free(xdg_config_home_path);
    xdg_config_home_path = NULL;
    g_free(xdg_data_home_path);
    xdg_data_home_path = NULL;
}

gchar *parse_expand_tilde(const gchar *f)
{
    gchar **spl;
    gchar *ret;

    if (!f)
        return NULL;
    spl = g_strsplit(f, "~", 0);
    ret = g_strjoinv(g_get_home_dir(), spl);
    g_strfreev(spl);
    return ret;
}

gboolean parse_mkdir(const gchar *path, gint mode)
{
    gboolean ret = TRUE;

    g_return_val_if_fail(path != NULL, FALSE);
    g_return_val_if_fail(path[0] != '\0', FALSE);

    if (!g_file_test(path, G_FILE_TEST_IS_DIR))
        if (mkdir(path, mode) == -1)
            ret = FALSE;

    return ret;
}

gboolean parse_mkdir_path(const gchar *path, gint mode)
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
            if (!(ret = parse_mkdir(c, mode)))
                goto parse_mkdir_path_end;
            *e = '/';
        }
        ret = parse_mkdir(c, mode);

    parse_mkdir_path_end:
        g_free(c);
    }

    return ret;
}

const gchar* parse_xdg_config_home_path()
{
    return xdg_config_home_path;
}

const gchar* parse_xdg_data_home_path()
{
    return xdg_data_home_path;
}

GSList* parse_xdg_config_dir_paths()
{
    return xdg_config_dir_paths;
}

GSList* parse_xdg_data_dir_paths()
{
    return xdg_data_dir_paths;
}
