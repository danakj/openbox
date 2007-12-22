#include "openbox/actions.h"
#include "openbox/misc.h"
#include "openbox/client.h"
#include "openbox/frame.h"
#include "openbox/screen.h"
#include <glib.h>

typedef struct {
    ObDirection dir;
} Options;

static gpointer setup_north_func(ObParseInst *i,
                                 xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_south_func(ObParseInst *i,
                                 xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_east_func(ObParseInst *i,
                                xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_west_func(ObParseInst *i,
                                xmlDocPtr doc, xmlNodePtr node);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_growtoedge_startup(void)
{
    actions_register("GrowToEdgeNorth", setup_north_func, g_free, run_func,
                     NULL, NULL);
    actions_register("GrowToEdgeSouth", setup_south_func, g_free, run_func,
                     NULL, NULL);
    actions_register("GrowToEdgeEast", setup_east_func, g_free, run_func,
                     NULL, NULL);
    actions_register("GrowToEdgeWest", setup_west_func, g_free, run_func,
                     NULL, NULL);
}

static gpointer setup_north_func(ObParseInst *i,
                                 xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    o->dir = OB_DIRECTION_NORTH;
    return o;
}

static gpointer setup_south_func(ObParseInst *i,
                                 xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    o->dir = OB_DIRECTION_SOUTH;
    return o;
}

static gpointer setup_east_func(ObParseInst *i,
                                xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    o->dir = OB_DIRECTION_EAST;
    return o;
}

static gpointer setup_west_func(ObParseInst *i,
                                xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    o->dir = OB_DIRECTION_WEST;
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

    /* try grow */
    client_find_resize_directional(data->client, o->dir, TRUE,
                                   &x, &y, &w, &h);
    if (do_grow(data, x, y, w, h))
        return FALSE;

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
