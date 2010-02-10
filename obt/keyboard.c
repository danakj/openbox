/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   obt/keyboard.c for the Openbox window manager
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

#include "obt/display.h"
#include "obt/keyboard.h"

#include <X11/Xlib.h>
#include <X11/keysym.h>

/* These masks are constants and the modifier keys are bound to them as
   anyone sees fit:
        ShiftMask (1<<0), LockMask (1<<1), ControlMask (1<<2), Mod1Mask (1<<3),
        Mod2Mask (1<<4), Mod3Mask (1<<5), Mod4Mask (1<<6), Mod5Mask (1<<7)
*/
#define NUM_MASKS 8
#define ALL_MASKS 0xf /* an or'ing of all 8 keyboard masks */

/* Get the bitflag for the n'th modifier mask */
#define nth_mask(n) (1 << n)

static void set_modkey_mask(guchar mask, KeySym sym);
void obt_keyboard_shutdown();

static XModifierKeymap *modmap;
static KeySym *keymap;
static gint min_keycode, max_keycode, keysyms_per_keycode;
/* This is a bitmask of the different masks for each modifier key */
static guchar modkeys_keys[OBT_KEYBOARD_NUM_MODKEYS];

static gboolean alt_l = FALSE;
static gboolean meta_l = FALSE;
static gboolean super_l = FALSE;
static gboolean hyper_l = FALSE;

static gboolean started = FALSE;

static XIM xim = NULL;
static XIMStyle xim_style = 0;
static XIC xic = NULL;
static Window xic_window = None;

void obt_keyboard_reload(void)
{
    gint i, j, k;
    gchar *aname, *aclass;
    gchar firstc[7];
    guint ctrl;

    ctrl = XkbPCF_GrabsUseXKBStateMask |
        XkbPCF_LookupStateWhenGrabbed;
    //XkbSetPerClientControls(obt_display, ctrl, &ctrl);

    if (started) obt_keyboard_shutdown(); /* free stuff */
    started = TRUE;

    /* initialize the Input Method */

    aname = g_strdup(g_get_prgname());
    if (!aname) aname = g_strdup("obt");

    /* capitalize first letter of the class */
    i = g_unichar_to_utf8(g_unichar_toupper(g_utf8_get_char(aname)),
                          firstc);
    firstc[i] = '\0';
    aclass = g_strdup_printf("%s%s", firstc, g_utf8_next_char(aname));

    g_print("Opening Input Method for %s %s\n", aname, aclass);
    xim = XOpenIM(obt_display, NULL, aname, aclass);

    g_free(aclass);
    g_free(aname);

    if (!xim)
        g_message("Failed to open an Input Method");
    else {
        XIMStyles *xim_styles = NULL;
        char *r;

        /* get the input method styles */
        r = XGetIMValues(xim, XNQueryInputStyle, &xim_styles, NULL);
        if (r || !xim_styles)
            g_message("Input Method does not support any styles");
        if (xim_styles) {
            /* pick a style that doesnt need preedit or status */
            for (i = 0; i < xim_styles->count_styles; ++i) {
                if (xim_styles->supported_styles[i] == 
                    (XIMPreeditNothing | XIMStatusNothing))
                {
                    xim_style = xim_styles->supported_styles[i];
                    break;
                }
            }
            XFree(xim_styles);
        }

        if (!xim_style)
            g_message("Input Method does not support a usable style");
        else if (xic_window)
            xic = XCreateIC(xim,
                            XNInputStyle, xim_style,
                            XNClientWindow, xic_window,
                            XNFocusWindow, xic_window,
                            NULL);
    }

    /* reset the keys to not be bound to any masks */
    for (i = 0; i < OBT_KEYBOARD_NUM_MODKEYS; ++i)
        modkeys_keys[i] = 0;

    modmap = XGetModifierMapping(obt_display);
    /* note: modmap->max_keypermod can be 0 when there is no valid key layout
       available */

    XDisplayKeycodes(obt_display, &min_keycode, &max_keycode);
    keymap = XGetKeyboardMapping(obt_display, min_keycode,
                                 max_keycode - min_keycode + 1,
                                 &keysyms_per_keycode);

    alt_l = meta_l = super_l = hyper_l = FALSE;

    /* go through each of the modifier masks (eg ShiftMask, CapsMask...) */
    for (i = 0; i < NUM_MASKS; ++i) {
        /* go through each keycode that is bound to the mask */
        for (j = 0; j < modmap->max_keypermod; ++j) {
            KeySym sym;
            /* get a keycode that is bound to the mask (i) */
            KeyCode keycode = modmap->modifiermap[i*modmap->max_keypermod + j];
            if (keycode) {
                /* go through each keysym bound to the given keycode */
                for (k = 0; k < keysyms_per_keycode; ++k) {
                    sym = keymap[(keycode-min_keycode) * keysyms_per_keycode +
                                 k];
                    if (sym != NoSymbol) {
                        /* bind the key to the mask (e.g. Alt_L => Mod1Mask) */
                        set_modkey_mask(nth_mask(i), sym);
                    }
                }
            }
        }
    }

    /* CapsLock, Shift, and Control are special and hard-coded */
    modkeys_keys[OBT_KEYBOARD_MODKEY_CAPSLOCK] = LockMask;
    modkeys_keys[OBT_KEYBOARD_MODKEY_SHIFT] = ShiftMask;
    modkeys_keys[OBT_KEYBOARD_MODKEY_CONTROL] = ControlMask;
}

