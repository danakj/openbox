/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   debug.c for the Openbox window manager
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

#include <glib.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

static gboolean show;

void ob_debug_show_output(gboolean enable)
{
    show = enable;
}

void ob_debug(const gchar *a, ...)
{
    va_list vl;

    if (show) {
        va_start(vl, a);
        vfprintf(stderr, a, vl);
        va_end(vl);
    }
}
