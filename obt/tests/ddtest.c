#/*
#!/bin/sh
#*/
#if 0
gcc -O0 -o ./ddtest `pkg-config --cflags --libs obt-3.5` ddtest.c && \
./ddtest
exit
#endif

/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   ddtest.c for the Openbox window manager
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "obt/paths.h"
#include "obt/link.h"
#include <glib.h>

gint main(int argc, char **argv)
{
    ObtPaths *obtpaths;
    ObtLink *dd;
    gchar *id;

    if (argc < 2) {
        g_print("pass path to .desktop\n");
        return 1;
    }

    obtpaths = obt_paths_new();
    dd = obt_link_from_ddfile(argv[1], obtpaths, "et", NULL, NULL);
    obt_paths_unref(obtpaths);
    if (dd) {
        g_print("Success\n");
        {
            gulong i, n;
            const GQuark *c = obt_link_app_categories(dd, &n);
            for (i = 0; i < n; ++i)
                g_print("Category: %s\n",
                        g_quark_to_string(c[i]));
        }
        obt_link_unref(dd);
    }
    return 0;
}
