#include "openbox/actions.h"
#include "openbox/client.h"

typedef struct {
    gint layer; /*!< -1 for below, 0 for normal, and 1 for above */
    gboolean toggle;
    gboolean on;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_toggletop_func(ObParseInst *i,
                                     xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_togglebottom_func(ObParseInst *i,
                                        xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_sendtop_func(ObParseInst *i,
                                     xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_sendbottom_func(ObParseInst *i,
                                        xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_sendnormal_func(ObParseInst *i,
                                      xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_layer_startup()
{
    actions_register("Layer",
                     setup_func,
                     free_func,
                     run_func,
                     NULL, NULL);
    actions_register("ToggleAlwaysOnTop",
                     setup_toggletop_func,
                     free_func,
                     run_func,
                     NULL, NULL);
    actions_register("ToggleAlwaysOnBottom",
                     setup_togglebottom_func,
                     free_func,
                     run_func,
                     NULL, NULL);
    actions_register("SendToTopLayer",
                     setup_sendtop_func,
                     free_func,
                     run_func,
                     NULL, NULL);
    actions_register("SendToBottomLayer",
                     setup_sendbottom_func,
                     free_func,
                     run_func,
                     NULL, NULL);
    actions_register("SendToNormalLayer",
                     setup_sendnormal_func,
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

    if ((n = parse_find_node("layer", node))) {
        gchar *s = parse_string(doc, n);
        if (!g_ascii_strcasecmp(s, "above") ||
            !g_ascii_strcasecmp(s, "top"))
            o->layer = 1;
        else if (!g_ascii_strcasecmp(s, "below") ||
                 !g_ascii_strcasecmp(s, "bottom"))
            o->layer = -1;
        else if (!g_ascii_strcasecmp(s, "normal") ||
                 !g_ascii_strcasecmp(s, "middle"))
            o->layer = 0;
        g_free(s);
    }
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

static gpointer setup_toggletop_func(ObParseInst *i,
                                     xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    o->toggle = TRUE;
    o->layer = 1;
    return o;
}

static gpointer setup_togglebottom_func(ObParseInst *i,
                                        xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    o->toggle = TRUE;
    o->layer = -1;
    return o;
}

static gpointer setup_sendtop_func(ObParseInst *i,
                                     xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    o->on = TRUE;
    o->layer = 1;
    return o;
}

static gpointer setup_sendbottom_func(ObParseInst *i,
                                        xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    o->on = TRUE;
    o->layer = -1;
    return o;
}

static gpointer setup_sendnormal_func(ObParseInst *i,
                                      xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    o->on = TRUE;
    o->layer = 0;
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
        ObClient *c = data->client;

        actions_client_move(data, TRUE);

        if (o->layer < 0) {
            if (o->toggle || c->below != o->on)
                client_set_layer(c, c->below ? 0 : -1);
        }
        else if (o->layer > 0) {
            if (o->toggle || c->above != o->on)
                client_set_layer(c, c->above ? 0 : 1);
        }
        else {
            if ((o->toggle || o->on) && (c->above || c->below))
                client_set_layer(c, 0);
        }

        actions_client_move(data, FALSE);
    }

    return FALSE;
}
