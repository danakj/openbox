#ifndef __config_h
#define __config_h

#include "misc.h"
#include "stacking.h"

#include <glib.h>

struct _ObParseInst;

/*! Should new windows be focused */
extern gboolean config_focus_new;
/*! Focus windows when the mouse enters them */
extern gboolean config_focus_follow;
/*! Focus the last focused window as a fallback */
extern gboolean config_focus_last;
/*! Focus the last focused window as a fallback when switching desktops */
extern gboolean config_focus_last_on_desktop;
/*! Show a popup dialog while cycling focus */
extern gboolean config_focus_popup;
/*! Show a popup dialog while cycling desktops */
extern gboolean config_desktop_popup;
/*! The number of slits to create 
  extern int config_slit_number;*/
/*! When true windows' contents are refreshed while they are resized; otherwise
  they are not updated until the resize is complete */
extern gboolean config_redraw_resize;

/*! The stacking layer the dock will reside in */
extern ObStackingLayer config_dock_layer;
/*! Is the dock floating */
extern gboolean config_dock_floating;
/*! Where to place the dock if not floating */
extern ObDirection config_dock_pos;
/*! If config_dock_floating, this is the top-left corner's
  position */
extern gint config_dock_x;
/*! If config_dock_floating, this is the top-left corner's
  position */
extern gint config_dock_y;
/*! Whether the dock places the dockapps in it horizontally or vertically */
extern ObOrientation config_dock_orient;
/*! Whether to auto-hide the dock when the pointer is not over it */
extern gboolean config_dock_hide;
/*! The number of milliseconds to wait before hiding the dock */
extern guint config_dock_hide_timeout;

/* The name of the theme */
char *config_theme;

/* Titlebar button layout */
gchar *config_title_layout;

/*! The number of desktops */
extern int config_desktops_num;
/*! Names for the desktops */
extern GSList *config_desktops_names;

/*! The keycode of the key combo which resets the keybaord chains */
guint config_keyboard_reset_keycode;
/*! The modifiers of the key combo which resets the keybaord chains */
guint config_keyboard_reset_state;

/*! Number of pixels a drag must go before being considered a drag */
extern gint config_mouse_threshold;
/*! Number of milliseconds within which 2 clicks must occur to be a
  double-click */
extern gint config_mouse_dclicktime;

/*! Number of pixels to resist while crossing another window's edge */
gint config_resist_win;
/*! Number of pixels to resist while crossing a screen's edge */
gint config_resist_edge;

/*! User-specified menu files */
extern GSList *config_menu_files;

void config_startup(struct _ObParseInst *i);
void config_shutdown();

#endif
