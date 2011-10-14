/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*- */

#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/config_value.h"
#include "openbox/client.h"
#include "openbox/client_set.h"

typedef enum {
    TASKBAR,
    PAGER
} SkipType;

typedef struct {
    SkipType type;
    gboolean toggle;
    gboolean set;
} Options;

static gpointer setup_taskbar_func(GHashTable *config);
static gpointer setup_pager_func(GHashTable *config);
static void free_func(gpointer o);
static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options);

void action_skip_taskbar_pager_startup(void)
{
    action_register("SkipTaskbar", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    setup_taskbar_func, free_func, run_func);
    action_register("SkipPager", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    setup_pager_func, free_func, run_func);
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

static gpointer setup_taskbar_func(GHashTable *config)
{
    Options *o = setup_func(config);
    o->type = TASKBAR;
    return o;
}

static gpointer setup_pager_func(GHashTable *config)
{
    Options *o = setup_func(config);
    o->type = PAGER;
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
    gboolean toggle;
    if (o->type == TASKBAR) {
        toggle = !data->target->skip_taskbar;
        client_set_skip_taskbar(data->target, (o->toggle ? toggle : o->set));
    }
    else {
        toggle = !data->target->skip_pager;
        client_set_skip_pager(data->target, (o->toggle ? toggle : o->set));
    }
    return TRUE;
}

/* Always return FALSE because its not interactive */
static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options)
{
    if (!client_set_is_empty(set))
        client_set_run(set, data, each_run, options);
    return FALSE;
}
