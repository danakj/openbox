/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   actions_value.c for the Openbox window manager
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

#include "actions_value.h"
#include "actions_list.h"
#include "geom.h"

#include "stdlib.h"

struct _ObActionsValue {
    gint ref;
    enum {
        OB_AV_STRING,
        OB_AV_ACTIONSLIST
    } type;
    union {
        gchar *string;
        gboolean boolean;
        guint integer;
        ObActionsList *actions;
    } v;
};

ObActionsValue* actions_value_new_string(const gchar *s)
{
    return actions_value_new_string_steal(g_strdup(s));
}

ObActionsValue* actions_value_new_string_steal(gchar *s)
{
    ObActionsValue *v = g_slice_new(ObActionsValue);
    v->ref = 1;
    v->type = OB_AV_STRING;
    v->v.string = s;
    return v;
}

ObActionsValue* actions_value_new_actions_list(ObActionsList *al)
{
    ObActionsValue *v = g_slice_new(ObActionsValue);
    v->ref = 1;
    v->type = OB_AV_ACTIONSLIST;
    v->v.actions = al;
    actions_list_ref(al);
    return v;
}

void actions_value_ref(ObActionsValue *v)
{
    ++v->ref;
}

void actions_value_unref(ObActionsValue *v)
{
    if (v && --v->ref < 1) {
        switch (v->type) {
        case OB_AV_STRING:
            g_free(v->v.string);
            break;
        case OB_AV_ACTIONSLIST:
            actions_list_unref(v->v.actions);
            break;
        }
        g_slice_free(ObActionsValue, v);
    }
}

gboolean actions_value_is_string(ObActionsValue *v)
{
    return v->type == OB_AV_STRING;
}

gboolean actions_value_is_actions_list(ObActionsValue *v)
{
    return v->type == OB_AV_ACTIONSLIST;
}

const gchar* actions_value_string(ObActionsValue *v)
{
    g_return_val_if_fail(v->type == OB_AV_STRING, NULL);
    return v->v.string;
}

gboolean actions_value_bool(ObActionsValue *v)
{
    g_return_val_if_fail(v->type == OB_AV_STRING, FALSE);
    if (g_strcasecmp(v->v.string, "true") == 0 ||
        g_strcasecmp(v->v.string, "yes") == 0)
        return TRUE;
    else
        return FALSE;
}

gint actions_value_int(ObActionsValue *v)
{
    gchar *s;

    g_return_val_if_fail(v->type == OB_AV_STRING, 0);
    s = v->v.string;
    return strtol(s, &s, 10);
}

void actions_value_fraction(ObActionsValue *v, gint *numer, gint *denom)
{
    gchar *s;

    *numer = *denom = 0;

    g_return_if_fail(v->type == OB_AV_STRING);
    s = v->v.string;

    *numer = strtol(s, &s, 10);
    if (*s == '%')
        *denom = 100;
    else if (*s == '/')
        *denom = atoi(s+1);
}

void actions_value_gravity_coord(ObActionsValue *v, GravityCoord *c)
{
    gchar *s;

    c->center = FALSE;
    c->pos = 0;
    c->denom = 0;

    g_return_if_fail(v->type == OB_AV_STRING);
    s = v->v.string;

    if (!g_ascii_strcasecmp(s, "center"))
        c->center = TRUE;
    else {
        if (s[0] == '-')
            c->opposite = TRUE;
        if (s[0] == '-' || s[0] == '+')
            ++s;

        c->pos = strtol(s, &s, 10);

        if (*s == '%')
            c->denom = 100;
        else if (*s == '/')
            c->denom = atoi(s+1);
    }
}

ObActionsList* actions_value_actions_list(ObActionsValue *v)
{
    g_return_val_if_fail(v->type == OB_AV_ACTIONSLIST, NULL);
    return v->v.actions;
}