void obt_keyboard_shutdown(void)
{
    XFreeModifiermap(modmap);
    modmap = NULL;
    XFree(keymap);
    keymap = NULL;
    if (xic) XDestroyIC(xic);
    xic = NULL;
    if (xim) XCloseIM(xim);
    xim = NULL;
    xim_style = 0;
    started = FALSE;
}

guint obt_keyboard_keycode_to_modmask(guint keycode)
{
    gint i, j;
    guint mask = 0;

    if (keycode == NoSymbol) return 0;

    /* go through each of the modifier masks (eg ShiftMask, CapsMask...) */
    for (i = 0; i < NUM_MASKS; ++i) {
        /* go through each keycode that is bound to the mask */
        for (j = 0; j < modmap->max_keypermod; ++j) {
            /* compare with a keycode that is bound to the mask (i) */
            if (modmap->modifiermap[i*modmap->max_keypermod + j] == keycode)
                mask |= nth_mask(i);
        }
    }
    return mask;
}

guint obt_keyboard_only_modmasks(guint mask)
{
    mask &= ALL_MASKS;
    /* strip off these lock keys. they shouldn't affect key bindings */
    mask &= ~LockMask; /* use the LockMask, not what capslock is bound to,
                          because you could bind it to something else and it
                          should work as that modifier then. i think capslock
                          is weird in xkb. */
    mask &= ~obt_keyboard_modkey_to_modmask(OBT_KEYBOARD_MODKEY_NUMLOCK);
    mask &= ~obt_keyboard_modkey_to_modmask(OBT_KEYBOARD_MODKEY_SCROLLLOCK);
    return mask;
}

guint obt_keyboard_modkey_to_modmask(ObtModkeysKey key)
{
    return modkeys_keys[key];
}

static void set_modkey_mask(guchar mask, KeySym sym)
{
    /* find what key this is, and bind it to the mask */

    if (sym == XK_Num_Lock)
        modkeys_keys[OBT_KEYBOARD_MODKEY_NUMLOCK] |= mask;
    else if (sym == XK_Scroll_Lock)
        modkeys_keys[OBT_KEYBOARD_MODKEY_SCROLLLOCK] |= mask;

    else if (sym == XK_Super_L && super_l)
        modkeys_keys[OBT_KEYBOARD_MODKEY_SUPER] |= mask;
    else if (sym == XK_Super_L && !super_l)
        /* left takes precident over right, so erase any masks the right
           key may have set */
        modkeys_keys[OBT_KEYBOARD_MODKEY_SUPER] = mask, super_l = TRUE;
    else if (sym == XK_Super_R && !super_l)
        modkeys_keys[OBT_KEYBOARD_MODKEY_SUPER] |= mask;

    else if (sym == XK_Hyper_L && hyper_l)
        modkeys_keys[OBT_KEYBOARD_MODKEY_HYPER] |= mask;
    else if (sym == XK_Hyper_L && !hyper_l)
        modkeys_keys[OBT_KEYBOARD_MODKEY_HYPER] = mask, hyper_l = TRUE;
    else if (sym == XK_Hyper_R && !hyper_l)
        modkeys_keys[OBT_KEYBOARD_MODKEY_HYPER] |= mask;

    else if (sym == XK_Alt_L && alt_l)
        modkeys_keys[OBT_KEYBOARD_MODKEY_ALT] |= mask;
    else if (sym == XK_Alt_L && !alt_l)
        modkeys_keys[OBT_KEYBOARD_MODKEY_ALT] = mask, alt_l = TRUE;
    else if (sym == XK_Alt_R && !alt_l)
        modkeys_keys[OBT_KEYBOARD_MODKEY_ALT] |= mask;

    else if (sym == XK_Meta_L && meta_l)
        modkeys_keys[OBT_KEYBOARD_MODKEY_META] |= mask;
    else if (sym == XK_Meta_L && !meta_l)
        modkeys_keys[OBT_KEYBOARD_MODKEY_META] = mask, meta_l = TRUE;
    else if (sym == XK_Meta_R && !meta_l)
        modkeys_keys[OBT_KEYBOARD_MODKEY_META] |= mask;

    /* CapsLock, Shift, and Control are special and hard-coded */
}

