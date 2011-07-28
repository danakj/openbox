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

ObActionParser* action_parser_new(void);

void action_parser_ref(ObActionParser *p);
void action_parser_unref(ObActionParser *p);

struct _ObActionList* action_parser_read_string(ObActionParser *p,
                                                const gchar *text);
struct _ObActionList* action_parser_read_file(ObActionParser *p,
                                              const gchar *file,
                                              GError **error);
