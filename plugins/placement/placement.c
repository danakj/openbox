#include "kernel/dispatch.h"
#include "kernel/client.h"
#include "kernel/group.h"
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

static Rect* pick_head(ObClient *c)
{
    /* try direct parent first */
    if (c->transient_for && c->transient_for != TRAN_GROUP) {
        return screen_area_monitor(c->desktop,
                                   client_monitor(c->transient_for));
    }

    /* more than one guy in his group (more than just him) */
    if (c->group && c->group->members->next) {
        GSList *it;

        /* try on the client's desktop */
        for (it = c->group->members; it; it = g_slist_next(it)) {
            ObClient *itc = it->data;            
            if (itc != c &&
                (itc->desktop == c->desktop ||
                 itc->desktop == DESKTOP_ALL || c->desktop == DESKTOP_ALL))
                return screen_area_monitor(c->desktop,
                                           client_monitor(it->data));
        }

        /* try on all desktops */
        for (it = c->group->members; it; it = g_slist_next(it)) {
            ObClient *itc = it->data;            
            if (itc != c)
                return screen_area_monitor(c->desktop,
                                           client_monitor(it->data));
        }
    }

    return NULL;
}

static void place_random(ObClient *c)
{
    int l, r, t, b;
    int x, y;
    Rect *area;

    if (ob_state() == OB_STATE_STARTING) return;
    
    area = pick_head(c);
    if (!area)
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

    if (e->data.c.client->transient_for) {
        if (e->data.c.client->transient_for != OB_TRAN_GROUP) {
            ObClient *c = e->data.c.client;
            ObClient *p = e->data.c.client->transient_for;
            int x = (p->frame->area.width - c->frame->area.width) / 2 +
                p->frame->area.x;
            int y = (p->frame->area.height - c->frame->area.height) / 2 +
                p->frame->area.y;
            client_configure(c, OB_CORNER_TOPLEFT, x, y,
                             c->area.width, c->area.height,
                             TRUE, TRUE);
            return;
        } else {
            GSList *it;
            ObClient *c = e->data.c.client;
            gboolean first = TRUE;
            int l, r, t, b;
            for (it = c->group->members; it; it = it->next) {
                ObClient *m = it->data;
                if (!(m == c || m->transient_for)) {
                    if (first) {
                        l = m->frame->area.x;
                        t = m->frame->area.y;
                        r = m->frame->area.x + m->frame->area.width - 1;
                        b = m->frame->area.y + m->frame->area.height - 1;
                        first = FALSE;
                    } else {
                        l = MIN(l, m->frame->area.x);
                        t = MIN(t, m->frame->area.y);
                        r = MAX(r, m->frame->area.x +m->frame->area.width - 1);
                        b = MAX(b, m->frame->area.y +m->frame->area.height -1);
                    }
                }
            }
            if (!first) {
                int x = ((r + 1 - l) - c->frame->area.width) / 2 + l; 
                int y = ((b + 1 - t) - c->frame->area.height) / 2 + t;
                client_configure(c, OB_CORNER_TOPLEFT, x, y,
                                 c->area.width, c->area.height,
                                 TRUE, TRUE);
                return;
            }
        }
    }

    /* center parentless dialogs on the screen */
    if (e->data.c.client->type == OB_CLIENT_TYPE_DIALOG) {
        Rect *area;
        ObClient *c = e->data.c.client;
        int x, y;

        area = pick_head(c);
        if (!area)
            area = screen_area_monitor(c->desktop, 0);

        x = (area->width - c->frame->area.width) / 2 + area->x;
        y = (area->height - c->frame->area.height) / 2 + area->y;
        client_configure(c, OB_CORNER_TOPLEFT, x, y,
                         c->area.width, c->area.height,
                         TRUE, TRUE);
        return;
    }

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
