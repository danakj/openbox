/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   translate.c for the Openbox window manager
   Copyright (c) 2004        Mikael Magnusson
   Copyright (c) 2003        Ben Jansens

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

#include "openbox.h"
#include "mouse.h"
#include <glib.h>
#include <string.h>
#include <stdlib.h>

static guint translate_modifier(gchar *str)
{
    if (!g_ascii_strcasecmp("Mod1", str) ||
        !g_ascii_strcasecmp("A", str)) return Mod1Mask;
    else if (!g_ascii_strcasecmp("Mod2", str)) return Mod2Mask;
    else if (!g_ascii_strcasecmp("Mod3", str) ||
             !g_ascii_strcasecmp("M", str)) return Mod3Mask;
    else if (!g_ascii_strcasecmp("Mod4", str) ||
             !g_ascii_strcasecmp("W", str)) return Mod4Mask;
    else if (!g_ascii_strcasecmp("Mod5", str)) return Mod5Mask;
    else if (!g_ascii_strcasecmp("Control", str) ||
             !g_ascii_strcasecmp("C", str)) return ControlMask;
    else if (!g_ascii_strcasecmp("Shift", str) ||
             !g_ascii_strcasecmp("S", str)) return ShiftMask;
    g_warning("Invalid modifier '%s' in binding.", str);
    return 0;
}

gboolean translate_button(const gchar *str, guint *state, guint *button)
{
    gchar **parsed;
    gchar *l;
    gint i;
    gboolean ret = FALSE;

    parsed = g_strsplit(str, "-", -1);
    
    /* first, find the button (last token) */
    l = NULL;
    for (i = 0; parsed[i] != NULL; ++i)
        l = parsed[i];
    if (l == NULL)
        goto translation_fail;

    /* figure out the mod mask */
    *state = 0;
    for (i = 0; parsed[i] != l; ++i) {
        guint m = translate_modifier(parsed[i]);
        if (!m) goto translation_fail;
        *state |= m;
    }

    /* figure out the button */
    if (!g_ascii_strcasecmp("Left", l)) *button = 1;
    else if (!g_ascii_strcasecmp("Middle", l)) *button = 2;
    else if (!g_ascii_strcasecmp("Right", l)) *button = 3;
    else if (!g_ascii_strcasecmp("Up", l)) *button = 4;
    else if (!g_ascii_strcasecmp("Down", l)) *button = 5;
    else if (!g_ascii_strncasecmp("Button", l, 6)) *button = atoi(l+6);
    if (!*button) {
        g_warning("Invalid button '%s' in pointer binding.", l);
        goto translation_fail;
    }

    ret = TRUE;

translation_fail:
    g_strfreev(parsed);
    return ret;
}

gboolean translate_key(const gchar *str, guint *state, guint *keycode)
{
    gchar **parsed;
    gchar *l;
    gint i;
    gboolean ret = FALSE;
    KeySym sym;

    parsed = g_strsplit(str, "-", -1);
    
    /* first, find the key (last token) */
    l = NULL;
    for (i = 0; parsed[i] != NULL; ++i)
        l = parsed[i];
    if (l == NULL)
        goto translation_fail;

    /* figure out the mod mask */
    *state = 0;
    for (i = 0; parsed[i] != l; ++i) {
        guint m = translate_modifier(parsed[i]);
        if (!m) goto translation_fail;
        *state |= m;
    }

    if (!g_ascii_strncasecmp("0x", l, 2)) {
        gchar *end;

        /* take it directly */
        *keycode = strtol(l, &end, 16);
        if (*l == '\0' || *end != '\0') {
            g_warning("Invalid key code '%s' in key binding.", l);
            goto translation_fail;
        }
    } else {
        /* figure out the keycode */
        sym = XStringToKeysym(l);
        if (sym == NoSymbol) {
            g_warning("Invalid key name '%s' in key binding.", l);
            goto translation_fail;
        }
        *keycode = XKeysymToKeycode(ob_display, sym);
    }
    if (!*keycode) {
        g_warning("Key '%s' does not exist on the display.", l); 
        goto translation_fail;
    }

    ret = TRUE;

translation_fail:
    g_strfreev(parsed);
    return ret;
}
