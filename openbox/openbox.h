/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   openbox.h for the Openbox window manager
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

#ifndef __openbox_h
#define __openbox_h

#include "misc.h"

#include "render/render.h"
#include "render/theme.h"

#include <glib.h>
#include <X11/Xlib.h>

struct _ObMainLoop;

extern RrInstance *ob_rr_inst;
extern RrTheme    *ob_rr_theme;

extern struct _ObMainLoop *ob_main_loop;

/*! The X display */
extern Display *ob_display; 

/*! The number of the screen on which we're running */
extern gint     ob_screen;

extern gchar   *ob_sm_id;
extern gboolean ob_sm_use;
extern gboolean ob_replace_wm;

/* The state of execution of the window manager */
ObState ob_state();

void ob_restart_other(const gchar *path);
void ob_restart();
void ob_exit(gint code);
void ob_exit_replace();

void ob_reconfigure();

void ob_exit_with_error(gchar *msg);

Cursor ob_cursor(ObCursor cursor);

KeyCode ob_keycode(ObKey key);

#endif
