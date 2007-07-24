/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   obt/display.c for the Openbox window manager
   Copyright (c) 2007        Dana Jansens

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

#include "obt/display.h"

#ifdef HAVE_STRING_H
#  include <string.h>
#endif
#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

static gint xerror_handler(Display *d, XErrorEvent *e);

static gboolean xerror_ignore = FALSE;
static gboolean xerror_occured = FALSE;

Display* obt_display_open(const char *display_name)
{
    gchar *n;
    Display *d = NULL;

    n = display_name ? g_strdup(display_name) : NULL;
    d = XOpenDisplay(n);
    if (d) {
        if (fcntl(ConnectionNumber(d), F_SETFD, 1) == -1)
            g_message("Failed to set display as close-on-exec");
        XSetErrorHandler(xerror_handler);
    }
    g_free(n);

    return d;
}

void obt_display_close(Display *d)
{
    if (d) XCloseDisplay(d);
}

static gint xerror_handler(Display *d, XErrorEvent *e)
{
#ifdef DEBUG
    gchar errtxt[128];

    XGetErrorText(d, e->error_code, errtxt, 127);
    if (!xerror_ignore) {
        if (e->error_code == BadWindow)
            /*g_message(_("X Error: %s\n"), errtxt)*/;
        else
            g_error("X Error: %s", errtxt);
    } else
        g_message("XError code %d '%s'", e->error_code, errtxt);
#else
    (void)d; (void)e;
#endif

    xerror_occured = TRUE;
    return 0;
}

void obt_display_ignore_errors(Display *d, gboolean ignore)
{
    XSync(d, FALSE);
    xerror_ignore = ignore;
    if (ignore) xerror_occured = FALSE;
}

gboolean obt_display_error_occured()
{
    return xerror_occured;
}
