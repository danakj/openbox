#include "openbox.h"
#include <glib.h>
#include <X11/Xlib.h>

static guint kgrabs, pgrabs, sgrabs;

void grab_keyboard(gboolean grab)
{
    if (grab) {
        if (kgrabs++ == 0)
            XGrabKeyboard(ob_display, ob_root, 0, GrabModeAsync, GrabModeSync,
                          CurrentTime);
    } else if (kgrabs > 0) {
        if (--kgrabs == 0)
            XUngrabKeyboard(ob_display, CurrentTime);
    }
}

void grab_pointer(gboolean grab, Cursor cur)
{
    if (grab) {
        if (pgrabs++ == 0)
            XGrabPointer(ob_display, ob_root, False, 0, GrabModeAsync,
                         GrabModeSync, FALSE, cur, CurrentTime);
    } else if (pgrabs > 0) {
        if (--pgrabs == 0)
            XUngrabPointer(ob_display, CurrentTime);
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
    kgrabs = pgrabs = sgrabs = 0;
}

void grab_shutdown()
{
    while (kgrabs) grab_keyboard(FALSE);
    while (pgrabs) grab_pointer(FALSE, None);
    while (sgrabs) grab_server(FALSE);
}
