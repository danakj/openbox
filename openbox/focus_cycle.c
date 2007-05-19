/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   focus_cycle.c for the Openbox window manager
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

#include "focus_cycle.h"
#include "focus_cycle_indicator.h"
#include "focus_cycle_popup.h"
#include "client.h"
#include "frame.h"
#include "focus.h"
#include "screen.h"
#include "openbox.h"
#include "debug.h"
#include "group.h"

#include <X11/Xlib.h>
#include <glib.h>

ObClient     *focus_cycle_target = NULL;

static void      focus_cycle_destroy_notify (ObClient *client, gpointer data);
static gboolean  focus_target_has_siblings  (ObClient *ft,
                                             gboolean iconic_windows,
                                             gboolean all_desktops);
static ObClient *focus_find_directional    (ObClient *c,
                                            ObDirection dir,
                                            gboolean dock_windows,
                                            gboolean desktop_windows);
static ObClient *focus_find_directional    (ObClient *c,
                                            ObDirection dir,
                                            gboolean dock_windows,
                                            gboolean desktop_windows);

void focus_cycle_startup(gboolean reconfig)
{
    if (reconfig) return;

    client_add_destroy_notify(focus_cycle_destroy_notify, NULL);
}

void focus_cycle_shutdown(gboolean reconfig)
{
    if (reconfig) return;

    client_remove_destroy_notify(focus_cycle_destroy_notify);
}

void focus_cycle_stop()
{
    if (focus_cycle_target)
        focus_cycle(TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE);
}

static void focus_cycle_destroy_notify(ObClient *client, gpointer data)
{
    /* end cycling if the target disappears. CurrentTime is fine, time won't
       be used
    */
    if (focus_cycle_target == client)
        focus_cycle(TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE);
}

/*! Returns if a focus target has valid group siblings that can be cycled
  to in its place */
static gboolean focus_target_has_siblings(ObClient *ft,
                                          gboolean iconic_windows,
                                          gboolean all_desktops)
                                                         
{
    GSList *it;

    if (!ft->group) return FALSE;

    for (it = ft->group->members; it; it = g_slist_next(it)) {
        ObClient *c = it->data;
        /* check that it's not a helper window to avoid infinite recursion */
        if (c != ft && !client_helper(c) &&
            focus_cycle_target_valid(c, iconic_windows, all_desktops, FALSE,
                                     FALSE))
        {
            return TRUE;
        }
    }
    return FALSE;
}

gboolean focus_cycle_target_valid(ObClient *ft,
                                  gboolean iconic_windows,
                                  gboolean all_desktops,
                                  gboolean dock_windows,
                                  gboolean desktop_windows)
{
    gboolean ok = FALSE;

    /* it's on this desktop unless you want all desktops.

       do this check first because it will usually filter out the most
       windows */
    ok = (all_desktops || ft->desktop == screen_desktop ||
          ft->desktop == DESKTOP_ALL);

    /* the window can receive focus somehow */
    ok = ok && (ft->can_focus || ft->focus_notify);

    /* the window is not iconic, or we're allowed to go to iconic ones */
    ok = ok && (iconic_windows || !ft->iconic);

    /* it's the right type of window */
    if (dock_windows || desktop_windows)
        ok = ok && ((dock_windows && ft->type == OB_CLIENT_TYPE_DOCK) ||
                    (desktop_windows && ft->type == OB_CLIENT_TYPE_DESKTOP));
    else
        /* normal non-helper windows are valid targets */
        ok = ok &&
            ((client_normal(ft) && !client_helper(ft))
             ||
             /* helper windows are valid targets it... */
             (client_helper(ft) &&
              /* ...a window in its group already has focus ... */
              ((focus_client && ft->group == focus_client->group) ||
               /* ... or if there are no other windows in its group 
                  that can be cycled to instead */
               !focus_target_has_siblings(ft, iconic_windows, all_desktops))));

    /* it's not set to skip the taskbar (unless it is a type that would be
       expected to set this hint */
    ok = ok && ((ft->type == OB_CLIENT_TYPE_DOCK ||
                 ft->type == OB_CLIENT_TYPE_DESKTOP ||
                 ft->type == OB_CLIENT_TYPE_TOOLBAR ||
                 ft->type == OB_CLIENT_TYPE_MENU ||
                 ft->type == OB_CLIENT_TYPE_UTILITY) ||
                !ft->skip_taskbar);

    /* it's not going to just send fous off somewhere else (modal window) */
    ok = ok && ft == client_focus_target(ft);

    return ok;
}

