#ifndef __parse_h
#define __parse_h

#include <libxml/parser.h>
#include <glib.h>

typedef void (*ParseCallback)(xmlDocPtr doc, xmlNodePtr node, void *data);

void parse_startup();
void parse_shutdown();

/* Loads Openbox's rc, from $HOME or $PREFIX as a fallback */
gboolean parse_load_rc(xmlDocPtr *doc, xmlNodePtr *root);

/* callbacks - must call parse_startup to use these */

void parse_register(const char *tag, ParseCallback func, void *data);
void parse_tree(xmlDocPtr doc, xmlNodePtr node, void *nothing);


/* open/close */

gboolean parse_load(const char *path, const char *rootname,
                    xmlDocPtr *doc, xmlNodePtr *root);
void parse_close(xmlDocPtr doc);


/* helpers */

xmlNodePtr parse_find_node(const char *tag, xmlNodePtr node);

char *parse_string(xmlDocPtr doc, xmlNodePtr node);
int parse_int(xmlDocPtr doc, xmlNodePtr node);
gboolean parse_bool(xmlDocPtr doc, xmlNodePtr node);

gboolean parse_contains(const char *val, xmlDocPtr doc, xmlNodePtr node);
gboolean parse_attr_contains(const char *val, xmlNodePtr node,
                             const char *name);

gboolean parse_attr_string(const char *name, xmlNodePtr node, char **value);
gboolean parse_attr_int(const char *name, xmlNodePtr node, int *value);

#endif
