#ifndef __grab_h
#define __grab_h

#include <glib.h>
#include <X11/Xlib.h>

void grab_startup();
void grab_shutdown();

void grab_keyboard(gboolean grab);
void grab_pointer(gboolean grab, Cursor cur);

#endif
