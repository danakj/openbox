#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/config_value.h"
#include "openbox/client.h"
#include "openbox/client_set.h"
#include "openbox/screen.h"
#include "openbox/frame.h"
#include "openbox/config.h"

typedef struct {
    gint x;
    gint x_denom;
    gint y;
    gint y_denom;
} Options;

static gpointer setup_func(GHashTable *config);
static void free_func(gpointer o);
static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options);

void action_moverelative_startup(void)
{
    action_register("MoveRelative", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    setup_func, free_func, run_func);
}

static gpointer setup_func(GHashTable *config)
{
    ObConfigValue *v;
    Options *o;

    o = g_slice_new0(Options);

    v = g_hash_table_lookup(config, "x");
    if (v && config_value_is_string(v))
        config_value_fraction(v, &o->x, &o->x_denom);
    v = g_hash_table_lookup(config, "y");
    if (v && config_value_is_string(v))
        config_value_fraction(v, &o->y, &o->y_denom);

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
    gint x, y, lw, lh, w, h;

    x = o->x;
    y = o->y;
    if (o->x_denom || o->y_denom) {
        const Rect *carea;

        carea = screen_area(c->desktop, client_monitor(c), NULL);
        if (o->x_denom)
            x = (x * carea->width) / o->x_denom;
        if (o->y_denom)
            y = (y * carea->height) / o->y_denom;
    }
    x = c->area.x + x;
    y = c->area.y + y;
    w = c->area.width;
    h = c->area.height;
    client_try_configure(c, &x, &y, &w, &h, &lw, &lh, TRUE);
    client_find_onscreen(c, &x, &y, w, h, FALSE);
    client_configure(c, x, y, w, h, TRUE, TRUE, FALSE);
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
