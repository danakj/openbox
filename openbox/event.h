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

typedef struct event_fd_handler {
    gint fd;
    gpointer data;
    void (*handler)(gint fd, gpointer data);
} event_fd_handler;

void event_add_fd_handler(event_fd_handler *handler);
void event_remove_fd(gint n);

void event_loop();

#endif
