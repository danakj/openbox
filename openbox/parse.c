#include "parse.h"
#include <glib.h>

struct Callback {
    char *tag;
    ParseCallback func;
    void *data;
};

static GHashTable *callbacks;
static xmlDocPtr doc_config = NULL;

static void destfunc(struct Callback *c)
{
    g_free(c->tag);
    g_free(c);
}

void parse_startup()
{
    callbacks = g_hash_table_new_full(g_str_hash, g_str_equal, NULL,
                                      (GDestroyNotify)destfunc);
}

void parse_shutdown()
{
    xmlFree(doc_config);
    doc_config = NULL;

    g_hash_table_destroy(callbacks);
}

void parse_register(const char *tag, ParseCallback func, void *data)
{
    struct Callback *c;

    if ((c = g_hash_table_lookup(callbacks, tag))) {
        g_warning("tag '%s' already registered", tag);
        return;
    }

    c = g_new(struct Callback, 1);
    c->tag = g_strdup(tag);
    c->func = func;
    c->data = data;
    g_hash_table_insert(callbacks, c->tag, c);
}

void parse_config()
{
    char *path;
    xmlNodePtr node = NULL;

    xmlLineNumbersDefault(1);

    path = g_build_filename(g_get_home_dir(), ".openbox", "rc3", NULL);
    if ((doc_config = xmlParseFile(path))) {
        node = xmlDocGetRootElement(doc_config);
        if (!node) {
            xmlFreeDoc(doc_config);
            doc_config = NULL;
            g_warning("%s is an empty document", path);
        } else {
            if (xmlStrcasecmp(node->name, (const xmlChar*)"openbox_config")) {
                xmlFreeDoc(doc_config);
                doc_config = NULL;
                g_warning("document %s is of wrong type. root node is "
                          "not 'openbox_config'", path);
            }
        }
    }
    g_free(path);
    if (!doc_config) {
        path = g_build_filename(RCDIR, "rc3", NULL);
        if ((doc_config = xmlParseFile(path))) {
            node = xmlDocGetRootElement(doc_config);
            if (!node) {
                xmlFreeDoc(doc_config);
                doc_config = NULL;
                g_warning("%s is an empty document", path);
            } else {
                if (xmlStrcasecmp(node->name,
                                  (const xmlChar*)"openbox_config")) {
                    xmlFreeDoc(doc_config);
                    doc_config = NULL;
                    g_warning("document %s is of wrong type. root node is "
                              "not 'openbox_config'", path);
                }
            }
        }
        g_free(path);
    }
    if (!doc_config) {
        g_message("unable to find a valid config file, using defaults");
    } else {
        parse_tree(doc_config, node->xmlChildrenNode, NULL);
    }
}

void parse_tree(xmlDocPtr doc, xmlNodePtr node, void *nothing)
{
    while (node) {
        struct Callback *c = g_hash_table_lookup(callbacks, node->name);

        if (c)
            c->func(doc, node->xmlChildrenNode, c->data);

        node = node->next;
    }
}

char *parse_string(xmlDocPtr doc, xmlNodePtr node)
{
    xmlChar *c = xmlNodeListGetString(doc, node->xmlChildrenNode, TRUE);
    char *s = g_strdup((char*)c);
    xmlFree(c);
    return s;
}

int parse_int(xmlDocPtr doc, xmlNodePtr node)
{
    xmlChar *c = xmlNodeListGetString(doc, node->xmlChildrenNode, TRUE);
    int i = atoi((char*)c);
    xmlFree(c);
    return i;
}

gboolean parse_bool(xmlDocPtr doc, xmlNodePtr node)
{
    xmlChar *c = xmlNodeListGetString(doc, node->xmlChildrenNode, TRUE);
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
    xmlChar *c = xmlNodeListGetString(doc, node->xmlChildrenNode, TRUE);
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

Action *parse_action(xmlDocPtr doc, xmlNodePtr node)
{
    char *actname;
    Action *act = NULL;
    xmlNodePtr n;

    if (parse_attr_string("name", node, &actname)) {
        if ((act = action_from_string(actname))) {
            if (act->func == action_execute || act->func == action_restart) {
                if ((n = parse_find_node("execute", node->xmlChildrenNode)))
                    act->data.execute.path = parse_string(doc, n);
            } else if (act->func == action_showmenu) {
                if ((n = parse_find_node("menu", node->xmlChildrenNode)))
                    act->data.showmenu.name = parse_string(doc, n);
            } else if (act->func == action_desktop) {
                if ((n = parse_find_node("desktop", node->xmlChildrenNode)))
                    act->data.desktop.desk = parse_int(doc, n);
                if (act->data.desktop.desk > 0) act->data.desktop.desk--;
            } else if (act->func == action_send_to_desktop) {
                if ((n = parse_find_node("desktop", node->xmlChildrenNode)))
                    act->data.sendto.desk = parse_int(doc, n);
                if (act->data.sendto.desk > 0) act->data.sendto.desk--;
            } else if (act->func == action_move_relative_horz ||
                       act->func == action_move_relative_vert ||
                       act->func == action_resize_relative_horz ||
                       act->func == action_resize_relative_vert) {
                if ((n = parse_find_node("delta", node->xmlChildrenNode)))
                    act->data.relative.delta = parse_int(doc, n);
            } else if (act->func == action_desktop_right ||
                       act->func == action_desktop_left ||
                       act->func == action_desktop_up ||
                       act->func == action_desktop_down) {
                if ((n = parse_find_node("wrap", node->xmlChildrenNode))) {
                    g_message("WRAP %d", parse_bool(doc, n));
                    act->data.desktopdir.wrap = parse_bool(doc, n);
                }
            } else if (act->func == action_send_to_desktop_right ||
                       act->func == action_send_to_desktop_left ||
                       act->func == action_send_to_desktop_up ||
                       act->func == action_send_to_desktop_down) {
                if ((n = parse_find_node("wrap", node->xmlChildrenNode)))
                    act->data.sendtodir.wrap = parse_bool(doc, n);
                if ((n = parse_find_node("follow", node->xmlChildrenNode)))
                    act->data.sendtodir.follow = parse_bool(doc, n);
            }
        }
    }
    return act;
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
