#ifndef __plugin_keyboard_tree_h
#define __plugin_keyboard_tree_h

#include "../../kernel/action.h"
#include <glib.h>

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

void tree_destroy(KeyBindingTree *tree);
KeyBindingTree *tree_build(GList *keylist);
void tree_assimilate(KeyBindingTree *node);
KeyBindingTree *tree_find(KeyBindingTree *search, gboolean *conflict);

#endif
