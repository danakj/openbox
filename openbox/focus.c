#include "event.h"
#include "openbox.h"
#include "framerender.h"
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
#include "popup.h"

#include <X11/Xlib.h>
#include <glib.h>

Client *focus_client = NULL;
GList **focus_order = NULL; /* these lists are created when screen_startup
                               sets the number of desktops */

Window focus_backup = None;

static Client *focus_cycle_target = NULL;
static Popup *focus_cycle_popup = NULL;

void focus_startup()
{
    /* create the window which gets focus when no clients get it. Have to
       make it override-redirect so we don't try manage it, since it is
       mapped. */
    XSetWindowAttributes attrib;

    focus_client = NULL;
    focus_cycle_popup = popup_new(TRUE);

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

    popup_free(focus_cycle_popup);
    focus_cycle_popup = NULL;

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

    /* in the middle of cycling..? kill it. */
    if (focus_cycle_target)
        focus_cycle(TRUE, TRUE, TRUE, TRUE);

    old = focus_client;
    focus_client = client;

    /* move to the top of the list */
    if (client != NULL)
        push_to_top(client);

    /* set the NET_ACTIVE_WINDOW hint, but preserve it on shutdown */
    if (ob_state != State_Exiting) {
        active = client ? client->window : None;
        PROP_SET32(ob_root, net_active_window, window, active);
    }

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
        if (it->data == top) return NULL;
        ret = find_transient_recursive(it->data, top, skip);
        if (ret && ret != skip && client_normal(ret)) return ret;
        if (it->data != skip && client_normal(it->data)) return it->data;
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
                        if (sit->data != old && client_normal(sit->data))
                            if (client_focus(sit->data))
                                return;
        }
    }

    for (it = focus_order[screen_desktop]; it != NULL; it = it->next)
        if (type != Fallback_Unfocusing || it->data != old)
            if (client_normal(it->data) &&
                /* dont fall back to 'anonymous' fullscreen windows. theres no
                   checks for this is in transient/group fallbacks. */
                !((Client*)it->data)->fullscreen &&
                client_focus(it->data))
                return;

    /* nothing to focus */
    focus_set_client(NULL);
}

static void popup_cycle(Client *c, gboolean show)
{
    if (!show) {
        popup_hide(focus_cycle_popup);
    } else {
        Rect *a;

        a = screen_area(c->desktop);
        popup_position(focus_cycle_popup, CenterGravity,
                       a->x + a->width / 2, a->y + a->height / 2);
/*        popup_size(focus_cycle_popup, a->height/2, a->height/16);
        popup_show(focus_cycle_popup, c->title,
                   client_icon(c, a->height/16, a->height/16));
*/
        /* XXX the size and the font extents need to be related on some level
         */
        popup_size(focus_cycle_popup, 320, 48);

        /* use the transient's parent's title/icon */
        while (c->transient_for && c->transient_for != TRAN_GROUP)
            c = c->transient_for;

        popup_show(focus_cycle_popup, (c->iconic ? c->icon_title : c->title),
                   client_icon(c, 48, 48));
    }
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
        /*if (first) client_focus(first); XXX*/
        if (focus_cycle_target)
            frame_adjust_focus(focus_cycle_target->frame, FALSE);
        if (focus_client)
            frame_adjust_focus(focus_client->frame, TRUE);
        goto done_cycle;
    } else if (done) {
        if (focus_cycle_target) {
            if (focus_cycle_target->iconic)
                client_iconify(focus_cycle_target, FALSE, FALSE);
            client_focus(focus_cycle_target);
            stacking_raise(focus_cycle_target);
        }
        goto done_cycle;
    }
    if (!first) first = focus_client;
    if (!focus_cycle_target) focus_cycle_target = focus_client;

    if (linear) list = client_list;
    else        list = focus_order[screen_desktop];

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
        /*ft = client_focus_target(it->data);*/
        ft = it->data;
        if (ft->transients == NULL && /*ft == it->data &&*/client_normal(ft) &&
            (ft->can_focus || ft->focus_notify) &&
            (ft->desktop == screen_desktop || ft->desktop == DESKTOP_ALL)) {
            if (ft != focus_cycle_target) { /* prevents flicker */
                if (focus_cycle_target)
                    frame_adjust_focus(focus_cycle_target->frame, FALSE);
                focus_cycle_target = ft;
                frame_adjust_focus(focus_cycle_target->frame, TRUE);
            }
            popup_cycle(ft, config_focus_popup);
            return ft;
        }
    } while (it != start);

done_cycle:
    t = NULL;
    first = NULL;
    focus_cycle_target = NULL;
    g_list_free(order);
    order = NULL;
    popup_cycle(ft, FALSE);
    return NULL;
}

void focus_order_add_new(Client *c)
{
    guint d, i;

    if (c->iconic)
        focus_order_to_top(c);
    else {
        d = c->desktop;
        if (d == DESKTOP_ALL) {
            for (i = 0; i < screen_num_desktops; ++i) {
                if (focus_order[i] && ((Client*)focus_order[i]->data)->iconic)
                    focus_order[i] = g_list_insert(focus_order[i], c, 0);
                else
                    focus_order[i] = g_list_insert(focus_order[i], c, 1);
            }
        } else
             if (focus_order[d] && ((Client*)focus_order[d]->data)->iconic)
                focus_order[d] = g_list_insert(focus_order[d], c, 0);
            else
                focus_order[d] = g_list_insert(focus_order[d], c, 1);
    }
}

void focus_order_remove(Client *c)
{
    guint d, i;

    d = c->desktop;
    if (d == DESKTOP_ALL) {
        for (i = 0; i < screen_num_desktops; ++i)
            focus_order[i] = g_list_remove(focus_order[i], c);
    } else
        focus_order[d] = g_list_remove(focus_order[d], c);
}

static void to_top(Client *c, guint d)
{
    focus_order[d] = g_list_remove(focus_order[d], c);
    if (!c->iconic) {
        focus_order[d] = g_list_prepend(focus_order[d], c);
    } else {
        GList *it;

        /* insert before first iconic window */
        for (it = focus_order[d];
             it && !((Client*)it->data)->iconic; it = it->next);
        g_list_insert_before(focus_order[d], it, c);
    }
}

void focus_order_to_top(Client *c)
{
    guint d, i;

    d = c->desktop;
    if (d == DESKTOP_ALL) {
        for (i = 0; i < screen_num_desktops; ++i)
            to_top(c, i);
    } else
        to_top(c, d);
}

static void to_bottom(Client *c, guint d)
{
    focus_order[d] = g_list_remove(focus_order[d], c);
    if (c->iconic) {
        focus_order[d] = g_list_append(focus_order[d], c);
    } else {
        GList *it;

        /* insert before first iconic window */
        for (it = focus_order[d];
             it && !((Client*)it->data)->iconic; it = it->next);
        g_list_insert_before(focus_order[d], it, c);
    }
}

void focus_order_to_bottom(Client *c)
{
    guint d, i;

    d = c->desktop;
    if (d == DESKTOP_ALL) {
        for (i = 0; i < screen_num_desktops; ++i)
            to_bottom(c, i);
    } else
        to_bottom(c, d);
}
