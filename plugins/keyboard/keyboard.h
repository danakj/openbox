#ifndef __plugin_keyboard_keybaord_h
#define __plugin_keyboard_keybaord_h

#include <glib.h>

#include "tree.h"

extern KeyBindingTree *firstnode;

gboolean kbind(GList *keylist, Action *action);

#endif
