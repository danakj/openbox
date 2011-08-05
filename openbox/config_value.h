/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   config_value.h for the Openbox window manager
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

#include "geom.h"

#include <glib.h>

struct _GravityCoord;
struct _ObActionList;

typedef struct _ObConfigValue ObConfigValue;
typedef struct _ObConfigValueEnum ObConfigValueEnum;

struct _ObConfigValueEnum {
    const gchar *name;
    guint value;
};

typedef enum {
    OB_CONFIG_VALUE_STRING,
    OB_CONFIG_VALUE_BOOLEAN,
    OB_CONFIG_VALUE_ENUM,
    OB_CONFIG_VALUE_INTEGER,
    OB_CONFIG_VALUE_FRACTION,
    OB_CONFIG_VALUE_GRAVITY_COORD,
    OB_CONFIG_VALUE_LIST,
    OB_CONFIG_VALUE_ACTIONLIST,
    NUM_OB_CONFIG_VALUE_TYPES
} ObConfigValueDataType;

/*! This holds a pointer to one of the possible types in ObConfigValueType. */
typedef union {
    const gpointer *pointer; /*!< Generic pointer */
    const gchar **string;
    gboolean *boolean;
    guint *integer;
    guint *enumeration;
    struct {
        guint numer;
        guint denom;
    } *fraction;
    GravityCoord *coord;
    const GList **list; /*!< A list of ObConfigValue objects */
    struct _ObActionList **actions;
} ObConfigValueDataPtr;

void config_value_ref(ObConfigValue *v);
void config_value_unref(ObConfigValue *v);

/*! Creates a new value by making a copy of the given string. */
ObConfigValue* config_value_new_string(const gchar *s);
/*! Creates a new value from a string, and steals ownership of the string. It
  will be freed when then value is destroyed. */
ObConfigValue* config_value_new_string_steal(gchar *s);

/*! Creates a config value which holds a list of other values
  This function copies the list and adds a refcount to the values within. */
ObConfigValue* config_value_new_list(GList *list);
/*! Creates a config value which holds a list of other values.
  This function steals ownership the list and the values within. */
ObConfigValue* config_value_new_list_steal(GList *list);
ObConfigValue* config_value_new_action_list(struct _ObActionList *al);

gboolean config_value_is_string(const ObConfigValue *v);
gboolean config_value_is_list(const ObConfigValue *v);
gboolean config_value_is_action_list(const ObConfigValue *v);

/*! Copies the data inside @v to the destination that the pointer @p is
  pointing to. */
void config_value_copy_ptr(ObConfigValue *v,
                           ObConfigValueDataType type,
                           ObConfigValueDataPtr *p,
                           const ObConfigValueEnum e[]);

/* These ones are valid on a string value */

const gchar* config_value_string(ObConfigValue *v);
gboolean config_value_bool(ObConfigValue *v);
guint config_value_int(ObConfigValue *v);
/*! returns (guint)-1 if an error */
guint config_value_enum(ObConfigValue *v, const ObConfigValueEnum e[]);
void config_value_fraction(ObConfigValue *v, gint *numer, gint *denom);
void config_value_gravity_coord(ObConfigValue *v, struct _GravityCoord *c);

/* These ones are valid on a list value */

GList* config_value_list(ObConfigValue *v);

/* These ones are value on a action list value */

struct _ObActionList* config_value_action_list(ObConfigValue *v);
