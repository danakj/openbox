#ifndef __config_h
#define __config_h

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

/* The name of the theme */
char *config_theme;

/*! The number of desktops */
extern int config_desktops_num;
/*! Names for the desktops */
extern GSList *config_desktops_names;


void config_startup();
void config_shutdown();

#endif
