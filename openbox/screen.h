/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   screen.h for the Openbox window manager
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

#ifndef __screen_h
#define __screen_h

#include "misc.h"
#include "geom.h"

struct _ObClient;

#define DESKTOP_ALL (0xffffffff)

/*! The number of available desktops */
extern guint screen_num_desktops;
/*! The number of virtual "xinerama" screens/heads */
extern guint screen_num_monitors;
/*! The current desktop */
extern guint screen_desktop;
/*! The desktop which was last visible */
extern guint screen_last_desktop;
/*! Are we in showing-desktop mode? */
extern gboolean screen_showing_desktop;
/*! The support window also used for focus and stacking */
extern Window screen_support_win;
/*! The last time at which the user changed desktops */
extern Time screen_desktop_user_time;

typedef struct DesktopLayout {
    ObOrientation orientation;
    ObCorner start_corner;
    guint rows;
    guint columns;
} DesktopLayout;
extern DesktopLayout screen_desktop_layout;

/*! An array of gchar*'s which are desktop names in UTF-8 format */
extern gchar **screen_desktop_names;

/*! Take over the screen, set the basic hints on it claming it as ours */
gboolean screen_annex();

/*! Once the screen is ours, set up its initial state */
void screen_startup(gboolean reconfig);
/*! Free resources */
void screen_shutdown(gboolean reconfig);

/*! Figure out the new size of the screen and adjust stuff for it */
void screen_resize();

/*! Change the number of available desktops */
void screen_set_num_desktops(guint num);
/*! Change the current desktop */
void screen_set_desktop(guint num, gboolean dofocus);
/*! Interactively change desktops */
guint screen_cycle_desktop(ObDirection dir, gboolean wrap, gboolean linear,
                           gboolean dialog, gboolean done, gboolean cancel);

/*! Show/hide the desktop popup (pager) for the given desktop */
void screen_desktop_popup(guint d, gboolean show);

/*! Shows and focuses the desktop and hides all the client windows, or
  returns to the normal state, showing client windows. */
void screen_show_desktop(gboolean show, gboolean restore_focus);

/*! Updates the desktop layout from the root property if available */
void screen_update_layout();

/*! Get desktop names from the root window property */
void screen_update_desktop_names();

/*! Installs or uninstalls a colormap for a client. If client is NULL, then
  it handles the root colormap. */
void screen_install_colormap(struct _ObClient *client, gboolean install);

void screen_update_areas();

Rect *screen_physical_area();

Rect *screen_physical_area_monitor(guint head);

Rect *screen_area(guint desktop);

Rect *screen_area_monitor(guint desktop, guint head);

/*! Determines which physical monitor a rectangle is on by calculating the
    area of the part of the rectable on each monitor.  The number of the
    monitor containing the greatest area of the rectangle is returned.*/
guint screen_find_monitor(Rect *search);

/*! Sets the root cursor. This function decides which cursor to use, but you
  gotta call it to let it know it should change. */
void screen_set_root_cursor();

gboolean screen_pointer_pos(gint *x, gint *y);

#endif
