#include <glib.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>

#include "kernel/menu.h"
#include "kernel/timer.h"
#include "kernel/action.h"
#include "kernel/event.h"

static char *PLUGIN_NAME = "include_menu";

void plugin_setup_config() { }
void plugin_startup()
{ }
void plugin_shutdown() { }

void include_menu_clean_up(ObMenu *m) { }

void *plugin_create(PluginMenuCreateData *data)
{
    char *id;
    char *label;
    char *filename;
    ObMenu *m;
    xmlDocPtr doc;

    parse_attr_string("id", data->node, &id);
    parse_attr_string("label", data->node, &label);
    
    m = menu_new( (label != NULL ? label : ""),
                  (id != NULL ? id : PLUGIN_NAME),
                  data->parent);

    m->plugin = PLUGIN_NAME;

    parse_attr_string("filename", data->node, &filename);

    doc = xmlParseFile(filename);
    if (doc) {
        xmlNodePtr node = xmlDocGetRootElement(doc);
        if (node) {
            parse_menu_full(doc, node, m, FALSE);
        }
        xmlFreeDoc(doc);
    }

    if (data->parent)
        menu_add_entry(data->parent, menu_entry_new_submenu(
                           (label != NULL ? label : ""),
                           m));

    return (void *)m;
}

void plugin_destroy (void *m)
{
    include_menu_clean_up(m);
}
