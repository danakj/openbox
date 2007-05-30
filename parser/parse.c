/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   parse.c for the Openbox window manager
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

#include "parse.h"
#include <glib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static gboolean xdg_start;
static gchar   *xdg_config_home_path;
static gchar   *xdg_data_home_path;
static GSList  *xdg_config_dir_paths;
static GSList  *xdg_data_dir_paths;

struct Callback {
    gchar *tag;
    ParseCallback func;
    gpointer data;
};

struct _ObParseInst {
    GHashTable *callbacks;
};

static void destfunc(struct Callback *c)
{
    g_free(c->tag);
    g_free(c);
}

ObParseInst* parse_startup()
{
    ObParseInst *i = g_new(ObParseInst, 1);
    i->callbacks = g_hash_table_new_full(g_str_hash, g_str_equal, NULL,
                                         (GDestroyNotify)destfunc);
    return i;
}

void parse_shutdown(ObParseInst *i)
{
    if (i) {
        g_hash_table_destroy(i->callbacks);
        g_free(i);
    }
}

void parse_register(ObParseInst *i, const gchar *tag,
                    ParseCallback func, gpointer data)
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

gboolean parse_load_rc(const gchar *type, xmlDocPtr *doc, xmlNodePtr *root)
{
    GSList *it;
    gboolean r = FALSE;
    gchar *fname;

    if (type == NULL)
        fname = g_strdup("rc.xml");
    else
        fname = g_strdup_printf("rc-%s.xml", type);

    for (it = xdg_config_dir_paths; !r && it; it = g_slist_next(it)) {
        gchar *path;

        path = g_build_filename(it->data, "openbox", fname, NULL);
        r = parse_load(path, "openbox_config", doc, root);
        g_free(path);
    }
    g_free(fname);

    return r;
}

gboolean parse_load_theme(const gchar *name, xmlDocPtr *doc, xmlNodePtr *root,
                          gchar **retpath)
{
    GSList *it;
    gchar *path;
    gboolean r = FALSE;
    gchar *eng;

    /* backward compatibility.. */
    path = g_build_filename(g_get_home_dir(), ".themes", name,
                            "openbox-3", "themerc.xml", NULL);
    if (parse_load(path, "openbox_theme", doc, root) &&
        parse_attr_string("engine", *root, &eng))
    {
        if (!strcmp(eng, "box")) {
            *retpath = g_path_get_dirname(path);
            r = TRUE;
        }
        g_free(eng);
    }
    g_free(path);

    if (!r) {
        for (it = xdg_data_dir_paths; !r && it; it = g_slist_next(it)) {
            path = g_build_filename(it->data, "themes", name, "openbox-3",
                                    "themerc.xml", NULL);
            if (parse_load(path, "openbox_theme", doc, root) &&
                parse_attr_string("engine", *root, &eng))
            {
                if (!strcmp(eng, "box")) {
                    *retpath = g_path_get_dirname(path);
                    r = TRUE;
                }
                g_free(eng);
            }
            g_free(path);
        }
    }
    return r;
}

gboolean parse_load_menu(const gchar *file, xmlDocPtr *doc, xmlNodePtr *root)
{
    GSList *it;
    gchar *path;
    gboolean r = FALSE;

    if (file[0] == '/') {
        r = parse_load(file, "openbox_menu", doc, root);
    } else {
        for (it = xdg_config_dir_paths; !r && it; it = g_slist_next(it)) {
            path = g_build_filename(it->data, "openbox", file, NULL);
            r = parse_load(path, "openbox_menu", doc, root);
            g_free(path);
        }
    }
    return r;
}

