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
#include "screen.h"
#include "focus.h"
#include "client.h"
#include "group.h"
#include "frame.h"
#include "window.h"
#include "event.h"
#include "unmanaged.h"
#include "debug.h"
#include "obt/prop.h"

/*! A node holding an unmanaged window for the secondary list stacking_ulist */
typedef struct _ObUNode {
    ObUnmanaged *um;
    /*! Points to the node in stacking_list which is the highest window in
      the list below this window */
    GList *belowme;
} ObUNode;

/* A list of managed ObWindow*s in stacking order from highest to lowest */
GList  *stacking_list = NULL;
/*! A list of unmanaged windows in OBUNode objects in stacking order from
  highest to lowest */
GList  *stacking_ulist = NULL;

static GHashTable *stacking_map = NULL;
static GHashTable *stacking_umap = NULL;
/*! When true, stacking changes will not be reflected on the screen.  This is
  to freeze the on-screen stacking order while a window is being temporarily
  raised during focus cycling */
static gboolean pause_changes = FALSE;

static void list_split(GList **l1, GList **l2)
{
    if (*l1 == *l2) *l2 = NULL;
    else if (*l2) {
        if ((*l2)->prev) (*l2)->prev->next = NULL;
        (*l2)->prev = NULL;
    }
}

static GList* list_insert_link_before(GList *list, GList *before, GList *link)
{
    /* split the list at 'before' */
    if (before) {
        if (before->prev) before->prev->next = NULL;
        before->prev = NULL;
        /* and stick it at the front of the 'before' list */
        link = g_list_concat(link, before);
        /* if before is the whole list, then we have replaced the list */
        if (before == list) list = NULL;
    }
    /* append the node and the rest of the list back onto the original list */
    return g_list_concat(list, link);
}

void stacking_startup(gboolean reconfig)
{
    if (reconfig) return;
    stacking_map = g_hash_table_new(g_int_hash, g_int_equal);
    stacking_umap = g_hash_table_new(g_int_hash, g_int_equal);
}

void stacking_shutdown(gboolean reconfig)
{
    if (reconfig) return;
    g_hash_table_destroy(stacking_map);
    stacking_map = NULL;
    g_hash_table_destroy(stacking_umap);
    stacking_umap = NULL;
}

void stacking_set_topmost(ObWindow *win)
{
    g_assert(win && stacking_list == NULL);
    g_assert(WINDOW_IS_INTERNAL(win));
    g_assert(window_layer(win) == OB_STACKING_LAYER_TOPMOST);
    stacking_list = g_list_prepend(stacking_list, win);
}

void stacking_set_list(void)
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

    OBT_PROP_SETA32(obt_root(ob_screen), NET_CLIENT_LIST_STACKING, WINDOW,
                    (gulong*)windows, i);

    g_free(windows);
}

static void do_restack(GList *wins, GList *before)
{
    GList *it;
    Window *win;
    gint i;

#ifdef DEBUG
    GList *next;

    g_assert(wins);
    /* pls only restack stuff in the same layer at a time */
    for (it = wins; it; it = next) {
        next = g_list_next(it);
        /* and there should be no unmanaged windows in the stacking_list */
        g_assert(!WINDOW_IS_UNMANAGED(it->data));
        if (!next) break;
        g_assert (window_layer(it->data) == window_layer(next->data));
    }
    if (before)
        g_assert(window_layer(it->data) >= window_layer(before->data));
#endif

    win = g_new(Window, g_list_length(wins) + 1);

    g_assert(before != stacking_list);

    if (!before)
        win[0] = window_top(g_list_last(stacking_list)->data);
    else
        win[0] = window_top(g_list_previous(before)->data);

    for (i = 1, it = wins; it; ++i, it = g_list_next(it)) {
        win[i] = window_top(it->data);
        /* don't call stacking shit before setting your top level window */
        g_assert(win[i] != None);
    }

    list_split(&stacking_list, &before);
    stacking_list = g_list_concat(stacking_list, g_list_concat(wins, before));

#ifdef DEBUG
    /* some debug checking of the stacking list's order */
    for (it = stacking_list; ; it = next) {
        next = g_list_next(it);
        g_assert(window_layer(it->data) != OB_STACKING_LAYER_INVALID);
        if (!next) break;
        g_assert(window_layer(it->data) >= window_layer(next->data));
    }
#endif

    if (!pause_changes)
        XRestackWindows(obt_display, win, i);
    g_free(win);

    stacking_set_list();
}

