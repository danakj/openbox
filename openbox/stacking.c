/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   stacking.c for the Openbox window manager
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

#include "openbox.h"
#include "prop.h"
#include "screen.h"
#include "focus.h"
#include "client.h"
#include "group.h"
#include "frame.h"
#include "window.h"
#include "debug.h"

GList  *stacking_list = NULL;

void stacking_set_list()
{
    Window *windows = NULL;
    GList *it;
    guint i = 0;

    /* on shutdown, don't update the properties, so that we can read it back
       in on startup and re-stack the windows as they were before we shut down
    */
    if (ob_state() == OB_STATE_EXITING) return;

    /* create an array of the window ids (from bottom to top,
       reverse order!) */
    if (stacking_list) {
        windows = g_new(Window, g_list_length(stacking_list));
        for (it = g_list_last(stacking_list); it; it = g_list_previous(it)) {
            if (WINDOW_IS_CLIENT(it->data))
                windows[i++] = WINDOW_AS_CLIENT(it->data)->window;
        }
    }

    PROP_SETA32(RootWindow(ob_display, ob_screen),
                net_client_list_stacking, window, (gulong*)windows, i);

    g_free(windows);
}

static void do_restack(GList *wins, GList *before)
{
    GList *it;
    Window *win;
    gint i;

#ifdef DEBUG
    GList *next;
    /* pls only restack stuff in the same layer at a time */
    for (it = wins; it; it = next) {
        next = g_list_next(it);
        if (!next) break;
        g_assert (window_layer(it->data) == window_layer(next->data));
    }
    if (before)
        g_assert(window_layer(it->data) >= window_layer(before->data));
#endif

    win = g_new(Window, g_list_length(wins) + 1);

    if (before == stacking_list)
        win[0] = screen_support_win;
    else if (!before)
        win[0] = window_top(g_list_last(stacking_list)->data);
    else
        win[0] = window_top(g_list_previous(before)->data);

    for (i = 1, it = wins; it; ++i, it = g_list_next(it)) {
        win[i] = window_top(it->data);
        g_assert(win[i] != None); /* better not call stacking shit before
                                     setting your top level window value */
        stacking_list = g_list_insert_before(stacking_list, before, it->data);
    }

#ifdef DEBUG
    /* some debug checking of the stacking list's order */
    for (it = stacking_list; ; it = next) {
        next = g_list_next(it);
        if (!next) break;
        g_assert(window_layer(it->data) >= window_layer(next->data));
    }
#endif

    XRestackWindows(ob_display, win, i);
    g_free(win);

    stacking_set_list();
}

static void do_raise(GList *wins)
{
    GList *it;
    GList *layer[OB_NUM_STACKING_LAYERS] = {NULL};
    gint i;

    for (it = wins; it; it = g_list_next(it)) {
        ObStackingLayer l;

        l = window_layer(it->data);
        layer[l] = g_list_append(layer[l], it->data);
    }

    it = stacking_list;
    for (i = OB_NUM_STACKING_LAYERS - 1; i >= 0; --i) {
        if (layer[i]) {
            for (; it; it = g_list_next(it)) {
                /* look for the top of the layer */
                if (window_layer(it->data) <= (ObStackingLayer) i)
                    break;
            }
            do_restack(layer[i], it);
            g_list_free(layer[i]);
        }
    }
}

static void do_lower(GList *wins)
{
    GList *it;
    GList *layer[OB_NUM_STACKING_LAYERS] = {NULL};
    gint i;

    for (it = wins; it; it = g_list_next(it)) {
        ObStackingLayer l;

        l = window_layer(it->data);
        layer[l] = g_list_append(layer[l], it->data);
    }

    it = stacking_list;
    for (i = OB_NUM_STACKING_LAYERS - 1; i >= 0; --i) {
        if (layer[i]) {
            for (; it; it = g_list_next(it)) {
                /* look for the top of the next layer down */
                if (window_layer(it->data) < (ObStackingLayer) i)
                    break;
            }
            do_restack(layer[i], it);
            g_list_free(layer[i]);
        }
    }
}

