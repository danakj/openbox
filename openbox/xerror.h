#ifndef __xerror_h
#define __xerror_h

#include <X11/Xlib.h>
#include <glib.h>

int xerror_handler(Display *, XErrorEvent *);

void xerror_set_ignore(gboolean ignore);

#endif
