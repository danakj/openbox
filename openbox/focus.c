#include "event.h"
#include "openbox.h"
#include "client.h"
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
				 -100, -100, 1, 1, 0, 0, InputOnly,
				 CopyFromParent, CWOverrideRedirect, &attrib);
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
    XSetInputFocus(ob_display, PointerRoot, RevertToNone, CurrentTime);
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
	XSetInputFocus(ob_display, focus_backup, RevertToNone, event_lasttime);
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
