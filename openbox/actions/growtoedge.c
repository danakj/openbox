#include "openbox/actions.h"
#include "openbox/actions_value.h"
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
static gboolean run_func(ObActionsData *data, gpointer options);
/* 3.4-compatibility */
static gpointer setup_north_func(GHashTable *config);
static gpointer setup_south_func(GHashTable *config);
static gpointer setup_east_func(GHashTable *config);
static gpointer setup_west_func(GHashTable *config);

void action_growtoedge_startup(void)
{
    actions_register("GrowToEdge", setup_func,
                     free_func, run_func);
    actions_register("ShrinkToEdge", setup_shrink_func,
                     free_func, run_func);
    /* 3.4-compatibility */
    actions_register("GrowToEdgeNorth", setup_north_func, free_func, run_func);
    actions_register("GrowToEdgeSouth", setup_south_func, free_func, run_func);
    actions_register("GrowToEdgeEast", setup_east_func, free_func, run_func);
    actions_register("GrowToEdgeWest", setup_west_func, free_func, run_func);
}

static gpointer setup_func(GHashTable *config)
{
    ObActionsValue *v;
    Options *o;

    o = g_slice_new0(Options);
    o->dir = OB_DIRECTION_NORTH;
    o->shrink = FALSE;

    v = g_hash_table_lookup(config, "direction");
    if (v && actions_value_is_string(v)) {
        const gchar *s = actions_value_string(v);
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

static gboolean do_grow(ObActionsData *data, gint x, gint y, gint w, gint h)
{
    gint realw, realh, lw, lh;

    realw = w;
    realh = h;
    client_try_configure(data->client, &x, &y, &realw, &realh,
                         &lw, &lh, TRUE);
    /* if it's going to be resized smaller than it intended, don't
       move the window over */
    if (x != data->client->area.x) x += w - realw;
    if (y != data->client->area.y) y += h - realh;

    if (x != data->client->area.x || y != data->client->area.y ||
        realw != data->client->area.width ||
        realh != data->client->area.height)
    {
        actions_client_move(data, TRUE);
        client_move_resize(data->client, x, y, realw, realh);
        actions_client_move(data, FALSE);
        return TRUE;
    }
    return FALSE;
}

static void free_func(gpointer o)
{
    g_slice_free(Options, o);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;
    gint x, y, w, h;
    ObDirection opp;
    gint half;

    if (!data->client ||
        /* don't allow vertical resize if shaded */
        ((o->dir == OB_DIRECTION_NORTH || o->dir == OB_DIRECTION_SOUTH) &&
         data->client->shaded))
    {
        return FALSE;
    }

    if (!o->shrink) {
        /* try grow */
        client_find_resize_directional(data->client, o->dir, TRUE,
                                       &x, &y, &w, &h);
        if (do_grow(data, x, y, w, h))
            return FALSE;
    }

    /* we couldn't grow, so try shrink! */
    opp = (o->dir == OB_DIRECTION_NORTH ? OB_DIRECTION_SOUTH :
           (o->dir == OB_DIRECTION_SOUTH ? OB_DIRECTION_NORTH :
            (o->dir == OB_DIRECTION_EAST ? OB_DIRECTION_WEST :
             OB_DIRECTION_EAST)));
    client_find_resize_directional(data->client, opp, FALSE,
                                   &x, &y, &w, &h);
    switch (opp) {
    case OB_DIRECTION_NORTH:
        half = data->client->area.y + data->client->area.height / 2;
        if (y > half) {
            h += y - half;
            y = half;
        }
        break;
    case OB_DIRECTION_SOUTH:
        half = data->client->area.height / 2;
        if (h < half)
            h = half;
        break;
    case OB_DIRECTION_WEST:
        half = data->client->area.x + data->client->area.width / 2;
        if (x > half) {
            w += x - half;
            x = half;
        }
        break;
    case OB_DIRECTION_EAST:
        half = data->client->area.width / 2;
        if (w < half)
            w = half;
        break;
    default: g_assert_not_reached();
    }
    if (do_grow(data, x, y, w, h))
        return FALSE;

    return FALSE;
}

/* 3.4-compatibility */
static gpointer setup_north_func(GHashTable *config)
{
    Options *o = g_slice_new0(Options);
    o->shrink = FALSE;
    o->dir = OB_DIRECTION_NORTH;
    return o;
}

static gpointer setup_south_func(GHashTable *config)
{
    Options *o = g_slice_new0(Options);
    o->shrink = FALSE;
    o->dir = OB_DIRECTION_SOUTH;
    return o;
}

static gpointer setup_east_func(GHashTable *config)
{
    Options *o = g_slice_new0(Options);
    o->shrink = FALSE;
    o->dir = OB_DIRECTION_EAST;
    return o;
}

static gpointer setup_west_func(GHashTable *config)
{
    Options *o = g_slice_new0(Options);
    o->shrink = FALSE;
    o->dir = OB_DIRECTION_WEST;
    return o;
}
