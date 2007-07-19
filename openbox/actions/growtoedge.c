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

void action_growtoedge_startup()
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

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;

    if (data->client) {
        gint x, y, w, h, realw, realh, lw, lh;

        /* don't allow vertical resize if shaded */
        if (o->dir != OB_DIRECTION_NORTH || o->dir != OB_DIRECTION_SOUTH ||
            !data->client->shaded)
        {
            client_find_resize_directional(data->client, o->dir, TRUE,
                                           &x, &y, &w, &h);
            realw = w;
            realh = h;
            client_try_configure(data->client, &x, &y, &realw, &realh,
                                 &lw, &lh, TRUE);
            /* if it's going to be resized smaller than it intended, don't
               move the window over */
            if (x != data->client->area.x) x += w - realw;
            if (y != data->client->area.y) y += h - realh;

            if (x != data->client->area.x || y != data->client->area.y ||
                w != data->client->area.width ||
                h != data->client->area.height)
            {
                actions_client_move(data, TRUE);
                client_move_resize(data->client, x, y, realw, realh);
                actions_client_move(data, FALSE);
            }
        }
    }

    return FALSE;
}
