/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   actions_list.c for the Openbox window manager
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

#include "actions_list.h"
#include "actions.h"
#include "actions_value.h"

#include <glib.h>

void actions_list_ref(ObActionsList *l)
{
    if (l) ++l->ref;
}

void actions_list_unref(ObActionsList *l)
{
    while (l && --l->ref < 1) {
        ObActionsList *n = l->next;

        if (l->isfilter) {
            actions_list_test_destroy(l->u.f.test);
            actions_list_unref(l->u.f.thendo);
            actions_list_unref(l->u.f.elsedo);
        }
        else {
            actions_act_unref(l->u.action);
        }
        g_slice_free(ObActionsList, l);
        l = n;
    }
}

void actions_list_test_destroy(ObActionsListTest *t)
{
    while (t) {
        ObActionsListTest *n = t->next;

        g_free(t->key);
        actions_value_unref(t->value);
        g_slice_free(ObActionsListTest, t);
        t = n;
    }
}

ObActionsList* actions_list_concat(ObActionsList *a, ObActionsList *b)
{
    ObActionsList *start = a;

    if (!start) return b;
    while (a->next) a = a->next;
    a->next = b;
    return start;
}
