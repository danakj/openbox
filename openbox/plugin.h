#ifndef __plugin_h
#define __plugin_h

void plugin_startup();
void plugin_shutdown();

void plugin_loadall();
void plugin_startall();

/* default plugin */
gboolean plugin_open(char *name);
/* load a plugin, but don't warn about reopens. for menus */
gboolean plugin_open_reopen(char *name);
void plugin_close(char *name);

/* call plugin's generic constructor */
void *plugin_create(char *name /* TODO */);
/* free memory allocated by plugin_create() */
void plugin_destroy(char *name, void *object);

#endif
