#ifndef __grab_h
#define __grab_h

#include "misc.h"

#include <glib.h>
#include <X11/Xlib.h>

void grab_startup(gboolean reconfig);
void grab_shutdown(gboolean reconfig);

gboolean grab_keyboard(gboolean grab);
gboolean grab_pointer(gboolean grab, ObCursor cur);
gboolean grab_pointer_window(gboolean grab, ObCursor cur, Window win);
gint grab_server(gboolean grab);

void grab_button(guint button, guint state, Window win, guint mask);
void grab_button_full(guint button, guint state, Window win, guint mask,
                      int pointer_mode, ObCursor cursor);
void ungrab_button(guint button, guint state, Window win);

void grab_key(guint keycode, guint state, Window win, int keyboard_mode);

void ungrab_all_keys(Window win);

#endif
