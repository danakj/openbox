#include "client.h"
#include "group.h"
#include "screen.h"
#include "frame.h"

static Rect* pick_head(ObClient *c)
{
    /* try direct parent first */
    if (c->transient_for && c->transient_for != OB_TRAN_GROUP) {
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

static gboolean place_random(ObClient *client, gint *x, gint *y)
{
    int l, r, t, b;
    Rect *area;

    area = pick_head(client);
    if (!area)
        area = screen_area_monitor(client->desktop,
                                   g_random_int_range(0, screen_num_monitors));

    l = area->x;
    t = area->y;
    r = area->x + area->width - client->frame->area.width;
    b = area->y + area->height - client->frame->area.height;

    if (r > l) *x = g_random_int_range(l, r + 1);
    else       *x = 0;
    if (b > t) *y = g_random_int_range(t, b + 1);
    else       *y = 0;

    /* get where the client should be */
    frame_frame_gravity(client->frame, x, y);

    return TRUE;
}

static gboolean place_transient(ObClient *client, gint *x, gint *y)
{
    if (client->transient_for) {
        if (client->transient_for != OB_TRAN_GROUP) {
            ObClient *c = client;
            ObClient *p = client->transient_for;
            *x = (p->frame->area.width - c->frame->area.width) / 2 +
                p->frame->area.x;
            *y = (p->frame->area.height - c->frame->area.height) / 2 +
                p->frame->area.y;
            return TRUE;
        } else {
            GSList *it;
            gboolean first = TRUE;
            int l, r, t, b;
            for (it = client->group->members; it; it = it->next) {
                ObClient *m = it->data;
                if (!(m == client || m->transient_for)) {
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
                *x = ((r + 1 - l) - client->frame->area.width) / 2 + l; 
                *y = ((b + 1 - t) - client->frame->area.height) / 2 + t;
                return TRUE;
            }
        }
    }
    return FALSE;
}

static gboolean place_dialog(ObClient *client, gint *x, gint *y)
{
    /* center parentless dialogs on the screen */
    if (client->type == OB_CLIENT_TYPE_DIALOG) {
        Rect *area;

        area = pick_head(client);
        if (!area)
            area = screen_area_monitor(client->desktop, 0);

        *x = (area->width - client->frame->area.width) / 2 + area->x;
        *y = (area->height - client->frame->area.height) / 2 + area->y;
        return TRUE;
    }
    return FALSE;
}

void place_client(ObClient *client, gint *x, gint *y)
{
    if (client->positioned)
        return;
    if (place_transient(client, x, y))
        return;
    if (place_dialog(client, x, y))
        return;
    if (place_random(client, x, y))
        return;
    g_assert_not_reached(); /* the last one better succeed */
}
