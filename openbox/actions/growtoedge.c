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
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_growtoedge_startup()
{
    actions_register("GrowToEdge",
                     setup_func,
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
        gint x, y, width, height, dest;
        ObClient *c = data->client;
        Rect *a;

        a = screen_area(c->desktop, SCREEN_AREA_ALL_MONITORS, &c->frame->area);
        x = c->frame->area.x;
        y = c->frame->area.y;
        /* get the unshaded frame's dimensions..if it is shaded */
        width = c->area.width + c->frame->size.left + c->frame->size.right;
        height = c->area.height + c->frame->size.top + c->frame->size.bottom;

#if 0
        dest = client_directional_edge_search(c, o->dir);

        switch(o->dir) {
        case OB_DIRECTION_NORTH:
            if (c->shaded) break; /* don't allow vertical resize if shaded */

            if (a->y == y)
                height = height / 2;
            else {
                height = c->frame->area.y + height - dest;
                y = dest;
            }
            break;
        case OB_DIRECTION_WEST:
            if (a->x == x)
                width = width / 2;
            else {
                width = c->frame->area.x + width - dest;
                x = dest;
            }
            break;
        case OB_DIRECTION_SOUTH:
            if (c->shaded) break; /* don't allow vertical resize if shaded */

            if (a->y + a->height == y + c->frame->area.height) {
                height = c->frame->area.height / 2;
                y = a->y + a->height - height;
            } else
                height = dest - c->frame->area.y;
            y += (height - c->frame->area.height) % c->size_inc.height;
            height -= (height - c->frame->area.height) % c->size_inc.height;
            break;
        case OB_DIRECTION_EAST:
            if (a->x + a->width == x + c->frame->area.width) {
                width = c->frame->area.width / 2;
                x = a->x + a->width - width;
            } else
                width = dest - c->frame->area.x;
            x += (width - c->frame->area.width) % c->size_inc.width;
            width -= (width - c->frame->area.width) % c->size_inc.width;
            break;
        default:
            g_assert_not_reached();
        }

        width -= c->frame->size.left + c->frame->size.right;
        height -= c->frame->size.top + c->frame->size.bottom;
        frame_frame_gravity(c->frame, &x, &y);

        actions_client_move(data, FALSE);
        client_move_resize(c, x, y, width, height);
        actions_client_move(data, TRUE);

#endif
        g_free(a);
    }

    return FALSE;
}
