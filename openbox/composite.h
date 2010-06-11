/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   composite.h for the Openbox window manager
   Copyright (c) 2010        Dana Jansens
   Copyright (c) 2010        Derek Foreman

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

#ifndef __composite_h
#define __composite_h

#include <X11/Xlib.h>
#include <glib.h>

struct _ObWindow;

extern Window composite_overlay;

void composite_startup(gboolean reconfig);
void composite_shutdown(gboolean reconfig);

/*! Draw everything */
void composite(void);

/*! Turn composite redirection on or off for a window */
void composite_redir(struct _ObWindow *w, gboolean on);
/*! Called when the screen changes its size */
void composite_resize(void);

#endif
