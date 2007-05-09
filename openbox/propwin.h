/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   propwin.h for the Openbox window manager
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

#ifndef __propwin_h
#define __propwin_h

#include <glib.h>
#include <X11/Xlib.h>

struct _ObClient;

typedef enum {
    OB_PROPWIN_USER_TIME,
    OB_NUM_PROPWIN_TYPES
} ObPropWinType;

void propwin_startup(gboolean reconfig);
void propwin_shutdown(gboolean reconfig);

void propwin_add(Window win, ObPropWinType type, struct _ObClient *client);
void propwin_remove(Window win, ObPropWinType type, struct _ObClient *client);

GSList* propwin_get_clients(Window win, ObPropWinType type);

#endif
