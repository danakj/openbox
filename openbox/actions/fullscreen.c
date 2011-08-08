#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/config_value.h"
#include "openbox/client.h"
#include "openbox/client_set.h"

typedef struct {
    gboolean toggle;
    gboolean set;
} Options;

static gpointer setup_func(GHashTable *config);
static void free_func(gpointer o);
static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options);

void action_fullscreen_startup(void)
{
    action_register("Fullscreen", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    setup_func, free_func, run_func);
}

static gpointer setup_func(GHashTable *config)
{
    ObConfigValue *v;
    Options *o;

    o = g_slice_new0(Options);
    o->toggle = TRUE;
    o->set = TRUE;

    v = g_hash_table_lookup(config, "set");
    if (v && config_value_is_string(v)) {
        const gchar *s = config_value_string(v);
        if (!g_ascii_strcasecmp(s, "on"))
            o->toggle = FALSE;
        else if (!g_ascii_strcasecmp(s, "off")) {
            o->toggle = FALSE;
            o->set = FALSE;
        }
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
    client_fullscreen(c, o->toggle ? !c->fullscreen : o->set);
    return TRUE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options)
{
    action_client_move(data, TRUE);
    client_set_run(set, data, each_run, options);
    action_client_move(data, FALSE);
    return FALSE;
}
