/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   obt/xevent.h for the Openbox window manager
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

#ifndef __obt_xevent_h
#define __obt_xevent_h

#include <X11/Xlib.h>
#include <glib.h>

G_BEGIN_DECLS

struct _ObtMainLoop;

typedef struct _ObtXEventHandler ObtXEventHandler;

typedef void (*ObtXEventCallback) (const XEvent *e, gpointer data);

ObtXEventHandler* xevent_new();
void              xevent_ref(ObtXEventHandler *h);
void              xevent_unref(ObtXEventHandler *h);

void              xevent_register(ObtXEventHandler *h,
                                  struct _ObtMainLoop *loop);


void xevent_set_handler(ObtXEventHandler *h, gint type, Window win,
                        ObtXEventCallback func, gpointer data);
void xevent_remove_handler(ObtXEventHandler *h, gint type, Window win);

G_END_DECLS

#endif /*__obt_xevent_h*/