void stacking_temp_raise(ObWindow *window)
{
    Window win[2];
    GList *it;
    gulong start;

    /* don't use this for internal windows..! it would lower them.. */
    g_assert(window_layer(window) < OB_STACKING_LAYER_INTERNAL);

    /* find the window to drop it underneath */
    win[0] = screen_support_win;
    for (it = stacking_list; it; it = g_list_next(it)) {
        ObWindow *w = it->data;
        if (window_layer(w) >= OB_STACKING_LAYER_INTERNAL)
            win[0] = window_top(w);
        else
            break;
    }

    win[1] = window_top(window);
    start = event_start_ignore_all_enters();
    XRestackWindows(obt_display, win, 2);
    event_end_ignore_all_enters(start);

    pause_changes = TRUE;
}

void stacking_restore(void)
{
    Window *win;
    GList *it;
    gint i;
    gulong start;

    win = g_new(Window, g_list_length(stacking_list) + 1);
    for (i = 0, it = stacking_list; it; ++i, it = g_list_next(it))
        win[i] = window_top(it->data);
    start = event_start_ignore_all_enters();
    XRestackWindows(obt_display, win, i);
    event_end_ignore_all_enters(start);
    g_free(win);

    pause_changes = FALSE;
}

static void do_raise(GList *wins)
{
    GList *it, *next;
    GList *layer[OB_NUM_STACKING_LAYERS] = {NULL};
    gint i;

    for (it = wins; it; it = next) {
        const ObStackingLayer l = window_layer(it->data);

        next = g_list_next(it);
        wins = g_list_remove_link(wins, it); /* remove it from wins */
        layer[l] = g_list_concat(layer[l], it); /* stick it in the layer */
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
        }
    }
}

static void do_lower(GList *wins)
{
    GList *it, *next;
    GList *layer[OB_NUM_STACKING_LAYERS] = {NULL};
    gint i;

    for (it = wins; it; it = next) {
        const ObStackingLayer l = window_layer(it->data);

        next = g_list_next(it);
        wins = g_list_remove_link(wins, it); /* remove it from wins */
        layer[l] = g_list_concat(layer[l], it); /* stick it in the layer */
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
        }
    }
}

