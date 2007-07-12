#include "openbox/actions.h"
#include "openbox/client.h"

typedef struct {
    gboolean toggle;
    gboolean on;
} Options;

static gpointer setup_on_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_off_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_toggle_func(ObParseInst *i,
                                  xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_shade_startup()
{
    actions_register("Shade",
                     setup_on_func,
                     free_func,
                     run_func,
                     NULL, NULL);
    actions_register("Unshade",
                     setup_off_func,
                     free_func,
                     run_func,
                     NULL, NULL);
    actions_register("ToggleShade",
                     setup_toggle_func,
                     free_func,
                     run_func,
                     NULL, NULL);
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
            client_shade(data->client, !data->client->shaded);
        else
            client_shade(data->client, o->on);
        actions_client_move(data, FALSE);
    }

    return FALSE;
}
