/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   resist.h for the Openbox window manager
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

#ifndef ob__resist_h
#define ob__resist_h

struct _ObClient;

#include <glib.h>

void resist_move_windows(struct _ObClient *c, gint *x, gint *y);
void resist_move_monitors(struct _ObClient *c, gint *x, gint *y);
void resist_size_windows(struct _ObClient *c, gint *w, gint *h, ObCorner corn);
void resist_size_monitors(struct _ObClient *c, gint *w, gint *h,ObCorner corn);

#endif
