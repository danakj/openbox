#ifndef __openbox_h
#define __openbox_h

#include "misc.h"

#include "render/render.h"
#include "render/theme.h"

#ifdef USE_LIBSN
#  define SN_API_NOT_YET_FROZEN
#  include <libsn/sn.h>
#endif

#include <glib.h>
#include <X11/Xlib.h>

extern RrInstance *ob_rr_inst;
extern RrTheme    *ob_rr_theme;

/*! The X display */
extern Display *ob_display; 

#ifdef USE_LIBSN
SnDisplay *ob_sn_display;
#endif

/*! The number of the screen on which we're running */
extern int      ob_screen;
/*! The root window */
extern Window   ob_root;

/* The state of execution of the window manager */
extern ObState ob_state;

/*! When set to true, Openbox will exit */
extern gboolean ob_shutdown;
/*! When set to true, Openbox will restart instead of shutting down */
extern gboolean ob_restart;
/*! When restarting, if this is not NULL, it will be executed instead of
  restarting Openbox. */
extern char *ob_restart_path;

/*! Runtime option to specify running on a remote display */
extern gboolean ob_remote;
/*! Runtime option to run in synchronous mode */
extern gboolean ob_sync;

/*! The path of the rc file. If NULL the default paths are searched for one. */
extern char *ob_rc_path;

Cursor ob_cursor(ObCursor cursor);

KeyCode ob_keycode(ObKey key);

/* cuz i have nowhere better to put it right now... */
gboolean ob_pointer_pos(int *x, int *y);

#endif
