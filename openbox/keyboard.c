/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   keyboard.c for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
   Copyright (c) 2003-2007   Dana Jansens

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
#include "menuframe.h"
#include "config.h"
#include "keytree.h"
#include "keyboard.h"
#include "translate.h"
#include "moveresize.h"
#include "popup.h"
#include "gettext.h"

#include <glib.h>

typedef struct {
    gboolean active;
    guint state;
    ObClient *client;
    ObAction *action;
} ObInteractiveState;

KeyBindingTree *keyboard_firstnode = NULL;
static ObPopup *popup = NULL;
static ObInteractiveState istate;
static KeyBindingTree *curpos;

static void grab_keys(gboolean grab)
{
    KeyBindingTree *p;

    ungrab_all_keys(RootWindow(ob_display, ob_screen));

    if (grab) {
        p = curpos ? curpos->first_child : keyboard_firstnode;
        while (p) {
            grab_key(p->key, p->state, RootWindow(ob_display, ob_screen),
                     GrabModeAsync);
            p = p->next_sibling;
        }
        if (curpos)
            grab_key(config_keyboard_reset_keycode,
                     config_keyboard_reset_state,
                     RootWindow(ob_display, ob_screen), GrabModeAsync);
    }
}

static gboolean chain_timeout(gpointer data)
{
    keyboard_reset_chains(0);
    return FALSE; /* don't repeat */
}

static void set_curpos(KeyBindingTree *newpos)
{
    if (curpos != newpos) {
        grab_keys(FALSE);
        curpos = newpos;
        grab_keys(TRUE);
    }

    if (curpos != NULL) {
        gchar *text = NULL;
        GList *it;

        for (it = curpos->keylist; it; it = g_list_next(it)) {
            gchar *oldtext = text;
            if (text == NULL)
                text = g_strdup(it->data);
            else
                text = g_strconcat(text, " - ", it->data, NULL);
            g_free(oldtext);
        }

        popup_position(popup, NorthWestGravity, 10, 10);
        /* 1 second delay for the popup to show */
        popup_delay_show(popup, G_USEC_PER_SEC, text);
        g_free(text);
    } else {
        popup_hide(popup);
    }
}

void keyboard_reset_chains(gint break_chroots)
{
    KeyBindingTree *p;

    for (p = curpos; p; p = p->parent) {
        if (p->chroot) {
            if (break_chroots == 0) break; /* stop here */
            if (break_chroots > 0)
                --break_chroots;
        }
    }
    set_curpos(p);
}

void keyboard_unbind_all()
{
    tree_destroy(keyboard_firstnode);
    keyboard_firstnode = NULL;
}

void keyboard_chroot(GList *keylist)
{
    /* try do it in the existing tree. if we can't that means it is an empty
       chroot binding. so add it to the tree then. */
    if (!tree_chroot(keyboard_firstnode, keylist)) {
        KeyBindingTree *tree;
        if (!(tree = tree_build(keylist)))
            return;
        tree_chroot(tree, keylist);
        tree_assimilate(tree);
    }
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
        g_message(_("Conflict with key binding in config file"));
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

static void keyboard_interactive_end(guint state, gboolean cancel, Time time,
                                     gboolean ungrab)
{
    GSList *alist;

    g_assert(istate.active);

    /* ungrab first so they won't be NotifyWhileGrabbed */
    if (ungrab)
        grab_keyboard(FALSE);

    alist = g_slist_append(NULL, istate.action);
    action_run_interactive(alist, istate.client, state, time, cancel, TRUE);
    g_slist_free(alist);

    istate.active = FALSE;
}

static void keyboard_interactive_end_client(ObClient *client, gpointer data)
{
    if (istate.active && istate.client == client)
        istate.client = NULL;
}

gboolean keyboard_interactive_grab(guint state, ObClient *client,
                                   ObAction *action)
{
    g_assert(action->data.any.interactive);

    if (!istate.active) {
        if (!grab_keyboard(TRUE))
            return FALSE;
    } else if (action->func != istate.action->func) {
        keyboard_interactive_end(state, FALSE, action->data.any.time, FALSE);
    }

    istate.active = TRUE;
    istate.state = state;
    istate.client = client;
    istate.action = action;

    return TRUE;
}

gboolean keyboard_process_interactive_grab(const XEvent *e, ObClient **client)
{
    gboolean handled = FALSE;
    gboolean done = FALSE;
    gboolean cancel = FALSE;

    if (istate.active) {
        if ((e->type == KeyRelease && !(istate.state & e->xkey.state))) {
            done = TRUE;
            handled = TRUE;
        } else if (e->type == KeyPress) {
            /*if (e->xkey.keycode == ob_keycode(OB_KEY_RETURN))
              done = TRUE;
              else */if (e->xkey.keycode == ob_keycode(OB_KEY_ESCAPE)) {
                  cancel = done = TRUE;
                  handled = TRUE;
              }
        } else if (e->type == ButtonPress) {
            cancel = TRUE;
            done = TRUE;
            handled = FALSE;
        }

        if (done)
            keyboard_interactive_end(e->xkey.state, cancel, e->xkey.time,TRUE);

        if (handled)
            *client = istate.client;
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
        ob_main_loop_timeout_remove(ob_main_loop, chain_timeout);
        keyboard_reset_chains(-1);
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
            /* if we hit a key binding, then close any open menus and run it */
            if (menu_frame_visible)
                menu_frame_hide_all();

            if (p->first_child != NULL) { /* part of a chain */
                ob_main_loop_timeout_remove(ob_main_loop, chain_timeout);
                /* 3 second timeout for chains */
                ob_main_loop_timeout_add(ob_main_loop, 3 * G_USEC_PER_SEC,
                                         chain_timeout, NULL,
                                         g_direct_equal, NULL);
                set_curpos(p);
            } else if (p->chroot)         /* an empty chroot */
                set_curpos(p);
            else {
                keyboard_reset_chains(0);

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
    return istate.active;
}

void keyboard_startup(gboolean reconfig)
{
    grab_keys(TRUE);
    popup = popup_new(FALSE);

    if (!reconfig)
        client_add_destructor(keyboard_interactive_end_client, NULL);
}

void keyboard_shutdown(gboolean reconfig)
{
    if (!reconfig)
        client_remove_destructor(keyboard_interactive_end_client);

    if (istate.active)
        keyboard_interactive_end(0, TRUE, 0, TRUE);

    ob_main_loop_timeout_remove(ob_main_loop, chain_timeout);

    keyboard_unbind_all();
    set_curpos(NULL);

    popup_free(popup);
    popup = NULL;
}

