#ifndef __openbox_h
#define __openbox_h

#include "obexport.h"
#include <glib.h>
#include <X11/Xlib.h>

/*! The X display */
extern Display *ob_display; 
/*! The number of the screen on which we're running */
extern int      ob_screen;
/*! The root window */
extern Window   ob_root;

/* The state of execution of the window manager */
State ob_state;

/*! When set to true, Openbox will exit */
extern gboolean ob_shutdown;
/*! When set to true, Openbox will restart instead of shutting down */
extern gboolean ob_restart;

/*! Runtime option to specify running on a remote display */
extern gboolean ob_remote;
/*! Runtime option to run in synchronous mode */
extern gboolean ob_sync;

typedef struct Cursors {
    Cursor left_ptr;
    Cursor ll_angle;
    Cursor lr_angle;
} Cursors;
Cursors ob_cursors;

#endif
