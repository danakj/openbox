/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   obt/ddparse.h for the Openbox window manager
   Copyright (c) 2009        Dana Jansens

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

typedef struct _ObtDDParseGroup ObtDDParseGroup;

typedef enum {
    OBT_DDPARSE_EXEC,
    OBT_DDPARSE_STRING,
    OBT_DDPARSE_LOCALESTRING,
    OBT_DDPARSE_STRINGS,
    OBT_DDPARSE_LOCALESTRINGS,
    OBT_DDPARSE_BOOLEAN,
    OBT_DDPARSE_NUMERIC,
    OBT_DDPARSE_ENUM_TYPE,
    OBT_DDPARSE_ENVIRONMENTS,
    OBT_DDPARSE_NUM_VALUE_TYPES
} ObtDDParseValueType;

typedef struct _ObtDDParseValue {
    ObtDDParseValueType type;
    union _ObtDDParseValueValue {
        gchar *string;
        struct _ObtDDParseValueStrings {
            gchar **a;
            gulong n;
        } strings;
        gboolean boolean;
        gfloat numeric;
        guint enumerable;
        guint environments; /*!< A mask of flags from ObtLinkEnvMask */
    } value;
} ObtDDParseValue;

/*! Parse a .desktop file.
  @param filename The full path to the .desktop file to be read.
  @return Returns a hash table where the keys are groups, and the values are
    ObtDDParseGroups */
GHashTable* obt_ddparse_file(const gchar *filename);

/*! Get the keys in a group from a .desktop file.
  The group comes from the hash table returned by obt_ddparse_file.
  @return Returns a hash table where the keys are "keys" in the .desktop file,
    represented as strings.  The values are "values" in the .desktop file, for
    the group @g. Each value will be a pointer to an ObtDDParseValue structure.
*/
GHashTable* obt_ddparse_group_keys(ObtDDParseGroup *g);

/*! Determine the id for a .desktop file.
  @param filename The path to the .desktop file, _relative to_ some
    basepath. filename must end with ".desktop" and be encoded in utf8.
  @return Returns a string which is the id for the given .desktop file in its
    current position.  Returns NULL if there is an error.
*/
gchar* obt_ddparse_file_to_id(const gchar *filename);
