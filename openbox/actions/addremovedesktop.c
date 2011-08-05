#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/config_value.h"
#include "openbox/client_set.h"
#include "openbox/screen.h"
#include <glib.h>

typedef struct {
    gboolean current;
    gboolean add;
} Options;

static gpointer setup_func(GHashTable *config);
static gpointer setup_add_func(GHashTable *config);
static gpointer setup_remove_func(GHashTable *config);
static void free_func(gpointer o);
static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options);

void action_addremovedesktop_startup(void)
{
    action_register("AddDesktop", OB_ACTION_DEFAULT_FILTER_EMPTY,
                    setup_add_func, free_func, run_func);
    action_register("RemoveDesktop", OB_ACTION_DEFAULT_FILTER_EMPTY,
                    setup_remove_func, free_func, run_func);
}

static gpointer setup_func(GHashTable *config)
{
    ObConfigValue *v;
    Options *o;

    o = g_slice_new0(Options);

    v = g_hash_table_lookup(config, "where");
    if (v && config_value_is_string(v)) {
        const gchar *s = config_value_string(v);
        if (!g_ascii_strcasecmp(s, "last"))
            o->current = FALSE;
        else if (!g_ascii_strcasecmp(s, "current"))
            o->current = TRUE;
    }

    return o;
}

static gpointer setup_add_func(GHashTable *config)
{
    Options *o = setup_func(config);
    o->add = TRUE;
    return o;
}

static gpointer setup_remove_func(GHashTable *config)
{
    Options *o = setup_func(config);
    o->add = FALSE;
    return o;
}

static void free_func(gpointer o)
{
    g_slice_free(Options, o);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options)
{
    Options *o = options;

    action_client_move(data, TRUE);

    if (o->add)
        screen_add_desktop(o->current);
    else
        screen_remove_desktop(o->current);

    action_client_move(data, FALSE);

    return FALSE;
}
