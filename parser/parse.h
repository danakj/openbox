#ifndef __parse_h
#define __parse_h

#include <libxml/parser.h>
#include <glib.h>

typedef struct _ObParseInst ObParseInst;

typedef void (*ParseCallback)(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                              gpointer data);

ObParseInst* parse_startup();
void parse_shutdown(ObParseInst *inst);

/* Loads Openbox's rc, from $HOME or $PREFIX as a fallback */
gboolean parse_load_rc(xmlDocPtr *doc, xmlNodePtr *root);

void parse_register(ObParseInst *inst, const char *tag,
                    ParseCallback func, gpointer data);
void parse_tree(ObParseInst *inst, xmlDocPtr doc, xmlNodePtr node);


/* open/close */

gboolean parse_load(const char *path, const char *rootname,
                    xmlDocPtr *doc, xmlNodePtr *root);
gboolean parse_load_mem(gpointer data, guint len, const char *rootname,
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
