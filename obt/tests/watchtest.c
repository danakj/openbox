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

void func(ObtWatch *w, const gchar *base_path,
          const gchar *subpath, ObtWatchNotifyType type,
          gpointer data)
{
    g_print("base path: %s subpath: %s type=%d\n", base_path, subpath, type);
}

gint main()
{
    ObtWatch *watch;
    GMainLoop *loop;

    watch = obt_watch_new();
    obt_watch_add(watch, "/tmp/a", FALSE, func, NULL);

    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    return 0;
}
