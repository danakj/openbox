/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   grab.c for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
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

#include "grab.h"
#include "modkeys.h"
#include "openbox.h"
#include "event.h"
#include "xerror.h"
#include "screen.h"
#include "debug.h"

#include <glib.h>
#include <X11/Xlib.h>

#define GRAB_PTR_MASK (ButtonPressMask | ButtonReleaseMask | PointerMotionMask)
#define GRAB_KEY_MASK (KeyPressMask | KeyReleaseMask)

#define MASK_LIST_SIZE 8

/*! A list of all possible combinations of keyboard lock masks */
static guint mask_list[MASK_LIST_SIZE];
static guint kgrabs = 0;
static guint pgrabs = 0;
/*! The time at which the last grab was made */
static Time  grab_time = CurrentTime;
static gint passive_count = 0;

static Time ungrab_time()
{
    Time t = event_curtime;
    if (grab_time == CurrentTime ||
        !(t == CurrentTime || event_time_after(t, grab_time)))
        /* When the time moves backward on the server, then we can't use
           the grab time because that will be in the future. So instead we
           have to use CurrentTime.

           "XUngrabPointer does not release the pointer if the specified time
           is earlier than the last-pointer-grab time or is later than the
           current X server time."
        */
        t = CurrentTime; /*grab_time;*/
    return t;
}

gboolean grab_on_keyboard()
{
    return kgrabs > 0;
}

gboolean grab_on_pointer()
{
    return pgrabs > 0;
}

gboolean grab_keyboard_full(gboolean grab)
{
    gboolean ret = FALSE;

    if (grab) {
        if (kgrabs++ == 0) {
            ret = XGrabKeyboard(ob_display, RootWindow(ob_display, ob_screen),
                                False, GrabModeAsync, GrabModeAsync,
                                event_curtime) == Success;
            if (!ret)
                --kgrabs;
            else {
                passive_count = 0;
                grab_time = event_curtime;
            }
        } else
            ret = TRUE;
    } else if (kgrabs > 0) {
        if (--kgrabs == 0) {
            XUngrabKeyboard(ob_display, ungrab_time());
        }
        ret = TRUE;
    }

    return ret;
}

gboolean grab_pointer_full(gboolean grab, gboolean owner_events,
                           gboolean confine, ObCursor cur)
{
    gboolean ret = FALSE;

    if (grab) {
        if (pgrabs++ == 0) {
            ret = XGrabPointer(ob_display, screen_support_win, owner_events,
                               GRAB_PTR_MASK,
                               GrabModeAsync, GrabModeAsync,
                               (confine ? RootWindow(ob_display, ob_screen) :
                                None),
                               ob_cursor(cur), event_curtime) == Success;
            if (!ret)
                --pgrabs;
            else
                grab_time = event_curtime;
        } else
            ret = TRUE;
    } else if (pgrabs > 0) {
        if (--pgrabs == 0) {
            XUngrabPointer(ob_display, ungrab_time());
        }
        ret = TRUE;
    }
    return ret;
}

gint grab_server(gboolean grab)
{
    static guint sgrabs = 0;
    if (grab) {
        if (sgrabs++ == 0) {
            XGrabServer(ob_display);
            XSync(ob_display, FALSE);
        }
    } else if (sgrabs > 0) {
        if (--sgrabs == 0) {
            XUngrabServer(ob_display);
            XFlush(ob_display);
        }
    }
    return sgrabs;
}

void grab_startup(gboolean reconfig)
{
    guint i = 0;
    guint num, caps, scroll;

    num = modkeys_key_to_mask(OB_MODKEY_KEY_NUMLOCK);
    caps = modkeys_key_to_mask(OB_MODKEY_KEY_CAPSLOCK);
    scroll = modkeys_key_to_mask(OB_MODKEY_KEY_SCROLLLOCK);

    mask_list[i++] = 0;
    mask_list[i++] = num;
    mask_list[i++] = caps;
    mask_list[i++] = scroll;
    mask_list[i++] = num | caps;
    mask_list[i++] = num | scroll;
    mask_list[i++] = caps | scroll;
    mask_list[i++] = num | caps | scroll;
    g_assert(i == MASK_LIST_SIZE);
}

void grab_shutdown(gboolean reconfig)
{
    if (reconfig) return;

    while (ungrab_keyboard());
    while (ungrab_pointer());
    while (grab_server(FALSE));
}

void grab_button_full(guint button, guint state, Window win, guint mask,
                      gint pointer_mode, ObCursor cur)
{
    guint i;

    xerror_set_ignore(TRUE); /* can get BadAccess from these */
    xerror_occured = FALSE;
    for (i = 0; i < MASK_LIST_SIZE; ++i)
        XGrabButton(ob_display, button, state | mask_list[i], win, False, mask,
                    pointer_mode, GrabModeAsync, None, ob_cursor(cur));
    xerror_set_ignore(FALSE);
    if (xerror_occured)
        ob_debug("Failed to grab button %d modifiers %d", button, state);
}

void ungrab_button(guint button, guint state, Window win)
{
    guint i;

    for (i = 0; i < MASK_LIST_SIZE; ++i)
        XUngrabButton(ob_display, button, state | mask_list[i], win);
}

void grab_key(guint keycode, guint state, Window win, gint keyboard_mode)
{
    guint i;

    xerror_set_ignore(TRUE); /* can get BadAccess' from these */
    xerror_occured = FALSE;
    for (i = 0; i < MASK_LIST_SIZE; ++i)
        XGrabKey(ob_display, keycode, state | mask_list[i], win, FALSE,
                 GrabModeAsync, keyboard_mode);
    xerror_set_ignore(FALSE);
    if (xerror_occured)
        ob_debug("Failed to grab keycode %d modifiers %d", keycode, state);
}

void ungrab_all_keys(Window win)
{
    XUngrabKey(ob_display, AnyKey, AnyModifier, win);
}

void grab_key_passive_count(int change)
{
    if (grab_on_keyboard()) return;
    passive_count += change;
    if (passive_count < 0) passive_count = 0;
}

void ungrab_passive_key()
{
    ob_debug("ungrabbing %d passive grabs\n", passive_count);
    if (passive_count) {
        /* kill out passive grab */
        XUngrabKeyboard(ob_display, event_curtime);
        passive_count = 0;
    }
}
