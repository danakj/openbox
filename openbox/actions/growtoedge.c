#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/action_value.h"
#include "openbox/misc.h"
#include "openbox/client.h"
#include "openbox/frame.h"
#include "openbox/screen.h"
#include <glib.h>

typedef struct {
    ObDirection dir;
    gboolean shrink;
} Options;

static gpointer setup_func(GHashTable *config);
static gpointer setup_shrink_func(GHashTable *config);
static void free_func(gpointer o);
static gboolean run_func(const ObActionListRun *data, gpointer options);

void action_growtoedge_startup(void)
{
    action_register("GrowToEdge", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    setup_func, free_func, run_func);
    action_register("ShrinkToEdge", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    setup_shrink_func, free_func, run_func);
}

static gpointer setup_func(GHashTable *config)
{
    ObActionValue *v;
    Options *o;

    o = g_slice_new0(Options);
    o->dir = OB_DIRECTION_NORTH;
    o->shrink = FALSE;

    v = g_hash_table_lookup(config, "direction");
    if (v && action_value_is_string(v)) {
        const gchar *s = action_value_string(v);
        if (!g_ascii_strcasecmp(s, "north") ||
            !g_ascii_strcasecmp(s, "up"))
            o->dir = OB_DIRECTION_NORTH;
        else if (!g_ascii_strcasecmp(s, "south") ||
                 !g_ascii_strcasecmp(s, "down"))
            o->dir = OB_DIRECTION_SOUTH;
        else if (!g_ascii_strcasecmp(s, "west") ||
                 !g_ascii_strcasecmp(s, "left"))
            o->dir = OB_DIRECTION_WEST;
        else if (!g_ascii_strcasecmp(s, "east") ||
                 !g_ascii_strcasecmp(s, "right"))
            o->dir = OB_DIRECTION_EAST;
    }

    return o;
}

static gpointer setup_shrink_func(GHashTable *config)
{
    Options *o;

    o = setup_func(config);
    o->shrink = TRUE;

    return o;
}

static gboolean do_grow(const ObActionListRun *data, gint x, gint y, gint w, gint h)
{
    gint realw, realh, lw, lh;

    realw = w;
    realh = h;
    client_try_configure(data->target, &x, &y, &realw, &realh,
                         &lw, &lh, TRUE);
    /* if it's going to be resized smaller than it intended, don't
       move the window over */
    if (x != data->target->area.x) x += w - realw;
    if (y != data->target->area.y) y += h - realh;

    if (x != data->target->area.x || y != data->target->area.y ||
        realw != data->target->area.width ||
        realh != data->target->area.height)
    {
        action_client_move(data, TRUE);
        client_move_resize(data->target, x, y, realw, realh);
        action_client_move(data, FALSE);
        return TRUE;
    }
    return FALSE;
}

static void free_func(gpointer o)
{
    g_slice_free(Options, o);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(const ObActionListRun *data, gpointer options)
{
    Options *o = options;
    gint x, y, w, h;
    ObDirection opp;
    gint half;

    if (!data->target ||
        /* don't allow vertical resize if shaded */
        ((o->dir == OB_DIRECTION_NORTH || o->dir == OB_DIRECTION_SOUTH) &&
         data->target->shaded))
    {
        return FALSE;
    }

    if (!o->shrink) {
        /* try grow */
        client_find_resize_directional(data->target, o->dir, TRUE,
                                       &x, &y, &w, &h);
        if (do_grow(data, x, y, w, h))
            return FALSE;
    }

    /* we couldn't grow, so try shrink! */
    opp = (o->dir == OB_DIRECTION_NORTH ? OB_DIRECTION_SOUTH :
           (o->dir == OB_DIRECTION_SOUTH ? OB_DIRECTION_NORTH :
            (o->dir == OB_DIRECTION_EAST ? OB_DIRECTION_WEST :
             OB_DIRECTION_EAST)));
    client_find_resize_directional(data->target, opp, FALSE,
                                   &x, &y, &w, &h);
    switch (opp) {
    case OB_DIRECTION_NORTH:
        half = data->target->area.y + data->target->area.height / 2;
        if (y > half) {
            h += y - half;
            y = half;
        }
        break;
    case OB_DIRECTION_SOUTH:
        half = data->target->area.height / 2;
        if (h < half)
            h = half;
        break;
    case OB_DIRECTION_WEST:
        half = data->target->area.x + data->target->area.width / 2;
        if (x > half) {
            w += x - half;
            x = half;
        }
        break;
    case OB_DIRECTION_EAST:
        half = data->target->area.width / 2;
        if (w < half)
            w = half;
        break;
    default: g_assert_not_reached();
    }
    if (do_grow(data, x, y, w, h))
        return FALSE;

    return FALSE;
}