void focus_cycle(gboolean forward, gboolean all_desktops,
                 gboolean dock_windows, gboolean desktop_windows,
                 gboolean linear, gboolean interactive,
                 gboolean dialog, gboolean done, gboolean cancel)
{
    static ObClient *first = NULL;
    static ObClient *t = NULL;
    static GList *order = NULL;
    GList *it, *start, *list;
    ObClient *ft = NULL;

    if (interactive) {
        if (cancel) {
            focus_cycle_target = NULL;
            goto done_cycle;
        } else if (done)
            goto done_cycle;

        if (!focus_order)
            goto done_cycle;

        if (!first) first = focus_client;

        if (linear) list = client_list;
        else        list = focus_order;
    } else {
        if (!focus_order)
            goto done_cycle;
        list = client_list;
    }
    if (!focus_cycle_target) focus_cycle_target = focus_client;

    start = it = g_list_find(list, focus_cycle_target);
    if (!start) /* switched desktops or something? */
        start = it = forward ? g_list_last(list) : g_list_first(list);
    if (!start) goto done_cycle;

    do {
        if (forward) {
            it = it->next;
            if (it == NULL) it = g_list_first(list);
        } else {
            it = it->prev;
            if (it == NULL) it = g_list_last(list);
        }
        ft = it->data;
        if (focus_cycle_target_valid(ft, TRUE,
                                     all_desktops, dock_windows,
                                     desktop_windows))
        {
            if (interactive) {
                if (ft != focus_cycle_target) { /* prevents flicker */
                    focus_cycle_target = ft;
                    focus_cycle_draw_indicator(ft);
                }
                if (dialog)
                    /* same arguments as focus_target_valid */
                    focus_cycle_popup_show(ft, TRUE,
                                           all_desktops, dock_windows,
                                           desktop_windows);
                return;
            } else if (ft != focus_cycle_target) {
                focus_cycle_target = ft;
                done = TRUE;
                break;
            }
        }
    } while (it != start);

done_cycle:
    if (done && focus_cycle_target)
        client_activate(focus_cycle_target, FALSE, TRUE);

    t = NULL;
    first = NULL;
    focus_cycle_target = NULL;
    g_list_free(order);
    order = NULL;

    if (interactive) {
        focus_cycle_draw_indicator(NULL);
        focus_cycle_popup_hide();
    }

    return;
}

