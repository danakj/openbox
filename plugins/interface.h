#ifndef __plugins_interface_h
#define __plugins_interface_h

/* plugin_setup_config() */
typedef void (*PluginSetupConfig)(void);

/* plugin_startup() */
typedef void (*PluginStartup)(void);

/* plugin_shutdown() */
typedef void (*PluginShutdown)(void);

/* plugin_create() - for menu plugins only */
typedef void *(*PluginCreate)(/* TODO */);

/* plugin_destroy() - for menu plugins only */
typedef void (*PluginDestroy)(void *);

#endif
