#include "focus.h"
#include "openbox.h"
#include "hooks.h"
#include "kbind.h"

#include <glib.h>
#ifdef HAVE_STRING_H
#  include <string.h>
#endif

typedef struct KeyBindingTree {
    guint state;
    guint key;
    GList *keylist;

    /* the next binding in the tree at the same level */
    struct KeyBindingTree *next_sibling; 
    /* the first child of this binding (next binding in a chained sequence).*/
    struct KeyBindingTree *first_child;
} KeyBindingTree;


static KeyBindingTree *firstnode, *curpos;
static guint reset_key, reset_state;
static gboolean grabbed, user_grabbed;

guint kbind_translate_modifier(char *str)
{
    if (!strcmp("Mod1", str)) return Mod1Mask;
    else if (!strcmp("Mod2", str)) return Mod2Mask;
    else if (!strcmp("Mod3", str)) return Mod3Mask;
    else if (!strcmp("Mod4", str)) return Mod4Mask;
    else if (!strcmp("Mod5", str)) return Mod5Mask;
    else if (!strcmp("C", str)) return ControlMask;
    else if (!strcmp("S", str)) return ShiftMask;
    g_warning("Invalid modifier '%s' in binding.", str);
    return 0;
}

static gboolean translate(char *str, guint *state, guint *keycode)
{
    char **parsed;
    char *l;
    int i;
    gboolean ret = FALSE;
    KeySym sym;

    parsed = g_strsplit(str, "-", -1);
    
    /* first, find the key (last token) */
    l = NULL;
    for (i = 0; parsed[i] != NULL; ++i)
	l = parsed[i];
    if (l == NULL)
	goto translation_fail;

    /* figure out the mod mask */
    *state = 0;
    for (i = 0; parsed[i] != l; ++i) {
	guint m = kbind_translate_modifier(parsed[i]);
	if (!m) goto translation_fail;
	*state |= m;
    }

    /* figure out the keycode */
    sym = XStringToKeysym(l);
    if (sym == NoSymbol) {
	g_warning("Invalid key name '%s' in key binding.", l);
	goto translation_fail;
    }
    *keycode = XKeysymToKeycode(ob_display, sym);
    if (!keycode) {
	g_warning("Key '%s' does not exist on the display.", l); 
	goto translation_fail;
    }

    ret = TRUE;

translation_fail:
    g_strfreev(parsed);
    return ret;
}

static void destroytree(KeyBindingTree *tree)
{
    KeyBindingTree *c;

    while (tree) {
	destroytree(tree->next_sibling);
	c = tree->first_child;
	if (c == NULL) {
	    GList *it;
	    for (it = tree->keylist; it != NULL; it = it->next)
		g_free(it->data);
	    g_list_free(tree->keylist);
	}
	g_free(tree);
	tree = c;
    }
}

static KeyBindingTree *buildtree(GList *keylist)
{
    GList *it;
    KeyBindingTree *ret = NULL, *p;

    if (g_list_length(keylist) <= 0)
	return NULL; /* nothing in the list.. */

    for (it = g_list_last(keylist); it != NULL; it = it->prev) {
	p = ret;
	ret = g_new(KeyBindingTree, 1);
	ret->next_sibling = NULL;
	if (p == NULL) {
	    GList *it;

	    /* this is the first built node, the bottom node of the tree */
	    ret->keylist = g_list_copy(keylist); /* shallow copy */
	    for (it = ret->keylist; it != NULL; it = it->next) /* deep copy */
		it->data = g_strdup(it->data);
	}
	ret->first_child = p;
	if (!translate(it->data, &ret->state, &ret->key)) {
	    destroytree(ret);
	    return NULL;
	}
    }
    return ret;
}

static void assimilate(KeyBindingTree *node)
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
	if (!(last->state == b->state && last->key == a->key))
	    last->next_sibling = b;
	else {
	    last->first_child = b->first_child;
	    g_free(b);
	}
    }
}

KeyBindingTree *find(KeyBindingTree *search, gboolean *conflict)
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
    return NULL; // it just isn't in here
}

