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

static GList *find_lowest_transient(Client *c)
{
    GList *it;
    GSList *sit;

    for (it = g_list_last(stacking_list); it; it = it->prev)
        for (sit = c->transients; sit; sit = sit->next)
            if (it->data == sit->data) /* found a transient */
                return it;
    return NULL;
}

static void raise_recursive(ObWindow *window)
{
    Window wins[2];  /* only ever restack 2 windows. */
    GList *it, *low;
    GSList *sit;

    g_assert(stacking_list != NULL); /* this would be bad */

    /* remove the window before looking so we can't run into ourselves and our
       transients can't either. */
    stacking_list = g_list_remove(stacking_list, window);

    /* raise transients first */
    if (WINDOW_IS_CLIENT(window)) {
        Client *client = WINDOW_AS_CLIENT(window);
        for (sit = client->transients; sit; sit = sit->next)
            raise_recursive(sit->data);
    }

    /* find 'it' where it is the positiion in the stacking order where
       'window' will be inserted *before* */

    if (WINDOW_IS_CLIENT(window))
        low = find_lowest_transient(WINDOW_AS_CLIENT(window));
    else
        low = NULL;
    /* the stacking list is from highest to lowest */
    for (it = g_list_last(stacking_list); it; it = it->prev) {
        if (it == low || window_layer(window) < window_layer(it->data)) {
            it = it->next;
            break;
        }
        if (it == stacking_list)
            break;
    }

    /*
      if our new position is the top, we want to stack under the focus_backup.
      otherwise, we want to stack under the previous window in the stack.
    */
    if (it == stacking_list)
	wins[0] = focus_backup;
    else if (it != NULL)
	wins[0] = window_top(it->prev->data);
    else
	wins[0] = window_top(g_list_last(stacking_list)->data);
    wins[1] = window_top(window);

    stacking_list = g_list_insert_before(stacking_list, it, window);

    XRestackWindows(ob_display, wins, 2);
}

void stacking_raise(ObWindow *window)
{
    g_assert(stacking_list != NULL); /* this would be bad */

    if (WINDOW_IS_CLIENT(window)) {
        Client *client = WINDOW_AS_CLIENT(window);
        /* move up the transient chain as far as possible first */
        if (client->transient_for) {
            if (client->transient_for != TRAN_GROUP) {
                stacking_raise(CLIENT_AS_WINDOW(client->transient_for));
                return;
            } else {
                GSList *it;

                /* the check for TRAN_GROUP is to prevent an infinate loop with
                   2 transients of the same group at the head of the group's
                   members list */
                for (it = client->group->members; it; it = it->next) {
                    Client *c = it->data;

                    if (c != client && c->transient_for != TRAN_GROUP)
                        stacking_raise(it->data);
                }
                if (it == NULL) return;
            }
        }
    }

    raise_recursive(window);

    stacking_set_list();
}

static void lower_recursive(ObWindow *window, ObWindow *above)
{
    Window wins[2];  /* only ever restack 2 windows. */
    GList *it;
    GSList *sit;

    /* find 'it' where 'it' is the position in the stacking_list where the
       'window' will be placed *after* */

    for (it = g_list_last(stacking_list); it != stacking_list; it = it->prev)
        if (window_layer(window) <= window_layer(it->data) &&
            it->data != above)
            break;

    if (it->data != window) { /* not already the bottom */
        wins[0] = window_top(it->data);
        wins[1] = window_top(window);

        stacking_list = g_list_remove(stacking_list, window);
        stacking_list = g_list_insert_before(stacking_list, it->next, window);
        XRestackWindows(ob_display, wins, 2);
    }

    if (WINDOW_IS_CLIENT(window)) {
        Client *client = WINDOW_AS_CLIENT(window);
        for (sit = client->transients; sit; sit = sit->next)
            lower_recursive(CLIENT_AS_WINDOW(sit->data), window);
    }
}

void stacking_lower(ObWindow *window)
{
    g_assert(stacking_list != NULL); /* this would be bad */

    if (WINDOW_IS_CLIENT(window)) {
        Client *client = WINDOW_AS_CLIENT(window);
        /* move up the transient chain as far as possible first */
        while (client->transient_for) {
            if (client->transient_for != TRAN_GROUP) {
                stacking_lower(CLIENT_AS_WINDOW(client->transient_for));
                return;
            } else {
                GSList *it;

                /* the check for TRAN_GROUP is to prevent an infinate loop with
                   2 transients of the same group at the head of the group's
                   members list */
                for (it = client->group->members; it; it = it->next) {
                    Client *c = it->data;

                    if (c != client && c->transient_for != TRAN_GROUP)
                        stacking_lower(it->data);
                }
                if (it == NULL) return;
            }
        }
        window = CLIENT_AS_WINDOW(client);
    }

    lower_recursive(window, NULL);

    stacking_set_list();
}

void stacking_add(ObWindow *win)
{
    stacking_list = g_list_append(stacking_list, win);
    stacking_raise(win);
}

void stacking_add_nonintrusive(ObWindow *win)
{
    Window wins[2];  /* only ever restack 2 windows. */

    if (!WINDOW_IS_CLIENT(win))
        stacking_add(win); /* no special rules for others */
    else {
        Client *client = WINDOW_AS_CLIENT(win);
        Client *parent = NULL;
        GList *it_before = NULL;

        /* insert above its highest parent */
        if (client->transient_for) {
            if (client->transient_for != TRAN_GROUP) {
                parent = client->transient_for;
            } else {
                GSList *sit;
                GList *it;

                /* the check for TRAN_GROUP is to prevent an infinate loop with
                   2 transients of the same group at the head of the group's
                   members list */
                for (it = stacking_list; !parent && it; it = it->next) {
                    for (sit = client->group->members; !parent && sit;
                         sit = sit->next) {
                        Client *c = sit->data;
                        if (sit->data == it->data &&
                            c->transient_for != TRAN_GROUP)
                            parent = it->data;
                    }
                }
            }
        }

        if (!(it_before = g_list_find(stacking_list, parent))) {
            /* no parent to put above, try find the focused client to go
               under */
            if ((it_before = g_list_find(stacking_list, focus_client)))
                it_before = it_before->next;
            else {
                /* out of ideas, just add it normally... */
                stacking_add(win);
                return;
            }
        }
        stacking_list = g_list_insert_before(stacking_list, it_before, win);

        it_before = g_list_find(stacking_list, win)->prev;
        if (!it_before)
            wins[0] = focus_backup;
        else
            wins[0] = window_top(it_before->data);
        wins[1] = window_top(win);

        XRestackWindows(ob_display, wins, 2);
    }
}
