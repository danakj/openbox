#ifndef __plugin_h
#define __plugin_h

void plugin_startup();
void plugin_shutdown();

void plugin_loadall();
void plugin_startall();

gboolean plugin_open(char *name);
void plugin_close(char *name);

#endif