static void grab_keys(gboolean grab)
{
    if (!grab) {
	XUngrabKey(ob_display, AnyKey, AnyModifier, ob_root);
    } else {
	KeyBindingTree *p = firstnode;
	while (p) {
	    XGrabKey(ob_display, p->key, p->state, ob_root, FALSE,
		     GrabModeAsync, GrabModeSync);
	    p = p->next_sibling;
	}
    }
}

void reset_chains()
{
    /* XXX kill timer */
    curpos = NULL;
    if (grabbed) {
	grabbed = FALSE;
	g_message("reset chains. user: %d", user_grabbed);
	if (!user_grabbed)
	    XUngrabKeyboard(ob_display, CurrentTime);
    }
}

void kbind_fire(guint state, guint key, gboolean press)
{
    EventData *data;
    struct Client *c = focus_client;
    GQuark context = c != NULL ? g_quark_try_string("client")
			       : g_quark_try_string("root");

    if (user_grabbed) {
	data = eventdata_new_key(press ? Key_Press : Key_Release,
				 context, c, state, key, NULL);
	g_assert(data != NULL);
	hooks_fire_keyboard(data);
	eventdata_free(data);
    }

    if (key == reset_key && state == reset_state) {
	reset_chains();
	XAllowEvents(ob_display, AsyncKeyboard, CurrentTime);
    } else {
	KeyBindingTree *p;
	if (curpos == NULL)
	    p = firstnode;
	else
	    p = curpos->first_child;
	while (p) {
	    if (p->key == key && p->state == state) {
		if (p->first_child != NULL) { /* part of a chain */
		    /* XXX TIMER */
		    if (!grabbed && !user_grabbed) {
			/*grab should never fail because we should have a sync
			  grab at this point */
			XGrabKeyboard(ob_display, ob_root, 0, GrabModeAsync, 
				      GrabModeSync, CurrentTime);
		    }
		    grabbed = TRUE;
		    curpos = p;
		    XAllowEvents(ob_display, AsyncKeyboard, CurrentTime);
		} else {
		    data = eventdata_new_key(press ? Key_Press : Key_Release,
					     context, c, state, key,
					     p->keylist);
		    g_assert(data != NULL);
		    hooks_fire(data);
		    eventdata_free(data);

		    XAllowEvents(ob_display, AsyncKeyboard, CurrentTime);
		    reset_chains();
		}
		break;
	    }
	    p = p->next_sibling;
	}
    }
}

gboolean kbind_add(GList *keylist)
{
    KeyBindingTree *tree, *t;
    gboolean conflict;

    if (!(tree = buildtree(keylist)))
	return FALSE; /* invalid binding requested */

    t = find(tree, &conflict);
    if (conflict) {
	/* conflicts with another binding */
	destroytree(tree);
	return FALSE;
    }

    if (t != NULL) {
	/* already bound to something */
	destroytree(tree);
    } else {
	/* grab the server here to make sure no key pressed go missed */
	XGrabServer(ob_display);
	XSync(ob_display, FALSE);

	grab_keys(FALSE);

	/* assimilate this built tree into the main tree */
	assimilate(tree); // assimilation destroys/uses the tree

	grab_keys(TRUE); 

	XUngrabServer(ob_display);
	XFlush(ob_display);
    }
 
    return TRUE;
}

void kbind_clearall()
{
    grab_keys(FALSE);
    destroytree(firstnode);
    firstnode = NULL;
    grab_keys(TRUE);
}

void kbind_startup()
{
    gboolean b;

    curpos = firstnode = NULL;
    grabbed = user_grabbed = FALSE;

    b = translate("C-G", &reset_state, &reset_key);
    g_assert(b);
}

void kbind_shutdown()
{
    if (grabbed || user_grabbed) {
	grabbed = FALSE;
	kbind_grab_keyboard(FALSE);
    }
    grab_keys(FALSE);
    destroytree(firstnode);
    firstnode = NULL;
}

gboolean kbind_grab_keyboard(gboolean grab)
{
    gboolean ret = TRUE;

    if (!grab)
	g_message("grab_keyboard(false). grabbed: %d", grabbed);

    user_grabbed = grab;
    if (!grabbed) {
	if (grab)
	    ret = XGrabKeyboard(ob_display, ob_root, 0, GrabModeAsync, 
				GrabModeAsync, CurrentTime) == GrabSuccess;
	else
	    XUngrabKeyboard(ob_display, CurrentTime);
    }
    return ret;
}
