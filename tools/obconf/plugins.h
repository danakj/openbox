#ifndef __plugins_h
#define __plugins_h

#include <libxml/parser.h>

typedef struct ConfigPlugin ConfigPlugin;

extern GSList *plugins_list;

void plugins_load();

gboolean plugins_edited(ConfigPlugin *p);
void plugins_load_settings(ConfigPlugin *p, xmlDocPtr doc, xmlNodePtr root);
void plugins_save_settings(ConfigPlugin *p, xmlDocPtr doc, xmlNodePtr root);

#endif
