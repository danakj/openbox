#include "openbox/actions.h"
#include "openbox/client.h"

typedef struct {
    gboolean toggle;
    gboolean on;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_toggle_func(ObParseInst *i,
                                  xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_decorations_startup()
{
    actions_register("Decorations",
                     setup_func,
                     free_func,
                     run_func,
                     NULL, NULL);
    actions_register("ToggleDecorations",
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
        if (o->toggle || o->on != !data->client->undecorated)
            client_set_undecorated(data->client, !data->client->undecorated);
        actions_client_move(data, FALSE);
    }

    return FALSE;
}
