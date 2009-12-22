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
#include "debug.h"
#include "obt/prop.h"

/*! The list of all windows in their stacking order, from top to bottom */
GList  *stacking_list = NULL;
/*! When true, stacking changes will not be reflected on the screen.  This is
  to freeze the on-screen stacking order while a window is being temporarily
  raised during focus cycling */
static gboolean pause_changes = FALSE;

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

/* steals the elements of the wins list, so don't free them */
static void do_restack(GList *wins, GList *before)
{
    GList *it, *after;
    Window *win;
    gint i;

#ifdef DEBUG
    GList *next;

    g_assert(wins);
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
    }

    /* wins are inserted before @before and after @after
       stacking_list = after..wins..before */
    if (before == stacking_list)
        after = NULL;
    else {
        after = stacking_list;
        /* split the list at before */
        if (before) {
            before->prev->next = NULL;
            before->prev = NULL;
        }
    }
    stacking_list = g_list_concat(g_list_concat(after, wins), before);

#ifdef DEBUG
    /* some debug checking of the stacking list's order */
    for (it = stacking_list; ; it = next) {
        next = g_list_next(it);
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
    win[0] = screen_support_win;
    for (i = 1, it = stacking_list; it; ++i, it = g_list_next(it))
        win[i] = window_top(it->data);
    start = event_start_ignore_all_enters();
    XRestackWindows(obt_display, win, i);
    event_end_ignore_all_enters(start);
    g_free(win);

    pause_changes = FALSE;
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
        }
    }
}

/*! Returns a position in the stacking_list, which is the first window
  from the top down that is in a layer <= the query layer, or NULL if there
  are no such windows.
*/
GList* find_top_of_layer(ObStackingLayer layer)
{
    GList *top, *it;

    top = NULL;
    /* start from the bottom of the list and go up */
    for (it = g_list_last(stacking_list); it; it = g_list_previous(it))
    {
        if (window_layer(it->data) > layer)
            break;
        top = it;
    }
    return top;
}

static void find_parents_r(ObClient *client, GHashTable *t)
{
    GSList *sit;
    for (sit = client->parents; sit; sit = g_slist_next(sit)) {
        g_hash_table_insert(t, sit->data, sit->data);
        find_parents_r(sit->data, t);
    }
}

static GList* get_parents(ObClient *client)
{
    GHashTable *parents;
    GList *it, *next, *ret;

    if (!client->parents) return NULL;

    /* collect all the parents */

    parents = g_hash_table_new(g_direct_hash, g_direct_equal);
    find_parents_r(client, parents);

    /* go through the stacking list and find all the parents, and put them
       into a list in order */
    ret = NULL;
    for (it = g_list_last(stacking_list); it; it = next) {
        next = g_list_previous(it);
        if (g_hash_table_lookup(parents, it->data)) {
            ret = g_list_prepend(ret, it->data);
            stacking_list = g_list_delete_link(stacking_list, it);
        }
    }

    g_hash_table_destroy(parents);
    return ret;
}

