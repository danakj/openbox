/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   place.c for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
   Copyright (c) 2003        Ben Jansens

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   See the COPYING file for a copy of the GNU General Public License.
*/

#include "client.h"
#include "group.h"
#include "screen.h"
#include "frame.h"
#include "focus.h"
#include "config.h"

static void add_choice(guint *choice, guint mychoice)
{
    guint i;
    for (i = 0; i < screen_num_monitors; ++i) {
        if (choice[i] == mychoice)
            return;
        else if (choice[i] == screen_num_monitors) {
            choice[i] = mychoice;
            return;
        }
    }
}

static Rect *pick_pointer_head(ObClient *c)
{
    guint i;
    gint px, py;

    screen_pointer_pos(&px, &py);
     
    for (i = 0; i < screen_num_monitors; ++i) {  
        if (RECT_CONTAINS(*screen_physical_area_monitor(i), px, py)) {
            return screen_area_monitor(c->desktop, i);
        }
    }
    g_assert_not_reached();
}

static Rect **pick_head(ObClient *c)
{
    Rect **area;
    guint *choice;
    guint i;
    gint px, py;

    area = g_new(Rect*, screen_num_monitors);
    choice = g_new(guint, screen_num_monitors);
    for (i = 0; i < screen_num_monitors; ++i)
        choice[i] = screen_num_monitors; /* make them all invalid to start */

    /* try direct parent first */
    if (c->transient_for && c->transient_for != OB_TRAN_GROUP) {
        add_choice(choice, client_monitor(c->transient_for));
    }

    /* more than one window in its group (more than just this window) */
    if (client_has_group_siblings(c)) {
        GSList *it;

        /* try on the client's desktop */
        for (it = c->group->members; it; it = g_slist_next(it)) {
            ObClient *itc = it->data;            
            if (itc != c &&
                (itc->desktop == c->desktop ||
                 itc->desktop == DESKTOP_ALL || c->desktop == DESKTOP_ALL))
                add_choice(choice, client_monitor(it->data));
        }

        /* try on all desktops */
        for (it = c->group->members; it; it = g_slist_next(it)) {
            ObClient *itc = it->data;            
            if (itc != c)
                add_choice(choice, client_monitor(it->data));
        }
    }

    if (focus_client)
        add_choice(choice, client_monitor(focus_client));

    screen_pointer_pos(&px, &py);

    for (i = 0; i < screen_num_monitors; i++)
        if (RECT_CONTAINS(*screen_physical_area_monitor(i), px, py)) {
            add_choice(choice, i);
            break;
        }

    /* add any leftover choices */
    for (i = 0; i < screen_num_monitors; ++i)
        add_choice(choice, i);

    for (i = 0; i < screen_num_monitors; ++i)
        area[i] = screen_area_monitor(c->desktop, choice[i]);

    return area;
}

