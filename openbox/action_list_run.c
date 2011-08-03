/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   action_list_run.c for the Openbox window manager
   Copyright (c) 2011        Dana Jansens

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

#include "action_list_run.h"
#include "action.h"
#include "action_filter.h"
#include "action_list.h"
#include "client.h"
#include "client_set.h"
#include "event.h"
#include "screen.h"

static gboolean run_list(ObActionList *acts, const ObActionListRun *data,
                         ObClientSet *set);
static gboolean run_filter(ObActionList *acts, const ObActionListRun *data,
                           ObClientSet *set);

gboolean action_list_run(ObActionList *acts,
                         ObUserAction uact,
                         guint state,
                         gint x,
                         gint y,
                         gint button,
                         ObFrameContext con,
                         struct _ObClient *client)
{
    ObActionListRun action_data;

    if (acts == NULL) return FALSE;

    /* Don't save the initial mod state when running things from the menu */
    if (uact == OB_USER_ACTION_MENU_SELECTION)
        state = 0;
    /* If x and y are < 0 then use the current pointer position */
    if (x < 0 && y < 0)
        screen_pointer_pos(&x, &y);

    action_data.user_act = uact;
    action_data.mod_state = state;
    action_data.pointer_x = x;
    action_data.pointer_y = y;
    action_data.pointer_button = button;
    action_data.pointer_context = con;
    action_data.target = client;
    /* if a pointer started the event clicking on a window, it must be under
       the pointer */
    action_data.pointer_over = client ? client : client_under_pointer();

    return run_list(acts, &action_data, NULL);
}

static gboolean run_list(ObActionList *acts, const ObActionListRun *data,
                         ObClientSet *set)
{
    gboolean interactive;
    ObClientSet *myset;

    if (!acts) return FALSE;
    if (acts->isfilterset) return run_filter(acts, data, set);

    /* if we're not given a filter, then make a default filter set,
       but don't pass it on to our siblings in the list. */
    myset = set;
    if (!myset) {
        switch (action_default_filter(acts->u.action)) {
        case OB_ACTION_DEFAULT_FILTER_SINGLE:
            myset = client_set_single(data->target); break;
        case OB_ACTION_DEFAULT_FILTER_EMPTY:
            myset = client_set_empty(); break;
        case OB_ACTION_DEFAULT_FILTER_ALL:
            myset = client_set_all(); break;
        case OB_NUM_ACTION_DEFAULT_FILTERS:
        default: g_assert_not_reached();
        }
    }

    interactive = action_run(acts->u.action, data, myset);
    if (set != myset) client_set_destroy(myset);

    if (interactive) return TRUE;
    return run_list(acts->next, data, set);
}

static gboolean run_filter(ObActionList *acts, const ObActionListRun *data,
                           ObClientSet *incoming_set)
{
    ObActionListTest *test = acts->u.f.test;
    ObClientSet *set, *and_set;
    gboolean prev_and;
    gboolean interactive;

    /* (a ^ b) | c | (d ^ e ^ f) | (g ^ h)

       - for each test in the filter:
         1) when we are at the first test, we make the test's set our current
            set
         2) when we are between two ORs, we add the test's set to our current
            set
         3) when we are to the left of an OR (or at the last test), we
            intersect our test's set to the and_set, and then add the add_set
            to our current set
         4) otherwise, we are to the left of an AND
            a) if we are to the right of an OR, we make and_set our test's set
            b) else we are between two ANDs, so we intersect and_set with
               the test's set
       - finally, we take the intersection of our created set with the
         incoming set.
    */

    g_assert(test != NULL);

    and_set = NULL;

    set = action_filter_set(test->filter, data);
    prev_and = test->and;
    test = test->next;
    while (test) {
        ObClientSet *const test_set = action_filter_set(test->filter, data);

        if (!prev_and && test->next && !test->and)
            set = client_set_union(set, test_set);
        else if (!test->and || !test->next) {
            if (and_set)
                and_set = client_set_intersection(and_set, test_set);
            else 
                and_set = test_set;
            set = client_set_union(set, and_set);
            and_set = NULL;
        }
        else {
            if (and_set)
                and_set = client_set_intersection(and_set, test_set);
            else
                and_set = test_set;
        }
        test = test->next;
    }

    if (incoming_set) {
        /* we don't want to destroy the incoming set so make a copy of it */
        set = client_set_intersection(set, client_set_clone(incoming_set));
    }

    if (client_set_test_boolean(set))
        interactive = run_list(acts->u.f.thendo, data, set);
    else
        interactive = run_list(acts->u.f.elsedo, data, set);
    client_set_destroy(set);

    if (interactive) return TRUE;
    return run_list(acts->next, data, incoming_set);
}
