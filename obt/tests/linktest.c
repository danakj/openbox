#/*
#!/bin/sh
#*/
#if 0
gcc -O0 -o ./linktest `pkg-config --cflags --libs obt-3.5` linktest.c && \
./linktest
exit
#endif

/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   linktest.c for the Openbox window manager
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

#include "obt/linkbase.h"
#include "obt/paths.h"
#include <glib.h>
#include <locale.h>

gint main()
{
    ObtLinkBase *base;
    ObtPaths *paths;
    GMainLoop *loop;

    paths = obt_paths_new();
    base = obt_linkbase_new(paths, setlocale(LC_MESSAGES, ""));
    printf("done\n");
    return 0;

    loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    return 0;
}
