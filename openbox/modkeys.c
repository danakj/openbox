/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   modkeys.c for the Openbox window manager
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

#include "modkeys.h"
#include "openbox.h"

#include <X11/Xlib.h>
#include <X11/keysym.h>

/* These masks are constants and the modifier keys are bound to them as
   anyone sees fit:
        ShiftMask (1<<0), LockMask (1<<1), ControlMask (1<<2), Mod1Mask (1<<3),
        Mod2Mask (1<<4), Mod3Mask (1<<5), Mod4Mask (1<<6), Mod5Mask (1<<7)
*/
#define NUM_MASKS 8
#define ALL_MASKS 0xff /* an or'ing of all 8 keyboard masks */

/* Get the bitflag for the n'th modifier mask */
#define nth_mask(n) (1 << n)

static void set_modkey_mask(guchar mask, KeySym sym);

/* This is 8 lists of keycodes that are bound to the given mod mask.
   If contains more than the one given to us by X cuz XKB is weird apparently.
   We will look up all keycodes for a given keysym that is bound to the mask,
   and add them all here.

   With XKB, you can have a keycode bound to a modifier that isn't in the
   modifier map somehow.  So this means that when we try translate from the
   KeyRelease to a mod mask, we are unable to.  So this array stores *all*
   the KeyCodes for each KeySym for each KeyCode bound to a mod mask.
   Confused? Haha...

   ModMask -> n KeyCodes -> n*m KeySyms (up to m for each KeyCode) ->
     n*m*p KeyCodes (up to p for each KeySym)
*/
static GArray *modmap[NUM_MASKS];
static KeySym *keymap;
static gint min_keycode, max_keycode, keysyms_per_keycode;
/* This is a bitmask of the different masks for each modifier key */
static guchar modkeys_keys[OB_MODKEY_NUM_KEYS];

static gboolean alt_l = FALSE;
static gboolean meta_l = FALSE;
static gboolean super_l = FALSE;
static gboolean hyper_l = FALSE;

void modkeys_startup(gboolean reconfigure)
{
    static XModifierKeymap *xmodmap;
    gint i, j, k, l, m;

    /* reset the keys to not be bound to any masks */
    for (i = 0; i < OB_MODKEY_NUM_KEYS; ++i)
        modkeys_keys[i] = 0;

    xmodmap = XGetModifierMapping(ob_display);
    g_assert(xmodmap->max_keypermod > 0);

    XDisplayKeycodes(ob_display, &min_keycode, &max_keycode);
    keymap = XGetKeyboardMapping(ob_display, min_keycode,
                                 max_keycode - min_keycode + 1,
                                 &keysyms_per_keycode);

    alt_l = meta_l = super_l = hyper_l = FALSE;

    /* go through each of the modifier masks (eg ShiftMask, CapsMask...) */
    for (i = 0; i < NUM_MASKS; ++i) {
        /* reset the modmap list */
        modmap[i] = g_array_new(FALSE, FALSE, sizeof(KeyCode));

        /* go through each keycode that is bound to the mask */
        for (j = 0; j < xmodmap->max_keypermod; ++j) {
            KeySym sym;
            /* get a keycode that is bound to the mask (i) */
            KeyCode keycode = xmodmap->modifiermap[i*xmodmap->max_keypermod+j];
            if (keycode) {
                /* go through each keysym bound to the given keycode */
                for (k = 0; k < keysyms_per_keycode; ++k) {
                    sym = keymap[(keycode-min_keycode) * keysyms_per_keycode +
                                 k];
                    if (sym != NoSymbol) {
                        /* find all keycodes for the given keysym */
                        for (l = min_keycode; l <= max_keycode; ++l)
                            for (m = 0; m < keysyms_per_keycode; ++m)
                                if (keymap[(l-min_keycode) *
                                           keysyms_per_keycode + m] == sym)
                                {
                                    /* add all keycodes for the keysym to our
                                       modmap */
                                    g_array_append_val(modmap[i], l);
                                }

                        /* bind the key to the mask (e.g. Alt_L => Mod1Mask) */
                        set_modkey_mask(nth_mask(i), sym);
                    }
                }
            }
        }
    }

    /* CapsLock, Shift, and Control are special and hard-coded */
    modkeys_keys[OB_MODKEY_KEY_CAPSLOCK] = LockMask;
    modkeys_keys[OB_MODKEY_KEY_SHIFT] = ShiftMask;
    modkeys_keys[OB_MODKEY_KEY_CONTROL] = ControlMask;

    XFreeModifiermap(xmodmap);
}

