/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   event.h for the Openbox window manager
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

#ifndef __events_h
#define __events_h

#include <X11/Xlib.h>
#include <glib.h>

struct _ObClient;

/*! Time at which the last event with a timestamp occured. */
extern Time event_curtime;

/*! The value of the mask for the NumLock modifier */
extern guint NumLockMask;
/*! The value of the mask for the ScrollLock modifier */
extern guint ScrollLockMask;

void event_startup(gboolean reconfig);
void event_shutdown(gboolean reconfig);

/*! Make as if the mouse just entered the client, use only when using focus
  follows mouse */
void event_enter_client(struct _ObClient *client);

/*! Make mouse focus not move if the mouse leaves this client from what
  has just transpired. */
void event_ignore_enters_leaving_window(struct _ObClient *c);

/*! Make mouse focus not move at all from the stuff that has happened up
  till now. */
void event_ignore_all_queued_enters();

/* Halts any focus delay in progress, use this when the user is selecting a
   window for focus */
void event_halt_focus_delay();

/*! Compare t1 and t2, taking into account wraparound. True if t1
  comes at the same time or later than t2. */
gboolean event_time_after(Time t1, Time t2);

#endif
