/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   obt/watch_manual.c for the Openbox window manager
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

/*! This will just do nothing if a better mechanism isn't present, requiring
  a manual refresh (via obt_watch_refresh()) to see updates in the filesystem.
*/

#ifndef HAVE_SYS_INOTIFY_H

#include "watch.h"
#include "watch_interface.h"

#include <glib.h>

typedef struct _ManualSource ManualSource;
typedef struct _ManualFile ManualFile;

static gboolean source_check(GSource *source);
static gboolean source_prepare(GSource *source, gint *timeout);
static gboolean source_read(GSource *source, GSourceFunc cb, gpointer data);
static void source_finalize(GSource *source);

static GSourceFuncs source_funcs = {
    source_prepare,
    source_check,
    source_read,
    source_finalize
};

GSource* watch_sys_create_source()
{
    return g_source_new(&source_funcs, sizeof(GSource));
}

gpointer watch_sys_add_target(ObtWatchTarget *target)
{
    return NULL;
}

void watch_sys_remove_target(ObtWatchTarget *target, gpointer data)
{
}

gpointer watch_sys_add_file(GSource *source,
                            ObtWatchTarget *target,
                            ObtWatchFile *file,
                            gboolean is_dir)
{
    return NULL;
}

void watch_sys_remove_file(GSource *source,
                           ObtWatchTarget *target,
                           ObtWatchFile *file,
                           gpointer data)
{
}

static gboolean source_prepare(GSource *source, gint *timeout)
{
    *timeout = -1;
    return FALSE;
}

static gboolean source_check(GSource *source)
{
    return FALSE;
}

static gboolean source_read(GSource *source, GSourceFunc cb, gpointer data)
{
    return TRUE;
}

static void source_finalize(GSource *source)
{
}

#endif
