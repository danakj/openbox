/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   obt/instance.h for the Openbox window manager
   Copyright (c) 2007        Dana Jansens

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

#ifndef __obt_instance_h
#define __obt_instance_h

#include <X11/Xlib.h>
#include <glib.h>

G_BEGIN_DECLS

typedef struct _ObtInstance ObtInstance;

/* Instance funcs */
ObtInstance* obt_instance_new   (const char *display_name);
void         obt_instance_ref   (ObtInstance *inst);
void         obt_instance_unref (ObtInstance *inst);

Display*     obt_display        (const ObtInstance *inst);

G_END_DECLS

#endif /*__obt_instance_h*/
