#include "openbox.h"
#include "event.h"
#include "xerror.h"

#include <glib.h>
#include <X11/Xlib.h>

#define GRAB_PTR_MASK (ButtonPressMask | ButtonReleaseMask | ButtonMotionMask)
#define GRAB_KEY_MASK (KeyPressMask | KeyReleaseMask)

#define MASK_LIST_SIZE 8

/*! A list of all possible combinations of keyboard lock masks */
static unsigned int mask_list[MASK_LIST_SIZE];

int grab_keyboard(gboolean grab)
{
    static guint kgrabs = 0;
    if (grab) {
        if (kgrabs++ == 0)
            XGrabKeyboard(ob_display, ob_root, FALSE, GrabModeAsync,
                          GrabModeAsync, event_lasttime);
    } else if (kgrabs > 0) {
        if (--kgrabs == 0)
            XUngrabKeyboard(ob_display, event_lasttime);
    }
    return kgrabs;
}

int grab_pointer(gboolean grab, Cursor cur)
{
    static guint pgrabs = 0;
    if (grab) {
        if (pgrabs++ == 0)
            XGrabPointer(ob_display, ob_root, False, GRAB_PTR_MASK,
                         GrabModeAsync, GrabModeAsync, FALSE, cur,
                         event_lasttime);
    } else if (pgrabs > 0) {
        if (--pgrabs == 0)
            XUngrabPointer(ob_display, event_lasttime);
    }
    return pgrabs;
}

int grab_pointer_window(gboolean grab, Cursor cur, Window win)
{
    static guint pgrabs = 0;
    if (grab) {
        if (pgrabs++ == 0)
            XGrabPointer(ob_display, win, False, GRAB_PTR_MASK, GrabModeAsync,
                         GrabModeAsync, TRUE, cur, event_lasttime);
    } else if (pgrabs > 0) {
        if (--pgrabs == 0)
            XUngrabPointer(ob_display, event_lasttime);
    }
    return pgrabs;
}

int grab_server(gboolean grab)
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

void grab_startup()
{
    guint i = 0;

    mask_list[i++] = 0;
    mask_list[i++] = LockMask;
    mask_list[i++] = NumLockMask;
    mask_list[i++] = LockMask | NumLockMask;
    mask_list[i++] = ScrollLockMask;
    mask_list[i++] = ScrollLockMask | LockMask;
    mask_list[i++] = ScrollLockMask | NumLockMask;
    mask_list[i++] = ScrollLockMask | LockMask | NumLockMask;
    g_assert(i == MASK_LIST_SIZE);
}

void grab_shutdown()
{
    while (grab_keyboard(FALSE));
    while (grab_pointer(FALSE, None));
    while (grab_pointer_window(FALSE, None, None));
    while (grab_server(FALSE));
}

void grab_button_full(guint button, guint state, Window win, guint mask,
                      int pointer_mode, Cursor cursor)
{
    guint i;

    for (i = 0; i < MASK_LIST_SIZE; ++i)
        XGrabButton(ob_display, button, state | mask_list[i], win, FALSE, mask,
                    pointer_mode, GrabModeSync, None, cursor);
}

void grab_button(guint button, guint state, Window win, guint mask)
{
    grab_button_full(button, state, win, mask, GrabModeAsync, None);
}

void ungrab_button(guint button, guint state, Window win)
{
    guint i;

    for (i = 0; i < MASK_LIST_SIZE; ++i)
        XUngrabButton(ob_display, button, state | mask_list[i], win);
}

void grab_key(guint keycode, guint state, int keyboard_mode)
{
    guint i;

    xerror_set_ignore(TRUE); /* can get BadAccess' from these */
    xerror_occured = FALSE;
    for (i = 0; i < MASK_LIST_SIZE; ++i)
        XGrabKey(ob_display, keycode, state | mask_list[i], ob_root, FALSE,
                 GrabModeSync, keyboard_mode);
    xerror_set_ignore(FALSE);
    if (xerror_occured)
        g_warning("failed to grab keycode %d modifiers %d", keycode, state);
}

void ungrab_all_keys()
{
    XUngrabKey(ob_display, AnyKey, AnyModifier, ob_root);
}
