#ifndef __openbox_h
#define __openbox_h

#include "misc.h"

#include "render/render.h"
#include "render/theme.h"

#include <glib.h>
#include <X11/Xlib.h>

struct _ObMainLoop;

extern RrInstance *ob_rr_inst;
extern RrTheme    *ob_rr_theme;

extern struct _ObMainLoop *ob_main_loop;

/*! The X display */
extern Display *ob_display; 

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

void ob_reconfigure();

void ob_exit_with_error(gchar *msg);

Cursor ob_cursor(ObCursor cursor);

KeyCode ob_keycode(ObKey key);

gchar *ob_expand_tilde(const gchar *f);

#endif
