#ifndef __plugin_h
#define __plugin_h

struct _ObParseInst;

void plugin_startup();
void plugin_shutdown();

void plugin_loadall(struct _ObParseInst *i);
void plugin_startall();

/* default plugin */
gboolean plugin_open(char *name, struct _ObParseInst *i);
/* load a plugin, but don't warn about reopens. for menus */
gboolean plugin_open_reopen(char *name, struct _ObParseInst *i);
void plugin_close(char *name);

/* call plugin's generic constructor */
void *plugin_create(char *name, void *data);
/* free memory allocated by plugin_create() */
void plugin_destroy(char *name, void *object);

#endif
