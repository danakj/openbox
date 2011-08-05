#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/config_value.h"
#include "openbox/client.h"
#include "openbox/client_set.h"

typedef struct {
    gint layer; /*!< -1 for below, 0 for normal, and 1 for above */
    gboolean toggle;
} Options;

static gpointer setup_func_top(GHashTable *config);
static gpointer setup_func_bottom(GHashTable *config);
static gpointer setup_func_send(GHashTable *config);
static void free_func(gpointer o);
static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options);

void action_layer_startup(void)
{
    action_register("ToggleAlwaysOnTop", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    setup_func_top, free_func, run_func);
    action_register("ToggleAlwaysOnBottom", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    setup_func_bottom, free_func, run_func);
    action_register("SendToLayer", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    setup_func_send, free_func, run_func);
}

static gpointer setup_func_top(GHashTable *config)
{
    Options *o = g_slice_new0(Options);
    o->layer = 1;
    o->toggle = TRUE;
    return o;
}

static gpointer setup_func_bottom(GHashTable *config)
{
    Options *o = g_slice_new0(Options);
    o->layer = -1;
    o->toggle = TRUE;
    return o;
}

static gpointer setup_func_send(GHashTable *config)
{
    ObConfigValue *v;
    Options *o;

    o = g_slice_new0(Options);

    v = g_hash_table_lookup(config, "layer");
    if (v && config_value_is_string(v)) {
        const gchar *s = config_value_string(v);
        if (!g_ascii_strcasecmp(s, "above") ||
            !g_ascii_strcasecmp(s, "top"))
            o->layer = 1;
        else if (!g_ascii_strcasecmp(s, "below") ||
                 !g_ascii_strcasecmp(s, "bottom"))
            o->layer = -1;
        else if (!g_ascii_strcasecmp(s, "normal") ||
                 !g_ascii_strcasecmp(s, "middle"))
            o->layer = 0;
    }

    return o;
}

static void free_func(gpointer o)
{
    g_slice_free(Options, o);
}

static gboolean each_run(ObClient *c, const ObActionListRun *data,
                         gpointer options)
{
    Options *o = options;

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
    return TRUE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options)
{
    if (!client_set_is_empty(set)) {
        action_client_move(data, TRUE);
        client_set_run(set, data, each_run, options);
        action_client_move(data, FALSE);
    }
    return FALSE;
}
