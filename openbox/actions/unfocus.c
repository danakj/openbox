#include "openbox/actions.h"
#include "openbox/focus.h"

typedef struct {
    gboolean tobottom;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_unfocus_startup()
{
    actions_register("Unfocus",
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
    o->tobottom = FALSE;

    if ((n = parse_find_node("tobottom", node)))
        o->tobottom = parse_bool(doc, n);
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

    if (data->client && data->client == focus_client) {
        if (o->tobottom)
            focus_order_to_bottom(data->client);
        focus_fallback(FALSE, FALSE, TRUE);
    }

    return FALSE;
}
