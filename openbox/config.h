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

/*! The number of desktops */
extern int config_desktops_num;
/*! Names for the desktops */
extern GSList *config_desktops_names;


void config_startup();
void config_shutdown();

#endif
