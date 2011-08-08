/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   config_value.c for the Openbox window manager
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

#include "config_value.h"
#include "action_list.h"
#include "geom.h"

#include "stdlib.h"

struct _ObConfigValue {
    gint ref;
    enum {
        OB_CV_STRING,
        OB_CV_LIST,
        OB_CV_ACTION_LIST
    } type;
    union {
        gchar *string;
        gchar **list;
        ObActionList *actions;
    } v;
};

void config_value_ref(ObConfigValue *v)
{
    ++v->ref;
}

void config_value_unref(ObConfigValue *v)
{
    if (v && --v->ref < 1) {
        switch (v->type) {
        case OB_CV_STRING:
            g_free(v->v.string);
            break;
        case OB_CV_LIST:
            g_strfreev(v->v.list);
            break;
        case OB_CV_ACTION_LIST:
            action_list_unref(v->v.actions);
            break;
        }
        g_slice_free(ObConfigValue, v);
    }
}

/*************************** describer functions ***************************/

gboolean config_value_is_string(const ObConfigValue *v)
{
    g_return_val_if_fail(v != NULL, FALSE);
    return v->type == OB_CV_STRING;
}

gboolean config_value_is_string_list(const ObConfigValue *v)
{
    g_return_val_if_fail(v != NULL, FALSE);
    return v->type == OB_CV_LIST;
}

gboolean config_value_is_action_list(const ObConfigValue *v)
{
    g_return_val_if_fail(v != NULL, FALSE);
    return v->type == OB_CV_ACTION_LIST;
}

/***************************** getter functions ****************************/

const gchar* config_value_string(ObConfigValue *v)
{
    g_return_val_if_fail(v != NULL, NULL);
    g_return_val_if_fail(config_value_is_string(v), NULL);
    return v->v.string;
}
gboolean config_value_bool(ObConfigValue *v)
{
    g_return_val_if_fail(v != NULL, FALSE);
    g_return_val_if_fail(config_value_is_string(v), FALSE);
    return (g_strcasecmp(v->v.string, "true") == 0 ||
            g_strcasecmp(v->v.string, "yes") == 0);
}
guint config_value_int(ObConfigValue *v)
{
    gchar *s;
    g_return_val_if_fail(v != NULL, FALSE);
    g_return_val_if_fail(config_value_is_string(v), FALSE);
    s = v->v.string;
    return strtol(s, &s, 10);
}
void config_value_fraction(ObConfigValue *v, gint *numer, gint *denom)
{
    gchar *s;

    *numer = *denom = 0;

    g_return_if_fail(v != NULL);
    g_return_if_fail(config_value_is_string(v));

    s = v->v.string;
    *numer = strtol(s, &s, 10);
    if (*s == '%')
        *denom = 100;
    else if (*s == '/')
        *denom = atoi(s+1);
    else
        *denom = 0;
}
void config_value_gravity_coord(ObConfigValue *v, GravityCoord *c)
{
    gchar *s;

    c->center = FALSE;
    c->pos = 0;
    c->denom = 0;

    g_return_if_fail(v != NULL);
    g_return_if_fail(config_value_is_string(v));

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
const gchar *const* config_value_string_list(ObConfigValue *v)
{
    g_return_val_if_fail(v != NULL, NULL);
    g_return_val_if_fail(config_value_is_string_list(v), NULL);
    return (const gchar**)v->v.list;
}
ObActionList* config_value_action_list(ObConfigValue *v)
{
    g_return_val_if_fail(v != NULL, NULL);
    g_return_val_if_fail(config_value_is_action_list(v), NULL);
    return v->v.actions;
}

/****************************** constructors ******************************/

ObConfigValue* config_value_new_string(const gchar *s)
{
    g_return_val_if_fail(s != NULL, NULL);
    return config_value_new_string_steal(g_strdup(s));
}

ObConfigValue* config_value_new_string_steal(gchar *s)
{
    ObConfigValue *v;
    g_return_val_if_fail(s != NULL, NULL);
    v = g_slice_new(ObConfigValue);
    v->ref = 1;
    v->type = OB_CV_STRING;
    v->v.string = s;
    return v;
}

ObConfigValue* config_value_new_string_list(gchar **list)
{
    return config_value_new_string_list_steal(g_strdupv(list));
}

ObConfigValue* config_value_new_string_list_steal(gchar **list)
{
    ObConfigValue *v = g_slice_new(ObConfigValue);
    v->ref = 1;
    v->type = OB_CV_LIST;
    v->v.list = list;
    return v;
}

ObConfigValue* config_value_new_action_list(ObActionList *al)
{
    ObConfigValue *v = g_slice_new(ObConfigValue);
    v->ref = 1;
    v->type = OB_CV_ACTION_LIST;
    v->v.actions = al;
    action_list_ref(al);
    return v;
}