KeyCode* obt_keyboard_keysym_to_keycode(KeySym sym)
{
    KeyCode *ret;
    gint i, j, n;

    ret = g_new(KeyCode, 1);
    n = 0;
    ret[n] = 0;

    /* go through each keycode and look for the keysym */
    for (i = min_keycode; i <= max_keycode; ++i)
        for (j = 0; j < keysyms_per_keycode; ++j)
            if (sym == keymap[(i-min_keycode) * keysyms_per_keycode + j]) {
                ret = g_renew(KeyCode, ret, ++n);
                ret[n-1] = i;
                ret[n] = 0;
            }
    return ret;
}

void obt_keyboard_set_input_context(Window window)
{
    if (xic) {
        XDestroyIC(xic);
        xic = NULL;
    }
    xic_window = window;
    if (xic_window)
        xic = XCreateIC(xim,
                        XNInputStyle, xim_style,
                        XNClientWindow, xic_window,
                        XNFocusWindow, xic_window,
                        NULL);
    g_message("Created Input Context (0x%lx) for window 0x%lx",
              (gulong)xic, (gulong)xic_window);
}

gunichar obt_keyboard_keypress_to_unichar(XKeyPressedEvent *ev)
{
    gunichar unikey = 0;
    KeySym sym;
    Status status;
    gchar *buf, fixbuf[4]; /* 4 is enough for a utf8 char */
    gint r, bufsz;
    gboolean got_string;

    g_print("state: %x\n", ev->state);
    got_string = FALSE;

    if (xic) {

        buf = fixbuf;
        bufsz = sizeof(fixbuf);

#ifdef X_HAVE_UTF8_STRING
        r = Xutf8LookupString(xic, ev, buf, bufsz, &sym, &status);
#else
        r = XmbLookupString(xic, ev, buf, bufsz, &sym, &status);
#endif
        g_assert(r <= bufsz);

        g_print("status %d\n", status);

        if (status == XBufferOverflow) {
            g_message("bufferoverflow, need %d bytes", r);
            buf = g_new(char, r);
            bufsz = r;

#ifdef X_HAVE_UTF8_STRING
            r = Xutf8LookupString(xic, ev, buf, bufsz, &sym, &status);
#else
            r = XmbLookupString(xic, ev, buf, bufsz, &sym, &status);
#endif
            buf[r] = '\0';

            g_message("bufferoverflow read %d bytes, status=%d", r, status);
            {
                int i;
                for (i = 0; i < r + 1; ++i)
                    g_print("%u", (guchar)buf[i]);
                g_print("\n");
            }
        }

        if ((status == XLookupChars || status == XLookupBoth)) {
            g_message("read %d bytes", r);
            if ((guchar)buf[0] >= 32) { /* not an ascii control character */
#ifndef X_HAVE_UTF8_STRING 
                /* convert to utf8 */
                gchar *buf2 = buf;
                buf = g_locale_to_utf8(buf2, r, NULL, NULL, NULL);
                g_free(buf2);
#endif

                got_string = TRUE;
            }
        }
        else
            g_message("Bad keycode lookup. Keysym 0x%x Status: %s\n",
                      (guint) sym,
                      (status == XBufferOverflow ? "BufferOverflow" :
                       status == XLookupNone ? "XLookupNone" :
                       status == XLookupKeySym ? "XLookupKeySym" :
                       "Unknown status"));
    }
    else {
        buf = fixbuf;
        bufsz = sizeof(fixbuf);
        r = XLookupString(ev, buf, bufsz, &sym, NULL);
        g_assert(r <= bufsz);
        if ((guchar)buf[0] >= 32) /* not an ascii control character */
            got_string = TRUE;
    }

    if (got_string) {
        unikey = g_utf8_get_char_validated(buf, -1);
        if (unikey == (gunichar)-1 || unikey == (gunichar)-2)
            unikey = 0;

        if (unikey) {
            char *key = g_strndup(buf, r);
            g_print("key %s\n", key);
            g_free(key);
        }
    }

    if (buf != fixbuf) g_free(buf);

    g_print("unikey: %lu\n", unikey);
    return unikey;
}
