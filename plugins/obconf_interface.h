#ifndef __obconf_plugin_interface_h
#define __obconf_plugin_interface_h

#include "parser/parse.h"

struct GtkWidget;

#define OBCONF_INTERFACE_VERSION 1

/* plugin_interface_version() */
typedef int (*PluginInterfaceVersionFunc)(void);

/* plugin_startup() */
typedef void (*PluginStartupFunc)(void);

/* plugin_shutdown() */
typedef void (*PluginShutdownFunc)(void);

/* plugin_name() - user friendly name of the plugin */
typedef char* (*PluginNameFunc)(void);

/* plugin_plugin_name() - the name of the plugin to load with openbox */
typedef char* (*PluginPluginNameFunc)(void);

/* plugin_icon() XXX FIXME */
typedef void (*PluginIconFunc)(void);

/* plugin_toplevel_widget() */
typedef struct _GtkWidget* (*PluginToplevelWidgetFunc)(void);

/* plugin_edited() */
typedef gboolean (*PluginEditedFunc)(void);

/* plugin_load() */
typedef void (*PluginLoadFunc)(xmlDocPtr doc, xmlNodePtr root);

/* plugin_save() */
typedef void (*PluginSaveFunc)(xmlDocPtr doc, xmlNodePtr root);

#endif
