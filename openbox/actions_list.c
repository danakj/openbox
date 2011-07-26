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

#include <glib.h>

struct _ObActionsListValue {
    gint ref;
    enum {
        OB_AL_STRING,
        OB_AL_INTEGER,
        OB_AL_ACTIONSLIST
    } type;
    union {
        gchar *string;
        guint integer;
        ObActionsList *actions;
    } v;
};

void actions_list_ref(ObActionsList *l)
{
    ++l->ref;
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
        actions_list_value_unref(t->value);
        g_slice_free(ObActionsListTest, t);
        t = n;
    }
}

ObActionsListValue* actions_list_value_new_string(const gchar *s)
{
    return actions_list_value_new_string_steal(g_strdup(s));
}

ObActionsListValue* actions_list_value_new_string_steal(gchar *s)
{
    ObActionsListValue *v = g_slice_new(ObActionsListValue);
    v->ref = 1;
    v->type = OB_AL_STRING;
    v->v.string = s;
    return v;
}

ObActionsListValue* actions_list_value_new_int(gint i)
{
    ObActionsListValue *v = g_slice_new(ObActionsListValue);
    v->ref = 1;
    v->type = OB_AL_INTEGER;
    v->v.integer = i;
    return v;
}

ObActionsListValue* actions_list_value_new_actions_list(ObActionsList *al)
{
    ObActionsListValue *v = g_slice_new(ObActionsListValue);
    v->ref = 1;
    v->type = OB_AL_ACTIONSLIST;
    v->v.actions = al;
    actions_list_ref(al);
    return v;
}

void actions_list_value_ref(ObActionsListValue *v)
{
    ++v->ref;
}

void actions_list_value_unref(ObActionsListValue *v)
{
    if (v && --v->ref < 1) {
        switch (v->type) {
        case OB_AL_STRING:
            g_free(v->v.string);
            break;
        case OB_AL_ACTIONSLIST:
            actions_list_unref(v->v.actions);
            break;
        case OB_AL_INTEGER:
            break;
        }
        g_slice_free(ObActionsListValue, v);
    }
}

gboolean actions_list_value_is_string(ObActionsListValue *v)
{
    return v->type == OB_AL_STRING;
}

gboolean actions_list_value_is_int(ObActionsListValue *v)
{
    return v->type == OB_AL_INTEGER;
}

gboolean actions_list_value_is_actions_list(ObActionsListValue *v)
{
    return v->type == OB_AL_ACTIONSLIST;
}

gchar* actions_list_value_string(ObActionsListValue *v)
{
    g_return_val_if_fail(v->type == OB_AL_STRING, NULL);
    return v->v.string;
}

gint actions_list_value_int(ObActionsListValue *v)
{
    g_return_val_if_fail(v->type == OB_AL_INTEGER, 0);
    return v->v.integer;
}

ObActionsList* actions_list_value_actions_list(ObActionsListValue *v)
{
    g_return_val_if_fail(v->type == OB_AL_ACTIONSLIST, NULL);
    return v->v.actions;
}

