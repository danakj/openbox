#include "openbox.h"
#include "client.h"
#include "screen.h"
#include "prop.h"
#include "dispatch.h"

#include <X11/Xlib.h>

Client *focus_client = NULL;

Window focus_backup = None;

void focus_set_client(Client *client);

void focus_startup()
{
    /* create the window which gets focus when no clients get it. Have to
       make it override-redirect so we don't try manage it, since it is
       mapped. */
    XSetWindowAttributes attrib;

    attrib.override_redirect = TRUE;
    focus_backup = XCreateWindow(ob_display, ob_root,
				 -100, -100, 1, 1, 0, 0, InputOnly,
				 CopyFromParent, CWOverrideRedirect, &attrib);
    XMapRaised(ob_display, focus_backup);

    /* start with nothing focused */
    focus_set_client(NULL);
}

void focus_set_client(Client *client)
{
    Window active;
     
    /* sometimes this is called with the already-focused window, this is
       important for the python scripts to work (eg, c = 0 twice). don't just
       return if _focused_client == c */

    /* uninstall the old colormap, and install the new one */
    screen_install_colormap(focus_client, FALSE);
    screen_install_colormap(client, TRUE);


    if (client == NULL) {
	/* when nothing will be focused, send focus to the backup target */
	XSetInputFocus(ob_display, focus_backup, RevertToNone, CurrentTime);
    }

    focus_client = client;

    /* set the NET_ACTIVE_WINDOW hint */
    active = client ? client->window : None;
    PROP_SET32(ob_root, net_active_window, window, active);

    if (focus_client != NULL) {
        dispatch_client(Event_Client_Focus, focus_client, 0, 0);
        dispatch_client(Event_Client_Unfocus, focus_client, 0, 0);
    }
}
