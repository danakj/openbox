/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   actions_value.h for the Openbox window manager
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
struct _ObActionsList;

typedef struct _ObActionsValue        ObActionsValue;

/*! Creates a new value by making a copy of the given string. */
ObActionsValue* actions_value_new_string(const gchar *s);
/*! Creates a new value from a string, and steals ownership of the string. It
  will be freed when then value is destroyed. */
ObActionsValue* actions_value_new_string_steal(gchar *s);
/*! Creates a new value with a given actions list. */
ObActionsValue* actions_value_new_actions_list(struct _ObActionsList *al);

void actions_value_ref(ObActionsValue *v);
void actions_value_unref(ObActionsValue *v);

gboolean actions_value_is_string(ObActionsValue *v);
gboolean actions_value_is_actions_list(ObActionsValue *v);

const gchar* actions_value_string(ObActionsValue *v);
gboolean actions_value_bool(ObActionsValue *v);
gint actions_value_int(ObActionsValue *v);
void actions_value_fraction(ObActionsValue *v, gint *numer, gint *denom);
void actions_value_gravity_coord(ObActionsValue *v, struct _GravityCoord *c);
struct _ObActionsList* actions_value_actions_list(ObActionsValue *v);
