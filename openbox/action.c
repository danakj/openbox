/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   action.c for the Openbox window manager
   Copyright (c) 2007-2011   Dana Jansens

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

#include "action.h"
#include "action_list.h"
#include "gettext.h"
#include "grab.h"
#include "screen.h"
#include "event.h"
#include "config.h"
#include "client.h"
#include "focus.h"
#include "openbox.h"
#include "debug.h"

#include "actions/all.h"

static void     action_definition_ref(ObActionDefinition *def);
static void     action_definition_unref(ObActionDefinition *def);
static gboolean action_interactive_begin_act(ObAction *act, guint state);
static void     action_interactive_end_act();
static ObAction* action_find_by_name(const gchar *name);

static ObAction *interactive_act = NULL;
static guint     interactive_initial_state = 0;

struct _ObActionDefinition {
    guint ref;

    gchar *name;

    gboolean canbeinteractive;
    union {
        ObActionIDataSetupFunc i;
        ObActionDataSetupFunc n;
    } setup;
    ObActionDataFreeFunc free;
    ObActionRunFunc run;
    ObActionShutdownFunc shutdown;
};

struct _ObAction {
    guint ref;

    ObActionDefinition *def;
    ObActionIPreFunc i_pre;
    ObActionIInputFunc i_input;
    ObActionICancelFunc i_cancel;
    ObActionIPostFunc i_post;
    gpointer options;
};

static GSList *registered = NULL;

void action_startup(gboolean reconfig)
{
    if (reconfig) return;

    actions_all_startup();
}

void action_shutdown(gboolean reconfig)
{
    action_interactive_cancel_act();

    if (reconfig) return;

    /* free all the registered actions */
    while (registered) {
        ObActionDefinition *d = registered->data;
        if (d->shutdown) d->shutdown();
        action_definition_unref(d);
        registered = g_slist_delete_link(registered, registered);
    }
}

ObActionDefinition* do_register(const gchar *name,
                                ObActionDataFreeFunc free,
                                ObActionRunFunc run)
{
    GSList *it;
    ObActionDefinition *def;

    g_assert(run != NULL);

    for (it = registered; it; it = g_slist_next(it)) {
        def = it->data;
        if (!g_ascii_strcasecmp(name, def->name)) /* already registered */
            return NULL;
    }

    def = g_slice_new(ObActionDefinition);
    def->ref = 1;
    def->name = g_strdup(name);
    def->free = free;
    def->run = run;
    def->shutdown = NULL;

    registered = g_slist_prepend(registered, def);
    return def;
}

gboolean action_register_i(const gchar *name,
                           ObActionIDataSetupFunc setup,
                           ObActionDataFreeFunc free,
                           ObActionRunFunc run)
{
    ObActionDefinition *def = do_register(name, free, run);
    if (def) {
        def->canbeinteractive = TRUE;
        def->setup.i = setup;
    }
    return def != NULL;
}

gboolean action_register(const gchar *name,
                         ObActionDataSetupFunc setup,
                         ObActionDataFreeFunc free,
                         ObActionRunFunc run)
{
    ObActionDefinition *def = do_register(name, free, run);
    if (def) {
        def->canbeinteractive = FALSE;
        def->setup.n = setup;
    }
    return def != NULL;
}

gboolean action_set_shutdown(const gchar *name,
                             ObActionShutdownFunc shutdown)
{
    GSList *it;
    ObActionDefinition *def;

    for (it = registered; it; it = g_slist_next(it)) {
        def = it->data;
        if (!g_ascii_strcasecmp(name, def->name)) {
            def->shutdown = shutdown;
            return TRUE;
        }
    }
    return FALSE;
}

static void action_definition_ref(ObActionDefinition *def)
{
    ++def->ref;
}

static void action_definition_unref(ObActionDefinition *def)
{
    if (def && --def->ref == 0) {
        g_free(def->name);
        g_slice_free(ObActionDefinition, def);
    }
}

static ObAction* action_find_by_name(const gchar *name)
{
    GSList *it;
    ObActionDefinition *def = NULL;
    ObAction *act = NULL;

    /* find the requested action */
    for (it = registered; it; it = g_slist_next(it)) {
        def = it->data;
        if (!g_ascii_strcasecmp(name, def->name))
            break;
        def = NULL;
    }

    /* if we found the action */
    if (def) {
        act = g_slice_new(ObAction);
        act->ref = 1;
        act->def = def;
        action_definition_ref(act->def);
        act->i_pre = NULL;
        act->i_input = NULL;
        act->i_cancel = NULL;
        act->i_post = NULL;
        act->options = NULL;
    } else
        g_message(_("Invalid action \"%s\" requested. No such action exists."),
                  name);

    return act;
}

ObAction* action_new(const gchar *name, GHashTable *config)
{
    ObAction *act = NULL;

    act = action_find_by_name(name);
    if (act) {
        /* there is more stuff to parse here */
        if (act->def->canbeinteractive) {
            if (act->def->setup.i)
                act->options = act->def->setup.i(config,
                                                 &act->i_pre,
                                                 &act->i_input,
                                                 &act->i_cancel,
                                                 &act->i_post);
        }
        else {
            if (act->def->setup.n)
                act->options = act->def->setup.n(config);
        }
    }

    return act;
}

gboolean action_is_interactive(ObAction *act)
{
    return act->i_input != NULL;
}

void action_ref(ObAction *act)
{
    ++act->ref;
}

