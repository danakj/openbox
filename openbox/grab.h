/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   grab.h for the Openbox window manager
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

#ifndef __grab_h
#define __grab_h

#include "misc.h"

#include <glib.h>
#include <X11/Xlib.h>

void grab_startup(gboolean reconfig);
void grab_shutdown(gboolean reconfig);

gboolean grab_keyboard(gboolean grab);
gboolean grab_pointer(gboolean grab, ObCursor cur);
gboolean grab_pointer_window(gboolean grab, ObCursor cur, Window win);
gint grab_server(gboolean grab);

gboolean grab_on_keyboard();
gboolean grab_on_pointer();

void grab_button(guint button, guint state, Window win, guint mask);
void grab_button_full(guint button, guint state, Window win, guint mask,
                      gint pointer_mode, ObCursor cursor);
void ungrab_button(guint button, guint state, Window win);

void grab_key(guint keycode, guint state, Window win, gint keyboard_mode);

void ungrab_all_keys(Window win);

#endif
