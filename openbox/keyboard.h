/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   keyboard.h for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
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

#ifndef ob__keybaord_h
#define ob__keybaord_h

#include "keytree.h"
#include "frame.h"

#include <glib.h>
#include <X11/Xlib.h>

struct _ObClient;
struct _ObAction;

extern KeyBindingTree *keyboard_firstnode;

void keyboard_startup(gboolean reconfig);
void keyboard_shutdown(gboolean reconfig);

gboolean keyboard_bind(GList *keylist, ObAction *action);
void keyboard_unbind_all();

void keyboard_event(struct _ObClient *client, const XEvent *e);
void keyboard_reset_chains();

gboolean keyboard_interactive_grab(guint state, struct _ObClient *client,
                                   struct _ObAction *action);
gboolean keyboard_process_interactive_grab(const XEvent *e,
                                           struct _ObClient **client);
gboolean keyboard_interactively_grabbed();

#endif
