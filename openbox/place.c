/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   place.c for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
   Copyright (c) 2003-2007   Dana Jansens

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
#include "dock.h"
#include "debug.h"

extern ObDock *dock;

static Rect *pick_pointer_head(ObClient *c)
{
    return screen_area(c->desktop, screen_monitor_pointer(), NULL);
}

/* use the following priority lists for pick_head()

   When a window is being placed in the FOREGROUND, use a monitor chosen in
   the following order:
   1. per-app settings
   2. same monitor as parent
   3. primary monitor if placement=PRIMARY
      active monitor if placement=ACTIVE
      pointer monitor if placement=MOUSE
   4. primary monitor
   5. other monitors where the window has group members on the same desktop
   6. other monitors where the window has group members on other desktops
   7. other monitors

   When a window is being placed in the BACKGROUND, use a monitor chosen in the
   following order:
   1. per-app settings
   2. same monitor as parent
   3. other monitors where the window has group members on the same desktop
    3a. primary monitor in this set
    3b. other monitors in this set
   4. other monitors where the window has group members on other desktops
    4a. primary monitor in this set
    4b. other monitors in this set
   5. other monitors
    5a. primary monitor in this set
    5b. other monitors in this set
*/

/*! One for each possible head, used to sort them in order of precedence. */
typedef struct {
    guint monitor;
    guint flags;
} ObPlaceHead;

/*! Flags for ObPlaceHead */
enum {
    HEAD_PARENT = 1 << 0, /* parent's monitor */
    HEAD_PLACED = 1 << 1, /* chosen monitor by placement */
    HEAD_PRIMARY = 1 << 2, /* primary monitor */
    HEAD_GROUP_DESK = 1 << 3, /* has a group member on the same desktop */
    HEAD_GROUP = 1 << 4, /* has a group member on another desktop */
    HEAD_PERAPP = 1 << 5, /* chosen by per-app settings */
};

gint cmp_foreground(const void *a, const void *b)
{
    const ObPlaceHead *h1 = a;
    const ObPlaceHead *h2 = b;
    gint i = 0;

    if (h1->monitor == h2->monitor) return 0;

    if (h1->flags & HEAD_PERAPP) --i;
    if (h2->flags & HEAD_PERAPP) ++i;
    if (i) return i;

    if (h1->flags & HEAD_PARENT) --i;
    if (h2->flags & HEAD_PARENT) ++i;
    if (i) return i;

    if (h1->flags & HEAD_PLACED) --i;
    if (h2->flags & HEAD_PLACED) ++i;
    if (i) return i;

    if (h1->flags & HEAD_PRIMARY) --i;
    if (h2->flags & HEAD_PRIMARY) ++i;
    if (i) return i;

    if (h1->flags & HEAD_GROUP_DESK) --i;
    if (h2->flags & HEAD_GROUP_DESK) ++i;
    if (i) return i;

    if (h1->flags & HEAD_GROUP) --i;
    if (h2->flags & HEAD_GROUP) ++i;
    if (i) return i;

    return h1->monitor - h2->monitor;
}

gint cmp_background(const void *a, const void *b)
{
    const ObPlaceHead *h1 = a;
    const ObPlaceHead *h2 = b;
    gint i = 0;

    if (h1->monitor == h2->monitor) return 0;

    if (h1->flags & HEAD_PERAPP) --i;
    if (h2->flags & HEAD_PERAPP) ++i;
    if (i) return i;

    if (h1->flags & HEAD_PARENT) --i;
    if (h2->flags & HEAD_PARENT) ++i;
    if (i) return i;

    if (h1->flags & HEAD_GROUP_DESK || h2->flags & HEAD_GROUP_DESK) {
        if (h1->flags & HEAD_GROUP_DESK) --i;
        if (h2->flags & HEAD_GROUP_DESK) ++i;
        if (i) return i;
        if (h1->flags & HEAD_PRIMARY) --i;
        if (h2->flags & HEAD_PRIMARY) ++i;
        if (i) return i;
    }

    if (h1->flags & HEAD_GROUP || h2->flags & HEAD_GROUP) {
        if (h1->flags & HEAD_GROUP) --i;
        if (h2->flags & HEAD_GROUP) ++i;
        if (i) return i;
        if (h1->flags & HEAD_PRIMARY) --i;
        if (h2->flags & HEAD_PRIMARY) ++i;
        if (i) return i;
    }

    if (h1->flags & HEAD_PRIMARY) --i;
    if (h2->flags & HEAD_PRIMARY) ++i;
    if (i) return i;

    return h1->monitor - h2->monitor;
}

