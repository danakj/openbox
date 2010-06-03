/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   obt/watch.h for the Openbox window manager
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

#ifndef __obt_watch_h
#define __obt_watch_h

#include <glib.h>

G_BEGIN_DECLS

typedef struct _ObtWatch ObtWatch;

struct _ObtMainLoop;

typedef void (*ObtWatchFunc)(ObtWatch *w, gchar *subpath, gpointer data);

ObtWatch* obt_watch_new();
void obt_watch_ref(ObtWatch *w);
void obt_watch_unref(ObtWatch *w);

void obt_watch_dir(ObtWatch *w, const gchar *path,
                   ObtWatchFunc func, gpointer data);

G_END_DECLS

#endif
