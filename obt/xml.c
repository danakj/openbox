/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   obt/xml.c for the Openbox window manager
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

#include "obt/xml.h"
#include "obt/paths.h"

#include <libxml/xinclude.h>
#include <glib.h>

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
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

struct Callback {
    gchar *tag;
    ObtXmlCallback func;
    gpointer data;
};

struct _ObtXmlInst {
    gint ref;
    ObtPaths *xdg_paths;
    GHashTable *callbacks;
    xmlDocPtr doc;
    xmlNodePtr root;
    gchar *path;
};

static void destfunc(struct Callback *c)
{
    g_free(c->tag);
    g_slice_free(struct Callback, c);
}

ObtXmlInst* obt_xml_instance_new(void)
{
    ObtXmlInst *i = g_slice_new(ObtXmlInst);
    i->ref = 1;
    i->xdg_paths = obt_paths_new();
    i->callbacks = g_hash_table_new_full(g_str_hash, g_str_equal, NULL,
                                         (GDestroyNotify)destfunc);
    i->doc = NULL;
    i->root = NULL;
    i->path = NULL;
    return i;
}

void obt_xml_instance_ref(ObtXmlInst *i)
{
    ++i->ref;
}

void obt_xml_instance_unref(ObtXmlInst *i)
{
    if (i && --i->ref == 0) {
        obt_paths_unref(i->xdg_paths);
        g_hash_table_destroy(i->callbacks);
        g_slice_free(ObtXmlInst, i);
    }
}

xmlDocPtr obt_xml_doc(ObtXmlInst *i)
{
    g_assert(i->doc); /* a doc is open? */
    return i->doc;
}

xmlNodePtr obt_xml_root(ObtXmlInst *i)
{
    g_assert(i->doc); /* a doc is open? */
    return i->root;
}

void obt_xml_register(ObtXmlInst *i, const gchar *tag,
                      ObtXmlCallback func, gpointer data)
{
    struct Callback *c;

    if (g_hash_table_lookup(i->callbacks, tag)) {
        g_error("Tag '%s' already registered", tag);
        return;
    }

    c = g_slice_new(struct Callback);
    c->tag = g_strdup(tag);
    c->func = func;
    c->data = data;
    g_hash_table_insert(i->callbacks, c->tag, c);
}

void obt_xml_unregister(ObtXmlInst *i, const gchar *tag)
{
    g_hash_table_remove(i->callbacks, tag);
}

void obt_xml_new_file(ObtXmlInst *i,
                      const gchar *root_node)
{
    xmlNodePtr old;

    g_assert(i->doc == NULL); /* another doc isn't open already? */

    i->doc = xmlNewDoc((xmlChar*)"1.0");
    i->root = xmlNewDocRawNode(i->doc, NULL, (xmlChar*)root_node, NULL);
    old = xmlDocSetRootElement(i->doc, i->root);
    if (old) xmlFreeNode(old);
}

