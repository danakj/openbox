/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   obt/mainloop.h for the Openbox window manager
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

#ifndef __obt_mainloop_h
#define __obt_mainloop_h

#include <X11/Xlib.h>
#include <glib.h>

G_BEGIN_DECLS

typedef struct _ObtMainLoop ObtMainLoop;

ObtMainLoop *obt_main_loop_new(void);
void        obt_main_loop_ref(ObtMainLoop *loop);
void        obt_main_loop_unref(ObtMainLoop *loop);

typedef void (*ObtMainLoopXHandler) (const XEvent *e, gpointer data);

void obt_main_loop_x_add(ObtMainLoop *loop,
                         ObtMainLoopXHandler handler,
                         gpointer data,
                         GDestroyNotify notify);
void obt_main_loop_x_remove(ObtMainLoop *loop,
                            ObtMainLoopXHandler handler);

typedef void (*ObtMainLoopFdHandler) (gint fd, gpointer data);

void obt_main_loop_fd_add(ObtMainLoop *loop,
                          gint fd,
                          ObtMainLoopFdHandler handler,
                          gpointer data,
                          GDestroyNotify notify);
void obt_main_loop_fd_remove(ObtMainLoop *loop,
                             gint fd);

typedef void (*ObtMainLoopSignalHandler) (gint signal, gpointer data);

void obt_main_loop_signal_add(ObtMainLoop *loop,
                              gint signal,
                              ObtMainLoopSignalHandler handler,
                              gpointer data,
                              GDestroyNotify notify);
void obt_main_loop_signal_remove(ObtMainLoop *loop,
                                 ObtMainLoopSignalHandler handler);

void obt_main_loop_timeout_add(ObtMainLoop *loop,
                               gulong microseconds,
                               GSourceFunc handler,
                               gpointer data,
                               GEqualFunc cmp,
                               GDestroyNotify notify);
void obt_main_loop_timeout_remove(ObtMainLoop *loop,
                                  GSourceFunc handler);
void obt_main_loop_timeout_remove_data(ObtMainLoop *loop,
                                       GSourceFunc handler,
                                       gpointer data,
                                       gboolean cancel_dest);

void obt_main_loop_run(ObtMainLoop *loop);
void obt_main_loop_exit(ObtMainLoop *loop);

G_END_DECLS

#endif
