#include "openbox/action.h"
#include "openbox/action_value.h"
#include "openbox/menu.h"
#include <glib.h>

typedef struct {
    gchar   *name;
} Options;

static gpointer setup_func(GHashTable *config);
static void     free_func(gpointer options);
static gboolean run_func(ObActionData *data, gpointer options);

void action_showmenu_startup(void)
{
    action_register("ShowMenu", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    setup_func, free_func, run_func);
}

static gpointer setup_func(GHashTable *config)
{
    ObActionValue *v;
    Options *o;

    o = g_slice_new0(Options);

    v = g_hash_table_lookup(config, "menu");
    if (v && action_value_is_string(v))
        o->name = g_strdup(action_value_string(v));
    return o;
}

static void free_func(gpointer options)
{
    Options *o = options;
    g_free(o->name);
    g_slice_free(Options, o);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionData *data, gpointer options)
{
    Options *o = options;

    /* you cannot call ShowMenu from inside a menu */
    if (data->uact != OB_USER_ACTION_MENU_SELECTION && o->name)
        menu_show(o->name, data->x, data->y, data->button != 0, data->client);

    return FALSE;
}
