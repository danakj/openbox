#/*
#!/bin/sh
#*/
#if 0
gcc -O0 -o ./bstest `pkg-config --cflags --libs obt-3.5` bstest.c && \
./bstest
exit
#endif

/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   bstest.c for the Openbox window manager
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

#include "../bsearch.h"
#include <stdio.h>

int main() {
    int ar[] = {
        2, 4, 5, 7, 12, 34, 45, 56, 57, 67, 67, 68, 68, 69, 70, 71, 89, 100 };
    int n = sizeof(ar)/sizeof(ar[0]);
    BSEARCH_SETUP(int);
    BSEARCH(int, ar, 0, n, 1);
    g_assert(!!BSEARCH_FOUND() == FALSE);
    BSEARCH(int, ar, 0, n, 0);
    g_assert(!!BSEARCH_FOUND() == FALSE);
    BSEARCH(int, ar, 0, n, 2);
    g_assert(!!BSEARCH_FOUND() == TRUE);
    g_assert(BSEARCH_AT() == 0);
    BSEARCH(int, ar, 0, n, 58);
    g_assert(!!BSEARCH_FOUND() == FALSE);
    BSEARCH(int, ar, 0, n, 57);
    g_assert(!!BSEARCH_FOUND() == TRUE);
    g_assert(BSEARCH_AT() == 8);
    BSEARCH(int, ar, 0, n, 55);
    g_assert(!!BSEARCH_FOUND() == FALSE);
    BSEARCH(int, ar, 0, n, 99);
    g_assert(!!BSEARCH_FOUND() == FALSE);
    BSEARCH(int, ar, 0, n, 100);
    g_assert(!!BSEARCH_FOUND() == TRUE);
    g_assert(BSEARCH_AT() == 17);
    BSEARCH(int, ar, 0, n, 101);
    g_assert(!!BSEARCH_FOUND() == FALSE);
    g_print("ok\n");
}
