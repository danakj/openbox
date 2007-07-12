#include "openbox/actions.h"
#include "openbox/misc.h"
#include "openbox/client.h"
#include "openbox/frame.h"
#include "openbox/screen.h"
#include <glib.h>

typedef struct {
    ObDirection dir;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_north_func(ObParseInst *i,
                                 xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_south_func(ObParseInst *i,
                                 xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_east_func(ObParseInst *i,
                                xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_west_func(ObParseInst *i,
                                xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_growtoedge_startup()
{
    actions_register("GrowToEdge",
                     setup_func,
                     free_func,
                     run_func,
                     NULL, NULL);
    actions_register("GrowToEdgeNorth",
                     setup_north_func,
                     free_func,
                     run_func,
                     NULL, NULL);
    actions_register("GrowToEdgeSouth",
                     setup_south_func,
                     free_func,
                     run_func,
                     NULL, NULL);
    actions_register("GrowToEdgeEast",
                     setup_east_func,
                     free_func,
                     run_func,
                     NULL, NULL);
    actions_register("GrowToEdgeWest",
                     setup_west_func,
                     free_func,
                     run_func,
                     NULL, NULL);
}

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_new0(Options, 1);
    o->dir = OB_DIRECTION_NORTH;

    if ((n = parse_find_node("direction", node))) {
        gchar *s = parse_string(doc, n);
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
        g_free(s);
    }

    return o;
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

static void free_func(gpointer options)
{
    Options *o = options;

    g_free(o);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;

    if (data->client) {
        gint x, y, w, h;

        /* don't allow vertical resize if shaded */
        if (o->dir != OB_DIRECTION_NORTH || o->dir != OB_DIRECTION_SOUTH ||
            !data->client->shaded)
        {
            client_find_resize_directional(data->client, o->dir, TRUE,
                                           &x, &y, &w, &h);
            if (x != data->client->area.x || y != data->client->area.y ||
                w != data->client->area.width ||
                h != data->client->area.height)
            {
                actions_client_move(data, TRUE);
                client_move_resize(data->client, x, y, w, h);
                actions_client_move(data, FALSE);
            }
        }
    }

    return FALSE;
}