/*! Pick a monitor to place a window on. */
static Rect *pick_head(ObClient *c, gboolean foreground,
                       ObAppSettings *settings)
{
    Rect *area;
    ObPlaceHead *choice;
    guint i;
    ObClient *p;
    GSList *it;
    gint cur_mon;

    ob_debug_type(OB_DEBUG_MULTIHEAD, "PICKING HEAD FOR %s...", c->title);

    if((cur_mon = g_slist_index(screen_visible_desktops, c->desktop)) > -1) {
        ob_debug_type(OB_DEBUG_MULTIHEAD, 
                 "\tpicked %d monitor because of desktop %d",
                 cur_mon, c->desktop);
        area = screen_area(c->desktop, cur_mon, NULL);

        return area;
    }

    ob_debug_type(OB_DEBUG_MULTIHEAD, "\tusing original picking algorithm because"
             " desktop %d is hidden...", c->desktop);

    choice = g_new(ObPlaceHead, screen_num_monitors);
    for (i = 0; i < screen_num_monitors; ++i) {
        choice[i].monitor = i;
        choice[i].flags = 0;
    }

    /* find monitors with group members */
    if (c->group) {
        for (it = c->group->members; it; it = g_slist_next(it)) {
            ObClient *itc = it->data;
            if (itc != c) {
                guint m = client_monitor(itc);

                if (m < screen_num_monitors) {
                    if (screen_compare_desktops(itc->desktop, c->desktop))
                        choice[m].flags |= HEAD_GROUP_DESK;
                    else
                        choice[m].flags |= HEAD_GROUP;
                }
            }
        }
    }

    i = screen_monitor_primary(FALSE);
    if (i < screen_num_monitors) {
        choice[i].flags |= HEAD_PRIMARY;
        if (config_place_monitor == OB_PLACE_MONITOR_PRIMARY)
            choice[i].flags |= HEAD_PLACED;
        if (settings &&
            settings->monitor_type == OB_PLACE_MONITOR_PRIMARY)
            choice[i].flags |= HEAD_PERAPP;
    }

    i = screen_monitor_active();
    if (i < screen_num_monitors) {
        if (config_place_monitor == OB_PLACE_MONITOR_ACTIVE)
            choice[i].flags |= HEAD_PLACED;
        if (settings &&
            settings->monitor_type == OB_PLACE_MONITOR_ACTIVE)
            choice[i].flags |= HEAD_PERAPP;
    }

    i = screen_monitor_pointer();
    if (i < screen_num_monitors) {
        if (config_place_monitor == OB_PLACE_MONITOR_MOUSE)
            choice[i].flags |= HEAD_PLACED;
        if (settings &&
            settings->monitor_type == OB_PLACE_MONITOR_MOUSE)
            choice[i].flags |= HEAD_PERAPP;
    }

    if (settings) {
        i = settings->monitor - 1;
        if (i < screen_num_monitors)
            choice[i].flags |= HEAD_PERAPP;
    }

    /* direct parent takes highest precedence */
    if ((p = client_direct_parent(c))) {
        i = client_monitor(p);
        if (i < screen_num_monitors)
            choice[i].flags |= HEAD_PARENT;
    }

    qsort(choice, screen_num_monitors, sizeof(ObPlaceHead),
          foreground ? cmp_foreground : cmp_background);

    /* save the areas of the monitors in order of their being chosen */
    for (i = 0; i < screen_num_monitors; ++i)
    {
        ob_debug("placement choice %d is monitor %d", i, choice[i].monitor);
        if (choice[i].flags & HEAD_PARENT)
            ob_debug("  - parent on monitor");
        if (choice[i].flags & HEAD_PLACED)
            ob_debug("  - placement choice");
        if (choice[i].flags & HEAD_PRIMARY)
            ob_debug("  - primary monitor");
        if (choice[i].flags & HEAD_GROUP_DESK)
            ob_debug("  - group on same desktop");
        if (choice[i].flags & HEAD_GROUP)
            ob_debug("  - group on other desktop");
    }

    area = screen_area(c->desktop, choice[0].monitor, NULL);

    g_free(choice);

    /* return the area for the chosen monitor */
    return area;
}