void action_unref(ObAction *act)
{
    if (act && --act->ref == 0) {
        /* free the action specific options */
        if (act->def->free)
            act->def->free(act->options);
        /* unref the definition */
        action_definition_unref(act->def);
        g_slice_free(ObAction, act);
    }
}

static void action_setup_data(ObActionData *data,
                              ObUserAction uact,
                              guint state,
                              gint x,
                              gint y,
                              gint button,
                              ObFrameContext con,
                              struct _ObClient *client)
{
    data->uact = uact;
    data->state = state;
    data->x = x;
    data->y = y;
    data->button = button;
    data->context = con;
    data->client = client;
}

gboolean action_run_acts(ObActionList *acts,
                         ObUserAction uact,
                         guint state,
                         gint x,
                         gint y,
                         gint button,
                         ObFrameContext con,
                         struct _ObClient *client)
{
    gboolean ran_interactive;
    gboolean update_user_time;

    /* Don't allow saving the initial state when running things from the
       menu */
    if (uact == OB_USER_ACTION_MENU_SELECTION)
        state = 0;
    /* If x and y are < 0 then use the current pointer position */
    if (x < 0 && y < 0)
        screen_pointer_pos(&x, &y);

    ran_interactive = FALSE;
    update_user_time = FALSE;
    while (acts) {
        ObAction *act;
        ObActionData data;
        gboolean ok = TRUE;

        if (acts->isfilter) {
            g_warning("filters not implemented!");
            acts = acts->next;
            continue;
        }
        else {
            act = acts->u.action;
        }

        action_setup_data(&data, uact, state, x, y, button, con, client);

        /* if they have the same run function, then we'll assume they are
           cooperating and not cancel eachother out */
        if (!interactive_act || interactive_act->def->run != act->def->run) {
            if (action_is_interactive(act)) {
                /* cancel the old one */
                if (interactive_act)
                    action_interactive_cancel_act();
                if (act->i_pre)
                    if (!act->i_pre(state, act->options))
                        act->i_input = NULL; /* remove the interactivity */
                ran_interactive = TRUE;
            }
            /* check again cuz it might have been cancelled */
            if (action_is_interactive(act)) {
                ok = action_interactive_begin_act(act, state);
                ran_interactive = TRUE;
            }
        }

        /* fire the action's run function with this data */
        if (ok) {
            if (!act->def->run(&data, act->options)) {
                if (action_is_interactive(act))
                    action_interactive_end_act();
                if (client && client == focus_client)
                    update_user_time = TRUE;
            } else {
                /* make sure its interactive if it returned TRUE */
                g_assert(act->i_input);

                /* no actions are run after the interactive one */
                break;
            }
        }
        acts = acts->next;
    }
    if (update_user_time)
        event_update_user_time();
    return ran_interactive;
}

gboolean action_interactive_act_running(void)
{
    return interactive_act != NULL;
}

void action_interactive_cancel_act(void)
{
    if (interactive_act) {
        if (interactive_act->i_cancel)
            interactive_act->i_cancel(interactive_act->options);
        action_interactive_end_act();
    }
}

static gboolean action_interactive_begin_act(ObAction *act, guint state)
{
    if (grab_keyboard()) {
        interactive_act = act;
        action_ref(interactive_act);

        interactive_initial_state = state;

        /* if using focus_delay, stop the timer now so that focus doesn't go
           moving on us, which would kill the action */
        event_halt_focus_delay();

        return TRUE;
    }
    else
        return FALSE;
}

static void action_interactive_end_act(void)
{
    if (interactive_act) {
        ObAction *ia = interactive_act;

        /* set this to NULL first so the i_post() function can't cause this to
           get called again (if it decides it wants to cancel any ongoing
           interactive action). */
        interactive_act = NULL;

        ungrab_keyboard();

        if (ia->i_post)
            ia->i_post(ia->options);

        action_unref(ia);
    }
}

gboolean action_interactive_input_event(XEvent *e)
{
    gboolean used = FALSE;
    if (interactive_act) {
        if (!interactive_act->i_input(interactive_initial_state, e,
                                      grab_input_context(),
                                      interactive_act->options, &used))
        {
            used = TRUE; /* if it cancelled the action then it has to of
                            been used */
            action_interactive_end_act();
        }
    }
    return used;
}

void action_client_move(ObActionData *data, gboolean start)
{
    static gulong ignore_start = 0;
    if (start)
        ignore_start = event_start_ignore_all_enters();
    else if (config_focus_follow &&
             data->context != OB_FRAME_CONTEXT_CLIENT)
    {
        if (data->uact == OB_USER_ACTION_MOUSE_PRESS) {
            struct _ObClient *c;

            /* usually this is sorta redundant, but with a press action
               that moves windows our from under the cursor, the enter
               event will come as a GrabNotify which is ignored, so this
               makes a fake enter event

               don't do this if there is a grab on the pointer.  enter events
               are ignored during a grab, so don't force fake ones when they
               should be ignored
            */
            if (!grab_on_pointer()) {
                if ((c = client_under_pointer()) && c != data->client) {
                    ob_debug_type(OB_DEBUG_FOCUS,
                                  "Generating fake enter because we did a "
                                  "mouse-event action");
                    event_enter_client(c);
                }
                else if (!c && c != data->client) {
                    ob_debug_type(OB_DEBUG_FOCUS,
                                  "Generating fake leave because we did a "
                                  "mouse-event action");
                    event_enter_client(data->client);
                }
            }
        }
        else if (!data->button && !config_focus_under_mouse)
            event_end_ignore_all_enters(ignore_start);
    }
}
