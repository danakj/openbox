#ifndef __config_h
#define __config_h

#include "dock.h"
#include "stacking.h"

#include <glib.h>

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
/*! The number of slits to create 
  extern int config_slit_number;*/
/*! When true windows are moved opaquely, when false just an outline is shown
  while they are moved */
extern gboolean config_opaque_move;
/*! When true windows are resize opaquely, when false just an outline is shown
  while they are resize */
extern gboolean config_opaque_resize;

/*! The stacking layer the dock will reside in */
extern StackLayer config_dock_layer;
/*! The position at which to place the dock */
extern DockPosition config_dock_pos;
/*! If config_dock_pos is DockPos_Floating, this is the top-left corner's
  position */
extern int config_dock_x;
/*! If config_dock_pos is DockPos_Floating, this is the top-left corner's
  position */
extern int config_dock_y;
/*! Whether the dock places the dockapps in it horizontally or vertically */
extern gboolean config_dock_horz;
/*! Whether to auto-hide the dock when the pointer is not over it */
extern gboolean config_dock_hide;
/*! The number of milliseconds to wait before hiding the dock */
extern guint config_dock_hide_timeout;

/* The name of the theme */
char *config_theme;

/*! The number of desktops */
extern int config_desktops_num;
/*! Names for the desktops */
extern GSList *config_desktops_names;


void config_startup();
void config_shutdown();

#endif
