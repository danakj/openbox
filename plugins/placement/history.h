#ifndef __plugin_placement_history_h
#define __plugin_placement_history_h

#include "kernel/client.h"
#include <glib.h>

void history_startup();
void history_shutdown();

gboolean place_history(Client *c);

#endif