static gboolean place_random(ObClient *client, Rect *area, gint *x, gint *y)
{
    gint l, r, t, b;

    ob_debug("placing randomly");

    l = area->x;
    t = area->y;
    r = area->x + area->width - client->frame->area.width;
    b = area->y + area->height - client->frame->area.height;

    if (r > l) *x = g_random_int_range(l, r + 1);
    else       *x = area->x;
    if (b > t) *y = g_random_int_range(t, b + 1);
    else       *y = area->y;

    return TRUE;
}

static GSList* area_add(GSList *list, Rect *a)
{
    Rect *r = g_slice_new(Rect);
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
            /* dont free r, it's moved to the result list */
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

            /* 'r' is not being added to the result list, so free it */
            g_slice_free(Rect, r);
        }
    }
    g_slist_free(list);
    return result;
}

enum {
    IGNORE_FULLSCREEN = 1,
    IGNORE_MAXIMIZED  = 2,
    IGNORE_MENUTOOL   = 3,
    /*IGNORE_SHADED     = 3,*/
    IGNORE_NONGROUP   = 4,
    IGNORE_BELOW      = 5,
    /*IGNORE_NONFOCUS   = 1 << 5,*/
    IGNORE_DOCK       = 6,
    IGNORE_END        = 7
};

static gboolean place_nooverlap(ObClient *c, Rect *area, gint *x, gint *y)
{
    gint ignore;
    gboolean ret;
    gint maxsize;
    GSList *spaces = NULL, *sit, *maxit;

    ob_debug("placing nonoverlap");

    ret = FALSE;
    maxsize = 0;
    maxit = NULL;

    /* try ignoring different things to find empty space */
    for (ignore = 0; ignore < IGNORE_END && !ret; ignore++) {
        GList *it;

        /* add the whole monitor */
        spaces = area_add(spaces, area);

        /* go thru all the windows */
        for (it = client_list; it; it = g_list_next(it)) {
            ObClient *test = it->data;

            /* should we ignore this client? */
            if (screen_showing_desktop) continue;
            if (c == test) continue;
            if (test->iconic) continue;
            if (c->desktop != DESKTOP_ALL) {
                if (test->desktop != c->desktop &&
                    test->desktop != DESKTOP_ALL) continue;
            } else {
                if (test->desktop != screen_desktop &&
                    test->desktop != DESKTOP_ALL) continue;
            }
            if (test->type == OB_CLIENT_TYPE_SPLASH ||
                test->type == OB_CLIENT_TYPE_DESKTOP) continue;


            if ((ignore >= IGNORE_FULLSCREEN) &&
                test->fullscreen) continue;
            if ((ignore >= IGNORE_MAXIMIZED) &&
                test->max_horz && test->max_vert) continue;
            if ((ignore >= IGNORE_MENUTOOL) &&
                (test->type == OB_CLIENT_TYPE_MENU ||
                 test->type == OB_CLIENT_TYPE_TOOLBAR) &&
                client_has_parent(c)) continue;
            /*
              if ((ignore >= IGNORE_SHADED) &&
              test->shaded) continue;
            */
            if ((ignore >= IGNORE_NONGROUP) &&
                client_has_group_siblings(c) &&
                test->group != c->group) continue;
            if ((ignore >= IGNORE_BELOW) &&
                test->layer < c->layer) continue;
            /*
              if ((ignore >= IGNORE_NONFOCUS) &&
              focus_client != test) continue;
            */
            /* don't ignore this window, so remove it from the available
               area */
            spaces = area_remove(spaces, &test->frame->area);
        }

        if (ignore < IGNORE_DOCK) {
            Rect a;
            dock_get_area(&a);
            spaces = area_remove(spaces, &a);
        }

        for (sit = spaces; sit; sit = g_slist_next(sit)) {
            Rect *r = sit->data;

            if (r->width >= c->frame->area.width &&
                r->height >= c->frame->area.height &&
                r->width * r->height > maxsize)
            {
                maxsize = r->width * r->height;
                maxit = sit;
            }
        }

        if (maxit) {
            Rect *r = maxit->data;

            /* center it in the area */
            *x = r->x;
            *y = r->y;
            if (config_place_center) {
                *x += (r->width - c->frame->area.width) / 2;
                *y += (r->height - c->frame->area.height) / 2;
            }
            ret = TRUE;
        }

        while (spaces) {
            g_slice_free(Rect, spaces->data);
            spaces = g_slist_delete_link(spaces, spaces);
        }
    }

    return ret;
}

