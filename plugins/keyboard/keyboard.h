#ifndef __plugin_keyboard_keybaord_h
#define __plugin_keyboard_keybaord_h

#include <glib.h>

#include "../../kernel/action.h"

typedef struct KeyBindingTree {
    guint state;
    guint key;
    GList *keylist;
    Action *action;

    /* the next binding in the tree at the same level */
    struct KeyBindingTree *next_sibling; 
    /* the first child of this binding (next binding in a chained sequence).*/
    struct KeyBindingTree *first_child;
} KeyBindingTree;

extern KeyBindingTree *firstnode;

guint keyboard_translate_modifier(char *str);

#endif
