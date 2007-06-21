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
#include "gettext.h"

static void actions_definition_ref(ObActionsDefinition *def);
static void actions_definition_unref(ObActionsDefinition *def);

struct _ObActionsDefinition {
    guint ref;

    gchar *name;
    gboolean allow_interactive;

    ObActionsDataSetupFunc setup;
    ObActionsDataFreeFunc free;
    ObActionsRunFunc run;
};

struct _ObActionsAct {
    guint ref;

    ObActionsDefinition *def;

    gpointer options;
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
        actions_definition_unref(registered->data);
        registered = g_slist_delete_link(registered, registered);
    }
}

gboolean actions_register(const gchar *name,
                          gboolean allow_interactive,
                          ObActionsDataSetupFunc setup,
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
    def->ref = 1;
    def->name = g_strdup(name);
    def->allow_interactive = allow_interactive;
    def->setup = setup;
    def->free = free;
    def->run = run;
    return TRUE;
}

static void actions_definition_ref(ObActionsDefinition *def)
{
    ++def->ref;
}

static void actions_definition_unref(ObActionsDefinition *def)
{
    if (def && --def->ref == 0) {
        g_free(def->name);
        g_free(def);
    }
}

ObActionsAct* actions_parse_string(const gchar *name)
{
    GSList *it;
    ObActionsDefinition *def;
    ObActionsAct *act = NULL;

    /* find the requested action */
    for (it = registered; it; it = g_slist_next(it)) {
        def = it->data;
        if (!g_ascii_strcasecmp(name, def->name))
            break;
        def = NULL;
    }

    /* if we found the action */
    if (def) {
        act = g_new(ObActionsAct, 1);
        act->ref = 1;
        act->def = def;
        actions_definition_ref(act->def);
        act->options = NULL;
    } else
        g_message(_("Invalid action '%s' requested. No such action exists."),
                  name);

    return act;
}

ObActionsAct* actions_parse(ObParseInst *i,
                            xmlDocPtr doc,
                            xmlNodePtr node)
{
    gchar *name;
    ObActionsAct *act = NULL;

    if (parse_attr_string("name", node, &name)) {
        if ((act = actions_parse_string(name)))
            /* there is more stuff to parse here */
            act->options = act->def->setup(i, doc, node->children);

        g_free(name);
    }

    return act;
}

void actions_act_ref(ObActionsAct *act)
{
    ++act->ref;
}

void actions_act_unref(ObActionsAct *act)
{
    if (act && --act->ref == 0) {
        /* free the action specific options */
        act->def->free(act->options);
        /* unref the definition */
        actions_definition_unref(act->def);
        g_free(act);
    }
}
