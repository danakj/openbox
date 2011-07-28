/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   action_value.c for the Openbox window manager
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

#include "action_value.h"
#include "action_list.h"
#include "geom.h"

#include "stdlib.h"

struct _ObActionValue {
    gint ref;
    enum {
        OB_AV_STRING,
        OB_AV_ACTIONLIST
    } type;
    union {
        gchar *string;
        gboolean boolean;
        guint integer;
        ObActionList *actions;
    } v;
};

ObActionValue* action_value_new_string(const gchar *s)
{
    return action_value_new_string_steal(g_strdup(s));
}

ObActionValue* action_value_new_string_steal(gchar *s)
{
    ObActionValue *v = g_slice_new(ObActionValue);
    v->ref = 1;
    v->type = OB_AV_STRING;
    v->v.string = s;
    return v;
}

ObActionValue* action_value_new_action_list(ObActionList *al)
{
    ObActionValue *v = g_slice_new(ObActionValue);
    v->ref = 1;
    v->type = OB_AV_ACTIONLIST;
    v->v.actions = al;
    action_list_ref(al);
    return v;
}

void action_value_ref(ObActionValue *v)
{
    ++v->ref;
}

void action_value_unref(ObActionValue *v)
{
    if (v && --v->ref < 1) {
        switch (v->type) {
        case OB_AV_STRING:
            g_free(v->v.string);
            break;
        case OB_AV_ACTIONLIST:
            action_list_unref(v->v.actions);
            break;
        }
        g_slice_free(ObActionValue, v);
    }
}

gboolean action_value_is_string(ObActionValue *v)
{
    return v->type == OB_AV_STRING;
}

gboolean action_value_is_action_list(ObActionValue *v)
{
    return v->type == OB_AV_ACTIONLIST;
}

const gchar* action_value_string(ObActionValue *v)
{
    g_return_val_if_fail(v->type == OB_AV_STRING, NULL);
    return v->v.string;
}

gboolean action_value_bool(ObActionValue *v)
{
    g_return_val_if_fail(v->type == OB_AV_STRING, FALSE);
    if (g_strcasecmp(v->v.string, "true") == 0 ||
        g_strcasecmp(v->v.string, "yes") == 0)
        return TRUE;
    else
        return FALSE;
}

gint action_value_int(ObActionValue *v)
{
    gchar *s;

    g_return_val_if_fail(v->type == OB_AV_STRING, 0);
    s = v->v.string;
    return strtol(s, &s, 10);
}

void action_value_fraction(ObActionValue *v, gint *numer, gint *denom)
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

void action_value_gravity_coord(ObActionValue *v, GravityCoord *c)
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

ObActionList* action_value_action_list(ObActionValue *v)
{
    g_return_val_if_fail(v->type == OB_AV_ACTIONLIST, NULL);
    return v->v.actions;
}
