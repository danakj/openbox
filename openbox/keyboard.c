#include "focus.h"
#include "screen.h"
#include "frame.h"
#include "openbox.h"
#include "event.h"
#include "grab.h"
#include "client.h"
#include "action.h"
#include "prop.h"
#include "timer.h"
#include "config.h"
#include "keytree.h"
#include "keyboard.h"
#include "translate.h"

#include <glib.h>

KeyBindingTree *keyboard_firstnode;

static KeyBindingTree *curpos;
static ObTimer *chain_timer;
static gboolean interactive_grab;
static guint grabbed_state;
static ObClient *grabbed_client;
static ObAction *grabbed_action;
static ObFrameContext grabbed_context;

static void grab_for_window(Window win, gboolean grab)
{
    KeyBindingTree *p;

    ungrab_all_keys(win);

    if (grab) {
        p = curpos ? curpos->first_child : keyboard_firstnode;
        while (p) {
            grab_key(p->key, p->state, win, GrabModeAsync);
            p = p->next_sibling;
        }
        if (curpos)
            grab_key(config_keyboard_reset_keycode,
                     config_keyboard_reset_state,
                     win, GrabModeAsync);
    }
}

void keyboard_grab_for_client(ObClient *c, gboolean grab)
{
    grab_for_window(c->window, grab);
}

static void grab_keys(gboolean grab)
{
    GList *it;

    grab_for_window(screen_support_win, grab);
    for (it = client_list; it; it = g_list_next(it))
        grab_for_window(((ObClient*)it->data)->frame->window, grab);
}

void keyboard_reset_chains()
{
    if (chain_timer) {
        timer_stop(chain_timer);
        chain_timer = NULL;
    }
    if (curpos) {
        curpos = NULL;
        grab_keys(TRUE);
    }
}

static void chain_timeout(ObTimer *t, void *data)
{
    keyboard_reset_chains();
}

gboolean keyboard_bind(GList *keylist, ObAction *action)
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
        g_warning("conflict with binding");
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

void keyboard_interactive_grab(guint state, ObClient *client,
                               ObFrameContext context, ObAction *action)
{
    if (!interactive_grab && grab_keyboard(TRUE)) {
        interactive_grab = TRUE;
        grabbed_state = state;
        grabbed_client = client;
        grabbed_action = action;
        grabbed_context = context;
        grab_pointer(TRUE, None);
    }
}

gboolean keyboard_process_interactive_grab(const XEvent *e,
                                           ObClient **client,
                                           ObFrameContext *context)
{
    gboolean handled = FALSE;
    gboolean done = FALSE;

    if (interactive_grab) {
        *client = grabbed_client;
        *context = grabbed_context;

        if ((e->type == KeyRelease && 
             !(grabbed_state & e->xkey.state)))
            done = TRUE;
        else if (e->type == KeyPress) {
            if (e->xkey.keycode == ob_keycode(OB_KEY_RETURN))
                done = TRUE;
            else if (e->xkey.keycode == ob_keycode(OB_KEY_ESCAPE)) {
                if (grabbed_action->func == action_cycle_windows) {
                    grabbed_action->data.cycle.cancel = TRUE;
                }
                if (grabbed_action->func == action_desktop_dir) {
                    grabbed_action->data.desktopdir.cancel = TRUE;
                }
                if (grabbed_action->func == action_send_to_desktop_dir)
                {
                    grabbed_action->data.sendtodir.cancel = TRUE;
                }
                done = TRUE;
            }
        }
        if (done) { 
            if (grabbed_action->func == action_cycle_windows) {
                grabbed_action->data.cycle.final = TRUE;
            }
            if (grabbed_action->func == action_desktop_dir) {
                grabbed_action->data.desktopdir.final = TRUE;
            }
            if (grabbed_action->func == action_send_to_desktop_dir) {
                grabbed_action->data.sendtodir.final = TRUE;
            }

            grabbed_action->func(&grabbed_action->data);

            interactive_grab = FALSE;
            grab_keyboard(FALSE);
            grab_pointer(FALSE, None);
            keyboard_reset_chains();

            handled = TRUE;
        }
    }

    return handled;
}

void keyboard_event(ObClient *client, const XEvent *e)
{
    KeyBindingTree *p;

    g_assert(e->type == KeyPress);

    if (e->xkey.keycode == config_keyboard_reset_keycode &&
        e->xkey.state == config_keyboard_reset_state)
    {
        keyboard_reset_chains();
        return;
    }

    if (curpos == NULL)
        p = keyboard_firstnode;
    else
        p = curpos->first_child;
    while (p) {
        if (p->key == e->xkey.keycode &&
            p->state == e->xkey.state) {
            if (p->first_child != NULL) { /* part of a chain */
                if (chain_timer) timer_stop(chain_timer);
                /* 5 second timeout for chains */
                chain_timer = timer_start(5000*1000, chain_timeout,
                                          NULL);
                curpos = p;
                grab_keys(TRUE);
            } else {
                GSList *it;
                for (it = p->actions; it; it = it->next) {
                    ObAction *act = it->data;
                    if (act->func != NULL) {
                        act->data.any.c = client;

                        if (act->func == action_cycle_windows)
                        {
                            act->data.cycle.final = FALSE;
                            act->data.cycle.cancel = FALSE;
                        }
                        if (act->func == action_desktop_dir)
                        {
                            act->data.desktopdir.final = FALSE;
                            act->data.desktopdir.cancel = FALSE;
                        }
                        if (act->func == action_send_to_desktop_dir)
                        {
                            act->data.sendtodir.final = FALSE;
                            act->data.sendtodir.cancel = FALSE;
                        }

                        if (act->func == action_moveresize)
                        {
                            screen_pointer_pos(&act->data.moveresize.x,
                                               &act->data.moveresize.y);
                        }

                        if ((act->func == action_cycle_windows ||
                             act->func == action_desktop_dir ||
                             act->func == action_send_to_desktop_dir))
                        {
                            keyboard_interactive_grab(e->xkey.state, client,
                                                      0, act);
                        }

                        if (act->func == action_showmenu)
                        {
                            act->data.showmenu.x = e->xkey.x_root;
                            act->data.showmenu.y = e->xkey.y_root;
                        }

                        act->data.any.c = client;
                        act->func(&act->data);
                    }
                }

                keyboard_reset_chains();
            }
            break;
        }
        p = p->next_sibling;
    }
}

void keyboard_startup()
{
    grab_keys(TRUE);
}

void keyboard_shutdown()
{
    tree_destroy(keyboard_firstnode);
    keyboard_firstnode = NULL;
    grab_keys(FALSE);
}

