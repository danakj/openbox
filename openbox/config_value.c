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
        GList *list;
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
        case OB_CV_LIST: {
            GList *it;
            for (it = v->v.list; it; it = g_list_next(it))
                config_value_unref(it->data);
            g_list_free(v->v.list);
            break;
        }
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

gboolean config_value_is_list(const ObConfigValue *v)
{
    g_return_val_if_fail(v != NULL, FALSE);
    return v->type == OB_CV_LIST;
}

gboolean config_value_is_action_list(const ObConfigValue *v)
{
    g_return_val_if_fail(v != NULL, FALSE);
    return v->type == OB_CV_ACTION_LIST;
}

/**************************** pointer functions ****************************/

void config_value_copy_ptr(ObConfigValue *v,
                           ObConfigValueDataType type,
                           ObConfigValueDataPtr p,
                           const ObConfigValueEnum e[])
{
    switch (type) {
    case OB_CONFIG_VALUE_STRING:
        *p.string = config_value_string(v);
        break;
    case OB_CONFIG_VALUE_BOOLEAN:
        *p.boolean = config_value_bool(v);
        break;
    case OB_CONFIG_VALUE_INTEGER:
        *p.integer = config_value_int(v);
        break;
    case OB_CONFIG_VALUE_ENUM:
        *p.enumeration = config_value_enum(v, e);
        break;
    case OB_CONFIG_VALUE_FRACTION: {
        gint n, d;
        config_value_fraction(v, &n, &d);
        p.fraction->numer = n;
        p.fraction->denom = d;
        break;
    }
    case OB_CONFIG_VALUE_GRAVITY_COORD: {
        GravityCoord c;
        config_value_gravity_coord(v, &c);
        *p.coord = c;
        break;
    }
    case OB_CONFIG_VALUE_LIST:
        *p.list = config_value_list(v);
        break;
    case OB_CONFIG_VALUE_ACTIONLIST:
        *p.actions = config_value_action_list(v);
        break;
    case NUM_OB_CONFIG_VALUE_TYPES:
    default:
        g_assert_not_reached();
    }
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
guint config_value_enum(ObConfigValue *v, const ObConfigValueEnum choices[])
{
    const ObConfigValueEnum *e;

    g_return_val_if_fail(v != NULL, (guint)-1);
    g_return_val_if_fail(config_value_is_string(v), (guint)-1);
    g_return_val_if_fail(choices != NULL, (guint)-1);

    for (e = choices; e->name; ++e)
        if (g_strcasecmp(v->v.string, e->name) == 0)
            return e->value;
    return (guint)-1;
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
GList* config_value_list(ObConfigValue *v)
{
    g_return_val_if_fail(v != NULL, NULL);
    g_return_val_if_fail(config_value_is_list(v), NULL);
    return v->v.list;
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

ObConfigValue* config_value_new_list(GList *list)
{
    GList *c = g_list_copy(list);
    GList *it;

    for (it = c; it; it = g_list_next(it))
        config_value_ref(it->data);
    return config_value_new_list_steal(c);
}

ObConfigValue* config_value_new_list_steal(GList *list)
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
