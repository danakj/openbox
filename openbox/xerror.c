/* -*- indent-tabs-mode: t; tab-width: 4; c-basic-offset: 4; -*-

   xerror.c for the Openbox window manager
   Copyright (c) 2003        Ben Jansens

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   See the COPYING file for a copy of the GNU General Public License.
*/

#include "openbox.h"
#include <glib.h>
#include <X11/Xlib.h>

static gboolean xerror_ignore = FALSE;
gboolean xerror_occured = FALSE;

int xerror_handler(Display *d, XErrorEvent *e)
{
    xerror_occured = TRUE;
#ifdef DEBUG
    if (!xerror_ignore) {
	char errtxt[128];
	  
	/*if (e->error_code != BadWindow) */
	{
	    XGetErrorText(d, e->error_code, errtxt, 127);
	    if (e->error_code == BadWindow)
		/*g_warning("X Error: %s", errtxt)*/;
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
