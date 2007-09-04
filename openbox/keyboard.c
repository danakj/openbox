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
#include "actions.h"
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

KeyBindingTree *keyboard_firstnode = NULL;
static ObPopup *popup = NULL;
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
    if (curpos == newpos) return;

    grab_keys(FALSE);
    curpos = newpos;
    grab_keys(TRUE);

    if (curpos != NULL) {
        gchar *text = NULL;
        GList *it;
        Rect *a;

        for (it = curpos->keylist; it; it = g_list_next(it)) {
            gchar *oldtext = text;
            if (text == NULL)
                text = g_strdup(it->data);
            else
                text = g_strconcat(text, " - ", it->data, NULL);
            g_free(oldtext);
        }

        a = screen_physical_area_active();
        popup_position(popup, NorthWestGravity, a->x + 10, a->y + 10);
        /* 1 second delay for the popup to show */
        popup_delay_show(popup, G_USEC_PER_SEC, text);
        g_free(text);
        g_free(a);
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

void keyboard_unbind_all(void)
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

gboolean keyboard_bind(GList *keylist, ObActionsAct *action)
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

    if (conflict) {
        g_message(_("Conflict with key binding in config file"));
        tree_destroy(tree);
        return FALSE;
    }

    /* find the bottom node */
    for (; t->first_child; t = t->first_child);

    /* set the action */
    t->actions = g_slist_append(t->actions, action);
    /* assimilate this built tree into the main tree. assimilation
       destroys/uses the tree */
    if (tree) tree_assimilate(tree);

    return TRUE;
}

#if 0
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
#endif

void keyboard_event(ObClient *client, const XEvent *e)
{
    KeyBindingTree *p;

    if (e->type == KeyRelease) {
        grab_key_passive_count(-1);
        return;
    }

    g_assert(e->type == KeyPress);
    grab_key_passive_count(1);

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
                GSList *it;

                for (it = p->actions; it; it = g_slist_next(it))
                    if (actions_act_is_interactive(it->data)) break;
                if (it == NULL) /* reset if the actions are not interactive */
                    keyboard_reset_chains(0);

                actions_run_acts(p->actions, OB_USER_ACTION_KEYBOARD_KEY,
                                 e->xkey.state, e->xkey.x_root, e->xkey.y_root,
                                 0, OB_FRAME_CONTEXT_NONE, client);
            }
            break;
        }
        p = p->next_sibling;
    }
}

void keyboard_startup(gboolean reconfig)
{
    grab_keys(TRUE);
    popup = popup_new();
    popup_set_text_align(popup, RR_JUSTIFY_CENTER);
}

void keyboard_shutdown(gboolean reconfig)
{
    ob_main_loop_timeout_remove(ob_main_loop, chain_timeout);

    keyboard_unbind_all();
    set_curpos(NULL);

    popup_free(popup);
    popup = NULL;
}

