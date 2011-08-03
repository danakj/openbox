#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/action_value.h"
#include "openbox/client_set.h"
#include "openbox/openbox.h"
#include "obt/paths.h"

typedef struct {
    gchar   *cmd;
} Options;

static gpointer setup_func(GHashTable *config);
static void     free_func(gpointer options);
static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options);

void action_restart_startup(void)
{
    action_register("Restart", OB_ACTION_DEFAULT_FILTER_EMPTY,
                    setup_func, free_func, run_func);
}

static gpointer setup_func(GHashTable *config)
{
    ObActionValue *v;
    Options *o;

    o = g_slice_new0(Options);

    v = g_hash_table_lookup(config, "command");
    if (v && action_value_is_string(v))
        o->cmd = obt_paths_expand_tilde(action_value_string(v));
    return o;
}

static void free_func(gpointer options)
{
    Options *o = options;
    g_free(o->cmd);
    g_slice_free(Options, o);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options)
{
    Options *o = options;

    ob_restart_other(o->cmd);

    return FALSE;
}
