#include "event.h"
#include "config.h"
#include "openbox.h"
#include "client.h"
#include "frame.h"
#include "screen.h"
#include "prop.h"
#include "dispatch.h"
#include "focus.h"

#include <X11/Xlib.h>
#include <glib.h>

Client *focus_client = NULL;
GList **focus_order = NULL; /* these lists are created when screen_startup
                               sets the number of desktops */

Window focus_backup = None;

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
    XMapRaised(ob_display, focus_backup);

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

void focus_set_client(Client *client)
{
    Window active;
    Client *old;
    guint desktop;

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
    if (client != NULL) {
        desktop = client->desktop;
        if (desktop == DESKTOP_ALL) desktop = screen_desktop;
        focus_order[desktop] = g_list_remove(focus_order[desktop], client);
        focus_order[desktop] = g_list_prepend(focus_order[desktop], client);
    }

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
    Window w;
    int i, x, y;
    guint u;
    GList *it;

    if (XQueryPointer(ob_display, ob_root, &w, &w, &x, &y, &i, &i, &u)) {
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

void focus_fallback(gboolean switching_desks)
{
    ConfigValue focus_follow;
    GList *it;
    gboolean under = FALSE;
    Client *old = NULL;

    old = focus_client;

    /* unfocus any focused clients.. they can be focused by Pointer events
       and such, and then when I try focus them, I won't get a FocusIn event
       at all for them.
    */
    focus_set_client(NULL);

    if (switching_desks) {
        /* don't skip any windows when switching desktops */
        old = NULL;
    } else {
        if (!config_get("focusFollowsMouse", Config_Bool, &focus_follow))
            g_assert_not_reached();
        if (focus_follow.bool)
            under = focus_under_pointer();
    }

    if (!under) {
        for (it = focus_order[screen_desktop]; it != NULL; it = it->next) {
            if (it->data != old && client_normal(it->data)) {
                /* if we're switching desktops, and we get the already focused
                   window, then we wont get a FocusIn for it, so just restore
                   the focus_client so that we know it is focused */
                if (client_focus(it->data))
                    break;
            }
        }
        if (it == NULL) /* nothing to focus */
            focus_set_client(NULL);
    }
}
