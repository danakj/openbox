#include "openbox.h"
#include "prop.h"
#include "focus.h"
#include "client.h"
#include "group.h"
#include "frame.h"
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
             it = it->prev, ++win_it)
	    *win_it = ((Client*)it->data)->window;
    } else
	windows = NULL;

    PROP_SETA32(ob_root, net_client_list_stacking, window, windows, size);

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

static void raise_recursive(Client *client)
{
    Window wins[2];  /* only ever restack 2 windows. */
    GList *it;
    GSList *sit;

    g_assert(stacking_list != NULL); /* this would be bad */

    /* remove the client before looking so we can't run into ourselves and our
       transients can't either. */
    stacking_list = g_list_remove(stacking_list, client);

    /* raise transients first */
    for (sit = client->transients; sit; sit = sit->next)
        raise_recursive(sit->data);

    /* find 'it' where it is the positiion in the stacking order where
       'client' will be inserted *before* */

    it = find_lowest_transient(client);
    if (it)
        it = it->next;
    else {
        /* the stacking list is from highest to lowest */
        for (it = stacking_list; it; it = it->next) {
            if (client->layer >= ((Client*)it->data)->layer)
                break;
        }
    }

    /*
      if our new position is the top, we want to stack under the focus_backup.
      otherwise, we want to stack under the previous window in the stack.
    */
    if (it == stacking_list)
	wins[0] = focus_backup;
    else if (it != NULL)
	wins[0] = ((Client*)it->prev->data)->frame->window;
    else
	wins[0] = ((Client*)g_list_last(stacking_list)->data)->frame->window;
    wins[1] = client->frame->window;

    stacking_list = g_list_insert_before(stacking_list, it, client);

    XRestackWindows(ob_display, wins, 2);
}

void stacking_raise(Client *client)
{
    g_assert(stacking_list != NULL); /* this would be bad */

    /* move up the transient chain as far as possible first */
    while (client->transient_for) {
        if (client->transient_for != TRAN_GROUP) {
            client = client->transient_for;
        } else {
            GSList *it;

            /* the check for TRAN_GROUP is to prevent an infinate loop with
               2 transients of the same group at the head of the group's
               members list */
            for (it = client->group->members; it; it = it->next) {
                Client *c = it->data;

                if (c != client && c->transient_for != TRAN_GROUP) {
                    client = it->data;
                    break;
                }
            }
            if (it == NULL) break;
        }
    }

    raise_recursive(client);

    stacking_set_list();
}

static void lower_recursive(Client *client, Client *above)
{
    Window wins[2];  /* only ever restack 2 windows. */
    GList *it;
    GSList *sit;

    /* find 'it' where 'it' is the position in the stacking_list where the
       'client' will be placed *after* */

    for (it = g_list_last(stacking_list); it != stacking_list; it = it->prev)
        if (client->layer <= ((Client*)it->data)->layer && it->data != above)
            break;

    if (it->data != client) { /* not already the bottom */
        wins[0] = ((Client*)it->data)->frame->window;
        wins[1] = client->frame->window;

        stacking_list = g_list_remove(stacking_list, client);
        stacking_list = g_list_insert_before(stacking_list, it->next, client);
        XRestackWindows(ob_display, wins, 2);
    }

    for (sit = client->transients; sit; sit = sit->next)
        lower_recursive(sit->data, client);
}

void stacking_lower(Client *client)
{
    g_assert(stacking_list != NULL); /* this would be bad */

    /* move up the transient chain as far as possible first */
    while (client->transient_for) {
        if (client->transient_for != TRAN_GROUP) {
            client = client->transient_for;
        } else {
            GSList *it;

            /* the check for TRAN_GROUP is to prevent an infinate loop with
               2 transients of the same group at the head of the group's
               members list */
            for (it = client->group->members; it; it = it->next) {
                Client *c = it->data;

                if (c != client && c->transient_for != TRAN_GROUP) {
                    client = it->data;
                    break;
                }
            }
            if (it == NULL) break;
        }
    }

    lower_recursive(client, NULL);

    stacking_set_list();
}

