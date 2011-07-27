/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   actions_parser.h for the Openbox window manager
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

struct _ObActionsList;

typedef struct _ObActionsParser ObActionsParser;

ObActionsParser* actions_parser_new(void);

void actions_parser_ref(ObActionsParser *p);
void actions_parser_unref(ObActionsParser *p);

struct _ObActionsList* actions_parser_read_string(ObActionsParser *p,
                                                  const gchar *text);
struct _ObActionsList* actions_parser_read_file(ObActionsParser *p,
                                                const gchar *file,
                                                GError **error);
