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
    GSList *thenacts;
    GSList *elseacts;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_if_startup()
{
    actions_register("If",
                     setup_func,
                     free_func,
                     run_func,
                     NULL, NULL);
}

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_new0(Options, 1);

    if ((n = parse_find_node("shaded", node))) {
        if (parse_bool(doc, n))
            o->shaded_on = TRUE;
        else
            o->shaded_off = TRUE;
    }
    if ((n = parse_find_node("maximized", node))) {
        if (parse_bool(doc, n))
            o->maxfull_on = TRUE;
        else
            o->maxfull_off = TRUE;
    }
    if ((n = parse_find_node("maximizedhorizontal", node))) {
        if (parse_bool(doc, n))
            o->maxhorz_on = TRUE;
        else
            o->maxhorz_off = TRUE;
    }
    if ((n = parse_find_node("maximizedvertical", node))) {
        if (parse_bool(doc, n))
            o->maxvert_on = TRUE;
        else
            o->maxvert_off = TRUE;
    }
    if ((n = parse_find_node("iconified", node))) {
        if (parse_bool(doc, n))
            o->iconic_on = TRUE;
        else
            o->iconic_off = TRUE;
    }
    if ((n = parse_find_node("focused", node))) {
        if (parse_bool(doc, n))
            o->focused = TRUE;
        else
            o->unfocused = TRUE;
    }

    if ((n = parse_find_node("then", node))) {
        xmlNodePtr m;

        m = parse_find_node("action", n->xmlChildrenNode);
        while (m) {
            ObActionsAct *action = actions_parse(i, doc, m);
            if (action) o->thenacts = g_slist_prepend(o->thenacts, action);
            m = parse_find_node("action", m->next);
        }
    }
    if ((n = parse_find_node("else", node))) {
        xmlNodePtr m;

        m = parse_find_node("action", n->xmlChildrenNode);
        while (m) {
            ObActionsAct *action = actions_parse(i, doc, m);
            if (action) o->elseacts = g_slist_prepend(o->elseacts, action);
            m = parse_find_node("action", m->next);
        }
    }

    return o;
}

static void free_func(gpointer options)
{
    Options *o = options;

    g_free(o);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;
    GSList *acts;
    ObClient *c = data->client;

    if ((!o->shaded_on || (c && c->shaded)) &&
        (!o->shaded_off || (c && !c->shaded)) &&
        (!o->iconic_on || (c && c->iconic)) &&
        (!o->iconic_off || (c && !c->iconic)) &&
        (!o->maxhorz_on || (c && c->max_horz)) &&
        (!o->maxhorz_off || (c && !c->max_horz)) &&
        (!o->maxvert_on || (c && c->max_vert)) &&
        (!o->maxvert_off || (c && !c->max_vert)) &&
        (!o->maxfull_on || (c && c->max_vert && c->max_horz)) &&
        (!o->maxfull_off || (c && !(c->max_vert && c->max_horz))) &&
        (!o->focused || (c && !(c == focus_client))) &&
        (!o->unfocused || (c && !(c != focus_client))))
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
