#ifndef __events_h
#define __events_h

#include <X11/Xlib.h>

/*! Time at which the last event with a timestamp occured. */
extern Time event_lasttime;

/*! The value of the mask for the NumLock modifier */
extern unsigned int NumLockMask;
/*! The value of the mask for the ScrollLock modifier */
extern unsigned int ScrollLockMask;

void event_startup();
void event_shutdown();

typedef struct event_fd_handler {
    int fd;
    void *data;
    void (*handler)(int fd, void *data);
} event_fd_handler;

void event_add_fd_handler(event_fd_handler *handler);
void event_remove_fd(int n);

void event_loop();

#endif
