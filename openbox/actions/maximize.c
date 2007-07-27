#include "openbox/actions.h"
#include "openbox/client.h"

/* These match the values for client_maximize */
typedef enum {
    BOTH = 0,
    HORZ = 1,
    VERT = 2
} MaxDirection;

typedef struct {
    MaxDirection dir;
} Options;

static gpointer setup_func(xmlNodePtr node);
static gboolean run_func_on(ObActionsData *data, gpointer options);
static gboolean run_func_off(ObActionsData *data, gpointer options);
static gboolean run_func_toggle(ObActionsData *data, gpointer options);

void action_maximize_startup(void)
{
    actions_register("Maximize", setup_func, g_free, run_func_on,
                     NULL, NULL);
    actions_register("Unmaximize", setup_func, g_free, run_func_off,
                     NULL, NULL);
    actions_register("ToggleMaximize", setup_func, g_free, run_func_toggle,
                     NULL, NULL);
}

static gpointer setup_func(xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_new0(Options, 1);
    o->dir = BOTH;

    if ((n = obt_parse_find_node(node, "direction"))) {
        gchar *s = obt_parse_node_string(n);
        if (!g_ascii_strcasecmp(s, "vertical") ||
            !g_ascii_strcasecmp(s, "vert"))
            o->dir = VERT;
        else if (!g_ascii_strcasecmp(s, "horizontal") ||
                 !g_ascii_strcasecmp(s, "horz"))
            o->dir = HORZ;
        g_free(s);
    }

    return o;
}

/* Always return FALSE because its not interactive */
static gboolean run_func_on(ObActionsData *data, gpointer options)
{
    Options *o = options;
    if (data->client) {
        actions_client_move(data, TRUE);
        client_maximize(data->client, TRUE, o->dir);
        actions_client_move(data, FALSE);
    }
    return FALSE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func_off(ObActionsData *data, gpointer options)
{
    Options *o = options;
    if (data->client) {
        actions_client_move(data, TRUE);
        client_maximize(data->client, FALSE, o->dir);
        actions_client_move(data, FALSE);
    }
    return FALSE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func_toggle(ObActionsData *data, gpointer options)
{
    Options *o = options;
    if (data->client) {
        gboolean toggle;
        actions_client_move(data, TRUE);
        toggle = ((o->dir == HORZ && !data->client->max_horz) ||
                  (o->dir == VERT && !data->client->max_vert) ||
                  (o->dir == BOTH &&
                   !(data->client->max_horz && data->client->max_vert)));
        client_maximize(data->client, toggle, o->dir);
        actions_client_move(data, FALSE);
    }
    return FALSE;
}
