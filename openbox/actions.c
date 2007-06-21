/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   actions.h for the Openbox window manager
   Copyright (c) 2007        Dana Jansens

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

static void actions_unregister(ObActionsDefinition *def);

struct _ObActionsDefinition {
    gchar *name;
    gboolean interactive;

    ObActionsDataParseFunc parse;
    ObActionsDataFreeFunc free;
    ObActionsRunFunc run;

    gpointer action_data;
};

static GSList *registered = NULL;


void actions_startup(gboolean reconfig)
{
    if (reconfig) return;

    
}

void actions_shutdown(gboolean reconfig)
{
    if (reconfig) return;

    /* free all the registered actions */
    while (registered) {
        actions_unregister(registered->data);
        registered = g_slist_delete_link(registered, registered);
    }
}

gboolean actions_register(const gchar *name,
                          gboolean interactive,
                          ObActionsDataSetupFunc setup,
                          ObActionsDataParseFunc parse,
                          ObActionsDataFreeFunc free,
                          ObActionsRunFunc run)
{
    GSList *it;
    ObActionsDefinition *def;

    for (it = registered; it; it = g_slist_next(it)) {
        def = it->data;
        if (!g_ascii_strcasecmp(name, def->name)) /* already registered */
            return FALSE;
    }

    def = g_new(ObActionsDefinition, 1);
    def->name = g_strdup(name);
    def->interactive = interactive;
    def->parse = parse;
    def->free = free;
    def->run = run;
    def->action_data = setup();
    return TRUE;
}

static void actions_unregister(ObActionsDefinition *def)
{
    if (def) {
        def->free(def->action_data);
        g_free(def->name);
        g_free(def);
    }
}
