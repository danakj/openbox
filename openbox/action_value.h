/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   action_value.h for the Openbox window manager
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

#include <glib.h>

struct _GravityCoord;
struct _ObActionList;

typedef struct _ObActionValue ObActionValue;

/*! Creates a new value by making a copy of the given string. */
ObActionValue* action_value_new_string(const gchar *s);
/*! Creates a new value from a string, and steals ownership of the string. It
  will be freed when then value is destroyed. */
ObActionValue* action_value_new_string_steal(gchar *s);
/*! Creates a new value with a given actions list. */
ObActionValue* action_value_new_action_list(struct _ObActionList *al);

void action_value_ref(ObActionValue *v);
void action_value_unref(ObActionValue *v);

gboolean action_value_is_string(ObActionValue *v);
gboolean action_value_is_action_list(ObActionValue *v);

const gchar* action_value_string(ObActionValue *v);
gboolean action_value_bool(ObActionValue *v);
gint action_value_int(ObActionValue *v);
void action_value_fraction(ObActionValue *v, gint *numer, gint *denom);
void action_value_gravity_coord(ObActionValue *v, struct _GravityCoord *c);
struct _ObActionList* action_value_action_list(ObActionValue *v);
