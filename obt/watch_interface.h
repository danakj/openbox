/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   obt/watch_interface.h for the Openbox window manager
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

#ifndef __obt_watch_interface_h
#define __obt_watch_interface_h

#include <glib.h>

G_BEGIN_DECLS

typedef struct _ObtWatchTarget ObtWatchTarget;
typedef struct _ObtWatchFile   ObtWatchFile;

/*! Initializes the watch subsystem, and returns a GSource for it.
  @param notify The GSource will call @notify when a watched file is changed.
  @return Returns a GSource* on success, and a NULL if an error occurred.
*/
GSource* watch_sys_create_source(void);

/*! Informs the watch system about a new file/dir.
  It should return a structure that it wants to store inside the file.
*/
gpointer watch_sys_add_file(GSource *source,
                            ObtWatchTarget *target,
                            ObtWatchFile *file,
                            gboolean is_dir);

/*! Informs the watch system about the destruction of a file.
  It can free the structure returned from watch_sys_add_file as this is given
  in @data.
*/
void watch_sys_remove_file(GSource *source,
                           ObtWatchTarget *target,
                           ObtWatchFile *file,
                           gpointer data);

/* These are in watch.c, they are part of the main watch system */
void watch_main_notify_add(ObtWatchTarget *target,
                           ObtWatchFile *parent,
                           const gchar *name);
void watch_main_notify_remove(ObtWatchTarget *target,
                              ObtWatchFile *file);
void watch_main_notify_modify(ObtWatchTarget *target,
                              ObtWatchFile *file);

gboolean watch_main_target_watch_hidden(ObtWatchTarget *target);
ObtWatchFile* watch_main_target_root(ObtWatchTarget *target);
gchar* watch_main_target_file_full_path(ObtWatchTarget *target,
                                        ObtWatchFile *file);

gchar* watch_main_file_sub_path(ObtWatchFile *file);
gboolean watch_main_file_is_dir(ObtWatchFile *file);
ObtWatchFile* watch_main_file_child(ObtWatchFile *file,
                                    const gchar *name);
GList* watch_main_file_children(ObtWatchFile *file);

G_END_DECLS

#endif
