#ifndef __plugin_keyboard_keybaord_h
#define __plugin_keyboard_keybaord_h

#include <glib.h>

#include "tree.h"

extern KeyBindingTree *firstnode;

guint keyboard_translate_modifier(char *str);

#endif
