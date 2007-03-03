/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   session.h for the Openbox window manager
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

#ifndef __ob__session_h
#define __ob__session_h

#include <glib.h>

struct _ObClient;

typedef struct _ObSessionState ObSessionState;

struct _ObSessionState {
    gchar *id, *name, *class, *role;
    guint stacking;
    guint desktop;
    gint x, y, w, h;
    gboolean shaded, iconic, skip_pager, skip_taskbar, fullscreen;
    gboolean above, below, max_horz, max_vert;

    gboolean matched;
};

extern GList *session_saved_state;

void session_startup(gint argc, gchar **argv);
void session_shutdown();

GList* session_state_find(struct _ObClient *c);
gboolean session_state_cmp(ObSessionState *s, struct _ObClient *c);
void session_state_free(ObSessionState *state);

#endif
