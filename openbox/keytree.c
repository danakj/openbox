/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   keytree.c for the Openbox window manager
   Copyright (c) 2003        Ben Jansens

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   See the COPYING file for a copy of the GNU General Public License.
*/

#include "keyboard.h"
#include "translate.h"
#include <glib.h>

void tree_destroy(KeyBindingTree *tree)
{
    KeyBindingTree *c;

    while (tree) {
        tree_destroy(tree->next_sibling);
        c = tree->first_child;
        if (c == NULL) {
            GList *it;
            GSList *sit;
            for (it = tree->keylist; it != NULL; it = it->next)
                g_free(it->data);
            g_list_free(tree->keylist);
            for (sit = tree->actions; sit != NULL; sit = sit->next)
                action_unref(sit->data);
            g_slist_free(tree->actions);
        }
        g_free(tree);
        tree = c;
    }
}

KeyBindingTree *tree_build(GList *keylist)
{
    GList *it;
    KeyBindingTree *ret = NULL, *p;

    if (g_list_length(keylist) <= 0)
        return NULL; /* nothing in the list.. */

    for (it = g_list_last(keylist); it; it = g_list_previous(it)) {
        p = ret;
        ret = g_new0(KeyBindingTree, 1);
        if (p == NULL) {
            GList *it;

            /* this is the first built node, the bottom node of the tree */
            ret->keylist = g_list_copy(keylist); /* shallow copy */
            for (it = ret->keylist; it; it = g_list_next(it)) /* deep copy */
                it->data = g_strdup(it->data);
        }
        ret->first_child = p;
        if (!translate_key(it->data, &ret->state, &ret->key)) {
            tree_destroy(ret);
            return NULL;
        }
    }
    return ret;
}

void tree_assimilate(KeyBindingTree *node)
{
    KeyBindingTree *a, *b, *tmp, *last;

    if (keyboard_firstnode == NULL) {
        /* there are no nodes at this level yet */
        keyboard_firstnode = node;
    } else {
        a = keyboard_firstnode;
        last = a;
        b = node;
        while (a) {
            last = a;
            if (!(a->state == b->state && a->key == b->key)) {
                a = a->next_sibling;
            } else {
                tmp = b;
                b = b->first_child;
                g_free(tmp);
                a = a->first_child;
            }
        }
        if (!(last->state == b->state && last->key == b->key))
            last->next_sibling = b;
        else {
            last->first_child = b->first_child;
            g_free(b);
        }
    }
}

KeyBindingTree *tree_find(KeyBindingTree *search, gboolean *conflict)
{
    KeyBindingTree *a, *b;

    *conflict = FALSE;

    a = keyboard_firstnode;
    b = search;
    while (a && b) {
        if (!(a->state == b->state && a->key == b->key)) {
            a = a->next_sibling;
        } else {
            if ((a->first_child == NULL) == (b->first_child == NULL)) {
                if (a->first_child == NULL) {
                    /* found it! (return the actual node, not the search's) */
                    return a;
                }
            } else {
                *conflict = TRUE;
                return NULL; /* the chain status' don't match (conflict!) */
            }
            b = b->first_child;
            a = a->first_child;
        }
    }
    return NULL; /* it just isn't in here */
}
