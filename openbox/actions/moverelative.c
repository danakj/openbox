#include "openbox/actions.h"
#include "openbox/actions_value.h"
#include "openbox/client.h"
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
static gboolean run_func(ObActionsData *data, gpointer options);

void action_moverelative_startup(void)
{
    actions_register("MoveRelative", setup_func, free_func, run_func);
}

static gpointer setup_func(GHashTable *config)
{
    ObActionsValue *v;
    Options *o;

    o = g_slice_new0(Options);

    v = g_hash_table_lookup(config, "x");
    if (v && actions_value_is_string(v))
        actions_value_fraction(v, &o->x, &o->x_denom);
    v = g_hash_table_lookup(config, "y");
    if (v && actions_value_is_string(v))
        actions_value_fraction(v, &o->y, &o->y_denom);

    return o;
}

static void free_func(gpointer o)
{
    g_slice_free(Options, o);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;

    if (data->client) {
        ObClient *c;
        gint x, y, lw, lh, w, h;

        c = data->client;
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

        actions_client_move(data, TRUE);
        client_configure(c, x, y, w, h, TRUE, TRUE, FALSE);
        actions_client_move(data, FALSE);
    }

    return FALSE;
}
