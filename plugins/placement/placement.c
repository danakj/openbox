#include "kernel/dispatch.h"
#include "kernel/client.h"
#include "kernel/frame.h"
#include "kernel/screen.h"
#include "kernel/openbox.h"
#include "parser/parse.h"
#include "history.h"
#include <glib.h>

static gboolean history;

static void parse_xml(xmlDocPtr doc, xmlNodePtr node, void *d)
{
    xmlNodePtr n;

    node = node->xmlChildrenNode;
    if ((n = parse_find_node("remember", node)))
        history = parse_bool(doc, n);
}

void plugin_setup_config()
{
    history = TRUE;

    parse_register("placement", parse_xml, NULL);
}

static void place_random(ObClient *c)
{
    int l, r, t, b;
    int x, y;
    Rect *area;

    if (ob_state() == OB_STATE_STARTING) return;

    area = screen_area_monitor(c->desktop,
                               g_random_int_range(0, screen_num_monitors));

    l = area->x;
    t = area->y;
    r = area->x + area->width - c->frame->area.width;
    b = area->y + area->height - c->frame->area.height;

    if (r > l) x = g_random_int_range(l, r + 1);
    else       x = 0;
    if (b > t) y = g_random_int_range(t, b + 1);
    else       y = 0;

    frame_frame_gravity(c->frame, &x, &y); /* get where the client should be */
    client_configure(c, OB_CORNER_TOPLEFT, x, y, c->area.width, c->area.height,
                     TRUE, TRUE);
}

static void event(ObEvent *e, void *foo)
{
    g_assert(e->type == Event_Client_New);

    /* requested a position */
    if (e->data.c.client->positioned) return;

    if (!history || !place_history(e->data.c.client))
        place_random(e->data.c.client);
}

void plugin_startup()
{
    dispatch_register(Event_Client_New, (EventHandler)event, NULL);

    history_startup();
}

void plugin_shutdown()
{
    dispatch_register(0, (EventHandler)event, NULL);

    history_shutdown();
}