static gboolean place_under_mouse(ObClient *client, gint *x, gint *y)
{
    gint l, r, t, b;
    gint px, py;
    Rect *area;

    ob_debug("placing under mouse");

    if (!screen_pointer_pos(&px, &py))
        return FALSE;
    area = pick_pointer_head(client);

    l = area->x;
    t = area->y;
    r = area->x + area->width - client->frame->area.width;
    b = area->y + area->height - client->frame->area.height;

    *x = px - client->area.width / 2 - client->frame->size.left;
    *x = MIN(MAX(*x, l), r);
    *y = py - client->area.height / 2 - client->frame->size.top;
    *y = MIN(MAX(*y, t), b);

    g_slice_free(Rect, area);

    return TRUE;
}

static gboolean place_per_app_setting(ObClient *client, Rect *screen,
                                      gint *x, gint *y,
                                      ObAppSettings *settings)
{
    if (!settings || (settings && !settings->pos_given))
        return FALSE;

    ob_debug("placing by per-app settings");

    if (settings->position.x.center)
        *x = screen->x + screen->width / 2 - client->area.width / 2;
    else if (settings->position.x.opposite)
        *x = screen->x + screen->width - client->frame->area.width -
            settings->position.x.pos;
    else
        *x = screen->x + settings->position.x.pos;
    if (settings->position.x.denom)
        *x = (*x * screen->width) / settings->position.x.denom;

    if (settings->position.y.center)
        *y = screen->y + screen->height / 2 - client->area.height / 2;
    else if (settings->position.y.opposite)
        *y = screen->y + screen->height - client->frame->area.height -
            settings->position.y.pos;
    else
        *y = screen->y + settings->position.y.pos;
    if (settings->position.y.denom)
        *y = (*y * screen->height) / settings->position.y.denom;

    return TRUE;
}

