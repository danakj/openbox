#ifndef __plugin_keyboard_tree_h
#define __plugin_keyboard_tree_h

#include "keyboard.h"
#include <glib.h>

void tree_destroy(KeyBindingTree *tree);
KeyBindingTree *tree_build(GList *keylist);
void tree_assimilate(KeyBindingTree *node);
KeyBindingTree *tree_find(KeyBindingTree *search, gboolean *conflict);

#endif
