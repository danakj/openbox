#include "event.h"
#include "openbox.h"
#include "client.h"
#include "config.h"
#include "frame.h"
#include "screen.h"
#include "group.h"
#include "prop.h"
#include "dispatch.h"
#include "focus.h"
#include "parse.h"
#include "stacking.h"

#include <X11/Xlib.h>
#include <glib.h>

Client *focus_client = NULL;
GList **focus_order = NULL; /* these lists are created when screen_startup
                               sets the number of desktops */

Window focus_backup = None;

static gboolean noreorder = 0;

void focus_startup()
{
    /* create the window which gets focus when no clients get it. Have to
       make it override-redirect so we don't try manage it, since it is
       mapped. */
    XSetWindowAttributes attrib;

    focus_client = NULL;

    attrib.override_redirect = TRUE;
    focus_backup = XCreateWindow(ob_display, ob_root,
				 -100, -100, 1, 1, 0,
                                 CopyFromParent, InputOutput, CopyFromParent,
                                 CWOverrideRedirect, &attrib);
    XMapWindow(ob_display, focus_backup);
    stacking_raise_internal(focus_backup);

    /* start with nothing focused */
    focus_set_client(NULL);
}

void focus_shutdown()
{
    guint i;

    for (i = 0; i < screen_num_desktops; ++i)
        g_list_free(focus_order[i]);
    g_free(focus_order);
    focus_order = NULL;

    XDestroyWindow(ob_display, focus_backup);

    /* reset focus to root */
    XSetInputFocus(ob_display, PointerRoot, RevertToPointerRoot,
                   event_lasttime);
}

static void push_to_top(Client *client)
{
    guint desktop;

    desktop = client->desktop;
    if (desktop == DESKTOP_ALL) desktop = screen_desktop;
    focus_order[desktop] = g_list_remove(focus_order[desktop], client);
    focus_order[desktop] = g_list_prepend(focus_order[desktop], client);
}

void focus_set_client(Client *client)
{
    Window active;
    Client *old;

    /* uninstall the old colormap, and install the new one */
    screen_install_colormap(focus_client, FALSE);
    screen_install_colormap(client, TRUE);

    if (client == NULL) {
	/* when nothing will be focused, send focus to the backup target */
	XSetInputFocus(ob_display, focus_backup, RevertToPointerRoot,
                       event_lasttime);
        XSync(ob_display, FALSE);
    }

    old = focus_client;
    focus_client = client;

    /* move to the top of the list */
    if (noreorder)
        --noreorder;
    else if (client != NULL)
        push_to_top(client);

    /* set the NET_ACTIVE_WINDOW hint */
    active = client ? client->window : None;
    PROP_SET32(ob_root, net_active_window, window, active);

    if (focus_client != NULL)
        dispatch_client(Event_Client_Focus, focus_client, 0, 0);
    if (old != NULL)
        dispatch_client(Event_Client_Unfocus, old, 0, 0);
}

static gboolean focus_under_pointer()
{
    int x, y;
    GList *it;

    if (ob_pointer_pos(&x, &y)) {
        for (it = stacking_list; it != NULL; it = it->next) {
            Client *c = it->data;
            if (c->desktop == screen_desktop &&
                RECT_CONTAINS(c->frame->area, x, y))
                break;
        }
        if (it != NULL)
            return client_normal(it->data) && client_focus(it->data);
    }
    return FALSE;
}

/* finds the first transient that isn't 'skip' and ensure's that client_normal
 is true for it */
static Client *find_transient_recursive(Client *c, Client *top, Client *skip)
{
    GSList *it;
    Client *ret;

    for (it = c->transients; it; it = it->next) {
        g_message("looking");
        if (it->data == top) return NULL;
        ret = find_transient_recursive(it->data, top, skip);
        if (ret && ret != skip && client_normal(ret)) return ret;
        if (it->data != skip && client_normal(it->data)) return it->data;
        g_message("not found");
    }
    return NULL;
}

static gboolean focus_fallback_transient(Client *top, Client *old)
{
    Client *target = find_transient_recursive(top, top, old);
    if (!target) {
        /* make sure client_normal is true always */
        if (!client_normal(top))
            return FALSE;
        target = top; /* no transient, keep the top */
    }
    return client_focus(target);
}

void focus_fallback(FallbackType type)
{
    GList *it;
    Client *old = NULL;

    old = focus_client;

    /* unfocus any focused clients.. they can be focused by Pointer events
       and such, and then when I try focus them, I won't get a FocusIn event
       at all for them.
    */
    focus_set_client(NULL);

    if (!(type == Fallback_Desktop ?
          config_focus_last_on_desktop : config_focus_last)) {
        if (config_focus_follow) focus_under_pointer();
        return;
    }

    if (type == Fallback_Unfocusing && old) {
        /* try for transient relations */
        if (old->transient_for) {
            if (old->transient_for == TRAN_GROUP) {
                for (it = focus_order[screen_desktop]; it; it = it->next) {
                    GSList *sit;

                    for (sit = old->group->members; sit; sit = sit->next)
                        if (sit->data == it->data)
                            if (focus_fallback_transient(sit->data, old))
                                return;
                }
            } else {
                if (focus_fallback_transient(old->transient_for, old))
                    return;
            }
        }

        /* try for group relations */
        if (old->group) {
            GSList *sit;

            for (it = focus_order[screen_desktop]; it != NULL; it = it->next)
                for (sit = old->group->members; sit; sit = sit->next)
                    if (sit->data == it->data)
                        if (sit->data != old && client_focus(sit->data))
                            return;
        }
    }

    for (it = focus_order[screen_desktop]; it != NULL; it = it->next)
        if (type != Fallback_Unfocusing || it->data != old)
            if (client_normal(it->data) && client_focus(it->data))
                return;

    /* nothing to focus */
    focus_set_client(NULL);
}

Client *focus_cycle(gboolean forward, gboolean linear, gboolean done,
                 gboolean cancel)
{
    static Client *first = NULL;
    static Client *t = NULL;
    static GList *order = NULL;
    GList *it, *start, *list;
    Client *ft;

    if (cancel) {
        if (first) client_focus(first);
        goto done_cycle;
    } else if (done) {
        if (focus_client) {
            push_to_top(focus_client); /* move to top of focus_order */
            stacking_raise(focus_client);
        }
        goto done_cycle;
    }
    if (!first) first = focus_client;

    if (linear) list = client_list;
    else        list = focus_order[screen_desktop];

    start = it = g_list_find(list, focus_client);
    if (!start) /* switched desktops or something? */
        start = it = forward ? g_list_last(list) : g_list_first(list);
    if (!start) goto done_cycle;

    do {
        if (forward) {
            it = it->next;
            if (it == NULL) it = list;
        } else {
            it = it->prev;
            if (it == NULL) it = g_list_last(list);
        }
        ft = client_focus_target(it->data);
        if (ft == it->data && focus_client != ft && client_normal(ft) &&
            client_focus(ft)) {
            noreorder++; /* avoid reordering the focus_order */
            return ft;
        }
    } while (it != start);
    return NULL;

done_cycle:
    t = NULL;
    first = NULL;
    g_list_free(order);
    order = NULL;
    return NULL;
}
