/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   unmanaged.h for the Openbox window manager
   Copyright (c) 2010        Dana Jansens

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

#ifndef __unmanaged_h
#define __unmanaged_h

#include <X11/Xlib.h>
#include <glib.h>

typedef struct _ObUnmanaged ObUnmanaged;

/*! Create a new structure to watch an unmanaged window */
ObUnmanaged* unmanaged_new(Window w);
void unmanaged_destroy(ObUnmanaged *self);

void unmanaged_destroy_all(void);

#endif
