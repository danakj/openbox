#ifndef __screen_h
#define __screen_h

#include "misc.h"
#include "geom.h"

struct _ObClient;

#define DESKTOP_ALL (0xffffffff)

/*! The number of available desktops */
extern guint screen_num_desktops;
/*! The number of virtual "xinerama" screens/heads */
extern guint screen_num_monitors;
/*! The current desktop */
extern guint screen_desktop;
/*! The desktop which was last visible */
extern guint screen_last_desktop;
/*! Are we in showing-desktop mode? */
extern gboolean screen_showing_desktop;
/*! The support window also used for focus and stacking */
extern Window screen_support_win;

typedef struct DesktopLayout {
    ObOrientation orientation;
    ObCorner start_corner;
    guint rows;
    guint columns;
} DesktopLayout;
extern DesktopLayout screen_desktop_layout;

/*! An array of gchar*'s which are desktop names in UTF-8 format */
extern char **screen_desktop_names;

/*! Take over the screen, set the basic hints on it claming it as ours */
gboolean screen_annex();

/*! Once the screen is ours, set up its initial state */
void screen_startup(gboolean reconfig);
/*! Free resources */
void screen_shutdown(gboolean reconfig);

/*! Figure out the new size of the screen and adjust stuff for it */
void screen_resize();

/*! Change the number of available desktops */
void screen_set_num_desktops(guint num);
/*! Change the current desktop */
void screen_set_desktop(guint num);
/*! Interactively change desktops */
guint screen_cycle_desktop(ObDirection dir, gboolean wrap, gboolean linear,
                           gboolean dialog, gboolean done, gboolean cancel);

/*! Shows and focuses the desktop and hides all the client windows, or
  returns to the normal state, showing client windows. */
void screen_show_desktop(gboolean show);

/*! Updates the desktop layout from the root property if available */
void screen_update_layout();

/*! Get desktop names from the root window property */
void screen_update_desktop_names();

/*! Installs or uninstalls a colormap for a client. If client is NULL, then
  it handles the root colormap. */
void screen_install_colormap(struct _ObClient *client, gboolean install);

void screen_update_areas();

Rect *screen_physical_area();

Rect *screen_physical_area_monitor(guint head);

Rect *screen_area(guint desktop);

Rect *screen_area_monitor(guint desktop, guint head);

gboolean screen_pointer_pos(int *x, int *y);

#endif
