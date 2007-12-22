#include "openbox/actions.h"
#include "openbox/client.h"

typedef struct {
    gint layer; /*!< -1 for below, 0 for normal, and 1 for above */
    gboolean toggle;
} Options;

static gpointer setup_func_top(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_func_bottom(ObParseInst *i, xmlDocPtr doc,
                                  xmlNodePtr node);
static gpointer setup_sendtop_func(ObParseInst *i,
                                     xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_sendbottom_func(ObParseInst *i,
                                        xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_sendnormal_func(ObParseInst *i,
                                      xmlDocPtr doc, xmlNodePtr node);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_layer_startup(void)
{
    actions_register("ToggleAlwaysOnTop", setup_func_top, g_free,
                     run_func, NULL, NULL);
    actions_register("ToggleAlwaysOnBottom", setup_func_bottom, g_free,
                     run_func, NULL, NULL);
    actions_register("SendToTopLayer", setup_sendtop_func, g_free,
                     run_func, NULL, NULL);
    actions_register("SendToBottomLayer", setup_sendbottom_func, g_free,
                     run_func, NULL, NULL);
    actions_register("SendToNormalLayer", setup_sendnormal_func, g_free,
                     run_func, NULL, NULL);
}

static gpointer setup_func_top(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    o->layer = 1;
    o->toggle = TRUE;
    return o;
}

static gpointer setup_func_bottom(ObParseInst *i, xmlDocPtr doc,
                                  xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    o->layer = -1;
    o->toggle = TRUE;
    return o;
}

static gpointer setup_sendtop_func(ObParseInst *i,
                                     xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    o->layer = 1;
    o->toggle = FALSE;
    return o;
}

static gpointer setup_sendbottom_func(ObParseInst *i,
                                        xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    o->layer = -1;
    o->toggle = FALSE;
    return o;
}

static gpointer setup_sendnormal_func(ObParseInst *i,
                                      xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    o->layer = 0;
    o->toggle = FALSE;
    return o;
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;

    if (data->client) {
        ObClient *c = data->client;

        actions_client_move(data, TRUE);

        if (o->layer < 0) {
            if (o->toggle || !c->below)
                client_set_layer(c, c->below ? 0 : -1);
        }
        else if (o->layer > 0) {
            if (o->toggle || !c->above)
                client_set_layer(c, c->above ? 0 : 1);
        }
        else if (c->above || c->below)
            client_set_layer(c, 0);

        actions_client_move(data, FALSE);
    }

    return FALSE;
}
