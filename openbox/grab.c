#include "openbox.h"
#include "event.h"

#include <glib.h>
#include <X11/Xlib.h>

static guint kgrabs, pgrabs, sgrabs;

#define MASK_LIST_SIZE 8

/*! A list of all possible combinations of keyboard lock masks */
static unsigned int mask_list[MASK_LIST_SIZE];

void grab_keyboard(gboolean grab)
{
    if (grab) {
        if (kgrabs++ == 0) {
            g_message("GRABBING KEYBOARD %d", kgrabs);
            XGrabKeyboard(ob_display, ob_root, 0, GrabModeAsync, GrabModeSync,
                          event_lasttime);
        } else
            g_message("NOT GRABBING KEYBOARD %d", kgrabs);
    } else if (kgrabs > 0) {
        if (--kgrabs == 0) {
            g_message("UNGRABBING KEYBOARD %d", kgrabs);
            XUngrabKeyboard(ob_display, event_lasttime);
        } else
            g_message("NOT UNGRABBING KEYBOARD %d", kgrabs);
    }
}

void grab_pointer(gboolean grab, Cursor cur)
{
    if (grab) {
        if (pgrabs++ == 0)
            XGrabPointer(ob_display, ob_root, False, 0, GrabModeAsync,
                         GrabModeAsync, FALSE, cur, event_lasttime);
    } else if (pgrabs > 0) {
        if (--pgrabs == 0)
            XUngrabPointer(ob_display, event_lasttime);
    }
}

void grab_server(gboolean grab)
{
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
}

void grab_startup()
{
    guint i = 0;

    kgrabs = pgrabs = sgrabs = 0;

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
    while (kgrabs) grab_keyboard(FALSE);
    while (pgrabs) grab_pointer(FALSE, None);
    while (sgrabs) grab_server(FALSE);
}

void grab_button(guint button, guint state, Window win, guint mask,
                 int pointer_mode)
{
    guint i;

    for (i = 0; i < MASK_LIST_SIZE; ++i)
        XGrabButton(ob_display, button, state | mask_list[i], win, FALSE, mask,
                    pointer_mode, GrabModeAsync, None, None);
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

    for (i = 0; i < MASK_LIST_SIZE; ++i)
        XGrabKey(ob_display, keycode, state | mask_list[i], ob_root, FALSE,
                 GrabModeAsync, keyboard_mode);
}

void ungrab_all_keys()
{
    XUngrabKey(ob_display, AnyKey, AnyModifier, ob_root);
}
