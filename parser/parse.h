#ifndef __parse_h
#define __parse_h

#include <libxml/parser.h>
#include <glib.h>

typedef struct _ObParseInst ObParseInst;

typedef void (*ParseCallback)(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                              gpointer data);

ObParseInst* parse_startup();
void parse_shutdown(ObParseInst *inst);

/* Loads Openbox's rc, from the normal paths */
gboolean parse_load_rc(xmlDocPtr *doc, xmlNodePtr *root);
/* Loads an Openbox menu, from the normal paths */
gboolean parse_load_menu(const gchar *file, xmlDocPtr *doc, xmlNodePtr *root);

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

/* paths */

void parse_paths_startup();
void parse_paths_shutdown();

const gchar* parse_xdg_config_home_path();
const gchar* parse_xdg_data_home_path();
GSList* parse_xdg_config_dir_paths();
GSList* parse_xdg_data_dir_paths();

/*! Expands the ~ character to the home directory throughout the given
  string */
gchar *parse_expand_tilde(const gchar *f);
/*! Makes a directory and all its parents */
void parse_mkdir_path(const gchar *path, gint mode);

#endif
