#include "openbox.h"
#include "prop.h"
#include "screen.h"
#include "focus.h"
#include "client.h"
#include "group.h"
#include "frame.h"
#include "window.h"
#include <glib.h>

GList  *stacking_list = NULL;

void stacking_set_list()
{
    Window *windows = NULL;
    GList *it;
    guint i = 0;

    /* on shutdown, don't update the properties, so that we can read it back
       in on startup and re-stack the windows as they were before we shut down
    */
    if (ob_state == OB_STATE_EXITING) return;

    /* create an array of the window ids (from bottom to top,
       reverse order!) */
    if (stacking_list) {
	windows = g_new(Window, g_list_length(stacking_list));
        for (it = g_list_last(stacking_list); it; it = g_list_previous(it)) {
            if (WINDOW_IS_CLIENT(it->data))
                windows[i++] = WINDOW_AS_CLIENT(it->data)->window;
        }
    }

    PROP_SETA32(ob_root, net_client_list_stacking, window,
                (guint32*)windows, i);

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

    /* XXX some debug checking of the stacking list's order */
    for (it = stacking_list; ; it = next) {
        next = g_list_next(it);
        if (!next) break;
        g_assert(window_layer(it->data) >= window_layer(next->data));
    }

    XRestackWindows(ob_display, win, i);
    g_free(win);

    stacking_set_list();
}

static void do_raise(GList *wins)
{
    GList *it;
    GList *layer[OB_NUM_STACKING_LAYERS] = {NULL};
    int i;

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
    int i;

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

static GList *pick_windows(ObClient *top, ObClient *selected, gboolean raise)
{
    GList *ret = NULL;
    GList *it, *next, *prev;
    GSList *sit;
    int i, n;
    GList *modals = NULL;
    GList *trans = NULL;
    GList *modal_sel_it = NULL; /* the selected guy if modal */
    GList *trans_sel_it = NULL; /* the selected guy if not */

    /* remove first so we can't run into ourself */
    stacking_list = g_list_remove(stacking_list, top);

    i = 0;
    n = g_slist_length(top->transients);
    for (it = stacking_list; i < n && it; it = next) {
        prev = g_list_previous(it);
        next = g_list_next(it);

        if ((sit = g_slist_find(top->transients, it->data))) {
            ObClient *c = sit->data;

            ++i;

            if (!c->modal) {
                if (c != selected) {
                    trans = g_list_concat(trans,
                                           pick_windows(c, selected, raise));
                } else {
                    g_assert(modal_sel_it == NULL);
                    g_assert(trans_sel_it == NULL);
                    trans_sel_it = pick_windows(c, selected, raise);
                }
            } else {
                if (c != selected) {
                    modals = g_list_concat(modals,
                                           pick_windows(c, selected, raise));
                } else {
                    g_assert(modal_sel_it == NULL);
                    g_assert(trans_sel_it == NULL);
                    modal_sel_it = pick_windows(c, selected, raise);
                }
            }
            /* if we dont have a prev then start back at the beginning,
               otherwise skip back to the prev's next */
            next = prev ? g_list_next(prev) : stacking_list;
        }
    }

    ret = g_list_concat((raise ? modal_sel_it : modals),
                        (raise ? modals : modal_sel_it));

    ret = g_list_concat(ret, (raise ? trans_sel_it : trans));
    ret = g_list_concat(ret, (raise ? trans : trans_sel_it));


    /* add itself */
    ret = g_list_append(ret, top);

    return ret;
}

static GList *pick_group_windows(ObClient *top, ObClient *selected, gboolean raise)
{
    GList *ret = NULL;
    GList *it, *next, *prev;
    GSList *sit;
    int i, n;

    /* add group members in their stacking order */
    if (top->group) {
        i = 0;
        n = g_slist_length(top->group->members) - 1;
        for (it = stacking_list; i < n && it; it = next) {
            prev = g_list_previous(it);
            next = g_list_next(it);

            if ((sit = g_slist_find(top->group->members, it->data))) {
                ++i;
                ret = g_list_concat(ret,
                                    pick_windows(sit->data, selected, raise)); 
                /* if we dont have a prev then start back at the beginning,
                   otherwise skip back to the prev's next */
                next = prev ? g_list_next(prev) : stacking_list;
            }
        }
    }
    return ret;
}

void stacking_raise(ObWindow *window)
{
    GList *wins;

    if (WINDOW_IS_CLIENT(window)) {
        ObClient *c;
        ObClient *selected;
        selected = WINDOW_AS_CLIENT(window);
        c = client_search_top_transient(selected);
        wins = pick_windows(c, selected, TRUE);
        wins = g_list_concat(wins, pick_group_windows(c, selected, TRUE));
    } else {
        wins = g_list_append(NULL, window);
        stacking_list = g_list_remove(stacking_list, window);
    }
    do_raise(wins);
    g_list_free(wins);
}

void stacking_lower(ObWindow *window)
{
    GList *wins;

    if (WINDOW_IS_CLIENT(window)) {
        ObClient *c;
        ObClient *selected;
        selected = WINDOW_AS_CLIENT(window);
        c = client_search_top_transient(selected);
        wins = pick_windows(c, selected, FALSE);
        wins = g_list_concat(pick_group_windows(c, selected, FALSE), wins);
    } else {
        wins = g_list_append(NULL, window);
        stacking_list = g_list_remove(stacking_list, window);
    }
    do_lower(wins);
    g_list_free(wins);
}

void stacking_add(ObWindow *win)
{
    ObStackingLayer l;
    GList *wins;

    g_assert(screen_support_win != None); /* make sure I dont break this in the
                                       future */

    l = window_layer(win);
    wins = g_list_append(NULL, win); /* list of 1 element */

    stacking_list = g_list_append(stacking_list, win);
    stacking_raise(win);
}

void stacking_add_nonintrusive(ObWindow *win)
{
    ObClient *client;
    ObClient *parent = NULL;
    GList *it_before = NULL;

    if (!WINDOW_IS_CLIENT(win)) {
        stacking_add(win); /* no special rules for others */
        return;
    }

    client = WINDOW_AS_CLIENT(win);

    /* insert above its highest parent */
    if (client->transient_for) {
        if (client->transient_for != OB_TRAN_GROUP) {
            parent = client->transient_for;
        } else {
            GSList *sit;
            GList *it;

            if (client->group)
                for (it = stacking_list; !parent && it; it = it->next) {
                    if ((sit = g_slist_find(client->group->members, it->data)))
                for (sit = client->group->members; !parent && sit;
                     sit = sit->next) {
                    ObClient *c = sit->data;
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
