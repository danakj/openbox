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
    guint    screendesktop_number;
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

static inline set_bool(xmlNodePtr node,
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

static gpointer setup_func(xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_slice_new0(Options);

    set_bool(node, "shaded", &o->shaded_on, &o->shaded_off);
    set_bool(node, "maximized", &o->maxfull_on, &o->maxfull_off);
    set_bool(node, "maximizedhorizontal", &o->maxhorz_on, &o->maxhorz_off);
    set_bool(node, "maximizedvertical", &o->maxvert_on, &o->maxvert_off);
    set_bool(node, "iconified", &o->iconic_on, &o->iconic_off);
    set_bool(node, "focused", &o->focused, &o->unfocused);
    set_bool(node, "urgent", &o->urgent_on, &o->urgent_off);
    set_bool(node, "undecorated", &o->decor_off, &o->decor_on);
    set_bool(node, "omnipresent", &o->omnipresent_on, &o->omnipresent_off);

    if ((n = obt_xml_find_node(node, "desktop"))) {
        gchar *s;
        if ((s = obt_xml_node_string(n))) {
            if (!g_ascii_strcasecmp(s, "current"))
                o->desktop_current = TRUE;
            if (!g_ascii_strcasecmp(s, "other"))
                o->desktop_other = TRUE;
            else
                o->desktop_number = atoi(s);
            g_free(s);
        }
    }
    if ((n = obt_xml_find_node(node, "activedesktop"))) {
        o->screendesktop_number = obt_xml_node_int(n);
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
        (!o->screendesktop_number || screen_desktop == o->screendesktop_number - 1) &&
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
