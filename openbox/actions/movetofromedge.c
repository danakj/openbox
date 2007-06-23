#include "openbox/actions.h"
#include "openbox/misc.h"
#include "openbox/client.h"
#include "openbox/frame.h"
#include <glib.h>

typedef struct {
    ObDirection dir;
    gboolean hang;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_to_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_from_func(ObParseInst *i,xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_movetofromedge_startup()
{
    actions_register("MoveToEdge",
                     setup_to_func,
                     free_func,
                     run_func,
                     NULL, NULL);
    actions_register("MoveFromEdge",
                     setup_from_func,
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

static gpointer setup_to_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = setup_func(i, doc, node);
    o->hang = FALSE;
    return o;
}

static gpointer setup_from_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = setup_func(i, doc, node);
    o->hang = TRUE;
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
        gint x, y;
        ObClient *c = data->client;

        x = c->frame->area.x;
        y = c->frame->area.y;
    
        switch(o->dir) {
        case OB_DIRECTION_NORTH:
            y = client_directional_edge_search(c, OB_DIRECTION_NORTH,
                                               o->hang)
                - (o->hang ? c->frame->area.height : 0);
            break;
        case OB_DIRECTION_WEST:
            x = client_directional_edge_search(c, OB_DIRECTION_WEST,
                                               o->hang)
                - (o->hang ? c->frame->area.width : 0);
            break;
        case OB_DIRECTION_SOUTH:
            y = client_directional_edge_search(c, OB_DIRECTION_SOUTH,
                                               o->hang)
                - (o->hang ? 0 : c->frame->area.height);
            break;
        case OB_DIRECTION_EAST:
            x = client_directional_edge_search(c, OB_DIRECTION_EAST,
                                               o->hang)
                - (o->hang ? 0 : c->frame->area.width);
            break;
        default:
            g_assert_not_reached();
        }
        frame_frame_gravity(c->frame, &x, &y);

        actions_client_move(data, FALSE);
        client_move(c, x, y);
        actions_client_move(data, TRUE);
    }

    return FALSE;
}
