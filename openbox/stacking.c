#include "openbox.h"
#include "prop.h"
#include "focus.h"
#include "client.h"
#include "group.h"
#include "frame.h"
#include "window.h"
#include <glib.h>

GList  *stacking_list = NULL;

void stacking_set_list()
{
    Window *windows, *win_it;
    GList *it;
    guint size = g_list_length(stacking_list);

    /* on shutdown, don't update the properties, so that we can read it back
       in on startup and re-stack the windows as they were before we shut down
    */
    if (ob_state == State_Exiting) return;

    /* create an array of the window ids (from bottom to top,
       reverse order!) */
    if (size > 0) {
	windows = g_new(Window, size);
	win_it = windows;
	for (it = g_list_last(stacking_list); it != NULL;
             it = it->prev)
            if (WINDOW_IS_CLIENT(it->data)) {
                *win_it = WINDOW_AS_CLIENT(it->data)->window;
                ++win_it;
            }
    } else
	windows = win_it = NULL;

    PROP_SETA32(ob_root, net_client_list_stacking, window,
                (guint32*)windows, win_it - windows);

    if (windows)
	g_free(windows);
}

static void do_restack(GList *wins, GList *before)
{
    GList *it, *next;
    Window *win;
    int i;

    /* pls only restack stuff in the same layer at a time */
    for (it = wins; it; it = next) {
        next = g_list_next(it);
        if (!next) break;
        g_assert (window_layer(it->data) == window_layer(next->data));
    }


    win = g_new(Window, g_list_length(wins) + 1);

    if (before == stacking_list)
        win[0] = focus_backup;
    else if (!before)
        win[0] = window_top(g_list_last(stacking_list)->data);
    else
        win[0] = window_top(g_list_previous(before)->data);

    for (i = 1, it = wins; it; ++i, it = g_list_next(it)) {
        win[i] = window_top(it->data);
        stacking_list = g_list_insert_before(stacking_list, before, it->data);
    }

    /* XXX some debug checking of the stacking list's order */
    for (it = stacking_list; ; it = next) {
        next = g_list_next(it);
        if (!next) break;
        g_assert(window_layer(it->data) >= window_layer(next->data));
    }

    XRestackWindows(ob_display, win, i);
    g_free(win);
}

static void raise(GList *wins)
{
    GList *it;
    GList *layer[NUM_STACKLAYER] = {NULL};
    int i;

    for (it = wins; it; it = g_list_next(it)) {
        StackLayer l;

        l = window_layer(it->data);
        layer[l] = g_list_append(layer[l], it->data);
    }

    it = stacking_list;
    for (i = NUM_STACKLAYER - 1; i >= 0; --i) {
        if (layer[i]) {
            for (; it; it = g_list_next(it)) {
                /* look for the top of the layer */
                if (window_layer(it->data) <= (StackLayer) i)
                    break;
            }
            do_restack(layer[i], it);
            g_list_free(layer[i]);
        }
    }
}

static void lower(GList *wins)
{
    GList *it;
    GList *layer[NUM_STACKLAYER] = {NULL};
    int i;

    for (it = wins; it; it = g_list_next(it)) {
        StackLayer l;

        l = window_layer(it->data);
        layer[l] = g_list_append(layer[l], it->data);
    }

    it = stacking_list;
    for (i = NUM_STACKLAYER - 1; i >= 0; --i) {
        if (layer[i]) {
            for (; it; it = g_list_next(it)) {
                /* look for the top of the next layer down */
                if (window_layer(it->data) < (StackLayer) i)
                    break;
            }
            do_restack(layer[i], it);
            g_list_free(layer[i]);
        }
    }
}

static GList *pick_windows(ObWindow *win)
{
    GList *ret = NULL;
    GList *it, *next;
    GSList *sit;
    Client *c;
    int i, n;

    if (!WINDOW_IS_CLIENT(win)) {
        ret = g_list_append(ret, win);
        stacking_list = g_list_remove(stacking_list, win);
        return ret;
    }

    c = WINDOW_AS_CLIENT(win);

    /* remove first so we can't run into ourself */
    stacking_list = g_list_remove(stacking_list, win);

    /* add transient children in their stacking order */
    i = 0;
    n = g_slist_length(c->transients);
    for (it = stacking_list; i < n && it; it = next) {
        next = g_list_next(it);
        if ((sit = g_slist_find(c->transients, it->data))) {
            ++i;
            ret = g_list_concat(ret, pick_windows(sit->data));
            it = stacking_list;
        }
    }

    /* add itself */
    ret = g_list_append(ret, win);

    return ret;
}

