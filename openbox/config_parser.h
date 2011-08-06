/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   config_parser.h for the Openbox window manager
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
#include "obt/xml.h"

#include <glib.h>

void config_parser_startup(gboolean reconfig);
void config_parser_shutdown(gboolean reconfig);

/*! Register the configuration option @name, with the default value @def.
  The value of the option will be stored in @v. */
void config_parser_bool(const gchar *name, const gchar *def, gboolean *v);
void config_parser_int(const gchar *name, const gchar *def, gint *v);
void config_parser_string(const gchar *name, const gchar *def,
                          const gchar **v);
void config_parser_enum(const gchar *name, const gchar *def, guint *v,
                        const ObConfigValueEnum choices[]);
void config_parser_list(const gchar *name, GList *def, const GList **v);


void config_parser_string_uniq(const gchar *name, const gchar *def,
                               const gchar **v);
void config_parser_string_path(const gchar *name, const gchar *def,
                               const gchar **v);
void config_parser_key(const gchar *name, const gchar *def,
                       const gchar **v);
