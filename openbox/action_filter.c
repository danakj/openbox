/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   action_filter.c for the Openbox window manager
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

#include "action_filter.h"
#include "gettext.h"

#include "filters/all.h"

typedef struct _ObActionFilterDefinition ObActionFilterDefinition;

struct _ObActionFilterDefinition {
    gchar *name;
    ObActionFilterSetupFunc setup;
    ObActionFilterDestroyFunc destroy;
    ObActionFilterReduceFunc reduce;
    ObActionFilterExpandFunc expand;
};

struct _ObActionFilter {
    gint ref;

    ObActionFilterDefinition *def;
    gpointer data;
};

static void action_filter_unregister(ObActionFilterDefinition *def);

/*! Holds all registered filters. */
static GSList *registered = NULL;

void action_filter_startup(gboolean reconfig)
{
    filters_all_startup();
}

void action_filter_shutdown(gboolean reconfig)
{
    GSList *it;

    for (it = registered; it; it = g_slist_next(it))
        action_filter_unregister(it->data);
    g_slist_free(it);
}

gboolean action_filter_register(const gchar *name,
                                ObActionFilterSetupFunc setup,
                                ObActionFilterDestroyFunc destroy,
                                ObActionFilterReduceFunc reduce,
                                ObActionFilterExpandFunc expand)
{
    ObActionFilterDefinition *def;
    GSList *it;

    g_return_val_if_fail(name != NULL, FALSE);
    g_return_val_if_fail(setup != NULL, FALSE);
    g_return_val_if_fail(reduce != NULL, FALSE);
    g_return_val_if_fail(expand != NULL, FALSE);

    for (it = registered; it; it = it->next) {
        def = it->data;
        if (g_strcasecmp(name, def->name) == 0)
            return FALSE; /* already registered */
    }

    def = g_slice_new(ObActionFilterDefinition);
    def->name = g_strdup(name);
    def->setup = setup;
    def->destroy = destroy;
    def->reduce = reduce;
    def->expand = expand;
    registered = g_slist_prepend(registered, def);

    return TRUE;
}

static void action_filter_unregister(ObActionFilterDefinition *def)
{
    if (def) {
        g_free(def->name);
        g_slice_free(ObActionFilterDefinition, def);
    }
}

ObActionFilter* action_filter_new(const gchar *key, struct _ObActionValue *v)
{
    ObActionFilterDefinition *def;
    ObActionFilter *filter;
    GSList *it;
    gboolean invert;

    invert = FALSE;
    if (key[0] == 'n' || key[0] == 'N')
        if (key[1] == 'o' || key[1] == 'O') {
            key += 2;
            invert = TRUE;
        }

    for (it = registered; it; it = g_slist_next(it)) {
        def = it->data;
        if (g_strcasecmp(key, def->name) == 0)
            break;
    }
    if (!it) {
        g_message(_("Invalid filter \"%s\" requested. No such filter exists."),
                  key);
        return NULL;
    }

    filter = g_slice_new(ObActionFilter);
    filter->ref = 1;
    filter->def = def;
    filter->data = def->setup(invert, v);
    return filter;
}

void action_filter_ref(ObActionFilter *f)
{
    if (f) ++f->ref;
}

void action_filter_unref(ObActionFilter *f)
{
    if (f && --f->ref < 1) {
        if (f->def->destroy) f->def->destroy(f->data);
        g_slice_free(ObActionFilter, f);
    }
}

void action_filter_expand(ObActionFilter *f, struct _ObClientSet *set)
{
    g_return_if_fail(f != NULL);
    return f->def->expand(set);
}

void action_filter_reduce(ObActionFilter *f, struct _ObClientSet *set)
{
    g_return_if_fail(f != NULL);
    return f->def->reduce(set);
}
