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
extern gint     ob_screen;

/* The state of execution of the window manager */
extern ObState ob_state;

void ob_restart_other(const gchar *path);
void ob_restart();
void ob_exit();

Cursor ob_cursor(ObCursor cursor);

KeyCode ob_keycode(ObKey key);

/* cuz i have nowhere better to put it right now... */
gboolean ob_pointer_pos(int *x, int *y);

#endif