static gboolean load_file(ObtXmlInst *i,
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

        if (!domain && !filename) /* given a full path to the file */
            path = g_strdup(it->data);
        else
            path = g_build_filename(it->data, domain, filename, NULL);

        if (stat(path, &s) >= 0) {
            /* XML_PARSE_BLANKS is needed apparently, or the tree can end up
               with extra nodes in it. */
            i->doc = xmlReadFile(path, NULL, (XML_PARSE_NOBLANKS |
                                              XML_PARSE_RECOVER));
            xmlXIncludeProcessFlags(i->doc, (XML_PARSE_NOBLANKS |
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

gboolean obt_xml_load_file(ObtXmlInst *i,
                           const gchar *path,
                           const gchar *root_node)
{
    GSList *paths;
    gboolean r;

    paths = g_slist_append(NULL, g_strdup(path));

    r = load_file(i, NULL, NULL, root_node, paths);

    while (paths) {
        g_free(paths->data);
        paths = g_slist_delete_link(paths, paths);
    }
    return r;
}

gboolean obt_xml_load_cache_file(ObtXmlInst *i,
                                 const gchar *domain,
                                 const gchar *filename,
                                 const gchar *root_node)
{
    GSList *paths = NULL;
    gboolean r;

    paths = g_slist_append(paths,
                           g_strdup(obt_paths_cache_home(i->xdg_paths)));

    r = load_file(i, domain, filename, root_node, paths);

    while (paths) {
        g_free(paths->data);
        paths = g_slist_delete_link(paths, paths);
    }
    return r;
}

gboolean obt_xml_load_config_file(ObtXmlInst *i,
                                  const gchar *domain,
                                  const gchar *filename,
                                  const gchar *root_node)
{
    GSList *it, *paths = NULL;
    gboolean r;

    for (it = obt_paths_config_dirs(i->xdg_paths); it; it = g_slist_next(it))
        paths = g_slist_append(paths, g_strdup(it->data));

    r = load_file(i, domain, filename, root_node, paths);

    while (paths) {
        g_free(paths->data);
        paths = g_slist_delete_link(paths, paths);
    }
    return r;
}

gboolean obt_xml_load_data_file(ObtXmlInst *i,
                                const gchar *domain,
                                const gchar *filename,
                                const gchar *root_node)
{
    GSList *it, *paths = NULL;
    gboolean r;

    for (it = obt_paths_data_dirs(i->xdg_paths); it; it = g_slist_next(it))
        paths = g_slist_append(paths, g_strdup(it->data));

    r = load_file(i, domain, filename, root_node, paths);

    while (paths) {
        g_free(paths->data);
        paths = g_slist_delete_link(paths, paths);
    }
    return r;
}

gboolean obt_xml_load_theme_file(ObtXmlInst *i,
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

    for (it = obt_paths_data_dirs(i->xdg_paths); it; it = g_slist_next(it))
        paths = g_slist_append
            (paths, g_build_filename(it->data, "themes", theme, NULL));

    r = load_file(i, domain, filename, root_node, paths);

    while (paths) {
        g_free(paths->data);
        paths = g_slist_delete_link(paths, paths);
    }
    return r;
}


gboolean obt_xml_load_mem(ObtXmlInst *i,
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

gboolean obt_xml_save_file(ObtXmlInst *inst,
                           const gchar *path,
                           gboolean pretty)
{
    return xmlSaveFormatFile(path, inst->doc, pretty) != -1;
}

gboolean obt_xml_save_cache_file(ObtXmlInst *inst,
                                 const gchar *domain,
                                 const gchar *filename,
                                 gboolean pretty)
{
    gchar *dpath, *fpath;
    gboolean ok;

    dpath = g_build_filename(obt_paths_cache_home(inst->xdg_paths),
                             domain, NULL);
    fpath = g_build_filename(dpath, filename, NULL);

    ok = obt_paths_mkdir_path(dpath, 0700);
    ok = ok && obt_xml_save_file(inst, fpath, pretty);

    g_free(fpath);
    g_free(dpath);
    return ok;
}

const gchar* obt_xml_file_path(ObtXmlInst *inst)
{
    return inst->path;
}

void obt_xml_close(ObtXmlInst *i)
{
    if (i && i->doc) {
        xmlFreeDoc(i->doc);
        g_free(i->path);
        i->doc = NULL;
        i->root = NULL;
        i->path = NULL;
    }
}

void obt_xml_tree(ObtXmlInst *i, xmlNodePtr node)
{
    g_assert(i->doc); /* a doc is open? */

    while (node) {
        if (node->name) {
            struct Callback *c = g_hash_table_lookup(i->callbacks, node->name);
            if (c) c->func(node, c->data);
        }
        node = node->next;
    }
}

void obt_xml_tree_from_root(ObtXmlInst *i)
{
    obt_xml_tree(i, i->root->children);
}

guint obt_xml_node_line(xmlNodePtr node)
{
    return XML_GET_LINE(node);
}

gchar *obt_xml_node_string(xmlNodePtr node)
{
    xmlChar *c;
    gchar *s;
    c = xmlNodeIsText(node->children) ? xmlNodeGetContent(node) : NULL;
    if (c) g_strstrip((char*)c); /* strip leading/trailing whitespace */
    s = g_strdup(c ? (gchar*)c : "");
    xmlFree(c);
    return s;
}

gchar *obt_xml_node_string_raw(xmlNodePtr node)
{
    xmlChar *c;
    gchar *s;
    c = xmlNodeIsText(node->children) ? xmlNodeGetContent(node) : NULL;
    s = g_strdup(c ? (gchar*)c : "");
    xmlFree(c);
    return s;
}

gint obt_xml_node_int(xmlNodePtr node)
{
    xmlChar *c;
    gint i;
    c = xmlNodeIsText(node->children) ? xmlNodeGetContent(node) : NULL;
    if (c) g_strstrip((char*)c); /* strip leading/trailing whitespace */
    i = c ? atoi((gchar*)c) : 0;
    xmlFree(c);
    return i;
}

gboolean obt_xml_node_bool(xmlNodePtr node)
{
    xmlChar *c;
    gboolean b = FALSE;
    c = xmlNodeIsText(node->children) ? xmlNodeGetContent(node) : NULL;
    if (c) g_strstrip((char*)c); /* strip leading/trailing whitespace */
    if (c && !xmlStrcasecmp(c, (const xmlChar*) "true"))
        b = TRUE;
    else if (c && !xmlStrcasecmp(c, (const xmlChar*) "yes"))
        b = TRUE;
    else if (c && !xmlStrcasecmp(c, (const xmlChar*) "on"))
        b = TRUE;
    xmlFree(c);
    return b;
}

gboolean obt_xml_node_contains(xmlNodePtr node, const gchar *val)
{
    xmlChar *c;
    gboolean r;
    c = xmlNodeIsText(node->children) ? xmlNodeGetContent(node) : NULL;
    if (c) g_strstrip((char*)c); /* strip leading/trailing whitespace */
    r = !xmlStrcasecmp(c, (const xmlChar*) val);
    xmlFree(c);
    return r;
}

xmlNodePtr obt_xml_find_sibling(xmlNodePtr node, const gchar *tag)
{
    while (node) {
        if (!xmlStrcmp(node->name, (const xmlChar*) tag))
            return node;
        node = node->next;
    }
    return NULL;
}

void obt_xml_node_set_string(xmlNodePtr node, const gchar *s)
{
    xmlNodeSetContent(node, (const xmlChar*)s);
}

void obt_xml_node_set_int(xmlNodePtr node, gint i)
{
    gchar *s = g_strdup_printf("%d", i);
    obt_xml_node_set_string(node, s);
    g_free(s);
}

void obt_xml_node_set_bool(xmlNodePtr node, gboolean b)
{
    obt_xml_node_set_string(node, b ? "yes" : "no");
}

gboolean obt_xml_attr_bool(xmlNodePtr node, const gchar *name,
                           gboolean *value)
{
    xmlChar *c = xmlGetProp(node, (const xmlChar*) name);
    gboolean r = FALSE;
    if (c) {
        g_strstrip((char*)c); /* strip leading/trailing whitespace */
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

gboolean obt_xml_attr_int(xmlNodePtr node, const gchar *name, gint *value)
{
    xmlChar *c = xmlGetProp(node, (const xmlChar*) name);
    gboolean r = FALSE;
    if (c) {
        g_strstrip((char*)c); /* strip leading/trailing whitespace */
        *value = atoi((gchar*)c);
        r = TRUE;
    }
    xmlFree(c);
    return r;
}

gboolean obt_xml_attr_string(xmlNodePtr node, const gchar *name,
                             gchar **value)
{
    xmlChar *c = xmlGetProp(node, (const xmlChar*) name);
    gboolean r = FALSE;
    if (c) {
        g_strstrip((char*)c); /* strip leading/trailing whitespace */
        *value = g_strdup((gchar*)c);
        r = TRUE;
    }
    xmlFree(c);
    return r;
}

gboolean obt_xml_attr_contains(xmlNodePtr node, const gchar *name,
                               const gchar *val)
{
    xmlChar *c = xmlGetProp(node, (const xmlChar*) name);
    gboolean r = FALSE;
    if (c) {
        g_strstrip((char*)c); /* strip leading/trailing whitespace */
        r = !xmlStrcasecmp(c, (const xmlChar*) val);
    }
    xmlFree(c);
    return r;
}

/*! Finds a sibling which matches a name and attributes.
  @first The first sibling in the list.
  @attrs NULL-terminated array of strings.  The first string is the name of
    the node.  The remaining values are key,value pairs for attributes the
    node should have.
*/
static xmlNodePtr find_sibling(xmlNodePtr first, gchar **attrs)
{
    xmlNodePtr c;
    gboolean ok;

    ok = FALSE;
    c = obt_xml_find_sibling(first, attrs[0]);
    while (c && !ok) {
        gint i;

        ok = TRUE;
        for (i = 1; attrs[i]; ++i) {
            gchar **eq = g_strsplit(attrs[i], "=", 2);
            if (eq[1] && !obt_xml_attr_contains(c, eq[0], eq[1]))
                ok = FALSE;
            g_strfreev(eq);
        }
        if (!ok)
            c = obt_xml_find_sibling(c->next, attrs[0]);
    }
    return ok ? c : NULL;
}

static xmlNodePtr create_child(xmlNodePtr parent, gchar *const*attrs,
                               const gchar *value)
{
    xmlNodePtr c;
    gint i;

    c = xmlNewTextChild(parent, NULL, (xmlChar*)attrs[0], (xmlChar*)value);

    for (i = 1; attrs[i]; ++i) {
        gchar **eq = g_strsplit(attrs[i], "=", 2);
        if (eq[1])
            xmlNewProp(c, (xmlChar*)eq[0], (xmlChar*)eq[1]);
        g_strfreev(eq);
    }
    return c;
}

xmlNodePtr obt_xml_path_get_node(xmlNodePtr subtree, const gchar *path,
                                 const gchar *default_value)
{
    xmlNodePtr n, c;
    gchar **nodes;
    gchar **it, **next;

    g_return_val_if_fail(subtree != NULL, NULL);

    n = subtree;

    nodes = g_strsplit(path, "/", 0);
    for (it = nodes; *it; it = next) {
        gchar **attrs;

        attrs = g_strsplit(*it, ":", 0);
        next = it + 1;

        /* match attributes */
        c = find_sibling(n->children, attrs);

        if (!c) {
            if (*next)
                c = create_child(n, attrs, NULL);
            else if (default_value)
                c = create_child(n, attrs, default_value);
        }
        n = c;

        g_strfreev(attrs);
    }

    g_strfreev(nodes);

    return n;
}

GList* obt_xml_path_get_list(xmlNodePtr subtree, const gchar *path)
{
    xmlNodePtr n, c;
    gchar **nodes;
    gchar **it, **next;
    GList *list;

    g_return_val_if_fail(subtree != NULL, NULL);

    n = subtree;
    list = NULL;

    nodes = g_strsplit(path, "/", 0);
    if (nodes[0]) {
        gchar **attrs;

        for (it = nodes, next = it + 1; *next; it = next) {
            attrs = g_strsplit(*it, ":", 0);
            next = it + 1;

            /* match attributes */
            c = find_sibling(n->children, attrs);
            if (!c) c = create_child(n, attrs, NULL);
            n = c;

            g_strfreev(attrs);
        }

        attrs = g_strsplit(*next, ":", 0);
        c = n->children;
        while (c && (c = find_sibling(c, attrs))) {
            list = g_list_append(list, c);
            c = c->next;
        }
        g_strfreev(attrs);
    }

    g_strfreev(nodes);

    return list;
}


void obt_xml_path_delete_node(xmlNodePtr subtree, const gchar *path)
{
    xmlNodePtr n;

    n = obt_xml_path_get_node(subtree, path, NULL);
    xmlUnlinkNode(n);
    xmlFreeNode(n);
}

gchar* obt_xml_path_string(xmlNodePtr subtree, const gchar *path,
                          const gchar *default_value)
{
    xmlNodePtr n;

    n = obt_xml_path_get_node(subtree, path, default_value);
    return n ? obt_xml_node_string(n) : NULL;
}

int obt_xml_path_int(xmlNodePtr subtree, const gchar *path,
                     const gchar *default_value)
{
    xmlNodePtr n;

    n = obt_xml_path_get_node(subtree, path, default_value);
    return n ? obt_xml_node_int(n) : 0;
}

gboolean obt_xml_path_bool(xmlNodePtr subtree, const gchar *path,
                           const gchar *default_value)
{
    xmlNodePtr n;

    n = obt_xml_path_get_node(subtree, path, default_value);
    return n ? obt_xml_node_bool(n) : FALSE;
}

void obt_xml_path_set_string(xmlNodePtr subtree, const gchar *path,
                             const gchar *value)
{
    xmlNodePtr n = obt_xml_path_get_node(subtree, path, "");
    obt_xml_node_set_string(n, value);
}

void obt_xml_path_set_int(xmlNodePtr subtree, const gchar *path,
                          gint value)
{
    xmlNodePtr n = obt_xml_path_get_node(subtree, path, "");
    obt_xml_node_set_int(n, value);
}

void obt_xml_path_set_bool(xmlNodePtr subtree, const gchar *path,
                           gboolean value)
{
    xmlNodePtr n = obt_xml_path_get_node(subtree, path, "");
    obt_xml_node_set_bool(n, value);
}
