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
                action_free(sit->data);
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

    for (it = g_list_last(keylist); it != NULL; it = it->prev) {
	p = ret;
	ret = g_new0(KeyBindingTree, 1);
	if (p == NULL) {
	    GList *it;

	    /* this is the first built node, the bottom node of the tree */
	    ret->keylist = g_list_copy(keylist); /* shallow copy */
	    for (it = ret->keylist; it != NULL; it = it->next) /* deep copy */
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

    if (firstnode == NULL) {
	/* there are no nodes at this level yet */
	firstnode = node;
    } else {
	a = firstnode;
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

    a = firstnode;
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