static void restack_windows(ObClient *selected, gboolean raise)
{
    GList *it, *last, *below, *above, *next;
    GList *wins = NULL;

    GList *group_modals = NULL;
    GList *group_trans = NULL;
    GList *modals = NULL;
    GList *trans = NULL;

    if (!raise && selected->transient_for) {
        GSList *top, *top_it;
        GSList *top_reorder = NULL;
        
        /* if it's a transient lowering, lower its parents so that we can lower
           this window, or it won't move */
        top = client_search_all_top_parents_layer(selected);

        /* that is, if it has any parents */
        if (!(top->data == selected && top->next == NULL)) {
            /* go thru stacking list backwards so we can use g_slist_prepend */
            for (it = g_list_last(stacking_list); it && top;
                 it = g_list_previous(it))
                if ((top_it = g_slist_find(top, it->data))) {
                    top_reorder = g_slist_prepend(top_reorder, top_it->data);
                    top = g_slist_delete_link(top, top_it);
                }
            g_assert(top == NULL);

            /* call restack for each of these to lower them */
            for (top_it = top_reorder; top_it; top_it = g_slist_next(top_it))
                restack_windows(top_it->data, raise);
            return;
        }
    }

    /* remove first so we can't run into ourself */
    it = g_list_find(stacking_list, selected);
    g_assert(it);
    stacking_list = g_list_delete_link(stacking_list, it);

    /* go from the bottom of the stacking list up */
    for (it = g_list_last(stacking_list); it; it = next) {
        next = g_list_previous(it);

        if (WINDOW_IS_CLIENT(it->data)) {
            ObClient *ch = it->data;

            /* only move windows in the same stacking layer */
            if (ch->layer == selected->layer &&
                client_search_transient(selected, ch))
            {
                if (client_is_direct_child(selected, ch)) {
                    if (ch->modal)
                        modals = g_list_prepend(modals, ch);
                    else
                        trans = g_list_prepend(trans, ch);
                }
                else {
                    if (ch->modal)
                        group_modals = g_list_prepend(group_modals, ch);
                    else
                        group_trans = g_list_prepend(group_trans, ch);
                }
                stacking_list = g_list_delete_link(stacking_list, it);
            }
        }
    }

    /* put transients of the selected window right above it */
    wins = g_list_concat(modals, trans);
    wins = g_list_append(wins, selected);

    /* if selected window is transient for group then raise it above others */
    if (selected->transient_for == OB_TRAN_GROUP) {
        /* if it's modal, raise it above those also */
        if (selected->modal) {
            wins = g_list_concat(wins, group_modals);
            group_modals = NULL;
        }
        wins = g_list_concat(wins, group_trans);
        group_trans = NULL;
    }

    /* find where to put the selected window, start from bottom of list,
       this is the window below everything we are re-adding to the list */
    last = NULL;
    for (it = g_list_last(stacking_list); it; it = g_list_previous(it))
    {
        if (window_layer(it->data) < selected->layer) {
            last = it;
            continue;
        }
        /* if lowering, stop at the beginning of the layer */
        if (!raise)
            break;
        /* if raising, stop at the end of the layer */
        if (window_layer(it->data) > selected->layer)
            break;

        last = it;
    }

    /* save this position in the stacking list */
    below = last;

    /* find where to put the group transients, start from the top of list */
    for (it = stacking_list; it; it = g_list_next(it)) {
        /* skip past higher layers */
        if (window_layer(it->data) > selected->layer)
            continue;
        /* if we reach the end of the layer (how?) then don't go further */
        if (window_layer(it->data) < selected->layer)
            break;
        /* stop when we reach the first window in the group */
        if (WINDOW_IS_CLIENT(it->data)) {
            ObClient *c = it->data;
            if (c->group == selected->group)
                break;
        }
        /* if we don't hit any other group members, stop here because this
           is where we are putting the selected window (and its children) */
        if (it == below)
            break;
    }

    /* save this position, this is the top of the group of windows between the
       group transient ones we're restacking and the others up above that we're
       restacking

       we actually want to save 1 position _above_ that, for for loops to work
       nicely, so move back one position in the list while saving it
    */
    above = it ? g_list_previous(it) : g_list_last(stacking_list);

    /* put the windows inside the gap to the other windows we're stacking
       into the restacking list, go from the bottom up so that we can use
       g_list_prepend */
    if (below) it = g_list_previous(below);
    else       it = g_list_last(stacking_list);
    for (; it != above; it = next) {
        next = g_list_previous(it);
        wins = g_list_prepend(wins, it->data);
        stacking_list = g_list_delete_link(stacking_list, it);
    }

    /* group transients go above the rest of the stuff acquired to now */
    wins = g_list_concat(group_trans, wins);
    /* group modals go on the very top */
    wins = g_list_concat(group_modals, wins);

    do_restack(wins, below);
    g_list_free(wins);
}