/*! reorders the windows from the stacking_list, removing them and placing
  them into some lists.
  @direct A list of windows directly related to the @selected window, and
          will contain @selected, ordered from top to bottom.
  @group_trans A list of windows indirectly related to the @selected window,
          which should be placed above @direct, ordered from top to bottom.
*/
static void get_restack_order(ObClient *selected, gboolean raise,
                              GList **direct, GList **grouped)
{
    GList *it, *next;
    GList *group_helpers = NULL;
    GList *group_modals = NULL;
    GList *group_trans = NULL;
    GList *modals = NULL;
    GList *trans = NULL;

    if (raise) {
        ObClient *p;

        /* if a window is modal for another single window, then raise it to the
           top too, the same is done with the focus order
           (transients are always in the same layer)
        */
        while (selected->modal && (p = client_direct_parent(selected)))
            selected = p;
    }

    /* remove first so we can't run into ourself */
    it = g_list_find(stacking_list, selected);
    g_assert(it);
    stacking_list = g_list_delete_link(stacking_list, it);

    /* go from the bottom of the stacking list up. don't move any other windows
       when lowering, we call this for each window independently */
    if (raise) {
        for (it = g_list_last(stacking_list); it; it = next) {
            next = g_list_previous(it);

            if (WINDOW_IS_CLIENT(it->data)) {
                ObClient *ch = it->data;

                /* only move windows in the same stacking layer */
                if (ch->layer == selected->layer &&
                    /* looking for windows that are transients, and so would
                       remain above the selected window */
                    client_search_transient(selected, ch))
                {
                    if (client_is_direct_child(selected, ch)) {
                        if (ch->modal)
                            modals = g_list_prepend(modals, ch);
                        else
                            trans = g_list_prepend(trans, ch);
                    }
                    else if (client_helper(ch)) {
                        if (selected->transient) {
                            /* helpers do not stay above transient windows */
                            continue;
                        }
                        group_helpers = g_list_prepend(group_helpers, ch);
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
    }

    /* put modals above other direct transients */
    *direct = g_list_concat(modals, trans);

    /* put helpers below direct transients */
    *direct = g_list_concat(*direct, group_helpers);

    /* put the selected window right below these children */
    *direct = g_list_append(*direct, selected);

    /* if selected window is transient for group then raise it above others */
    if (selected->transient_for_group) {
        /* if it's modal, raise it above those also */
        if (selected->modal) {
            *direct = g_list_concat(*direct, group_modals);
            group_modals = NULL;
        }
        *direct = g_list_concat(*direct, group_trans);
        group_trans = NULL;
    }

    /* if not raising, the selected window can't go below its parents, so
       put them all in the direct list below it */
    if (!raise)
        *direct = g_list_concat(*direct, get_parents(selected));

    /* modal group transients go above other group transients */
    *grouped = g_list_concat(group_modals, group_trans);
}

static GList* get_app_windows(ObClient *selected)
{
    GList *it, *next, *app;

    app = NULL;
    for (it = g_list_last(stacking_list); it; it = next) {
        next = g_list_previous(it);

        if (WINDOW_IS_CLIENT(it->data) &&
            window_layer(it->data) == selected->layer &&
            client_is_in_application(WINDOW_AS_CLIENT(it->data), selected))
        {
            stacking_list = g_list_remove_link(stacking_list, it);
            app = g_list_concat(it, app);
        }
    }
    return app;
}

static void restack_windows(ObClient *selected, gboolean raise, gboolean app)
{
    GList *direct, *group_trans, *app_members;

    GList *it, *next, *direct_below, *group_trans_below, *between;

    /* get the restacking order for the selected window and its relatives */
    get_restack_order(selected, raise, &direct, &group_trans);

    /* find any remaining app windows */
    if (app)
        app_members = get_app_windows(selected);
    else
        app_members = NULL;

    /* stick the selected window and all its direct-relations above this
       spot */
    direct_below = find_top_of_layer(raise ?
                                     selected->layer : selected->layer - 1);

    /* stick the group transients just above the highest group member left
       in the stacking order (or else just above the direct windows */
    group_trans_below = direct_below;
    if (selected->group && group_trans) {
        /* find the highest member of @selected's group */
        it = group_trans_below;
        for (it = g_list_previous(it); it; it = g_list_previous(it)) {
            if (window_layer(it->data) > selected->layer)
                break;
            if (WINDOW_IS_CLIENT(it->data)) {
                ObClient *c = WINDOW_AS_CLIENT(it->data);
                if (c->group == selected->group)
                    group_trans_below = it;
            }
        }
    }
    else
        g_assert(group_trans == NULL);

    /* grab all the stuff between group_trans_below and direct_below */
    between = NULL;
    for (it = direct_below; it && it != group_trans_below; it = next) {
        next = g_list_previous(it);
        stacking_list = g_list_remove_link(stacking_list, it);
        between = g_list_concat(it, between);
    }

    /* stick them all together (group_trans, between, direct, app_members) */
    direct = g_list_concat(group_trans,
                           g_list_concat(between,
                                         raise ?
                                         /* if raising it goes above app */
                                         g_list_concat(direct, app_members) :
                                         /* if lowering it goes below */
                                         g_list_concat(app_members, direct)));

    /* restack them */
    do_restack(direct, direct_below);
}

void stacking_raise_app(ObClient *client)
{
    restack_windows(client, TRUE, TRUE);
}

void stacking_raise(ObWindow *window)
{
    if (WINDOW_IS_CLIENT(window)) {
        ObClient *selected;
        selected = WINDOW_AS_CLIENT(window);
        restack_windows(selected, TRUE, FALSE);
    } else {
        GList *wins;
        wins = g_list_append(NULL, window);
        stacking_list = g_list_remove(stacking_list, window);
        do_raise(wins);
        g_list_free(wins);
    }
}

void stacking_lower_app(struct _ObClient *client)
{
    restack_windows(client, FALSE, TRUE);
}

void stacking_lower(ObWindow *window)
{
    if (WINDOW_IS_CLIENT(window)) {
        ObClient *selected;
        selected = WINDOW_AS_CLIENT(window);
        restack_windows(selected, FALSE, FALSE);
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
    GList *wins;

    if (!WINDOW_IS_CLIENT(win)) {
        stacking_add(win); /* no special rules for others */
        return;
    }

    client = WINDOW_AS_CLIENT(win);

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

    wins = g_list_append(NULL, win);
    do_restack(wins, it_below);
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
