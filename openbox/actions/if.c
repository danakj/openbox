/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   if.c for the Openbox window manager
   Copyright (c) 2007        Mikael Magnusson
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

#include "openbox/actions.h"
#include "openbox/misc.h"
#include "openbox/client.h"
#include "openbox/frame.h"
#include "openbox/screen.h"
#include "openbox/focus.h"
#include <glib.h>

typedef enum {
    QUERY_TARGET_IS_ACTION_TARGET,
    QUERY_TARGET_IS_FOCUS_TARGET,
} QueryTarget;

typedef struct {
    QueryTarget target;
    gboolean shaded_on;
    gboolean shaded_off;
    gboolean maxvert_on;
    gboolean maxvert_off;
    gboolean maxhorz_on;
    gboolean maxhorz_off;
    gboolean maxfull_on;
    gboolean maxfull_off;
    gboolean iconic_on;
    gboolean iconic_off;
    gboolean focused;
    gboolean unfocused;
    gboolean urgent_on;
    gboolean urgent_off;
    gboolean decor_off;
    gboolean decor_on;
    gboolean omnipresent_on;
    gboolean omnipresent_off;
    gboolean desktop_current;
    gboolean desktop_other;
    guint    desktop_number;
    guint    screendesktop_number;
    guint    client_monitor;
    GPatternSpec *matchtitle;
    GRegex *regextitle;
    gchar  *exacttitle;
} Query;

typedef struct {
    GArray* queries;
    GSList *thenacts;
    GSList *elseacts;
} Options;

