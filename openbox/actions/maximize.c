#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/action_value.h"
#include "openbox/client.h"
#include "openbox/client_set.h"

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
static gboolean run_func_on(const ObClientSet *set,
                            const ObActionListRun *data, gpointer options);
static gboolean run_func_off(const ObClientSet *set,
                             const ObActionListRun *data, gpointer options);
static gboolean run_func_toggle(const ObClientSet *set,
                                const ObActionListRun *data, gpointer options);

void action_maximize_startup(void)
{
    action_register("Maximize", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    setup_func, free_func, run_func_on);
    action_register("Unmaximize", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    setup_func, free_func, run_func_off);
    action_register("ToggleMaximize", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    setup_func, free_func, run_func_toggle);
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

static gboolean each_on(ObClient *c, const ObActionListRun *data,
                        gpointer options)
{
    Options *o = options;
    if (data->target) {
        action_client_move(data, TRUE);
        client_maximize(data->target, TRUE, o->dir);
        action_client_move(data, FALSE);
    }
    return TRUE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func_on(const ObClientSet *set,
                            const ObActionListRun *data, gpointer options)
{
    if (!client_set_is_empty(set)) {
        action_client_move(data, TRUE);
        client_set_run(set, data, each_on, options);
        action_client_move(data, FALSE);
    }
    return FALSE;
}

static gboolean each_off(ObClient *c, const ObActionListRun *data,
                         gpointer options)
{
    Options *o = options;
    if (data->target) {
        action_client_move(data, TRUE);
        client_maximize(data->target, FALSE, o->dir);
        action_client_move(data, FALSE);
    }
    return TRUE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func_off(const ObClientSet *set,
                             const ObActionListRun *data, gpointer options)
{
    if (!client_set_is_empty(set)) {
        action_client_move(data, TRUE);
        client_set_run(set, data, each_off, options);
        action_client_move(data, FALSE);
    }
    return FALSE;
}

static gboolean each_toggle(ObClient *c, const ObActionListRun *data,
                            gpointer options)
{
    Options *o = options;
    gboolean toggle;
    toggle = ((o->dir == HORZ && !data->target->max_horz) ||
              (o->dir == VERT && !data->target->max_vert) ||
              (o->dir == BOTH &&
               !(data->target->max_horz && data->target->max_vert)));
    client_maximize(data->target, toggle, o->dir);
    return TRUE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func_toggle(const ObClientSet *set,
                                const ObActionListRun *data, gpointer options)
{
    if (!client_set_is_empty(set)) {
        action_client_move(data, TRUE);
        client_set_run(set, data, each_toggle, options);
        action_client_move(data, FALSE);
    }
    return FALSE;
}
