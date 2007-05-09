/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   masks.h for the Openbox window manager
   Copyright (c) 2007        Dana Jansens

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

#ifndef ob__modkeys_h
#define ob__modkeys_h

#include <glib.h>
#include <X11/Xlib.h>

/*! These keys are bound to the modifier masks in any fashion */
typedef enum {
    OB_MODKEY_KEY_CAPSLOCK,
    OB_MODKEY_KEY_NUMLOCK,
    OB_MODKEY_KEY_SCROLLLOCK,
    OB_MODKEY_KEY_SHIFT,
    OB_MODKEY_KEY_CONTROL,
    OB_MODKEY_KEY_SUPER,
    OB_MODKEY_KEY_HYPER,
    OB_MODKEY_KEY_META,
    OB_MODKEY_KEY_ALT,

    OB_MODKEY_NUM_KEYS
} ObModkeysKey;

void modkeys_startup(gboolean reconfigure);
void modkeys_shutdown(gboolean reconfigure);

/*! Get the modifier masks for a keycode. (eg. a keycode bound to Alt_L could
  return a mask of (Mod1Mask | Mask3Mask)) */
guint modkeys_keycode_to_mask(guint keycode);

/*! Strip off all modifiers except for the modifier keys. This strips stuff
  like Button1Mask, and also LockMask, NumLockMask, and ScrollLockMask */
guint modkeys_only_modifier_masks(guint mask);

/*! Get the modifier masks for a modifier key. This includes both the left and
  right keys when there are both. */
guint modkeys_key_to_mask(ObModkeysKey key);

/*! Convert a KeySym to a KeyCode, because the X function is terrible - says
  valgrind. */
KeyCode modkeys_sym_to_code(KeySym sym);

#endif