static void restack_windows(ObClient *selected, gboolean raise)
{
    GList *it, *last, *below, *above, *next, *selit;
    GList *wins = NULL;

    GList *group_helpers = NULL;
    GList *group_modals = NULL;
    GList *group_trans = NULL;
    GList *modals = NULL;
    GList *trans = NULL;

    if (raise) {
        ObClient *p;

        /* if a window is modal for another single window, then raise it to the
           top too, the same is done with the focus order */
        while (selected->modal && (p = client_direct_parent(selected)))
            selected = p;
    }

    /* remove first so we can't run into ourself */
    selit = g_list_find(stacking_list, selected);
    g_assert(selit);
    stacking_list = g_list_remove_link(stacking_list, selit);

    /* go from the bottom of the stacking list up. don't move any other windows
       when lowering, we call this for each window independently */
    if (raise) {
        for (it = g_list_last(stacking_list); it; it = next) {
            next = g_list_previous(it);

            if (WINDOW_IS_CLIENT(it->data)) {
                ObClient *ch = it->data;
                GList **addto = NULL;

                /* only move windows in the same stacking layer */
                if (ch->layer == selected->layer &&
                    /* looking for windows that are transients, and so would
                       remain above the selected window */
                    client_search_transient(selected, ch))
                {
                    if (client_is_direct_child(selected, ch)) {
                        if (ch->modal)
                            addto = &modals;
                        else
                            addto = &trans;
                    }
                    else if (client_helper(ch)) {
                        if (selected->transient) {
                            /* helpers do not stay above transient windows */
                            continue;
                        }
                        addto = &group_helpers;
                    }
                    else {
                        if (ch->modal)
                            addto = &group_modals;
                        else
                            addto = &group_trans;
                    }
                    g_assert(addto != NULL);
                    /* move it from the stacking list onto the front of the
                       list pointed at by addto */
                    stacking_list = g_list_remove_link(stacking_list, it);
                    *addto = g_list_concat(it, *addto);
                }
            }
        }
    }

    /* put modals above other direct transients */
    wins = g_list_concat(modals, trans);

    /* put helpers below direct transients */
    wins = g_list_concat(wins, group_helpers);

    /* put the selected window right below these children */
    wins = g_list_concat(wins, selit);

    /* if selected window is transient for group then raise it above others */
    if (selected->transient_for_group) {
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

    /* lower our parents after us, so they go below us */
    if (!raise && selected->parents) {
        GSList *parents_copy, *sit;
        GSList *reorder = NULL;

        parents_copy = g_slist_copy(selected->parents);

        /* go thru stacking list backwards so we can use g_slist_prepend */
        for (it = g_list_last(stacking_list); it && parents_copy;
             it = g_list_previous(it))
            if ((sit = g_slist_find(parents_copy, it->data))) {
                reorder = g_slist_prepend(reorder, sit->data);
                parents_copy = g_slist_delete_link(parents_copy, sit);
            }
        g_assert(parents_copy == NULL);

        /* call restack for each of these to lower them */
        for (sit = reorder; sit; sit = g_slist_next(sit))
            restack_windows(sit->data, raise);
    }
}

void stacking_raise(ObWindow *window)
{
    if (WINDOW_IS_CLIENT(window)) {
        ObClient *selected;
        selected = WINDOW_AS_CLIENT(window);
        restack_windows(selected, TRUE);
    } else {
        GList *selit = g_list_find(stacking_list, window);
        stacking_list = g_list_remove_link(stacking_list, selit);
        do_raise(selit);
    }
}

void stacking_lower(ObWindow *window)
{
    if (WINDOW_IS_CLIENT(window)) {
        ObClient *selected;
        selected = WINDOW_AS_CLIENT(window);
        restack_windows(selected, FALSE);
    } else {
        GList *selit = g_list_find(stacking_list, window);
        stacking_list = g_list_remove_link(stacking_list, selit);
        do_lower(selit);
    }
}

void stacking_below(ObWindow *window, ObWindow *below)
{
    GList *selit, *before;

    if (window_layer(window) != window_layer(below))
        return;

    selit = g_list_find(stacking_list, window);
    stacking_list = g_list_remove_link(stacking_list, selit);
    before = g_list_next(g_list_find(stacking_list, below));
    do_restack(selit, before);
}

void stacking_add(ObWindow *win)
{
    /* the topmost window should already be present */
    g_assert(stacking_list != NULL);

    /* don't add windows that are being unmanaged ! */
    if (WINDOW_IS_CLIENT(win)) g_assert(WINDOW_AS_CLIENT(win)->managed);

    if (WINDOW_IS_UNMANAGED(win)) {
        ObUNode *n = g_slice_new(ObUNode);
        g_assert(window_top(win) != screen_support_win);
        n->um = WINDOW_AS_UNMANAGED(win);
        n->belowme = stacking_list;
        stacking_ulist = g_list_prepend(stacking_ulist, n);
        g_hash_table_insert(stacking_umap, &window_top(win), stacking_ulist);
    }
    else {
        GList *newit = g_list_append(NULL, win);
        stacking_list = g_list_concat(stacking_list, newit);
        stacking_raise(win);
        g_hash_table_insert(stacking_map, &window_top(win), newit);
    }
}

void stacking_remove(ObWindow *win)
{
    if (WINDOW_IS_UNMANAGED(win)) {
        GList *it;
        for (it = stacking_ulist; it; it = g_list_next(it)) {
            ObUNode *n = it->data;
            if (n->um == WINDOW_AS_UNMANAGED(win)) {
                stacking_ulist = g_list_delete_link(stacking_ulist, it);
                g_slice_free(ObUNode, n);
                break;
            }
        }
        g_hash_table_remove(stacking_umap, &window_top(win));
    }
    else {
        stacking_list = g_list_remove(stacking_list, win);
        g_hash_table_remove(stacking_map, &window_top(win));
    }
}

static GList *find_highest_relative(ObClient *client)
{
    GList *ret = NULL;

    if (client->parents) {
        GList *it;
        GSList *top;

        /* get all top level relatives of this client */
        top = client_search_all_top_parents_layer(client);

        /* go from the top of the stacking order down */
        for (it = stacking_list; !ret && it; it = g_list_next(it)) {
            if (WINDOW_IS_CLIENT(it->data)) {
                ObClient *c = it->data;
                /* only look at windows in the same layer and that are
                   visible */
                if (c->layer == client->layer &&
                    !c->iconic &&
                    (c->desktop == client->desktop ||
                     c->desktop == DESKTOP_ALL ||
                     client->desktop == DESKTOP_ALL))
                {
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
    GList *it_below = NULL; /* this client will be below us */
    GList *it_above;
    GList *newit;

    if (!WINDOW_IS_CLIENT(win)) {
        stacking_add(win); /* no special rules for others */
        return;
    }

    client = WINDOW_AS_CLIENT(win);

    /* don't add windows that are being unmanaged ! */
    g_assert(client->managed);

    /* insert above its highest parent (or its highest child !) */
    it_below = find_highest_relative(client);

    if (!it_below) {
        /* nothing to put it directly above, so try find the focused client
           to put it underneath it */
        if (focus_client && client != focus_client &&
            focus_client->layer == client->layer)
        {
            it_below = g_list_find(stacking_list, focus_client);
            /* this can give NULL, but it means the focused window is on the
               bottom of the stacking order, so go to the bottom in that case,
               below it */
            it_below = g_list_next(it_below);
        }
        else {
            /* There is no window to put this directly above, so put it at the
               top, so you know it is there.

               It used to do this only if the window was focused and lower
               it otherwise.

               We also put it at the top not the bottom to fix a bug with
               fullscreen windows. When focusLast is off and followsMouse is
               on, when you switch desktops, the fullscreen window loses
               focus and goes into its lower layer. If this puts it at the
               bottom then when you come back to the desktop, the window is
               at the bottom and won't get focus back.
            */
            it_below = stacking_list;
        }
    }

    /* make sure it's not in the wrong layer though ! */
    for (; it_below; it_below = g_list_next(it_below)) {
        /* stop when the window is not in a higher layer than the window
           it is going above (it_below) */
        if (client->layer >= window_layer(it_below->data))
            break;
    }
    for (; it_below != stacking_list; it_below = it_above) {
        /* stop when the window is not in a lower layer than the
           window it is going under (it_above) */
        it_above = it_below ?
            g_list_previous(it_below) : g_list_last(stacking_list);
        if (client->layer <= window_layer(it_above->data))
            break;
    }

    newit = g_list_append(NULL, win);
    do_restack(newit, it_below);
    g_hash_table_insert(stacking_map, &window_top(win), newit);
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
            if (found && !c->iconic &&
                (c->desktop == DESKTOP_ALL || client->desktop == DESKTOP_ALL ||
                 c->desktop == client->desktop) &&
                !client_search_transient(client, c))
            {
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

/*! Returns TRUE if client occludes the sibling. If sibling is NULL it tries
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
            if (found && !c->iconic &&
                (c->desktop == DESKTOP_ALL || client->desktop == DESKTOP_ALL ||
                 c->desktop == client->desktop) &&
                !client_search_transient(c, client))
            {
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
                    else if (c->layer < client->layer)
                        break; /* we past its layer */
                }
            }
            else if (c == client)
                found = TRUE;
        }
    return occludes;
}

gboolean stacking_restack_request(ObClient *client, ObClient *sibling,
                                  gint detail)
{
    gboolean ret = FALSE;

    if (sibling && ((client->desktop != sibling->desktop &&
                     client->desktop != DESKTOP_ALL &&
                     sibling->desktop != DESKTOP_ALL) ||
                    sibling->iconic))
    {
        ob_debug("Setting restack sibling to NULL, they are not on the same "
                 "desktop or it is iconified");
        sibling = NULL;
    }

    switch (detail) {
    case Below:
        ob_debug("Restack request Below for client %s sibling %s",
                 client->title, sibling ? sibling->title : "(all)");
        /* just lower it */
        stacking_lower(CLIENT_AS_WINDOW(client));
        ret = TRUE;
        break;
    case BottomIf:
        ob_debug("Restack request BottomIf for client %s sibling %s",
                 client->title, sibling ? sibling->title : "(all)");
        /* if this client occludes sibling (or anything if NULL), then
           lower it to the bottom */
        if (stacking_occludes(client, sibling)) {
            stacking_lower(CLIENT_AS_WINDOW(client));
            ret = TRUE;
        }
        break;
    case Above:
        ob_debug("Restack request Above for client %s sibling %s",
                 client->title, sibling ? sibling->title : "(all)");
        stacking_raise(CLIENT_AS_WINDOW(client));
        ret = TRUE;
        break;
    case TopIf:
        ob_debug("Restack request TopIf for client %s sibling %s",
                 client->title, sibling ? sibling->title : "(all)");
        if (stacking_occluded(client, sibling)) {
            stacking_raise(CLIENT_AS_WINDOW(client));
            ret = TRUE;
        }
        break;
    case Opposite:
        ob_debug("Restack request Opposite for client %s sibling %s",
                 client->title, sibling ? sibling->title : "(all)");
        if (stacking_occluded(client, sibling)) {
            stacking_raise(CLIENT_AS_WINDOW(client));
            ret = TRUE;
        }
        else if (stacking_occludes(client, sibling)) {
            stacking_lower(CLIENT_AS_WINDOW(client));
            ret = TRUE;
        }
        break;
    }
    return ret;
}

void stacking_unmanaged_above_notify(ObUnmanaged *win, Window above)
{
    GList *aboveit, *winit, *uit, *mit;
    ObUNode *un, *an;

    g_assert(WINDOW_IS_UNMANAGED(win));

    winit = g_hash_table_lookup(stacking_umap, &window_top(win));

    if (!above) {
        /* at the very bottom of the stacking order */
        stacking_ulist = g_list_remove_link(stacking_ulist, winit);
        stacking_ulist = list_insert_link_before(stacking_ulist, NULL, winit);
        un = winit->data;
        un->belowme = NULL;
    }
    else if ((aboveit = g_hash_table_lookup(stacking_map, &above))) {
        /* directly above a managed window, so put it at the bottom of
           any siblings which are also above it */

        stacking_ulist = g_list_remove_link(stacking_ulist, winit);
        un = winit->data;
        un->belowme = aboveit;

        /* go through the managed windows in the stacking list from top to
           bottom.
           follow along in the list of unmanaged windows, until we come to the
           managed window @winit is now above.  then keep moving through the
           unmanaged windows until we find something above a different
           managed window, and insert @winit into the unmanaged list before it.
        */
        mit = stacking_list;
        uit = stacking_ulist;
        for (; mit; mit = g_list_next(mit)) {
            /* skip thru the unmanged windows above 'mit' */
            while (uit && ((ObUNode*)uit->data)->belowme == mit)
                uit = g_list_next(uit);
            if (mit == aboveit) {
                /* @win is above 'mit', so stick it in the unmanaged list
                   before 'uit' (the first window above something lower in the
                   managed stacking list */
                stacking_ulist = list_insert_link_before(stacking_ulist,
                                                         uit, winit);
                break; /* done */
            }
        }
    }
    else if ((aboveit = g_hash_table_lookup(stacking_umap, &above))) {
        /* directly above another unmanaged window, put it in that position
           in the stacking_ulist */

        stacking_ulist = g_list_remove_link(stacking_ulist, winit);
        stacking_ulist = list_insert_link_before(stacking_ulist,
                                                 aboveit, winit);
        /* we share the same neighbour in stacking_list */
        un = winit->data;
        an = aboveit->data;
        un->belowme = an->belowme;
    }
}

struct _ObStackingIter {
    GList *mit, *uit, *mitprev, *uitprev;
};

ObStackingIter* stacking_iter_head(void)
{
    ObStackingIter *it = g_slice_new(ObStackingIter);
    it->mit = stacking_list;
    it->mitprev = NULL;
    it->uit = stacking_ulist;
    it->uitprev = NULL;
    return it;
}

ObStackingIter* stacking_iter_tail(void)
{
    ObStackingIter *it = g_slice_new(ObStackingIter);
    it->uit = g_list_last(stacking_ulist);
    if (it->uit && ((ObUNode*)it->uit->data)->belowme == NULL) {
        /* it is below all managed windows */
        it->uitprev = g_list_previous(it->uit);
        it->mit = NULL;
        it->mitprev = g_list_last(stacking_list);
    }
    else {
        /* it is above some managed window */
        it->uitprev = it->uit;
        it->uit = NULL;
        it->mit = g_list_last(stacking_list);
        it->mitprev = it->mit ? g_list_previous(it->mit) : NULL;
    }
    return it;
}

/*! Returns 1 if it->mit is the current, and 2 if it->uit is. */
static gint stacking_iter_current(ObStackingIter *it)
{
    if (!it->uit)
        return 1;
    else {
        ObUNode *un = it->uit->data;
        if (un->belowme == it->mit) return 2;
        else return 1;
    }
}

void stacking_iter_next(ObStackingIter *it)
{
    g_assert(it->mit || it->uit); /* went past the end of the list */

    if (!it->uit) it->mit = g_list_next(it->mit);
    else {
        gint at = stacking_iter_current(it);
        if (at == 1) {
            it->mitprev = it->mit;
            it->mit = g_list_next(it->mit);
        }
        else {
            it->uitprev = it->uit;
            it->uit = g_list_next(it->uit);
        }
    }
}

void stacking_iter_prev(ObStackingIter *it)
{
    g_assert(it->mit || it->uit); /* went past the end of the list */

    /* if the prev unmanged points at the managed, it should be the
       previous position */
    if (it->uitprev && ((ObUNode*)it->uitprev->data)->belowme == it->mit) {
        it->uit = it->uitprev;
        it->uitprev = g_list_previous(it->uitprev);
    }
    else {
        it->mit = it->mitprev;
        if (it->mitprev) it->mitprev = g_list_previous(it->mitprev);
        if (!it->mit) it->uit = NULL;
    }
}

ObWindow* stacking_iter_win(ObStackingIter *it)
{
    gint at;

    if (!it->mit && !it->uit) return NULL;

    at = stacking_iter_current(it);
    if (at == 1) return it->mit->data; /* list of ObWindow */
    else return UNMANAGED_AS_WINDOW(((ObUNode*)it->uit->data)->um);
}

void stacking_iter_free(ObStackingIter *it)
{
    g_slice_free(ObStackingIter, it);
}

ObWindow* stacking_topmost_window(void)
{
    ObUNode *un = stacking_ulist ? stacking_ulist->data : NULL;
    if (un && un->belowme == stacking_list)
        /* the topmost unmanaged window is higher than the topmost
           managed window, return it */
        return UNMANAGED_AS_WINDOW(un->um);
    else if (stacking_list)
        /* the topmost managed window exists so it must be the highest */
        return stacking_list->data;
    else
        /* there is no topmost managed window, and there must not be an
           unmanaged window either, as it would be above NULL */
        return NULL;
}
