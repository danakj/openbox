#include "../../kernel/focus.h"
#include "../../kernel/dispatch.h"
#include "../../kernel/openbox.h"
#include "../../kernel/grab.h"
#include "../../kernel/action.h"
#include "tree.h"
#include "keyboard.h"
#include "keyaction.h"
#include <glib.h>

KeyBindingTree *firstnode;

static KeyBindingTree *curpos;
static guint reset_key, reset_state;
static gboolean grabbed;

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

static void reset_chains()
{
    /* XXX kill timer */
    curpos = NULL;
    if (grabbed) {
	grabbed = FALSE;
        grab_keyboard(FALSE);
    }
}

static void clearall()
{
    grab_keys(FALSE);
    tree_destroy(firstnode);
    firstnode = NULL;
    grab_keys(TRUE);
}

static gboolean bind(GList *keylist, KeyAction *action)
{
    KeyBindingTree *tree, *t;
    gboolean conflict;

    if (!(tree = tree_build(keylist))) {
        g_warning("invalid binding");
        return FALSE;
    }

    t = tree_find(tree, &conflict);
    if (conflict) {
        g_warning("conflict with binding");
        tree_destroy(tree);
        return FALSE;
    }
    if (t != NULL) {
	/* already bound to something */
	g_warning("keychain is already bound");
        tree_destroy(tree);
        return FALSE;
    }

    /* grab the server here to make sure no key pressed go missed */
    grab_server(TRUE);

    grab_keys(FALSE);

    /* set the function */
    t = tree;
    while (t->first_child) t = t->first_child;
    t->action.action = action->action;
    t->action.type[0] = action->type[0];
    t->action.type[1] = action->type[1];
    t->action.data[0] = action->data[0];
    t->action.data[1] = action->data[1];

    /* assimilate this built tree into the main tree */
    tree_assimilate(tree); /* assimilation destroys/uses the tree */

    grab_keys(TRUE); 

    grab_server(FALSE);

    return TRUE;
}

static void press(ObEvent *e, void *foo)
{
    if (e->data.x.e->xkey.keycode == reset_key &&
        e->data.x.e->xkey.state == reset_state) {
        reset_chains();
        XAllowEvents(ob_display, AsyncKeyboard, CurrentTime);
    } else {
        KeyBindingTree *p;
        if (curpos == NULL)
            p = firstnode;
        else
            p = curpos->first_child;
        while (p) {
            if (p->key == e->data.x.e->xkey.keycode &&
                p->state == e->data.x.e->xkey.state) {
                if (p->first_child != NULL) { /* part of a chain */
                    /* XXX TIMER */
                    if (!grabbed) {
                        grab_keyboard(TRUE);
                        grabbed = TRUE;
                        XAllowEvents(ob_display, AsyncKeyboard, CurrentTime);
                    }
                    curpos = p;
                } else {
                    keyaction_do(&p->action, focus_client);

                    XAllowEvents(ob_display, AsyncKeyboard, CurrentTime);
                    reset_chains();
                }
                break;
            }
            p = p->next_sibling;
        }
    }
}

static void binddef()
{
    GList *list = g_list_append(NULL, NULL);
    KeyAction a;

    list->data = "C-Right";
    a.action = Action_NextDesktop;
    keyaction_set_bool(&a, 0, TRUE);
    keyaction_set_none(&a, 1);
    bind(list, &a);

    list->data = "C-Left";
    a.action = Action_PreviousDesktop;
    keyaction_set_bool(&a, 0, TRUE);
    keyaction_set_none(&a, 1);
    bind(list, &a);

    list->data = "C-1";
    a.action = Action_Desktop;
    keyaction_set_uint(&a, 0, 0);
    keyaction_set_none(&a, 1);
    bind(list, &a);

    list->data = "C-2";
    a.action = Action_Desktop;
    keyaction_set_uint(&a, 0, 1);
    keyaction_set_none(&a, 1);
    bind(list, &a);

    list->data = "C-3";
    a.action = Action_Desktop;
    keyaction_set_uint(&a, 0, 2);
    keyaction_set_none(&a, 1);
    bind(list, &a);

    list->data = "C-4";
    a.action = Action_Desktop;
    keyaction_set_uint(&a, 0, 3);
    keyaction_set_none(&a, 1);
    bind(list, &a);

    list->data = "C-space";
    a.action = Action_Execute;
    keyaction_set_string(&a, 0, "xterm");
    keyaction_set_none(&a, 1);
    bind(list, &a);
}

void plugin_startup()
{
    dispatch_register(Event_X_KeyPress, (EventHandler)press, NULL);

    /* XXX parse config file! */
    binddef();
}

void plugin_shutdown()
{
    dispatch_register(0, (EventHandler)press, NULL);
    clearall();
}

