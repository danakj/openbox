#include "../../kernel/focus.h"
#include "../../kernel/dispatch.h"
#include "../../kernel/openbox.h"
#include "../../kernel/grab.h"
#include "../../kernel/action.h"
#include "tree.h"
#include "keyboard.h"
#include "keysrc.h"
#include <glib.h>

void plugin_setup_config()
{
}

KeyBindingTree *firstnode;

static KeyBindingTree *curpos;
static guint reset_key, reset_state;
static gboolean grabbed;

static void grab_keys(gboolean grab)
{
    if (!grab) {
        ungrab_all_keys();
    } else {
	KeyBindingTree *p = firstnode;
	while (p) {
            grab_key(p->key, p->state, GrabModeSync);
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

gboolean kbind(GList *keylist, Action *action)
{
    KeyBindingTree *tree, *t;
    gboolean conflict;

    g_assert(keylist != NULL);
    g_assert(action != NULL);

    if (!(tree = tree_build(keylist)))
        return FALSE;
    if ((t = tree_find(tree, &conflict)) != NULL) {
	/* already bound to something */
	g_warning("keychain is already bound");
        tree_destroy(tree);
        return FALSE;
    }
    if (conflict) {
        g_warning("conflict with binding");
        tree_destroy(tree);
        return FALSE;
    }

    /* grab the server here to make sure no key presses go missed */
    grab_server(TRUE);
    grab_keys(FALSE);

    /* set the action */
    t = tree;
    while (t->first_child) t = t->first_child;
    t->action = action;
    /* assimilate this built tree into the main tree. assimilation
       destroys/uses the tree */
    tree_assimilate(tree);

    grab_keys(TRUE); 
    grab_server(FALSE);

    return TRUE;
}

static void press(ObEvent *e, void *foo)
{
    if (e->data.x.e->xkey.keycode == reset_key &&
        e->data.x.e->xkey.state == reset_state) {
        reset_chains();
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
                    }
                    curpos = p;
                } else {
                    if (p->action->func != NULL) {
                        p->action->data.any.c = focus_client;

                        g_assert(!(p->action->func == action_move ||
                                   p->action->func == action_resize));

                        p->action->func(&p->action->data);
                    }

                    reset_chains();
                }
                break;
            }
            p = p->next_sibling;
        }
    }
    XAllowEvents(ob_display, AsyncKeyboard, e->data.x.e->xkey.time);
}

void plugin_startup()
{
    dispatch_register(Event_X_KeyPress, (EventHandler)press, NULL);

    keysrc_parse();
}

void plugin_shutdown()
{
    dispatch_register(0, (EventHandler)press, NULL);

    grab_keys(FALSE);
    tree_destroy(firstnode);
    firstnode = NULL;
    grab_keys(TRUE);
}

