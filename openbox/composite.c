/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   composite.c for the Openbox window manager
   Copyright (c) 2010        Dana Jansens
   Copyright (c) 2010        Derek Foreman

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

#include "composite.h"
#include "config.h"
#include "obt/display.h"

#include <X11/Xlib.h>
#include <glib.h>

void composite_startup(gboolean reconfig)
{
    /* This function will try enable composite if config_comp is TRUE.  At the
       end of this process, config_comp will be set to TRUE only if composite
       is enabled, and FALSE otherwise. */
#ifdef USE_COMPOSITING
    gboolean try;

    if (reconfig) return;
    if (!(try = config_comp)) return;

    config_comp = FALSE;

    /* XXX test GL things we need also */
    if (!obt_display_extension_composite ||
        !obt_display_extension_damage ||
        !obt_display_extension_fixes)
    {
        try = FALSE;
    }

    config_comp = try;
    if (!config_comp)
        g_message("Failed to enable composite.  A better explanation why would be nice");
#endif
}

void composite_shutdown(gboolean reconfig)
{
#ifdef USE_COMPOSITING
    if (reconfig) return;
#endif
}
