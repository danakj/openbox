#include "openbox/actions.h"
#include "openbox/misc.h"
#include "openbox/client.h"
#include "openbox/frame.h"
#include "openbox/screen.h"
#include "openbox/focus.h"
#include <glib.h>

typedef struct {
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
    GPatternSpec *matchtitle;
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

static gpointer setup_func(xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_slice_new0(Options);

    if ((n = obt_xml_find_node(node, "shaded"))) {
        if (obt_xml_node_bool(n))
            o->shaded_on = TRUE;
        else
            o->shaded_off = TRUE;
    }
    if ((n = obt_xml_find_node(node, "maximized"))) {
        if (obt_xml_node_bool(n))
            o->maxfull_on = TRUE;
        else
            o->maxfull_off = TRUE;
    }
    if ((n = obt_xml_find_node(node, "maximizedhorizontal"))) {
        if (obt_xml_node_bool(n))
            o->maxhorz_on = TRUE;
        else
            o->maxhorz_off = TRUE;
    }
    if ((n = obt_xml_find_node(node, "maximizedvertical"))) {
        if (obt_xml_node_bool(n))
            o->maxvert_on = TRUE;
        else
            o->maxvert_off = TRUE;
    }
    if ((n = obt_xml_find_node(node, "iconified"))) {
        if (obt_xml_node_bool(n))
            o->iconic_on = TRUE;
        else
            o->iconic_off = TRUE;
    }
    if ((n = obt_xml_find_node(node, "focused"))) {
        if (obt_xml_node_bool(n))
            o->focused = TRUE;
        else
            o->unfocused = TRUE;
    }
    if ((n = obt_xml_find_node(node, "urgent"))) {
        if (obt_xml_node_bool(n))
            o->urgent_on = TRUE;
        else
            o->urgent_off = TRUE;
    }
    if ((n = obt_xml_find_node(node, "undecorated"))) {
        if (obt_xml_node_bool(n))
            o->decor_off = TRUE;
        else
            o->decor_on = TRUE;
    }
    if ((n = obt_xml_find_node(node, "desktop"))) {
        gchar *s = obt_xml_node_string(n);
        if (!g_ascii_strcasecmp(s, "current"))
            o->desktop_current = TRUE;
        if (!g_ascii_strcasecmp(s, "other"))
            o->desktop_other = TRUE;
        else
            o->desktop_number = atoi(s);
    }
    if ((n = obt_xml_find_node(node, "omnipresent"))) {
        if (obt_xml_node_bool(n))
            o->omnipresent_on = TRUE;
        else
            o->omnipresent_off = TRUE;
    }
    if ((n = obt_xml_find_node(node, "title"))) {
        gchar *s;
        if ((s = obt_xml_node_string(n))) {
            o->matchtitle = g_pattern_spec_new(s);
            g_free(s);
        }
    }

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

    return o;
}

static void free_func(gpointer options)
{
    Options *o = options;

    while (o->thenacts) {
        actions_act_unref(o->thenacts->data);
        o->thenacts = g_slist_delete_link(o->thenacts, o->thenacts);
    }
    while (o->elseacts) {
        actions_act_unref(o->elseacts->data);
        o->elseacts = g_slist_delete_link(o->elseacts, o->elseacts);
    }
    if (o->matchtitle)
        g_pattern_spec_free(o->matchtitle);

    g_slice_free(Options, o);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;
    GSList *acts;
    ObClient *c = data->client;

    if (c &&
        (!o->shaded_on   ||  c->shaded) &&
        (!o->shaded_off  || !c->shaded) &&
        (!o->iconic_on   ||  c->iconic) &&
        (!o->iconic_off  || !c->iconic) &&
        (!o->maxhorz_on  ||  c->max_horz) &&
        (!o->maxhorz_off || !c->max_horz) &&
        (!o->maxvert_on  ||  c->max_vert) &&
        (!o->maxvert_off || !c->max_vert) &&
        (!o->maxfull_on  ||  (c->max_vert && c->max_horz)) &&
        (!o->maxfull_off || !(c->max_vert && c->max_horz)) &&
        (!o->focused     ||  (c == focus_client)) &&
        (!o->unfocused   || !(c == focus_client)) &&
        (!o->urgent_on   ||  (c->urgent || c->demands_attention)) &&
        (!o->urgent_off  || !(c->urgent || c->demands_attention)) &&
        (!o->decor_off   ||  (c->undecorated || !(c->decorations & OB_FRAME_DECOR_TITLEBAR))) &&
        (!o->decor_on    ||  (!c->undecorated && (c->decorations & OB_FRAME_DECOR_TITLEBAR))) &&
        (!o->omnipresent_on  || (c->desktop == DESKTOP_ALL)) &&
        (!o->omnipresent_off || (c->desktop != DESKTOP_ALL)) &&
        (!o->desktop_current || ((c->desktop == screen_desktop) ||
                                 (c->desktop == DESKTOP_ALL))) &&
        (!o->desktop_other   || ((c->desktop != screen_desktop) &&
                                 (c->desktop != DESKTOP_ALL))) &&
        (!o->desktop_number  || ((c->desktop == o->desktop_number - 1) ||
                                 (c->desktop == DESKTOP_ALL))) &&
        (!o->matchtitle ||
         (g_pattern_match_string(o->matchtitle, c->original_title))))
    {
        acts = o->thenacts;
    }
    else
        acts = o->elseacts;

    actions_run_acts(acts, data->uact, data->state,
                     data->x, data->y, data->button,
                     data->context, data->client);

    return FALSE;
}
