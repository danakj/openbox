#include "kernel/focus.h"
#include "kernel/dispatch.h"
#include "kernel/openbox.h"
#include "kernel/event.h"
#include "kernel/grab.h"
#include "kernel/action.h"
#include "kernel/parse.h"
#include "tree.h"
#include "keyboard.h"
#include "keyparse.h"
#include "translate.h"
#include <glib.h>

void plugin_setup_config()
{
    parse_reg_section("keyboard", keyparse, NULL);
}

KeyBindingTree *firstnode = NULL;

static KeyBindingTree *curpos;
static guint reset_key, reset_state, button_return, button_escape;
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
    } else
        XAllowEvents(ob_display, AsyncKeyboard, event_lasttime);
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
	g_message("keychain is already bound");
        tree_destroy(tree);
        return FALSE;
    }
    if (conflict) {
        g_message("conflict with binding");
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

static void event(ObEvent *e, void *foo)
{
    static KeyBindingTree *grabbed_key = NULL;

    if (grabbed_key) {
        gboolean done = FALSE;

        if ((e->type == Event_X_KeyRelease && 
             !(grabbed_key->state & e->data.x.e->xkey.state)))
            done = TRUE;
        else if (e->type == Event_X_KeyPress) {
            if (e->data.x.e->xkey.keycode == button_return)
                done = TRUE;
            else if (e->data.x.e->xkey.keycode == button_escape) {
                grabbed_key->action->data.cycle.cancel = TRUE;
                done = TRUE;
            }
        }
        if (done) {
            grabbed_key->action->data.cycle.final = TRUE;
            grabbed_key->action->func(&grabbed_key->action->data);
            grab_keyboard(FALSE);
            grabbed_key = NULL;
            reset_chains();
            return; 
        }
    }
    if (e->type == Event_X_KeyRelease)
        return;

    g_assert(e->type == Event_X_KeyPress);

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
                        XAllowEvents(ob_display, AsyncKeyboard,
                                     event_lasttime);
                    }
                    curpos = p;
                } else {
                    if (p->action->func != NULL) {
                        p->action->data.any.c = focus_client;

                        g_assert(!(p->action->func == action_move ||
                                   p->action->func == action_resize));

                        if (p->action->func == action_cycle_windows) {
                            p->action->data.cycle.final = FALSE;
                            p->action->data.cycle.cancel = FALSE;
                        }

                        p->action->func(&p->action->data);

                        if (p->action->func == action_cycle_windows &&
                            !grabbed_key) {
                            grab_keyboard(TRUE);
                            grabbed_key = p;
                        }
                    }

                    reset_chains();
                }
                break;
            }
            p = p->next_sibling;
        }
    }
}

void plugin_startup()
{
    guint i;

    curpos = NULL;
    grabbed = FALSE;

    dispatch_register(Event_X_KeyPress | Event_X_KeyRelease, (EventHandler)event, NULL);

    translate_key("C-g", &reset_state, &reset_key);
    translate_key("Escape", &i, &button_escape);
    translate_key("Return", &i, &button_return);
}

void plugin_shutdown()
{
    dispatch_register(0, (EventHandler)event, NULL);

    grab_keys(FALSE);
    tree_destroy(firstnode);
    firstnode = NULL;
    grab_keys(TRUE);
}