static gpointer setup_func(xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_if_startup(void)
{
    actions_register("If", setup_func, free_func, run_func);
}

static inline void set_bool(xmlNodePtr node,
                            const char *name,
                            gboolean *on,
                            gboolean *off)
{
    xmlNodePtr n;

    if ((n = obt_xml_find_node(node, name))) {
        if (obt_xml_node_bool(n))
            *on = TRUE;
        else
            *off = TRUE;
    }
}

static void setup_query(Options* o, xmlNodePtr node, QueryTarget target) {
    Query *q = g_slice_new0(Query);
    g_array_append_val(o->queries, q);

    q->target = target;

    set_bool(node, "shaded", &q->shaded_on, &q->shaded_off);
    set_bool(node, "maximized", &q->maxfull_on, &q->maxfull_off);
    set_bool(node, "maximizedhorizontal", &q->maxhorz_on, &q->maxhorz_off);
    set_bool(node, "maximizedvertical", &q->maxvert_on, &q->maxvert_off);
    set_bool(node, "iconified", &q->iconic_on, &q->iconic_off);
    set_bool(node, "focused", &q->focused, &q->unfocused);
    set_bool(node, "urgent", &q->urgent_on, &q->urgent_off);
    set_bool(node, "undecorated", &q->decor_off, &q->decor_on);
    set_bool(node, "omnipresent", &q->omnipresent_on, &q->omnipresent_off);

    xmlNodePtr n;
    if ((n = obt_xml_find_node(node, "desktop"))) {
        gchar *s;
        if ((s = obt_xml_node_string(n))) {
            if (!g_ascii_strcasecmp(s, "current"))
                q->desktop_current = TRUE;
            if (!g_ascii_strcasecmp(s, "other"))
                q->desktop_other = TRUE;
            else
                q->desktop_number = atoi(s);
            g_free(s);
        }
    }
    if ((n = obt_xml_find_node(node, "activedesktop"))) {
        q->screendesktop_number = obt_xml_node_int(n);
    }
    if ((n = obt_xml_find_node(node, "title"))) {
        gchar *s, *type = NULL;
        if ((s = obt_xml_node_string(n))) {
            if (!obt_xml_attr_string(n, "type", &type) ||
                !g_ascii_strcasecmp(type, "pattern"))
            {
                q->matchtitle = g_pattern_spec_new(s);
            } else if (type && !g_ascii_strcasecmp(type, "regex")) {
                q->regextitle = g_regex_new(s, 0, 0, NULL);
            } else if (type && !g_ascii_strcasecmp(type, "exact")) {
                q->exacttitle = g_strdup(s);
            }
            g_free(s);
        }
    }
    if ((n = obt_xml_find_node(node, "monitor"))) {
        q->client_monitor = obt_xml_node_int(n);
    }
}

static gpointer setup_func(xmlNodePtr node)
{
    Options *o = g_slice_new0(Options);

    gboolean zero_terminated = FALSE;
    gboolean clear_to_zero_on_alloc = FALSE;
    o->queries = g_array_new(zero_terminated,
                             clear_to_zero_on_alloc,
                             sizeof(Query*));

    xmlNodePtr n;
    if ((n = obt_xml_find_node(node, "then"))) {
        xmlNodePtr m;

        m = obt_xml_find_node(n->children, "action");
        while (m) {
            ObActionsAct *action = actions_parse(m);
            if (action) o->thenacts = g_slist_append(o->thenacts, action);
            m = obt_xml_find_node(m->next, "action");
        }
    }
    if ((n = obt_xml_find_node(node, "else"))) {
        xmlNodePtr m;

        m = obt_xml_find_node(n->children, "action");
        while (m) {
            ObActionsAct *action = actions_parse(m);
            if (action) o->elseacts = g_slist_append(o->elseacts, action);
            m = obt_xml_find_node(m->next, "action");
        }
    }

    xmlNodePtr query_node = obt_xml_find_node(node, "query");
    if (!query_node) {
        /* The default query if none is specified. It uses the conditions
           found in the action's node. */
        setup_query(o,
                    node,
                    QUERY_TARGET_IS_ACTION_TARGET);
    } else {
        while (query_node) {
            QueryTarget query_target = QUERY_TARGET_IS_ACTION_TARGET;
            if (obt_xml_attr_contains(query_node, "target", "focus"))
                query_target = QUERY_TARGET_IS_FOCUS_TARGET;

            setup_query(o, query_node->children, query_target);

            query_node = obt_xml_find_node(query_node->next, "query");
        }
    }

    return o;
}

static void free_func(gpointer options)
{
    Options *o = options;

    guint i;
    for (i = 0; i < o->queries->len; ++i) {
        Query *q = g_array_index(o->queries, Query*, i);

        if (q->matchtitle)
            g_pattern_spec_free(q->matchtitle);
        if (q->regextitle)
            g_regex_unref(q->regextitle);
        if (q->exacttitle)
            g_free(q->exacttitle);

        g_slice_free(Query, q);
    }

    while (o->thenacts) {
        actions_act_unref(o->thenacts->data);
        o->thenacts = g_slist_delete_link(o->thenacts, o->thenacts);
    }
    while (o->elseacts) {
        actions_act_unref(o->elseacts->data);
        o->elseacts = g_slist_delete_link(o->elseacts, o->elseacts);
    }

    g_array_unref(o->queries);
    g_slice_free(Options, o);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;
    ObClient *action_target = data->client;
    gboolean is_true = TRUE;

    guint i;
    for (i = 0; i < o->queries->len; ++i) {
        Query *q = g_array_index(o->queries, Query*, i);
        ObClient *query_target = NULL;

        switch (q->target) {
        case QUERY_TARGET_IS_ACTION_TARGET:
            query_target = data->client;
            break;
        case QUERY_TARGET_IS_FOCUS_TARGET:
            query_target = focus_client;
            break;
        }

        /* If there's no client to query, then false. */
        is_true &= query_target != NULL;

        if (q->shaded_on)
            is_true &= query_target->shaded;
        if (q->shaded_off)
            is_true &= !query_target->shaded;

        if (q->iconic_on)
            is_true &= query_target->iconic;
        if (q->iconic_off)
            is_true &= !query_target->iconic;

        if (q->maxhorz_on)
            is_true &= query_target->max_horz;
        if (q->maxhorz_off)
            is_true &= !query_target->max_horz;

        if (q->maxvert_on)
            is_true &= query_target->max_vert;
        if (q->maxvert_off)
            is_true &= !query_target->max_vert;

        gboolean is_max_full =
            query_target->max_vert && query_target->max_horz;
        if (q->maxfull_on)
            is_true &= is_max_full;
        if (q->maxfull_off)
            is_true &= !is_max_full;

        if (q->focused)
            is_true &= query_target == focus_client;
        if (q->unfocused)
            is_true &= query_target != focus_client;

        gboolean is_urgent =
            query_target->urgent || query_target->demands_attention;
        if (q->urgent_on)
            is_true &= is_urgent;
        if (q->urgent_off)
            is_true &= !is_urgent;

        gboolean has_visible_title_bar =
            !query_target->undecorated &&
            (query_target->decorations & OB_FRAME_DECOR_TITLEBAR);
        if (q->decor_on)
            is_true &= has_visible_title_bar;
        if (q->decor_off)
            is_true &= !has_visible_title_bar;

        if (q->omnipresent_on)
            is_true &= query_target->desktop == DESKTOP_ALL;
        if (q->omnipresent_off)
            is_true &= query_target->desktop != DESKTOP_ALL;

        gboolean is_on_current_desktop =
            query_target->desktop == screen_desktop ||
            query_target->desktop == DESKTOP_ALL;
        if (q->desktop_current)
            is_true &= is_on_current_desktop;
        if (q->desktop_other)
            is_true &= !is_on_current_desktop;

        if (q->desktop_number) {
            gboolean is_on_desktop =
                query_target->desktop == q->desktop_number - 1 ||
                query_target->desktop == DESKTOP_ALL;
            is_true &= is_on_desktop;
        }

        if (q->screendesktop_number)
            is_true &= screen_desktop == q->screendesktop_number - 1;

        if (q->matchtitle) {
            is_true &= g_pattern_match_string(q->matchtitle,
                                              query_target->original_title);
        }
        if (q->regextitle) {
            is_true &= g_regex_match(q->regextitle,
                                     query_target->original_title,
                                     0,
                                     NULL);
        }
        if (q->exacttitle)
            is_true &= !strcmp(q->exacttitle, query_target->original_title);

        if (q->client_monitor)
            is_true &= client_monitor(query_target) == q->client_monitor - 1;

    }

    GSList *acts;
    if (is_true)
        acts = o->thenacts;
    else
        acts = o->elseacts;

    actions_run_acts(acts, data->uact, data->state,
                     data->x, data->y, data->button,
                     data->context, action_target);

    return FALSE;
}
