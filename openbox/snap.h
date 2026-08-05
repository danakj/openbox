/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   snap.h for the Openbox window manager

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

#ifndef __snap_h
#define __snap_h

#include <X11/Xlib.h>
#include <glib.h>

struct _ObClient;

void snap_startup(gboolean reconfig);
void snap_shutdown(gboolean reconfig);

/*! The pointer has entered the maximize button of a client's titlebar. After
  the configured delay the layout popup will be shown for that client. */
void snap_hover_begin(struct _ObClient *client);
/*! The pointer has left the maximize button. The popup is hidden shortly
  after, unless the pointer moves into the popup itself. */
void snap_hover_end(void);

/*! Show the popup for a client right now, without any delay. */
void snap_show(struct _ObClient *client);
/*! Hide the popup right now, if it is visible or about to be shown. */
void snap_hide(void);

gboolean snap_visible(void);

/*! Is this one of the popup's windows? */
gboolean snap_is_popup_window(Window win);

/*! Handle an event which occured on one of the popup's windows.
  @return TRUE if the event was used. */
gboolean snap_event(XEvent *e);

#endif
