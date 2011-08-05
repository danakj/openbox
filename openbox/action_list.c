/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   action_list.c for the Openbox window manager
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

#include "action_list.h"
#include "action.h"
#include "action_filter.h"

#include <glib.h>

void action_list_ref(ObActionList *l)
{
    if (l) ++l->ref;
}

void action_list_unref(ObActionList *l)
{
    while (l && --l->ref < 1) {
        ObActionList *n = l->next;

        if (l->isfilterset) {
            action_list_test_destroy(l->u.f.test);
            action_list_unref(l->u.f.thendo);
            action_list_unref(l->u.f.elsedo);
        }
        else {
            action_unref(l->u.action);
        }
        g_slice_free(ObActionList, l);
        l = n;
    }
}

void action_list_test_destroy(ObActionListTest *t)
{
    while (t) {
        ObActionListTest *n = t->next;

        action_filter_unref(t->filter);
        g_slice_free(ObActionListTest, t);
        t = n;
    }
}

ObActionList* action_list_concat(ObActionList *a, ObActionList *b)
{
    ObActionList *start = a;

    if (!start) return b;
    while (a->next) a = a->next;
    a->next = b;
    return start;
}