static GList *pick_group_windows(ObWindow *win)
{
    GList *ret = NULL;
    GList *it, *next;
    GSList *sit;
    Client *c;
    int i, n;

    if (!WINDOW_IS_CLIENT(win))
        return NULL;

    c = WINDOW_AS_CLIENT(win);

    /* add group members in their stacking order */
    if (c->group) {
        i = 0;
        n = g_slist_length(c->group->members) - 1;
        for (it = stacking_list; i < n && it; it = next) {
            next = g_list_next(it);
            if ((sit = g_slist_find(c->group->members, it->data))) {
                ++i;
                ret = g_list_concat(ret, pick_windows(sit->data)); 
                it = stacking_list;
            }
        }
    }
    return ret;
}

static ObWindow *top_transient(ObWindow *window)
{
    Client *client;

    if (!WINDOW_IS_CLIENT(window))
        return window;

    client = WINDOW_AS_CLIENT(window);

    /* move up the transient chain as far as possible */
    if (client->transient_for) {
        if (client->transient_for != TRAN_GROUP) {
            return top_transient(CLIENT_AS_WINDOW(client->transient_for));
        } else {
            GSList *it;

            for (it = client->group->members; it; it = it->next) {
                Client *c = it->data;

                /* checking transient_for prevents infinate loops! */
                if (c != client && !c->transient_for)
                    break;
            }
            if (it)
                return it->data;
        }
    }

    return window;
}

void stacking_raise(ObWindow *window)
{
    GList *wins;

    window = top_transient(window);
    wins = pick_windows(window);
    wins = g_list_concat(wins, pick_group_windows(window));
    raise(wins);
    g_list_free(wins);
}

void stacking_lower(ObWindow *window)
{
    GList *wins;

    window = top_transient(window);
    wins = pick_windows(window);
    wins = g_list_concat(wins, pick_group_windows(window));
    lower(wins);
    g_list_free(wins);
}

void stacking_add(ObWindow *win)
{
    StackLayer l;
    GList *wins, *it;

    l = window_layer(win);
    wins = g_list_append(NULL, win); /* list of 1 element */

    for (it = stacking_list; it; it = g_list_next(it))
        if (window_layer(it->data) <= l)
            break;
    do_restack(wins, it);
    g_list_free(wins);

    stacking_raise(win);
}

void stacking_add_nonintrusive(ObWindow *win)
{
    Client *client;
    Client *parent = NULL;
    GList *it_before = NULL;

    if (!WINDOW_IS_CLIENT(win)) {
        stacking_add(win); /* no special rules for others */
        return;
    }

    client = WINDOW_AS_CLIENT(win);

    /* insert above its highest parent */
    if (client->transient_for) {
        if (client->transient_for != TRAN_GROUP) {
            parent = client->transient_for;
        } else {
            GSList *sit;
            GList *it;

            if (client->group)
                for (it = stacking_list; !parent && it; it = it->next) {
                    if ((sit = g_slist_find(client->group->members, it->data)))
                for (sit = client->group->members; !parent && sit;
                     sit = sit->next) {
                    Client *c = sit->data;
                    /* checking transient_for prevents infinate loops! */
                    if (sit->data == it->data && !c->transient_for)
                        parent = it->data;
                }
            }
        }
    }

    if (!(it_before = g_list_find(stacking_list, parent))) {
        /* no parent to put above, try find the focused client to go
           under */
        if (focus_client && focus_client->layer == client->layer) {
            if ((it_before = g_list_find(stacking_list, focus_client)))
                it_before = it_before->next;
        }
    }
    if (!it_before) {
        /* out of ideas, just add it normally... */
        stacking_add(win);
    } else {
        GList *wins = g_list_append(NULL, win);
        do_restack(wins, it_before);
        g_list_free(wins);
    }
}
