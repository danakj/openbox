#ifndef __events_h
#define __events_h

#include <X11/Xlib.h>

/*! Time at which the last event with a timestamp occured. */
extern Time event_lasttime;

/*! The value of the mask for the NumLock modifier */
extern guint NumLockMask;
/*! The value of the mask for the ScrollLock modifier */
extern guint ScrollLockMask;

void event_startup();
void event_shutdown();

#endif
