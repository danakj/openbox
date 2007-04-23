/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   moveresize.h for the Openbox window manager
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

#ifndef __moveresize_h
#define __moveresize_h

#include <glib.h>

struct _ObClient;

extern gboolean moveresize_in_progress;
extern struct _ObClient *moveresize_client;

void moveresize_startup(gboolean reconfig);
void moveresize_shutdown(gboolean reconfig);

void moveresize_start(struct _ObClient *c,
                      gint x, gint y, guint button, guint32 corner);
void moveresize_end(gboolean cancel);

void moveresize_event(XEvent *e);

#endif
