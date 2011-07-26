/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   actions_list.h for the Openbox window manager
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

#include "actions.h"

#include <glib.h>

typedef struct _ObActionsList      ObActionsList;
typedef struct _ObActionsListTest  ObActionsListTest;
typedef struct _ObActionsListValue ObActionsListValue;

/*! Each node of the Actions list is an action itself (or a filter bound to
  an action). */
struct _ObActionsList {
    gint ref;
    gboolean isfilter;
    union {
        struct _ObActionsListFilter {
            ObActionsListTest *test; /* can be null */
            ObActionsList *thendo; /* can be null */
            ObActionsList *elsedo; /* can be null */
        } f;
        ObActionsAct *action;
    } u;
    ObActionsList *next;
};

struct _ObActionsListTest {
    gchar *key;
    ObActionsListValue *value; /* can be null */
    gboolean and;
    ObActionsListTest *next;
};

void actions_list_ref(ObActionsList *l);
void actions_list_unref(ObActionsList *l);

void actions_list_test_destroy(ObActionsListTest *t);

/*! Creates a new value by making a copy of the given string. */
ObActionsListValue* actions_list_value_new_string(const gchar *s);
/*! Creates a new value from a string, and steals ownership of the string. It
  will be freed when then value is destroyed. */
ObActionsListValue* actions_list_value_new_string_steal(gchar *s);
/*! Creates a new value that holds an integer. */
ObActionsListValue* actions_list_value_new_int(gint i);
/*! Creates a new value with a given actions list. */
ObActionsListValue* actions_list_value_new_actions_list(ObActionsList *al);

void actions_list_value_ref(ObActionsListValue *v);
void actions_list_value_unref(ObActionsListValue *v);

gboolean actions_list_value_is_string(ObActionsListValue *v);
gboolean actions_list_value_is_int(ObActionsListValue *v);
gboolean actions_list_value_is_actions_list(ObActionsListValue *v);

gchar* actions_list_value_string(ObActionsListValue *v);
gint actions_list_value_int(ObActionsListValue *v);
ObActionsList* actions_list_value_actions_list(ObActionsListValue *v);