gboolean parse_load(const gchar *path, const gchar *rootname,
                    xmlDocPtr *doc, xmlNodePtr *root)
{
    struct stat s;

    g_print("Trying to load file %s for %s\n", path, rootname);

    if (stat(path, &s) < 0)
        return FALSE;

    /* XML_PARSE_BLANKS is needed apparently. When it loads a theme file,
       without this option, the tree is weird and has extra nodes in it. */
    if ((*doc = xmlReadFile(path, NULL,
                            XML_PARSE_NOBLANKS | XML_PARSE_RECOVER))) {
        *root = xmlDocGetRootElement(*doc);
        if (!*root) {
            xmlFreeDoc(*doc);
            *doc = NULL;
            g_message("%s is an empty document", path);
        } else {
            if (xmlStrcmp((*root)->name, (const xmlChar*)rootname)) {
                xmlFreeDoc(*doc);
                *doc = NULL;
                g_message("XML Document %s is of wrong type. Root "
                          "node is not '%s'", path, rootname);
            }
        }
    }
    if (!*doc)
        return FALSE;
    return TRUE;
}

gboolean parse_load_mem(gpointer data, guint len, const gchar *rootname,
                        xmlDocPtr *doc, xmlNodePtr *root)
{
    if ((*doc = xmlParseMemory(data, len))) {
        *root = xmlDocGetRootElement(*doc);
        if (!*root) {
            xmlFreeDoc(*doc);
            *doc = NULL;
            g_message("Given memory is an empty document");
        } else {
            if (xmlStrcmp((*root)->name, (const xmlChar*)rootname)) {
                xmlFreeDoc(*doc);
                *doc = NULL;
                g_message("XML Document in given memory is of wrong "
                          "type. Root node is not '%s'\n", rootname);
            }
        }
    }
    if (!*doc)
        return FALSE;
    return TRUE;
}

void parse_close(xmlDocPtr doc)
{
    xmlFreeDoc(doc);
}

void parse_tree(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node)
{
    while (node) {
        struct Callback *c = g_hash_table_lookup(i->callbacks, node->name);

        if (c)
            c->func(i, doc, node, c->data);

        node = node->next;
    }
}

gchar *parse_string(xmlDocPtr doc, xmlNodePtr node)
{
    xmlChar *c = xmlNodeListGetString(doc, node->children, TRUE);
    gchar *s = g_strdup(c ? (gchar*)c : "");
    xmlFree(c);
    return s;
}

gint parse_int(xmlDocPtr doc, xmlNodePtr node)
{
    xmlChar *c = xmlNodeListGetString(doc, node->children, TRUE);
    gint i = atoi((gchar*)c);
    xmlFree(c);
    return i;
}

gboolean parse_bool(xmlDocPtr doc, xmlNodePtr node)
{
    xmlChar *c = xmlNodeListGetString(doc, node->children, TRUE);
    gboolean b = FALSE;
    if (!xmlStrcasecmp(c, (const xmlChar*) "true"))
        b = TRUE;
    else if (!xmlStrcasecmp(c, (const xmlChar*) "yes"))
        b = TRUE;
    else if (!xmlStrcasecmp(c, (const xmlChar*) "on"))
        b = TRUE;
    xmlFree(c);
    return b;
}

gboolean parse_contains(const gchar *val, xmlDocPtr doc, xmlNodePtr node)
{
    xmlChar *c = xmlNodeListGetString(doc, node->children, TRUE);
    gboolean r;
    r = !xmlStrcasecmp(c, (const xmlChar*) val);
    xmlFree(c);
    return r;
}

xmlNodePtr parse_find_node(const gchar *tag, xmlNodePtr node)
{
    while (node) {
        if (!xmlStrcmp(node->name, (const xmlChar*) tag))
            return node;
        node = node->next;
    }
    return NULL;
}

gboolean parse_attr_bool(const gchar *name, xmlNodePtr node, gboolean *value)
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

gboolean parse_attr_int(const gchar *name, xmlNodePtr node, gint *value)
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

gboolean parse_attr_string(const gchar *name, xmlNodePtr node, gchar **value)
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

gboolean parse_attr_contains(const gchar *val, xmlNodePtr node,
                             const gchar *name)
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
