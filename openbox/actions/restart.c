#include "openbox/actions.h"
#include "openbox/actions_value.h"
#include "openbox/openbox.h"
#include "obt/paths.h"

typedef struct {
    gchar   *cmd;
} Options;

static gpointer setup_func(GHashTable *config);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_restart_startup(void)
{
    actions_register("Restart", setup_func, free_func, run_func);
}

static gpointer setup_func(GHashTable *config)
{
    ObActionsValue *v;
    Options *o;

    o = g_slice_new0(Options);

    v = g_hash_table_lookup(config, "command");
    if (v && actions_value_is_string(v))
        o->cmd = obt_paths_expand_tilde(actions_value_string(v));
    return o;
}

static void free_func(gpointer options)
{
    Options *o = options;
    g_free(o->cmd);
    g_slice_free(Options, o);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;

    ob_restart_other(o->cmd);

    return FALSE;
}
