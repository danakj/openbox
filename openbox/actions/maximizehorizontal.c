#include "openbox/actions.h"
#include "openbox/client.h"

typedef struct {
    gboolean toggle;
    gboolean on;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_on_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_off_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_toggle_func(ObParseInst *i,
                                  xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_maximizehorizontal_startup()
{
    actions_register("MaximizeHorizontal",
                     setup_func,
                     free_func,
                     run_func,
                     NULL, NULL);
    actions_register("MaximizeHorz",
                     setup_on_func,
                     free_func,
                     run_func,
                     NULL, NULL);
    actions_register("UnmaximizeHorz",
                     setup_off_func,
                     free_func,
                     run_func,
                     NULL, NULL);
    actions_register("ToggleMaximizeHorz",
                     setup_toggle_func,
                     free_func,
                     run_func,
                     NULL, NULL);
}

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_new0(Options, 1);
    o->toggle = TRUE;

    if ((n = parse_find_node("state", node))) {
        gchar *s = parse_string(doc, n);
        if (g_ascii_strcasecmp(s, "toggle")) {
            o->toggle = FALSE;
            o->on = parse_bool(doc, n);
        }
        g_free(s);
    }

    return o;
}

static gpointer setup_on_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    o->on = TRUE;
    return o;
}

static gpointer setup_off_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    o->on = FALSE;
    return o;
}

static gpointer setup_toggle_func(ObParseInst *i,
                                  xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    o->toggle = TRUE;
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

    if (data->client) {
        actions_client_move(data, TRUE);

        if (o->toggle)
            client_maximize(data->client, !data->client->max_horz, 1);
        else
            client_maximize(data->client, o->on, 1);

        actions_client_move(data, FALSE);
    }

    return FALSE;
}
