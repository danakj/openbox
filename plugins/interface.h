#ifndef __plugins_interface_h
#define __plugins_interface_h

struct _ObParseInst;

/* plugin_setup_config() */
typedef void (*PluginSetupConfig)(struct _ObParseInst *i);

/* plugin_startup() */
typedef void (*PluginStartup)(void);

/* plugin_shutdown() */
typedef void (*PluginShutdown)(void);

#endif
