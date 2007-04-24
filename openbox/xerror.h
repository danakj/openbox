/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   xerror.h for the Openbox window manager
   Copyright (c) 2003-2007   Dana Jansens

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

#ifndef __xerror_h
#define __xerror_h

#include <X11/Xlib.h>
#include <glib.h>

/* can be used to track errors */
extern gboolean xerror_occured;

gint xerror_handler(Display *, XErrorEvent *);

void xerror_set_ignore(gboolean ignore);

#endif
