#include "parse.h"
#include <glib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

static gboolean xdg_start;
static gchar   *xdg_config_home_path;
static gchar   *xdg_data_home_path;
static GSList  *xdg_config_dir_paths;
static GSList  *xdg_data_dir_paths;

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
    GSList *it;
    gchar *path;
    gboolean r = FALSE;

    for (it = xdg_config_dir_paths; !r && it; it = g_slist_next(it)) {
        path = g_build_filename(it->data, "openbox", "rc.xml", NULL);
        r = parse_load(path, "openbox_config", doc, root);
        g_free(path);
    }
    if (!r)
        g_warning("unable to find a valid config file, using defaults");
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
    if (!r)
        g_warning("unable to find a valid menu file '%s'", file);
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

static GSList* split_paths(const gchar *paths)
{
    GSList *list = NULL;
    gchar *c, *e, *s;

    c = g_strdup(paths);
    s = c;
    e = c - 1;
    g_message("paths %s", paths);
    while ((e = strchr(e + 1, ':'))) {
        *e = '\0';
        g_message("s %s", s);
        if (s[0] != '\0')
            list = g_slist_append(list, g_strdup(s));
        s = e + 1;
    }
    if (s[0] != '\0')
        list = g_slist_append(list, g_strdup(s));
    g_free(c);
    return list;
}

void parse_paths_startup()
{
    gchar *path;

    if (xdg_start)
        return;
    xdg_start = TRUE;

    path = getenv("XDG_CONFIG_HOME");
    if (path && path[0] != '\0') /* not unset or empty */
        xdg_config_home_path = g_build_filename(path, NULL);
    else
        xdg_config_home_path = g_build_filename(g_get_home_dir(), ".config",
                                                NULL);

    path = getenv("XDG_DATA_HOME");
    if (path && path[0] != '\0') /* not unset or empty */
        xdg_data_home_path = g_build_filename(path, NULL);
    else
        xdg_data_home_path = g_build_filename(g_get_home_dir(), ".local",
                                              "share", NULL);

    path = getenv("XDG_CONFIG_DIRS");
    if (path && path[0] != '\0') /* not unset or empty */
        xdg_config_dir_paths = split_paths(path);
    else {
        xdg_config_dir_paths = g_slist_append(xdg_config_dir_paths,
                                              g_build_filename
                                              (G_DIR_SEPARATOR_S,
                                               "etc", "xdg", NULL));
        xdg_config_dir_paths = g_slist_append(xdg_config_dir_paths,
                                              g_strdup(CONFIGDIR));
    }
    xdg_config_dir_paths = g_slist_prepend(xdg_config_dir_paths,
                                           xdg_config_home_path);
    
    path = getenv("XDG_DATA_DIRS");
    if (path && path[0] != '\0') /* not unset or empty */
        xdg_data_dir_paths = split_paths(path);
    else {
        xdg_data_dir_paths = g_slist_append(xdg_data_dir_paths,
                                            g_build_filename
                                            (G_DIR_SEPARATOR_S,
                                             "usr", "local", "share", NULL));
        xdg_data_dir_paths = g_slist_append(xdg_data_dir_paths,
                                            g_build_filename
                                            (G_DIR_SEPARATOR_S,
                                             "usr", "share", NULL));
        xdg_config_dir_paths = g_slist_append(xdg_config_dir_paths,
                                              g_strdup(DATADIR));
    }
    xdg_data_dir_paths = g_slist_prepend(xdg_data_dir_paths,
                                         xdg_data_home_path);
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

void parse_mkdir_path(const gchar *path, gint mode)
{
    gchar *c, *e;

    g_assert(path[0] == '/');

    c = g_strdup(path);
    e = c;
    while ((e = strchr(e + 1, '/'))) {
        *e = '\0';
        mkdir(c, mode);
        *e = '/';
    }
    mkdir(c, mode);
    g_free(c);
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
