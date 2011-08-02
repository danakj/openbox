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
#include "action_list_run.h"
#include "action_filter.h"
#include "gettext.h"
#include "grab.h"
#include "screen.h"
#include "event.h"
#include "config.h"
#include "client.h"
#include "client_set.h"
#include "focus.h"
#include "openbox.h"
#include "debug.h"

#include "actions/_all.h"

static void     action_definition_ref(ObActionDefinition *def);
static void     action_definition_unref(ObActionDefinition *def);
static gboolean action_interactive_begin_act(ObAction *act, guint state);
static void     action_interactive_end_act();
static ObAction* action_find_by_name(const gchar *name);

static ObAction *current_i_act = NULL;
static guint     current_i_initial_state = 0;

struct _ObActionDefinition {
    guint ref;

    gchar *name;

    gboolean canbeinteractive;
    ObActionDefaultFilter def_filter;
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

    actions__all_startup();
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
                                ObActionDefaultFilter def_filter,
                                ObActionDataFreeFunc free,
                                ObActionRunFunc run)
{
    GSList *it;
    ObActionDefinition *def;

    g_return_val_if_fail(def_filter < OB_NUM_ACTION_DEFAULT_FILTERS, NULL);
    g_return_val_if_fail(run != NULL, NULL);

    for (it = registered; it; it = g_slist_next(it)) {
        def = it->data;
        if (!g_ascii_strcasecmp(name, def->name)) /* already registered */
            return NULL;
    }

    def = g_slice_new(ObActionDefinition);
    def->ref = 1;
    def->name = g_strdup(name);
    def->def_filter = def_filter;
    def->free = free;
    def->run = run;
    def->shutdown = NULL;

    registered = g_slist_prepend(registered, def);
    return def;
}

gboolean action_register_i(const gchar *name,
                           ObActionDefaultFilter def_filter,
                           ObActionIDataSetupFunc setup,
                           ObActionDataFreeFunc free,
                           ObActionRunFunc run)
{
    ObActionDefinition *def = do_register(name, def_filter, free, run);
    if (def) {
        def->canbeinteractive = TRUE;
        def->setup.i = setup;
    }
    return def != NULL;
}

gboolean action_register(const gchar *name,
                         ObActionDefaultFilter def_filter,
                         ObActionDataSetupFunc setup,
                         ObActionDataFreeFunc free,
                         ObActionRunFunc run)
{
    ObActionDefinition *def = do_register(name, def_filter, free, run);
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

gboolean action_run(ObAction *act, const ObActionListRun *data,
                    struct _ObClientSet *set)
{
    gboolean ran_interactive;
    gboolean update_user_time;
    gboolean run, run_i;

    ran_interactive = FALSE;
    update_user_time = FALSE;

    /* If we're starting an interactive action:
       - if the current interactive action is the same, do nothing and
         just use the run function.
       - otherwise...
       - cancel the current interactive action (if any)
       - run the pre function. if it returns false then the action will
         not be treated as interactive.
       - set up for a new interactive action with action_interactive_begin_act.
         this may fail in which case we don't run the action at all.
       Then execute the action's run function.
       If the action is doing something to the currently focused window,
         then we want to update its user_time to indicate it was used by a
         human now.
    */

    run_i = FALSE;
    if (action_is_interactive(act)) {
        ObActionRunFunc this_run = act->def->run;
        ObActionRunFunc i_run = (current_i_act ?
                                 current_i_act->def->run : NULL);

        if (i_run && i_run != this_run)
            action_interactive_cancel_act();
        run_i = TRUE;
        if (i_run != this_run && act->i_pre)
            run_i = act->i_pre(data->mod_state, act->options);
    }

    run = TRUE;
    if (run_i) {
        run = action_interactive_begin_act(act, data->mod_state);
        ran_interactive = TRUE;
    }

    if (run) {
        gboolean end;

        /* XXX pass the set here */
        end = !act->def->run(data, act->options);
        g_assert(end || action_is_interactive(act));

        if (end) {
            if (action_is_interactive(act))
                action_interactive_end_act();
            /* XXX else if (client_set_contains(focus_client)) */
            else if (data->target && data->target == focus_client)
                event_update_user_time();
        }
    }

    return ran_interactive;
}

gboolean action_interactive_act_running(void)
{
    return current_i_act != NULL;
}

void action_interactive_cancel_act(void)
{
    if (current_i_act) {
        if (current_i_act->i_cancel)
            current_i_act->i_cancel(current_i_act->options);
        action_interactive_end_act();
    }
}

static gboolean action_interactive_begin_act(ObAction *act, guint state)
{
    if (grab_keyboard()) {
        current_i_act = act;
        action_ref(current_i_act);

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
    if (current_i_act) {
        ObAction *ia = current_i_act;

        /* set this to NULL first so the i_post() function can't cause this to
           get called again (if it decides it wants to cancel any ongoing
           interactive action). */
        current_i_act = NULL;

        ungrab_keyboard();

        if (ia->i_post)
            ia->i_post(ia->options);

        action_unref(ia);
    }
}

gboolean action_interactive_input_event(XEvent *e)
{
    gboolean used = FALSE;
    if (current_i_act) {
        if (!current_i_act->i_input(current_i_initial_state, e,
                                    grab_input_context(),
                                    current_i_act->options, &used))
        {
            used = TRUE; /* if it cancelled the action then it has to of
                            been used */
            action_interactive_end_act();
        }
    }
    return used;
}

void action_client_move(const ObActionListRun *data, gboolean start)
{
    static gulong ignore_start = 0;
    if (start)
        ignore_start = event_start_ignore_all_enters();
    else if (config_focus_follow &&
             data->pointer_context != OB_FRAME_CONTEXT_CLIENT)
    {
        if (data->user_act == OB_USER_ACTION_MOUSE_PRESS) {
            /* usually this is sorta redundant, but with a press action
               that moves windows our from under the cursor, the enter
               event will come as a GrabNotify which is ignored, so this
               makes a fake enter event

               don't do this if there is a grab on the pointer.  enter events
               are ignored during a grab, so don't force fake ones when they
               should be ignored
            */
            if (!grab_on_pointer()) {
                struct _ObClient *under = client_under_pointer();
                if (under && under != data->pointer_over) {
                    ob_debug_type(OB_DEBUG_FOCUS,
                                  "Generating fake enter because we did a "
                                  "mouse-event action");
                    event_enter_client(under);
                }
                else if (!under && under != data->pointer_over) {
                    ob_debug_type(OB_DEBUG_FOCUS,
                                  "Generating fake leave because we did a "
                                  "mouse-event action");
                    event_enter_client(data->target);
                }
            }
        }
        else if (!data->pointer_button && !config_focus_under_mouse)
            event_end_ignore_all_enters(ignore_start);
    }
}

ObActionDefaultFilter action_default_filter(ObAction *act)
{
    return act->def->def_filter;
}
