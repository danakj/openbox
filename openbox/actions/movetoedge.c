#include "openbox/actions.h"
#include "openbox/misc.h"
#include "openbox/client.h"
#include "openbox/frame.h"
#include "openbox/geom.h"
#include <glib.h>

typedef struct {
    ObDirection dir;
} Options;

static gpointer setup_north_func(ObParseInst *i, xmlDocPtr doc,
                                 xmlNodePtr node);
static gpointer setup_south_func(ObParseInst *i, xmlDocPtr doc,
                                 xmlNodePtr node);
static gpointer setup_east_func(ObParseInst *i, xmlDocPtr doc,
                                xmlNodePtr node);
static gpointer setup_west_func(ObParseInst *i, xmlDocPtr doc,
                                xmlNodePtr node);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_movetoedge_startup()
{
    actions_register("MoveToEdgeNorth", setup_north_func, g_free, run_func,
                     NULL, NULL);
    actions_register("MoveToEdgeSouth", setup_south_func, g_free, run_func,
                     NULL, NULL);
    actions_register("MoveToEdgeEast", setup_east_func, g_free, run_func,
                     NULL, NULL);
    actions_register("MoveToEdgeWest", setup_west_func, g_free, run_func,
                     NULL, NULL);
}

static gpointer setup_north_func(ObParseInst *i, xmlDocPtr doc,
                                 xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    o->dir = OB_DIRECTION_NORTH;
    return o;
}

static gpointer setup_south_func(ObParseInst *i, xmlDocPtr doc,
                                 xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    o->dir = OB_DIRECTION_SOUTH;
    return o;
}

static gpointer setup_east_func(ObParseInst *i, xmlDocPtr doc,
                                 xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    o->dir = OB_DIRECTION_EAST;
    return o;
}

static gpointer setup_west_func(ObParseInst *i, xmlDocPtr doc,
                                 xmlNodePtr node)
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
        gint x, y;

        client_find_move_directional(data->client, o->dir, &x, &y);
        if (x != data->client->area.x || y != data->client->area.y) {
            actions_client_move(data, TRUE);
            client_move(data->client, x, y);
            actions_client_move(data, FALSE);
        }
    }

    return FALSE;
}