static gboolean place_transient_splash(ObClient *client, Rect *area,
                                       gint *x, gint *y)
{
    if (client->type == OB_CLIENT_TYPE_DIALOG) {
        GSList *it;
        gboolean first = TRUE;
        gint l, r, t, b;

        ob_debug("placing dialog");

        for (it = client->parents; it; it = g_slist_next(it)) {
            ObClient *m = it->data;
            if (!m->iconic) {
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
            if (!first) {
                *x = ((r + 1 - l) - client->frame->area.width) / 2 + l;
                *y = ((b + 1 - t) - client->frame->area.height) / 2 + t;
                return TRUE;
            }
        }
    }

    if (client->type == OB_CLIENT_TYPE_DIALOG ||
        client->type == OB_CLIENT_TYPE_SPLASH)
    {
        ob_debug("placing dialog or splash");

        *x = (area->width - client->frame->area.width) / 2 + area->x;
        *y = (area->height - client->frame->area.height) / 2 + area->y;
        return TRUE;
    }

    return FALSE;
}

/* Make the following function a special case of something more general.
 * We need to be able to abstract the origin of where the client *is*.
 * i.e., when a client is unmaximized we need to determine where it needs to
 * go. Particularly, before it is actually moved. So we need a "here's where
 * the client wants to go, is that right for the monitor i gave you? if not,
 * adjust it please."
 */
gboolean place_onscreen(guint new_mon, gint *x, gint *y, gint *width,
                        gint *height)
{
    guint last_mon;
    guint last_desk, new_desk;
    float xrat, yrat;
    Rect *last_area, *new_area, *cur_area;

    cur_area = g_slice_new(Rect);
    RECT_SET(*cur_area, *x, *y, *width, *height);

    last_mon = screen_find_monitor(cur_area);
    if (new_mon == last_mon) {
        g_slice_free(Rect, cur_area);
        return FALSE;
    }

    last_desk = g_slist_nth(screen_visible_desktops, last_mon)->data;
    new_desk = g_slist_nth(screen_visible_desktops, new_mon)->data;

    last_area = screen_area(last_desk, last_mon, NULL);
    new_area = screen_area(new_desk, new_mon, NULL);

    ob_debug_type(OB_DEBUG_MULTIHEAD, "\told monitor: (%d, %d) %dx%d",
             last_area->x, last_area->y, last_area->width, last_area->height);
    ob_debug_type(OB_DEBUG_MULTIHEAD, "\tnew monitor: (%d, %d) %dx%d",
             new_area->x, new_area->y, new_area->width, new_area->height);

    xrat = (float)new_area->width / (float)last_area->width;
    yrat = (float)new_area->height / (float)last_area->height;

    ob_debug_type(OB_DEBUG_MULTIHEAD, "\tx ratio %.4f", xrat);
    ob_debug_type(OB_DEBUG_MULTIHEAD, "\ty ratio %.4f", yrat);

    *x = new_area->x + (*x - last_area->x) * xrat;
    *y = new_area->y + (*y - last_area->y) * yrat;

    if (config_scale_windows) {
        *width *= xrat;
        *height *= yrat;
    }
    
    ob_debug_type(OB_DEBUG_MULTIHEAD, "\tnew x %d", *x);
    ob_debug_type(OB_DEBUG_MULTIHEAD, "\tnew y %d", *y);
    ob_debug_type(OB_DEBUG_MULTIHEAD, "\tnew width %d", *width);
    ob_debug_type(OB_DEBUG_MULTIHEAD, "\tnew height %d", *height);

    g_slice_free(Rect, cur_area);
    g_slice_free(Rect, last_area);
    g_slice_free(Rect, new_area);

    return TRUE;
}

gboolean place_client_onscreen(ObClient *client, guint new_mon,
                               gint *x, gint *y, gint *width, gint *height)
{
    Rect *client_area;

    if (!client_normal(client))
        return FALSE;

    client_area = g_slice_new(Rect);
    RECT_SET(*client_area, client->frame->area.x, client->frame->area.y,
             client->frame->area.width, client->frame->area.height);

    ob_debug_type(OB_DEBUG_MULTIHEAD, 
             "Move client %s from %d monitor to %d monitor?", client->title, 
             client_monitor(client), new_mon);
    ob_debug_type(OB_DEBUG_MULTIHEAD, 
             "\tOriginal client geometry (%d, %d) %dx%d",
             client_area->x, client_area->y,
             client_area->width, client_area->height);

    if (!place_onscreen(new_mon, &client_area->x, &client_area->y,
                        &client_area->width, &client_area->height)) {
        ob_debug_type(OB_DEBUG_MULTIHEAD, "Skip set monitor for %s", client->title);
        g_slice_free(Rect, client_area);
        return FALSE;
    }

    /* Adjust the width/height for decorations */
    client_area->width -= (client->frame->area.width - client->area.width);
    client_area->height -= (client->frame->area.height - client->area.height);

    ob_debug_type(OB_DEBUG_MULTIHEAD, "\tNew client geometry (%d, %d) %dx%d",
             client_area->x, client_area->y,
             client_area->width, client_area->height);

    *x = client_area->x;
    *y = client_area->y;
    *width = client_area->width;
    *height = client_area->height;

    g_slice_free(Rect, client_area);

    return TRUE;
}

/*! Return TRUE if openbox chose the position for the window, and FALSE if
  the application chose it */
gboolean place_client(ObClient *client, gboolean foreground, gint *x, gint *y,
                      ObAppSettings *settings)
{
    Rect *area;
    gboolean ret;

    /* per-app settings override program specified position
     * but not user specified, unless pos_force is enabled */
    if (((client->positioned & USPosition) &&
         !(settings && settings->pos_given && settings->pos_force)) ||
        ((client->positioned & PPosition) &&
         !(settings && settings->pos_given)))
        return FALSE;

    area = pick_head(client, foreground, settings);

    /* try a number of methods */
    ret = place_per_app_setting(client, area, x, y, settings) ||
        place_transient_splash(client, area, x, y) ||
        (config_place_policy == OB_PLACE_POLICY_MOUSE &&
         place_under_mouse(client, x, y)) ||
        place_nooverlap(client, area, x, y) ||
        place_random(client, area, x, y);
    g_assert(ret);

    g_slice_free(Rect, area);

    /* get where the client should be */
    frame_frame_gravity(client->frame, x, y);
    return TRUE;
}