void modkeys_shutdown(gboolean reconfigure)
{
    guint i;

    for (i = 0; i < NUM_MASKS; ++i)
        g_array_free(modmap[i], TRUE);
    XFree(keymap);
}

guint modkeys_keycode_to_mask(guint keycode)
{
    guint i, j;
    guint mask = 0;

    if (keycode == NoSymbol) return 0;

    /* go through each of the modifier masks (eg ShiftMask, CapsMask...) */
    for (i = 0; i < NUM_MASKS; ++i) {
        /* go through each keycode that is bound to the mask */
        for (j = 0; j < modmap[i]->len; ++j) {
            /* compare with a keycode that is bound to the mask (i) */
            if (g_array_index(modmap[i], KeyCode, j) == keycode)
                mask |= nth_mask(i);
        }
    }
    return mask;
}

guint modkeys_only_modifier_masks(guint mask)
{
    mask &= ALL_MASKS;
    /* strip off these lock keys. they shouldn't affect key bindings */
    mask &= ~LockMask; /* use the LockMask, not what capslock is bound to,
                          because you could bind it to something else and it
                          should work as that modifier then. i think capslock
                          is weird in xkb. */
    mask &= ~modkeys_key_to_mask(OB_MODKEY_KEY_NUMLOCK);
    mask &= ~modkeys_key_to_mask(OB_MODKEY_KEY_SCROLLLOCK);
    return mask;
}

guint modkeys_key_to_mask(ObModkeysKey key)
{
    return modkeys_keys[key];
}

static void set_modkey_mask(guchar mask, KeySym sym)
{
    /* find what key this is, and bind it to the mask */

    if (sym == XK_Num_Lock)
        modkeys_keys[OB_MODKEY_KEY_NUMLOCK] |= mask;
    else if (sym == XK_Scroll_Lock)
        modkeys_keys[OB_MODKEY_KEY_SCROLLLOCK] |= mask;

    else if (sym == XK_Super_L && super_l)
        modkeys_keys[OB_MODKEY_KEY_SUPER] |= mask;
    else if (sym == XK_Super_L && !super_l)
        /* left takes precident over right, so erase any masks the right
           key may have set */
        modkeys_keys[OB_MODKEY_KEY_SUPER] = mask, super_l = TRUE;
    else if (sym == XK_Super_R && !super_l)
        modkeys_keys[OB_MODKEY_KEY_SUPER] |= mask;

    else if (sym == XK_Hyper_L && hyper_l)
        modkeys_keys[OB_MODKEY_KEY_HYPER] |= mask;
    else if (sym == XK_Hyper_L && !hyper_l)
        modkeys_keys[OB_MODKEY_KEY_HYPER] = mask, hyper_l = TRUE;
    else if (sym == XK_Hyper_R && !hyper_l)
        modkeys_keys[OB_MODKEY_KEY_HYPER] |= mask;

    else if (sym == XK_Alt_L && alt_l)
        modkeys_keys[OB_MODKEY_KEY_ALT] |= mask;
    else if (sym == XK_Alt_L && !alt_l)
        modkeys_keys[OB_MODKEY_KEY_ALT] = mask, alt_l = TRUE;
    else if (sym == XK_Alt_R && !alt_l)
        modkeys_keys[OB_MODKEY_KEY_ALT] |= mask;

    else if (sym == XK_Meta_L && meta_l)
        modkeys_keys[OB_MODKEY_KEY_META] |= mask;
    else if (sym == XK_Meta_L && !meta_l)
        modkeys_keys[OB_MODKEY_KEY_META] = mask, meta_l = TRUE;
    else if (sym == XK_Meta_R && !meta_l)
        modkeys_keys[OB_MODKEY_KEY_META] |= mask;

    /* CapsLock, Shift, and Control are special and hard-coded */
}

KeyCode modkeys_sym_to_code(KeySym sym)
{
    gint i, j;

    /* go through each keycode and look for the keysym */
    for (i = min_keycode; i <= max_keycode; ++i)
        for (j = 0; j < keysyms_per_keycode; ++j)
            if (sym == keymap[(i-min_keycode) * keysyms_per_keycode + j])
                return i;
    return 0;
}

