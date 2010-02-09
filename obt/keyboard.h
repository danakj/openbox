/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   obt/keyboard.h for the Openbox window manager
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

#ifndef __obt_keyboard_h
#define __obt_keyboard_h

#include <glib.h>
#include <X11/Xlib.h>

G_BEGIN_DECLS

/*! These keys are bound to the modifier masks in any fashion,
  except for CapsLock, Shift, and Control. */
typedef enum {
    OBT_KEYBOARD_MODKEY_CAPSLOCK,
    OBT_KEYBOARD_MODKEY_NUMLOCK,
    OBT_KEYBOARD_MODKEY_SCROLLLOCK,
    OBT_KEYBOARD_MODKEY_SHIFT,
    OBT_KEYBOARD_MODKEY_CONTROL,
    OBT_KEYBOARD_MODKEY_SUPER,
    OBT_KEYBOARD_MODKEY_HYPER,
    OBT_KEYBOARD_MODKEY_META,
    OBT_KEYBOARD_MODKEY_ALT,

    OBT_KEYBOARD_NUM_MODKEYS
} ObtModkeysKey;

void obt_keyboard_reload(void);

/*! Get the modifier mask(s) for a KeyCode. (eg. a keycode bound to Alt_L could
  return a mask of (Mod1Mask | Mask3Mask)) */
guint obt_keyboard_keycode_to_modmask(guint keycode);

/*! Strip off all modifiers except for the modifier keys. This strips stuff
  like Button1Mask, and also LockMask, NumlockMask, and ScrolllockMask */
guint obt_keyboard_only_modmasks(guint mask);

/*! Get the modifier masks for a modifier key. This includes both the left and
  right keys when there are both. */
guint obt_keyboard_modkey_to_modmask(ObtModkeysKey key);

/*! Convert a KeySym to all the KeyCodes which generate it. */
KeyCode* obt_keyboard_keysym_to_keycode(KeySym sym);

/*! Set the input context to be the given window.  This must be called
  before using obt_keyboard_keycode_to_unichar() and
  obt_keyboard_keycode_to_string().
  @window The window where input focus is (logically).  Can be None.
*/
void obt_keyboard_set_input_context(Window window);

/*! Give the string form of a KeyPressEvent */
gchar *obt_keyboard_keypress_to_string(XKeyPressedEvent *ev);

/*! Translate a KeyPressEvent to the unicode character it represents */
gunichar obt_keyboard_keypress_to_unichar(XKeyPressedEvent *ev);


G_END_DECLS

#endif /* __obt_keyboard_h */
