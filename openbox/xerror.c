#include "openbox.h"
#include <glib.h>
#include <X11/Xlib.h>

static gboolean xerror_ignore = FALSE;

int xerror_handler(Display *d, XErrorEvent *e)
{
#ifdef DEBUG
    if (!xerror_ignore) {
	char errtxt[128];
	  
	/*if (e->error_code != BadWindow) */
	{
	    XGetErrorText(d, e->error_code, errtxt, 127);
	    if (e->error_code == BadWindow)
		g_warning("X Error: %s", errtxt);
	    else
		g_error("X Error: %s", errtxt);
	}
    }
#else
    (void)d; (void)e;
#endif
    return 0;
}

void xerror_set_ignore(gboolean ignore)
{
    XSync(ob_display, FALSE);
    xerror_ignore = ignore;
}
