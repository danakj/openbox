#ifndef __plugin_keyboard_keybaord_h
#define __plugin_keyboard_keybaord_h

#include "keyaction.h"
#include <glib.h>

typedef struct KeyBindingTree {
    guint state;
    guint key;
    GList *keylist;
    KeyAction action;

    /* the next binding in the tree at the same level */
    struct KeyBindingTree *next_sibling; 
    /* the first child of this binding (next binding in a chained sequence).*/
    struct KeyBindingTree *first_child;
} KeyBindingTree;

extern KeyBindingTree *firstnode;

guint keyboard_translate_modifier(char *str);

#endif
