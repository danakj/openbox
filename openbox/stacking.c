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

    PROP_SET32A(ob_root, net_client_list_stacking, window, windows, size);

    if (windows)
	g_free(windows);
}

void stacking_raise(Client *client)
{
    Window wins[2];  /* only ever restack 2 windows. */
    GList *it;
    Client *m;

    g_assert(stacking_list != NULL); /* this would be bad */

    m = client_find_modal_child(client);
    /* if we have a modal child, raise it instead, we'll go along tho later */
    if (m) stacking_raise(m);
  
    /* remove the client before looking so we can't run into ourselves */
    stacking_list = g_list_remove(stacking_list, client);
  
    /* the stacking list is from highest to lowest */
    it = stacking_list;
    while (it != NULL) {
	Client *c = it->data;
	if (client->layer >= c->layer && m != c)
	    break;
	it = it->next;
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

    stacking_set_list();
}

void stacking_lower(Client *client)
{
    Window wins[2];  /* only ever restack 2 windows. */
    GList *it;

    g_assert(stacking_list != NULL); /* this would be bad */

    if (client->modal && client->transient_for) {
        if (client->transient_for == TRAN_GROUP) {
            /* don't let a modal of the group lower below any other windows
               in the group */
            for (it = stacking_list; it; it = it->next) {
                GSList *sit;
                Client *c = it->data;

                if (it->data == client) continue;

                for (sit = c->group->members; sit; sit = sit->next)
                    if (sit->data == it->data) break;
                if (sit) break; /* got it */
            }
            if (it == NULL)
                goto lower_no_parent;
        } else {
            /* don't let a modal window lower below its transient_for */
            it = g_list_find(stacking_list, client->transient_for);
        }
        g_assert(it != NULL);

	wins[0] = (it == stacking_list ? focus_backup :
		   ((Client*)it->prev->data)->frame->window);
	wins[1] = client->frame->window;
	if (wins[0] == wins[1]) return; /* already right above the window */

	stacking_list = g_list_remove(stacking_list, client);
	stacking_list = g_list_insert_before(stacking_list, it, client);
    } else {
    lower_no_parent:

        it = g_list_last(stacking_list);

	while (it != stacking_list) {
	    Client *c = it->data;
	    if (client->layer <= c->layer)
		break;
	    it = it->prev;
	}
	if (it->data == client) return; /* already the bottom, return */

	wins[0] = ((Client*)it->data)->frame->window;
	wins[1] = client->frame->window;

	stacking_list = g_list_remove(stacking_list, client);
	stacking_list = g_list_insert_before(stacking_list,
					     it->next, client);
    }

    XRestackWindows(ob_display, wins, 2);
    stacking_set_list();
}

