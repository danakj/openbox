#include "kernel/focus.h"
#include "kernel/dispatch.h"
#include "kernel/openbox.h"
#include "kernel/event.h"
#include "kernel/grab.h"
#include "kernel/action.h"
#include "kernel/parse.h"
#include "kernel/timer.h"
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
static Timer *chain_timer;

static void grab_keys()
{
    KeyBindingTree *p;

    ungrab_all_keys();

    p = curpos ? curpos->first_child : firstnode;
    while (p) {
        grab_key(p->key, p->state, GrabModeAsync);
        p = p->next_sibling;
    }
    grab_key(reset_key, reset_state, GrabModeAsync);
}

static void reset_chains()
{
    if (chain_timer) {
        timer_stop(chain_timer);
        chain_timer = NULL;
    }
    if (curpos) {
        curpos = NULL;
        grab_keys();
    }
}

static void chain_timeout(void *data)
{
    reset_chains();
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
	/* already bound to something, use the existing tree */
        tree_destroy(tree);
        tree = NULL;
    } else
        t = tree;
    while (t->first_child) t = t->first_child;

    if (conflict) {
        g_message("conflict with binding");
        tree_destroy(tree);
        return FALSE;
    }

    /* set the action */
    t->actions = g_slist_append(t->actions, action);
    /* assimilate this built tree into the main tree. assimilation
       destroys/uses the tree */
    if (tree) tree_assimilate(tree);

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
                GSList *it;
                for (it = grabbed_key->actions; it; it = it->next) {
                    Action *act = it->data;
                    act->data.cycle.cancel = TRUE;
                }
                done = TRUE;
            }
        }
        if (done) { 
            GSList *it;
            for (it = grabbed_key->actions; it; it = it->next) {
                Action *act = it->data;
                act->data.cycle.final = TRUE;
                act->func(&act->data);
                grabbed_key = NULL;
                grab_keyboard(FALSE);
                reset_chains();
                return;
            }
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
                    if (chain_timer) timer_stop(chain_timer);
                    /* 5 second timeout for chains */
                    chain_timer = timer_start(5000*1000, chain_timeout,
                                              NULL);
                    curpos = p;
                    grab_keys();
                } else {
                    GSList *it;
                    for (it = p->actions; it; it = it->next) {
                        Action *act = it->data;
                        if (act->func != NULL) {
                            act->data.any.c = focus_client;

                            if (act->func == action_cycle_windows) {
                                act->data.cycle.final = FALSE;
                                act->data.cycle.cancel = FALSE;
                            }

                            act->func(&act->data);

                            if (act->func == action_cycle_windows &&
                                !grabbed_key) {
                                grabbed_key = p;
                                grab_keyboard(TRUE);
                                break;
                            }
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
    chain_timer = NULL;

    dispatch_register(Event_X_KeyPress | Event_X_KeyRelease,
                      (EventHandler)event, NULL);

    translate_key("C-g", &reset_state, &reset_key);
    translate_key("Escape", &i, &button_escape);
    translate_key("Return", &i, &button_return);

    grab_keys();
}

void plugin_shutdown()
{
    dispatch_register(0, (EventHandler)event, NULL);

    tree_destroy(firstnode);
    firstnode = NULL;
    ungrab_all_keys();
}

