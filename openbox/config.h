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

/*! The engine to load */
extern char *config_engine_name;
/*! The theme to load */
extern char *config_engine_theme;
/*! The titlebar layout */
extern char *config_engine_layout;
/*! The titlebar font */
extern char *config_engine_font;
/*! The titlebar font's shadow */
extern gboolean config_engine_shadow;
/*! The titlebar font's shadow offset */
extern int config_engine_shadow_offset;
/*! The titlebar font's shadow transparency */
extern int config_engine_shadow_tint;

/*! The number of desktops */
extern int config_desktops_num;
/*! Names for the desktops */
extern GSList *config_desktops_names;


void config_startup();
void config_shutdown();

#endif
