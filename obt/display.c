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
#include "obt/util.h"

#ifdef HAVE_STRING_H
#  include <string.h>
#endif
#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

Display* obt_display_open(const char *display_name)
{
    gchar *n;
    Display *d = NULL;

    n = display_name ? g_strdup(display_name) : NULL;
    d = XOpenDisplay(n);
    if (d) {
        if (fcntl(ConnectionNumber(d), F_SETFD, 1) == -1)
            g_message("Failed to set display as close-on-exec");
    }
    g_free(n);

    return d;
}

void obt_display_close(Display *d)
{
    if (d) XCloseDisplay(d);
}
