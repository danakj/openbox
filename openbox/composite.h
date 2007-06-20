#ifndef __composite_h
#define __composite_h

#include <glib.h>
#include <X11/Xlib.h>

void composite_startup(gboolean reconfig);
void composite_shutdown(gboolean reconfig);

gboolean composite_window_has_alpha(Visual *vis);
XID composite_get_window_picture(Window win, Visual *vis);
Pixmap composite_get_window_pixmap(Window win);

#endif
