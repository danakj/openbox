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
typedef enum _ObtWatchNotifyType ObtWatchNotifyType;

/*! Notification function for changes in a watch file/directory.
  @param base_path is the path to the watch target (file or directory).
  @param sub_path is a path relative to the watched directory.  If the
    notification is about the watch target itself, the subpath will be
    an empty string.
*/
typedef void (*ObtWatchFunc)(ObtWatch *w, const gchar *base_path,
                             const gchar *sub_path, const gchar *full_path,
                             ObtWatchNotifyType type, gpointer data);

enum _ObtWatchNotifyType {
    OBT_WATCH_ADDED, /*!< A file/dir was added in a watched dir */
    OBT_WATCH_REMOVED, /*!< A file/dir was removed in a watched dir */
    OBT_WATCH_MODIFIED, /*!< A watched file, or a file in a watched dir, was
                             modified */
    OBT_WATCH_SELF_REMOVED /*!< The watched target was removed. */ 
};

ObtWatch* obt_watch_new();
void obt_watch_ref(ObtWatch *w);
void obt_watch_unref(ObtWatch *w);

/*! Start watching a target file or directory.
  If the target is a directory, the watch is performed recursively.
  On start, if the target is a directory, an ADDED notification will come for
  each file in the directory, and its subdirectories.
  @param path The path to the target to watch.  Must be an absolute path that
    starts with a /.
  @param watch_hidden If TRUE, and if the target is a directory, dot-files (and
    dot-subdirectories) will be included in the watch.  If the target is a
    file, this parameter is ignored.
*/
gboolean obt_watch_add(ObtWatch *w, const gchar *path,
                       gboolean watch_hidden,
                       ObtWatchFunc func, gpointer data);
void obt_watch_remove(ObtWatch *w, const gchar *path);

G_END_DECLS

#endif
