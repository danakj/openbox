#include "parse.h"
#include <glib.h>

struct Callback {
    char *tag;
    ParseCallback func;
    void *data;
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

void parse_register(ObParseInst *i, const char *tag,
                    ParseCallback func, void *data)
{
    struct Callback *c;

    if ((c = g_hash_table_lookup(i->callbacks, tag))) {
        g_warning("tag '%s' already registered", tag);
        return;
    }

    c = g_new(struct Callback, 1);
    c->tag = g_strdup(tag);
    c->func = func;
    c->data = data;
    g_hash_table_insert(i->callbacks, c->tag, c);
}

gboolean parse_load_rc(xmlDocPtr *doc, xmlNodePtr *root)
{
    char *path;
    gboolean r = FALSE;

    path = g_build_filename(g_get_home_dir(), ".openbox", "rc.xml", NULL);
    if (parse_load(path, "openbox_config", doc, root)) {
        r = TRUE;
    } else {
        g_free(path);
        path = g_build_filename(RCDIR, "rc.xml", NULL);
        if (parse_load(path, "openbox_config", doc, root)) {
            r = TRUE;
        }
    }
    g_free(path);
    if (!r)
        g_warning("unable to find a valid config file, using defaults");
    return r;
}

gboolean parse_load(const char *path, const char *rootname,
                    xmlDocPtr *doc, xmlNodePtr *root)
{
    if ((*doc = xmlParseFile(path))) {
        *root = xmlDocGetRootElement(*doc);
        if (!*root) {
            xmlFreeDoc(*doc);
            *doc = NULL;
            g_warning("%s is an empty document", path);
        } else {
            if (xmlStrcasecmp((*root)->name, (const xmlChar*)rootname)) {
                xmlFreeDoc(*doc);
                *doc = NULL;
                g_warning("document %s is of wrong type. root node is "
                          "not '%s'", path, rootname);
            }
        }
    }
    if (!*doc)
        return FALSE;
    return TRUE;
}

gboolean parse_load_mem(gpointer data, guint len, const char *rootname,
                        xmlDocPtr *doc, xmlNodePtr *root)
{
    if ((*doc = xmlParseMemory(data, len))) {
        *root = xmlDocGetRootElement(*doc);
        if (!*root) {
            xmlFreeDoc(*doc);
            *doc = NULL;
            g_warning("Given memory is an empty document");
        } else {
            if (xmlStrcasecmp((*root)->name, (const xmlChar*)rootname)) {
                xmlFreeDoc(*doc);
                *doc = NULL;
                g_warning("document in given memory is of wrong type. root "
                          "node is not '%s'", rootname);
            }
        }
    }
    if (!*doc)
        return FALSE;
    return TRUE;
}

void parse_close(xmlDocPtr doc)
{
    xmlFree(doc);
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

char *parse_string(xmlDocPtr doc, xmlNodePtr node)
{
    xmlChar *c = xmlNodeListGetString(doc, node->children, TRUE);
    char *s = g_strdup(c ? (char*)c : "");
    xmlFree(c);
    return s;
}

int parse_int(xmlDocPtr doc, xmlNodePtr node)
{
    xmlChar *c = xmlNodeListGetString(doc, node->children, TRUE);
    int i = atoi((char*)c);
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

gboolean parse_contains(const char *val, xmlDocPtr doc, xmlNodePtr node)
{
    xmlChar *c = xmlNodeListGetString(doc, node->children, TRUE);
    gboolean r;
    r = !xmlStrcasecmp(c, (const xmlChar*) val);
    xmlFree(c);
    return r;
}

xmlNodePtr parse_find_node(const char *tag, xmlNodePtr node)
{
    while (node) {
        if (!xmlStrcasecmp(node->name, (const xmlChar*) tag))
            return node;
        node = node->next;
    }
    return NULL;
}

gboolean parse_attr_int(const char *name, xmlNodePtr node, int *value)
{
    xmlChar *c = xmlGetProp(node, (const xmlChar*) name);
    gboolean r = FALSE;
    if (c) {
        *value = atoi((char*)c);
        r = TRUE;
    }
    xmlFree(c);
    return r;
}

gboolean parse_attr_string(const char *name, xmlNodePtr node, char **value)
{
    xmlChar *c = xmlGetProp(node, (const xmlChar*) name);
    gboolean r = FALSE;
    if (c) {
        *value = g_strdup((char*)c);
        r = TRUE;
    }
    xmlFree(c);
    return r;
}

gboolean parse_attr_contains(const char *val, xmlNodePtr node,
                             const char *name)
{
    xmlChar *c = xmlGetProp(node, (const xmlChar*) name);
    gboolean r;
    r = !xmlStrcasecmp(c, (const xmlChar*) val);
    xmlFree(c);
    return r;
}
