#include "client.h"
#include "group.h"
#include "screen.h"
#include "frame.h"
#include "focus.h"
#include "config.h"

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

    return TRUE;
}

static GSList* area_add(GSList *list, Rect *a)
{
    Rect *r = g_new(Rect, 1);
    *r = *a;
    return g_slist_prepend(list, r);
}

static GSList* area_remove(GSList *list, Rect *a)
{
    GSList *sit;
    GSList *result = NULL;

    for (sit = list; sit; sit = g_slist_next(sit)) {
        Rect *r = sit->data;

        if (!RECT_INTERSECTS_RECT(*r, *a)) {
            result = g_slist_prepend(result, r);
            r = NULL; /* dont free it */
        } else {
            Rect isect, extra;

            /* Use an intersection of a and r to determine the space
               around r that we can use.

               NOTE: the spaces calculated can overlap.
            */

            RECT_SET_INTERSECTION(isect, *r, *a);

            if (RECT_LEFT(isect) > RECT_LEFT(*r)) {
                RECT_SET(extra, r->x, r->y,
                         RECT_LEFT(isect) - r->x, r->height);
                result = area_add(result, &extra);
            }

            if (RECT_TOP(isect) > RECT_TOP(*r)) {
                RECT_SET(extra, r->x, r->y,
                         r->width, RECT_TOP(isect) - r->y + 1);
                result = area_add(result, &extra);
            }

            if (RECT_RIGHT(isect) < RECT_RIGHT(*r)) {
                RECT_SET(extra, RECT_RIGHT(isect) + 1, r->y,
                         RECT_RIGHT(*r) - RECT_RIGHT(isect), r->height);
                result = area_add(result, &extra);
            }

            if (RECT_BOTTOM(isect) < RECT_BOTTOM(*r)) {
                RECT_SET(extra, r->x, RECT_BOTTOM(isect) + 1,
                         r->width, RECT_BOTTOM(*r) - RECT_BOTTOM(isect));
                result = area_add(result, &extra);
            }
        }

        g_free(r);
    }
    g_slist_free(list);
    return result;
}

static gint area_cmp(gconstpointer p1, gconstpointer p2)
{
    const Rect *a1 = p1, *a2 = p2;

    return a1->width * a1->height - a2->width * a2->height;
}

static gboolean place_smart(ObClient *client, gint *x, gint *y,
                            gboolean only_focused)
{
    guint i;
    gboolean ret = FALSE;
    GSList *spaces = NULL, *sit;
    GList *it, *list;

    list = focus_order[client->desktop == DESKTOP_ALL ?
                       screen_desktop : client->desktop];

    for (i = 0; i < screen_num_monitors; ++i)
        spaces = area_add(spaces, screen_area_monitor(client->desktop, i));

    for (it = list; it; it = g_list_next(it)) {
        ObClient *c = it->data;

        if (c != client && !c->shaded && client_normal(c)) {
            spaces = area_remove(spaces, &c->frame->area);
            if (only_focused)
                break;
        }
    }

    spaces = g_slist_sort(spaces, area_cmp);

    for (sit = spaces; sit; sit = g_slist_next(sit)) {
        Rect *r = sit->data;

        if (!ret) {
            if (r->width >= client->frame->area.width &&
                r->height >= client->frame->area.height) {
                ret = TRUE;
                *x = r->x + (r->width - client->frame->area.width) / 2;
                *y = r->y + (r->height - client->frame->area.height) / 2;
            }
        }

        g_free(r);
    }
    g_slist_free(spaces);

    return ret;
}

static gboolean place_under_mouse(ObClient *client, gint *x, gint *y)
{
    guint i;
    gint l, r, t, b;
    gint px, py;
    Rect *area;

    screen_pointer_pos(&px, &py);

    for (i = 0; i < screen_num_monitors; ++i) {
        area = screen_area_monitor(client->desktop, i);
        if (RECT_CONTAINS(*area, px, py))
            break;
    }
    if (i == screen_num_monitors)
        area = screen_area_monitor(client->desktop, 0);

    l = area->x;
    t = area->y;
    r = area->x + area->width - client->frame->area.width;
    b = area->y + area->height - client->frame->area.height;

    *x = px - client->area.width / 2 - client->frame->size.left;
    *x = MIN(MAX(*x, l), r);
    *y = py - client->area.height / 2 - client->frame->size.top;
    *y = MIN(MAX(*y, t), b);

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
                        l = RECT_LEFT(m->frame->area);
                        t = RECT_TOP(m->frame->area);
                        r = RECT_RIGHT(m->frame->area);
                        b = RECT_BOTTOM(m->frame->area);
                        first = FALSE;
                    } else {
                        l = MIN(l, RECT_LEFT(m->frame->area));
                        t = MIN(t, RECT_TOP(m->frame->area));
                        r = MAX(r, RECT_RIGHT(m->frame->area));
                        b = MAX(b, RECT_BOTTOM(m->frame->area));
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
    if (place_transient(client, x, y)    ||
        place_dialog(client, x, y)       ||
        place_smart(client, x, y, FALSE) ||
        place_smart(client, x, y, TRUE)  ||
        (config_focus_follow ?
         place_under_mouse(client, x, y) :
         place_random(client, x, y)))
    {
        /* get where the client should be */
        frame_frame_gravity(client->frame, x, y);
    } else
        g_assert_not_reached(); /* the last one better succeed */
}