void stacking_raise(ObWindow *window)
{
    if (WINDOW_IS_CLIENT(window)) {
        ObClient *selected;
        selected = WINDOW_AS_CLIENT(window);
        restack_windows(selected, TRUE);
    } else {
        GList *wins;
        wins = g_list_append(NULL, window);
        stacking_list = g_list_remove(stacking_list, window);
        do_raise(wins);
        g_list_free(wins);
    }
}

void stacking_lower(ObWindow *window)
{
    if (WINDOW_IS_CLIENT(window)) {
        ObClient *selected;
        selected = WINDOW_AS_CLIENT(window);
        restack_windows(selected, FALSE);
    } else {
        GList *wins;
        wins = g_list_append(NULL, window);
        stacking_list = g_list_remove(stacking_list, window);
        do_lower(wins);
        g_list_free(wins);
    }
}

void stacking_below(ObWindow *window, ObWindow *below)
{
    GList *wins, *before;

    if (window_layer(window) != window_layer(below))
        return;

    wins = g_list_append(NULL, window);
    stacking_list = g_list_remove(stacking_list, window);
    before = g_list_next(g_list_find(stacking_list, below));
    do_restack(wins, before);
    g_list_free(wins);
}

void stacking_add(ObWindow *win)
{
    g_assert(screen_support_win != None); /* make sure I dont break this in the
                                             future */

    stacking_list = g_list_append(stacking_list, win);
    stacking_raise(win);
}

static GList *find_highest_relative(ObClient *client)
{    
    GList *ret = NULL;

    if (client->transient_for) {
        GList *it;
        GSList *top;

        /* get all top level relatives of this client */
        top = client_search_all_top_parents_layer(client);

        /* go from the top of the stacking order down */
        for (it = stacking_list; !ret && it; it = g_list_next(it)) {
            if (WINDOW_IS_CLIENT(it->data)) {
                ObClient *c = it->data;
                /* only look at windows in the same layer */
                if (c->layer == client->layer) {
                    GSList *sit;

                    /* go through each top level parent and see it this window
                       is related to them */
                    for (sit = top; !ret && sit; sit = g_slist_next(sit)) {
                        ObClient *topc = sit->data;

                        /* are they related ? */
                        if (topc == c || client_search_transient(topc, c))
                            ret = it;
                    }
                }
            }
        }
    }
    return ret;
}

void stacking_add_nonintrusive(ObWindow *win)
{
    ObClient *client;
    GList *it_below = NULL;

    if (!WINDOW_IS_CLIENT(win)) {
        stacking_add(win); /* no special rules for others */
        return;
    }

    client = WINDOW_AS_CLIENT(win);

    /* insert above its highest parent (or its highest child !) */
    it_below = find_highest_relative(client);

    if (!it_below && client != focus_client) {
        /* nothing to put it directly above, so try find the focused client to
           put it underneath it */
        if (focus_client && focus_client->layer == client->layer) {
            if ((it_below = g_list_find(stacking_list, focus_client)))
                it_below = it_below->next;
        }
    }
    if (!it_below) {
        if (client == focus_client) {
            /* it's focused so put it at the top */
            stacking_list = g_list_append(stacking_list, win);
            stacking_raise(win);
        } else {
            /* there is no window to put this directly above, so put it at the
               bottom */
            stacking_list = g_list_prepend(stacking_list, win);
            stacking_lower(win);
        }
    } else {
        /* make sure it's not in the wrong layer though ! */
        for (; it_below; it_below = g_list_next(it_below))
        {
            /* stop when the window is not in a higher layer than the window
               it is going above (it_below) */
            if (client->layer >= window_layer(it_below->data))
                break;
        }
        for (; it_below != stacking_list;
             it_below = g_list_previous(it_below))
        {
            /* stop when the window is not in a lower layer than the
               window it is going under (it_above) */
            GList *it_above = g_list_previous(it_below);
            if (client->layer <= window_layer(it_above->data))
                break;
        }

        GList *wins = g_list_append(NULL, win);
        do_restack(wins, it_below);
        g_list_free(wins);
    }
}