/* this be mostly ripped from fvwm */
static ObClient *focus_find_directional(ObClient *c, ObDirection dir,
                                        gboolean dock_windows,
                                        gboolean desktop_windows) 
{
    gint my_cx, my_cy, his_cx, his_cy;
    gint offset = 0;
    gint distance = 0;
    gint score, best_score;
    ObClient *best_client, *cur;
    GList *it;

    if(!client_list)
        return NULL;

    /* first, find the centre coords of the currently focused window */
    my_cx = c->frame->area.x + c->frame->area.width / 2;
    my_cy = c->frame->area.y + c->frame->area.height / 2;

    best_score = -1;
    best_client = NULL;

    for(it = g_list_first(client_list); it; it = g_list_next(it)) {
        cur = it->data;

        /* the currently selected window isn't interesting */
        if(cur == c)
            continue;
        if (!focus_cycle_target_valid(it->data, FALSE, FALSE, dock_windows,
                                      desktop_windows))
            continue;

        /* find the centre coords of this window, from the
         * currently focused window's point of view */
        his_cx = (cur->frame->area.x - my_cx)
            + cur->frame->area.width / 2;
        his_cy = (cur->frame->area.y - my_cy)
            + cur->frame->area.height / 2;

        if(dir == OB_DIRECTION_NORTHEAST || dir == OB_DIRECTION_SOUTHEAST ||
           dir == OB_DIRECTION_SOUTHWEST || dir == OB_DIRECTION_NORTHWEST) {
            gint tx;
            /* Rotate the diagonals 45 degrees counterclockwise.
             * To do this, multiply the matrix /+h +h\ with the
             * vector (x y).                   \-h +h/
             * h = sqrt(0.5). We can set h := 1 since absolute
             * distance doesn't matter here. */
            tx = his_cx + his_cy;
            his_cy = -his_cx + his_cy;
            his_cx = tx;
        }

        switch(dir) {
        case OB_DIRECTION_NORTH:
        case OB_DIRECTION_SOUTH:
        case OB_DIRECTION_NORTHEAST:
        case OB_DIRECTION_SOUTHWEST:
            offset = (his_cx < 0) ? -his_cx : his_cx;
            distance = ((dir == OB_DIRECTION_NORTH ||
                         dir == OB_DIRECTION_NORTHEAST) ?
                        -his_cy : his_cy);
            break;
        case OB_DIRECTION_EAST:
        case OB_DIRECTION_WEST:
        case OB_DIRECTION_SOUTHEAST:
        case OB_DIRECTION_NORTHWEST:
            offset = (his_cy < 0) ? -his_cy : his_cy;
            distance = ((dir == OB_DIRECTION_WEST ||
                         dir == OB_DIRECTION_NORTHWEST) ?
                        -his_cx : his_cx);
            break;
        }

        /* the target must be in the requested direction */
        if(distance <= 0)
            continue;

        /* Calculate score for this window.  The smaller the better. */
        score = distance + offset;

        /* windows more than 45 degrees off the direction are
         * heavily penalized and will only be chosen if nothing
         * else within a million pixels */
        if(offset > distance)
            score += 1000000;

        if(best_score == -1 || score < best_score)
            best_client = cur,
                best_score = score;
    }

    return best_client;
}

void focus_directional_cycle(ObDirection dir, gboolean dock_windows,
                             gboolean desktop_windows, gboolean interactive,
                             gboolean dialog, gboolean done, gboolean cancel)
{
    static ObClient *first = NULL;
    ObClient *ft = NULL;

    if (!interactive)
        return;

    if (cancel) {
        focus_cycle_target = NULL;
        goto done_cycle;
    } else if (done)
        goto done_cycle;

    if (!focus_order)
        goto done_cycle;

    if (!first) first = focus_client;
    if (!focus_cycle_target) focus_cycle_target = focus_client;

    if (focus_cycle_target)
        ft = focus_find_directional(focus_cycle_target, dir, dock_windows,
                                    desktop_windows);
    else {
        GList *it;

        for (it = focus_order; it; it = g_list_next(it))
            if (focus_cycle_target_valid(it->data, FALSE, FALSE, dock_windows,
                                         desktop_windows))
                ft = it->data;
    }
        
    if (ft) {
        if (ft != focus_cycle_target) {/* prevents flicker */
            focus_cycle_target = ft;
            focus_cycle_draw_indicator(ft);
        }
    }
    if (focus_cycle_target && dialog) {
        /* same arguments as focus_target_valid */
        focus_cycle_popup_single_show(focus_cycle_target,
                                      FALSE, FALSE, dock_windows,
                                      desktop_windows);
        return;
    }

done_cycle:
    if (done && focus_cycle_target)
        client_activate(focus_cycle_target, FALSE, TRUE);

    first = NULL;
    focus_cycle_target = NULL;

    focus_cycle_draw_indicator(NULL);
    focus_cycle_popup_single_hide();

    return;
}

void focus_order_add_new(ObClient *c)
{
    if (c->iconic)
        focus_order_to_top(c);
    else {
        g_assert(!g_list_find(focus_order, c));
        /* if there are any iconic windows, put this above them in the order,
           but if there are not, then put it under the currently focused one */
        if (focus_order && ((ObClient*)focus_order->data)->iconic)
            focus_order = g_list_insert(focus_order, c, 0);
        else
            focus_order = g_list_insert(focus_order, c, 1);
    }
}