static gboolean place_random(ObClient *client, gint *x, gint *y)
{
    gint l, r, t, b;
    Rect **areas;
    guint i;

    areas = pick_head(client);
    i = g_random_int_range(0, screen_num_monitors);

    l = areas[i]->x;
    t = areas[i]->y;
    r = areas[i]->x + areas[i]->width - client->frame->area.width;
    b = areas[i]->y + areas[i]->height - client->frame->area.height;

    if (r > l) *x = g_random_int_range(l, r + 1);
    else       *x = 0;
    if (b > t) *y = g_random_int_range(t, b + 1);
    else       *y = 0;

    g_free(areas);

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

static gint area_cmp(gconstpointer p1, gconstpointer p2, gpointer data)
{
    ObClient *c = data;
    Rect *carea = &c->frame->area;
    const Rect *a1 = p1, *a2 = p2;
    gboolean diffhead = FALSE;
    guint i;
    Rect *a;

    for (i = 0; i < screen_num_monitors; ++i) {
        a = screen_physical_area_monitor(i);
        if (RECT_CONTAINS(*a, a1->x, a1->y) &&
            !RECT_CONTAINS(*a, a2->x, a2->y))
        {
            diffhead = TRUE;
            break;
        }
    }

    /* has to be more than me in the group */
    if (diffhead && client_has_group_siblings(c)) {
        guint *num, most;
        GSList *it;

        /* find how many clients in the group are on each monitor, use the
           monitor with the most in it */
        num = g_new0(guint, screen_num_monitors);
        for (it = c->group->members; it; it = g_slist_next(it))
            if (it->data != c)
                ++num[client_monitor(it->data)];
        most = 0;
        for (i = 1; i < screen_num_monitors; ++i)
            if (num[i] > num[most])
                most = i;

        g_free(num);

        a = screen_physical_area_monitor(most);
        if (RECT_CONTAINS(*a, a1->x, a1->y))
            return -1;
        if (RECT_CONTAINS(*a, a2->x, a2->y))
            return 1;
    }

    return MIN((a1->width - carea->width), (a1->height - carea->height)) -
        MIN((a2->width - carea->width), (a2->height - carea->height));
}

typedef enum
{
    SMART_FULL,
    SMART_GROUP,
    SMART_FOCUSED
} ObSmartType;

#define SMART_IGNORE(placer, c) \
    (placer == c || !c->frame->visible || c->shaded || !client_normal(c) || \
     (c->desktop != DESKTOP_ALL && \
      c->desktop != (placer->desktop == DESKTOP_ALL ? \
                     screen_desktop : placer->desktop)))

static gboolean place_smart(ObClient *client, gint *x, gint *y,
                            ObSmartType type)
{
    gboolean ret = FALSE;
    GSList *spaces = NULL, *sit;
    GList *it;
    Rect **areas;
    guint i;

    if (type == SMART_GROUP) {
        /* has to be more than me in the group */
        if (!client_has_group_siblings(client))
            return FALSE;
    }

    areas = pick_head(client);

    for (i = 0; i < screen_num_monitors; ++i) {
        spaces = area_add(spaces, areas[i]);

        /* stay out from under windows in higher layers */
        for (it = stacking_list; it; it = g_list_next(it)) {
            ObClient *c;

            if (WINDOW_IS_CLIENT(it->data)) {
                c = it->data;
                if (c->fullscreen)
                    continue;
            } else
                continue;

            if (c->layer > client->layer) {
                if (!SMART_IGNORE(client, c))
                    spaces = area_remove(spaces, &c->frame->area);
            } else
                break;
        }

        if (type == SMART_FULL || type == SMART_FOCUSED) {
            gboolean found_foc = FALSE, stop = FALSE;
            ObClient *foc;
            GList *list;

            list = focus_order[client->desktop == DESKTOP_ALL ?
                               screen_desktop : client->desktop];
            foc = list ? list->data : NULL;

            for (; it && !stop; it = g_list_next(it)) {
                ObClient *c;

                if (WINDOW_IS_CLIENT(it->data)) {
                    c = it->data;
                    if (c->fullscreen)
                        continue;
                } else
                    continue;

                if (!SMART_IGNORE(client, c)) {
                    if (type == SMART_FOCUSED)
                        if (found_foc)
                            stop = TRUE;
                    if (!stop)
                        spaces = area_remove(spaces, &c->frame->area);
                }

                if (c == foc)
                    found_foc = TRUE;
            }
        } else if (type == SMART_GROUP) {
            for (sit = client->group->members; sit; sit = g_slist_next(sit)) {
                ObClient *c = sit->data;
                if (!SMART_IGNORE(client, c))
                    spaces = area_remove(spaces, &c->frame->area);
            }
        } else
            g_assert_not_reached();

        spaces = g_slist_sort_with_data(spaces, area_cmp, client);

        for (sit = spaces; sit; sit = g_slist_next(sit)) {
            Rect *r = sit->data;

            if (!ret) {
                if (r->width >= client->frame->area.width &&
                    r->height >= client->frame->area.height) {
                    ret = TRUE;
                    if (client->type == OB_CLIENT_TYPE_DIALOG ||
                        type != SMART_FULL)
                    {
                        *x = r->x + (r->width - client->frame->area.width)/2;
                        *y = r->y + (r->height - client->frame->area.height)/2;
                    } else {
                        *x = r->x;
                        *y = r->y;
                    }
                }
            }

            g_free(r);
        }
        g_slist_free(spaces);
        spaces = NULL;
    }

    g_free(areas);

    return ret;
}

static gboolean place_under_mouse(ObClient *client, gint *x, gint *y)
{
    gint l, r, t, b;
    gint px, py;
    Rect *area;

    area = pick_pointer_head(client);

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

static gboolean place_per_app_setting(ObClient *client, gint *x, gint *y,
                                      ObAppSettings *settings)
{
    Rect *screen;

    if (!settings || (settings && !settings->pos_given))
        return FALSE;

    /* Find which head the pointer is on */
    if (settings->head == -1)
        screen = pick_pointer_head(client);
    else
        screen = screen_area_monitor(client->desktop, settings->head);

    if (settings->center_x)
        *x = screen->x + screen->width / 2 - client->area.width / 2;
    else
        *x = screen->x + settings->position.x;

    if (settings->center_y)
        *y = screen->y + screen->height / 2 - client->area.height / 2;
    else
        *y = screen->y + settings->position.y;

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
            gint l, r, t, b;
            for (it = client->group->members; it; it = g_slist_next(it)) {
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

/* Return TRUE if we want client.c to enforce on-screen-keeping */
gboolean place_client(ObClient *client, gint *x, gint *y,
                      ObAppSettings *settings)
{
    gboolean ret = FALSE;
    if (client->positioned)
        return FALSE;
    if (place_transient(client, x, y))
        ret = TRUE;
    else if (!(
        place_per_app_setting(client, x, y, settings) ||
        ((config_place_policy == OB_PLACE_POLICY_MOUSE) ?
         place_under_mouse(client, x, y) :
         place_smart(client, x, y, SMART_FULL)    ||
         place_smart(client, x, y, SMART_GROUP)   ||
         place_smart(client, x, y, SMART_FOCUSED) ||
         place_random(client, x, y))))
        g_assert_not_reached(); /* the last one better succeed */
    /* get where the client should be */
    frame_frame_gravity(client->frame, x, y);
    return ret;
}