/*! Returns TRUE if client is occluded by the sibling. If sibling is NULL it
  tries against all other clients.
*/
static gboolean stacking_occluded(ObClient *client, ObClient *sibling)
{
    GList *it;
    gboolean occluded = FALSE;
    gboolean found = FALSE;

    /* no need for any looping in this case */
    if (sibling && client->layer != sibling->layer)
        return occluded;

    for (it = stacking_list; it;
         it = (found ? g_list_previous(it) :g_list_next(it)))
        if (WINDOW_IS_CLIENT(it->data)) {
            ObClient *c = it->data;
            if (found) {
                if (RECT_INTERSECTS_RECT(c->frame->area, client->frame->area))
                {
                    if (sibling != NULL) {
                        if (c == sibling) {
                            occluded = TRUE;
                            break;
                        }
                    }
                    else if (c->layer == client->layer) {
                        occluded = TRUE;
                        break;
                    }
                    else if (c->layer > client->layer)
                        break; /* we past its layer */
                }
            }
            else if (c == client)
                found = TRUE;
        }
    return occluded;
}

/*! Returns TRUE if client is occludes the sibling. If sibling is NULL it tries
  against all other clients.
*/
static gboolean stacking_occludes(ObClient *client, ObClient *sibling)
{
    GList *it;
    gboolean occludes = FALSE;
    gboolean found = FALSE;

    /* no need for any looping in this case */
    if (sibling && client->layer != sibling->layer)
        return occludes;

    for (it = stacking_list; it; it = g_list_next(it))
        if (WINDOW_IS_CLIENT(it->data)) {
            ObClient *c = it->data;
            if (found) {
                if (RECT_INTERSECTS_RECT(c->frame->area, client->frame->area))
                {
                    if (sibling != NULL) {
                        if (c == sibling) {
                            occludes = TRUE;
                            break;
                        }
                    }
                    else if (c->layer == client->layer) {
                        occludes = TRUE;
                        break;
                    }
                    else if (c->layer > client->layer)
                        break; /* we past its layer */
                }
            }
            else if (c == client)
                found = TRUE;
        }
    return occludes;
}

void stacking_restack_request(ObClient *client, ObClient *sibling,
                              gint detail, gboolean activate)
{
    switch (detail) {
    case Below:
        ob_debug("Restack request Below for client %s sibling %s\n",
                 client->title, sibling ? sibling->title : "(all)");
        /* just lower it */
        stacking_lower(CLIENT_AS_WINDOW(client));
        break;
    case BottomIf:
        ob_debug("Restack request BottomIf for client %s sibling "
                 "%s\n",
                 client->title, sibling ? sibling->title : "(all)");
        /* if this client occludes sibling (or anything if NULL), then
           lower it to the bottom */
        if (stacking_occludes(client, sibling))
            stacking_lower(CLIENT_AS_WINDOW(client));
        break;
    case Above:
        ob_debug("Restack request Above for client %s sibling %s\n",
                 client->title, sibling ? sibling->title : "(all)");
        if (activate)
            /* use user=TRUE because it is impossible to get a timestamp
               for this */
            client_activate(client, FALSE, TRUE);
        else
            stacking_raise(CLIENT_AS_WINDOW(client));
        break;
    case TopIf:
        ob_debug("Restack request TopIf for client %s sibling %s\n",
                 client->title, sibling ? sibling->title : "(all)");
        if (stacking_occluded(client, sibling)) {
            if (activate)
                /* use user=TRUE because it is impossible to get a timestamp
                   for this */
                client_activate(client, FALSE, TRUE);
            else
                stacking_raise(CLIENT_AS_WINDOW(client));
        }
        break;
    case Opposite:
        ob_debug("Restack request Opposite for client %s sibling "
                 "%s\n",
                 client->title, sibling ? sibling->title : "(all)");
        if (stacking_occluded(client, sibling)) {
            if (activate)
                /* use user=TRUE because it is impossible to get a timestamp
                   for this */
                client_activate(client, FALSE, TRUE);
            else
                stacking_raise(CLIENT_AS_WINDOW(client));
        }
        else if (stacking_occludes(client, sibling))
            stacking_lower(CLIENT_AS_WINDOW(client));
        break;
    }
}
