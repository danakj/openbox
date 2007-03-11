/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   keyboard.c for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
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

#include "mainloop.h"
#include "focus.h"
#include "screen.h"
#include "frame.h"
#include "openbox.h"
#include "event.h"
#include "grab.h"
#include "client.h"
#include "action.h"
#include "prop.h"
#include "config.h"
#include "keytree.h"
#include "keyboard.h"
#include "translate.h"
#include "moveresize.h"

#include <glib.h>

KeyBindingTree *keyboard_firstnode;

typedef struct {
    guint state;
    ObClient *client;
    GSList *actions;
    ObFrameContext context;
} ObInteractiveState;

static GSList *interactive_states;

static KeyBindingTree *curpos;

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
        grab_for_window(((ObClient*)it->data)->window, grab);
}

static gboolean chain_timeout(gpointer data)
{
    keyboard_reset_chains();

    return FALSE; /* don't repeat */
}

void keyboard_reset_chains()
{
    ob_main_loop_timeout_remove(ob_main_loop, chain_timeout);

    if (curpos) {
        grab_keys(FALSE);
        curpos = NULL;
        grab_keys(TRUE);
    }
}

void keyboard_unbind_all()
{
    tree_destroy(keyboard_firstnode);
    keyboard_firstnode = NULL;
    grab_keys(FALSE);
    curpos = NULL;
}

gboolean keyboard_bind(GList *keylist, ObAction *action)
{
    KeyBindingTree *tree, *t;
    gboolean conflict;
    gboolean mods = TRUE;

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

    if (conflict) {
        g_warning("conflict with binding");
        tree_destroy(tree);
        return FALSE;
    }

    /* find if every key in this chain has modifiers, and also find the
       bottom node of the tree */
    while (t->first_child) {
        if (!t->state)
            mods = FALSE;
        t = t->first_child;
    }

    /* when there are no modifiers in the binding, then the action cannot
       be interactive */
    if (!mods && action->data.any.interactive) {
        action->data.any.interactive = FALSE;
        action->data.inter.final = TRUE;
    }

    /* set the action */
    t->actions = g_slist_append(t->actions, action);
    /* assimilate this built tree into the main tree. assimilation
       destroys/uses the tree */
    if (tree) tree_assimilate(tree);

    return TRUE;
}

gboolean keyboard_interactive_grab(guint state, ObClient *client,
                                   ObAction *action)
{
    ObInteractiveState *s;

    g_assert(action->data.any.interactive);

    if (!interactive_states) {
        if (!grab_keyboard(TRUE))
            return FALSE;
        if (!grab_pointer(TRUE, OB_CURSOR_NONE)) {
            grab_keyboard(FALSE);
            return FALSE;
        }
    }

    s = g_new(ObInteractiveState, 1);

    s->state = state;
    s->client = client;
    s->actions = g_slist_append(NULL, action);

    interactive_states = g_slist_append(interactive_states, s);

    return TRUE;
}

void keyboard_interactive_end(ObInteractiveState *s,
                              guint state, gboolean cancel, Time time)
{
    action_run_interactive(s->actions, s->client, state, time, cancel, TRUE);

    g_slist_free(s->actions);
    g_free(s);

    interactive_states = g_slist_remove(interactive_states, s);

    if (!interactive_states) {
        grab_keyboard(FALSE);
        grab_pointer(FALSE, OB_CURSOR_NONE);
        keyboard_reset_chains();
    }
}

void keyboard_interactive_end_client(ObClient *client, gpointer data)
{
    GSList *it, *next;

    for (it = interactive_states; it; it = next) {
        ObInteractiveState *s = it->data;

        next = g_slist_next(it);

        if (s->client == client)
            s->client = NULL;
    }
}

gboolean keyboard_process_interactive_grab(const XEvent *e, ObClient **client)
{
    GSList *it, *next;
    gboolean handled = FALSE;
    gboolean done = FALSE;
    gboolean cancel = FALSE;

    for (it = interactive_states; it; it = next) {
        ObInteractiveState *s = it->data;

        next = g_slist_next(it);
        
        if ((e->type == KeyRelease && 
             !(s->state & e->xkey.state)))
            done = TRUE;
        else if (e->type == KeyPress) {
            /*if (e->xkey.keycode == ob_keycode(OB_KEY_RETURN))
                done = TRUE;
            else */if (e->xkey.keycode == ob_keycode(OB_KEY_ESCAPE))
                cancel = done = TRUE;
        }
        if (done) {
            keyboard_interactive_end(s, e->xkey.state, cancel, e->xkey.time);

            handled = TRUE;
        } else
            *client = s->client;
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
            p->state == e->xkey.state)
        {
            if (p->first_child != NULL) { /* part of a chain */
                ob_main_loop_timeout_remove(ob_main_loop, chain_timeout);
                /* 5 second timeout for chains */
                ob_main_loop_timeout_add(ob_main_loop, 5 * G_USEC_PER_SEC,
                                         chain_timeout, NULL, NULL);
                grab_keys(FALSE);
                curpos = p;
                grab_keys(TRUE);
            } else {

                keyboard_reset_chains();

                action_run_key(p->actions, client, e->xkey.state,
                               e->xkey.x_root, e->xkey.y_root,
                               e->xkey.time);
            }
            break;
        }
        p = p->next_sibling;
    }
}

gboolean keyboard_interactively_grabbed()
{
    return !!interactive_states;
}

void keyboard_startup(gboolean reconfig)
{
    grab_keys(TRUE);

    if (!reconfig)
        client_add_destructor(keyboard_interactive_end_client, NULL);
}

void keyboard_shutdown(gboolean reconfig)
{
    GSList *it;

    if (!reconfig)
        client_remove_destructor(keyboard_interactive_end_client);

    for (it = interactive_states; it; it = g_slist_next(it))
        g_free(it->data);
    g_slist_free(interactive_states);
    interactive_states = NULL;

    ob_main_loop_timeout_remove(ob_main_loop, chain_timeout);

    keyboard_unbind_all();
}

