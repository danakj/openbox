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
/*! The atom for the selection we hold on the screen to claim ourselves as the
  current composite manager */
extern Atom composite_cm_atom;

void composite_startup(gboolean reconfig);
void composite_shutdown(gboolean reconfig);

/*! Turn composite on */
gboolean composite_enable(void);
/*! Turn composite off */
void composite_disable(void);

/*! Flag need for a redraw */
void composite_dirty(void);

/*! Called when the screen changes its size */
void composite_resize(void);

void composite_window_setup(struct _ObWindow *w);
void composite_window_cleanup(struct _ObWindow *w);

/*! Called when a window's pixmaps become invalid and need to be destroyed */
void composite_window_invalid(struct _ObWindow *w);

/*! Called when the root pixmap changes */
void composite_root_invalid(void);

#endif
