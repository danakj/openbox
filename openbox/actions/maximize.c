#include "openbox/actions.h"
#include "openbox/client.h"

typedef struct {
    gboolean vertical;
    gboolean horizontal;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_maximize_startup()
{
    actions_register("Maximize",
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
    o->vertical = TRUE;
    o->horizontal = TRUE;

    if ((n = parse_find_node("vertical", node)))
        o->vertical = parse_bool(doc, n);
    if ((n = parse_find_node("horizontal", node)))
        o->horizontal = parse_bool(doc, n);
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

        if (o->horizontal && !o->vertical)
            client_maximize(data->client, !data->client->max_horz, 1);
        else if (!o->horizontal && o->vertical)
            client_maximize(data->client, !data->client->max_vert, 2);
        else if (o->horizontal && o->vertical)
            client_maximize(data->client,
                            !data->client->max_horz || !data->client->max_vert,
                            0);

        actions_client_move(data, FALSE);
    }

    return FALSE;
}
