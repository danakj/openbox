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

extern gchar   *ob_sm_id;
extern gboolean ob_sm_use;
extern gboolean ob_replace_wm;

/* The state of execution of the window manager */
ObState ob_state();

void ob_restart_other(const gchar *path);
void ob_restart();
void ob_exit();

void ob_exit_with_error(gchar *msg);

Cursor ob_cursor(ObCursor cursor);

KeyCode ob_keycode(ObKey key);

#endif
