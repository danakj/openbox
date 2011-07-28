#include "openbox/action.h"
#include "openbox/action_value.h"
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

static gpointer setup_func(GHashTable *config);
static void free_func(gpointer o);
static gboolean run_func_on(ObActionData *data, gpointer options);
static gboolean run_func_off(ObActionData *data, gpointer options);
static gboolean run_func_toggle(ObActionData *data, gpointer options);

void action_maximize_startup(void)
{
    action_register("Maximize", setup_func, free_func, run_func_on);
    action_register("Unmaximize", setup_func, free_func, run_func_off);
    action_register("ToggleMaximize", setup_func, free_func, run_func_toggle);
}

static gpointer setup_func(GHashTable *config)
{
    ObActionValue *v;
    Options *o;

    o = g_slice_new0(Options);
    o->dir = BOTH;

    v = g_hash_table_lookup(config, "dir");
    if (v && action_value_is_string(v)) {
        const gchar *s = action_value_string(v);
        if (!g_ascii_strcasecmp(s, "vertical") ||
            !g_ascii_strcasecmp(s, "vert"))
            o->dir = VERT;
        else if (!g_ascii_strcasecmp(s, "horizontal") ||
                 !g_ascii_strcasecmp(s, "horz"))
            o->dir = HORZ;
    }

    return o;
}

static void free_func(gpointer o)
{
    g_slice_free(Options, o);
}

/* Always return FALSE because its not interactive */
static gboolean run_func_on(ObActionData *data, gpointer options)
{
    Options *o = options;
    if (data->client) {
        action_client_move(data, TRUE);
        client_maximize(data->client, TRUE, o->dir);
        action_client_move(data, FALSE);
    }
    return FALSE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func_off(ObActionData *data, gpointer options)
{
    Options *o = options;
    if (data->client) {
        action_client_move(data, TRUE);
        client_maximize(data->client, FALSE, o->dir);
        action_client_move(data, FALSE);
    }
    return FALSE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func_toggle(ObActionData *data, gpointer options)
{
    Options *o = options;
    if (data->client) {
        gboolean toggle;
        action_client_move(data, TRUE);
        toggle = ((o->dir == HORZ && !data->client->max_horz) ||
                  (o->dir == VERT && !data->client->max_vert) ||
                  (o->dir == BOTH &&
                   !(data->client->max_horz && data->client->max_vert)));
        client_maximize(data->client, toggle, o->dir);
        action_client_move(data, FALSE);
    }
    return FALSE;
}
