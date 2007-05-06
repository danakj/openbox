/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   debug.c for the Openbox window manager
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

#include "debug.h"

#include <glib.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

static gboolean show;

void ob_debug_show_output(gboolean enable)
{
    show = enable;
}

void ob_debug(const gchar *a, ...)
{
    va_list vl;

    if (show) {
        fprintf(stderr, "DEBUG: ");
        va_start(vl, a);
        vfprintf(stderr, a, vl);
        va_end(vl);
    }
}

static gboolean enabled_types[OB_DEBUG_TYPE_NUM] = {FALSE};

void ob_debug_enable(ObDebugType type, gboolean enable)
{
    g_assert(type < OB_DEBUG_TYPE_NUM);
    enabled_types[type] = enable;
}

void ob_debug_type(ObDebugType type, const gchar *a, ...)
{
    va_list vl;

    g_assert(type < OB_DEBUG_TYPE_NUM);

    if (show && enabled_types[type]) {
        switch (type) {
        case OB_DEBUG_FOCUS:
            fprintf(stderr, "FOCUS: ");
            break;
        case OB_DEBUG_APP_BUGS:
            fprintf(stderr, "APPLICATION BUG: ");
            break;
        case OB_DEBUG_SM:
            fprintf(stderr, "SESSION: ");
            break;
        default:
            g_assert_not_reached();
        }

        va_start(vl, a);
        vfprintf(stderr, a, vl);
        va_end(vl);
    }
}
