#/*
#!/bin/sh
#*/
#if 0
gcc -O0 -o ./watchtest `pkg-config --cflags --libs obt-3.5` watchtest.c && \
./watchtest
exit
#endif

/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   watchtest.c for the Openbox window manager
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

#include "obt/watch.h"
#include <glib.h>

static GMainLoop *loop;

void func(ObtWatch *w, const gchar *base_path, const gchar *subpath,
          ObtWatchNotifyType type, gpointer data)
{
    g_print("base path: %s subpath: %s type=%d\n", base_path, subpath, type);
}

gboolean force_refresh(gpointer data)
{
    ObtWatch *w = data;
    obt_watch_refresh(w);
    return TRUE;
}

static void sighandler(gint signal)
{
    g_main_loop_quit(loop);
}

gint main()
{
    ObtWatch *watch;
    struct sigaction action, oldaction;
    sigset_t sigset;

    loop = g_main_loop_new(NULL, FALSE);
    g_return_val_if_fail(loop != NULL, 1);

    watch = obt_watch_new();
    obt_watch_add(watch, "/tmp/a", FALSE, func, NULL);


    sigemptyset(&sigset);
    action.sa_handler = sighandler;
    action.sa_mask = sigset;
    action.sa_flags = SA_NOCLDSTOP;
    sigaction(SIGINT, &action, &oldaction);

    g_timeout_add(5000, force_refresh, watch);

    g_main_loop_run(loop);

    g_print("bye\n");

    obt_watch_unref(watch);

    return 0;
}
