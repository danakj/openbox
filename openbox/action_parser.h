/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   action_parser.h for the Openbox window manager
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

struct _ObActionList;

typedef struct _ObActionParser ObActionParser;
typedef struct _ObActionParserError ObActionParserError;

struct _ObActionParserError {
    gchar   *source;
    guint    line;
    gchar   *message;
    gboolean is_error; /*!< If false, it was a warning and not fatal. */
};

/*! A callback for when an error occurs while parsing.  Return TRUE to show
  the error, and FALSE otherwise.  This function can adjust the line number
  if it is parsing data nested within a source.
*/
typedef gboolean (*ObActionParserErrorFunc)(guint *line, const gchar *message,
                                            gpointer user_data);

ObActionParser* action_parser_new(const gchar *source);

void action_parser_ref(ObActionParser *p);
void action_parser_unref(ObActionParser *p);

void action_parser_set_on_error(ObActionParser *p,
                                ObActionParserErrorFunc callback,
                                gpointer user_data);

/*! Returns the last parsing error, or NULL if no errors occured. */
const ObActionParserError* action_parser_get_last_error(void);

void action_parser_reset_last_error(void);

struct _ObActionList* action_parser_read_string(ObActionParser *p,
                                                const gchar *text);
struct _ObActionList* action_parser_read_file(ObActionParser *p,
                                              const gchar *file,
                                              GError **error);
