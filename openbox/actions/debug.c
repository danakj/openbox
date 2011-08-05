#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/config_value.h"
#include "openbox/client_set.h"
#include <glib.h>

typedef struct {
    gchar   *str;
} Options;

static gpointer setup_func(GHashTable *config);
static void     free_func(gpointer options);
static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options);

void action_debug_startup(void)
{
    action_register("Debug", OB_ACTION_DEFAULT_FILTER_EMPTY,
                    setup_func, free_func, run_func);
}

static gpointer setup_func(GHashTable *config)
{
    ObConfigValue *v;
    Options *o;

    o = g_slice_new0(Options);

    v = g_hash_table_lookup(config, "string");
    if (v && config_value_is_string(v))
        o->str = g_strdup(config_value_string(v));
    return o;
}

static void free_func(gpointer options)
{
    Options *o = options;
    g_free(o->str);
    g_slice_free(Options, o);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options)
{
    Options *o = options;

    if (o->str) g_print("%s\n", o->str);

    return FALSE;
}
