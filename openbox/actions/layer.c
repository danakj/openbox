#include "openbox/actions.h"
#include "openbox/client.h"

typedef struct {
    gint layer; /*!< -1 for below, 0 for normal, and 1 for above */
    gboolean toggle;
} Options;

static gpointer setup_func_top(xmlNodePtr node);
static gpointer setup_func_bottom(xmlNodePtr node);
static gpointer setup_func_send(xmlNodePtr node);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_layer_startup(void)
{
    actions_register("ToggleAlwaysOnTop", setup_func_top, g_free,
                     run_func, NULL, NULL);
    actions_register("ToggleAlwaysOnBottom", setup_func_bottom, g_free,
                     run_func, NULL, NULL);
    actions_register("SendToLayer", setup_func_send, g_free,
                     run_func, NULL, NULL);
}

static gpointer setup_func_top(xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    o->layer = 1;
    o->toggle = TRUE;
    return o;
}

static gpointer setup_func_bottom(xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    o->layer = -1;
    o->toggle = TRUE;
    return o;
}

static gpointer setup_func_send(xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_new0(Options, 1);

    if ((n = obt_parse_find_node(node, "layer"))) {
        gchar *s = obt_parse_node_string(n);
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
