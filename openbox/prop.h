#ifndef __atoms_h
#define __atoms_h

#include <X11/Xlib.h>
#include <glib.h>
#ifdef HAVE_STRING_H
#  include <string.h>
#endif

#include "openbox.h"

/*! The atoms on the X server which this class will cache */
typedef struct Atoms {
    /* types */
    Atom cardinal; /*!< The atom which represents the Cardinal data type */
    Atom window;   /*!< The atom which represents window ids */
    Atom pixmap;   /*!< The atom which represents pixmap ids */
    Atom atom;     /*!< The atom which represents atom values */
    Atom string;   /*!< The atom which represents ascii strings */
    Atom utf8;     /*!< The atom which represents utf8-encoded strings */

    /* window hints */
    Atom wm_colormap_windows;
    Atom wm_protocols;
    Atom wm_state;
    Atom wm_delete_window;
    Atom wm_take_focus;
    Atom wm_change_state;
    Atom wm_name;
    Atom wm_icon_name;
    Atom wm_class;
    Atom wm_window_role;
    Atom motif_wm_hints;

    /* NETWM atoms */
     
    /* root window properties */
    Atom net_supported;
    Atom net_client_list;
    Atom net_client_list_stacking;
    Atom net_number_of_desktops;
    Atom net_desktop_geometry;
    Atom net_desktop_viewport;
    Atom net_current_desktop;
    Atom net_desktop_names;
    Atom net_active_window;
    Atom net_workarea;
    Atom net_supporting_wm_check;
/*  Atom net_virtual_roots; */
    Atom net_desktop_layout;
    Atom net_showing_desktop;
    /* root window messages */
    Atom net_close_window;
    Atom net_wm_moveresize;
    /* application window properties */
/*  Atom net_properties; */
    Atom net_wm_name;
    Atom net_wm_visible_name;
    Atom net_wm_icon_name;
    Atom net_wm_visible_icon_name;
    Atom net_wm_desktop;
    Atom net_wm_window_type;
    Atom net_wm_state;
    Atom net_wm_strut;
/*  Atom net_wm_icon_geometry; */
    Atom net_wm_icon;
/*  Atom net_wm_pid; */
/*  Atom net_wm_handled_icons; */
    Atom net_wm_allowed_actions;
    /* application protocols */
/*  Atom   Atom net_wm_ping; */

    Atom net_wm_window_type_desktop;
    Atom net_wm_window_type_dock;
    Atom net_wm_window_type_toolbar;
    Atom net_wm_window_type_menu;
    Atom net_wm_window_type_utility;
    Atom net_wm_window_type_splash;
    Atom net_wm_window_type_dialog;
    Atom net_wm_window_type_normal;

    Atom net_wm_moveresize_size_topleft;
    Atom net_wm_moveresize_size_topright;
    Atom net_wm_moveresize_size_bottomleft;
    Atom net_wm_moveresize_size_bottomright;
    Atom net_wm_moveresize_move;

    Atom net_wm_action_move;
    Atom net_wm_action_resize;
    Atom net_wm_action_minimize;
    Atom net_wm_action_shade;
    Atom net_wm_action_stick;
    Atom net_wm_action_maximize_horz;
    Atom net_wm_action_maximize_vert;
    Atom net_wm_action_fullscreen;
    Atom net_wm_action_change_desktop;
    Atom net_wm_action_close;

    Atom net_wm_state_modal;
    Atom net_wm_state_sticky;
    Atom net_wm_state_maximized_vert;
    Atom net_wm_state_maximized_horz;
    Atom net_wm_state_shaded;
    Atom net_wm_state_skip_taskbar;
    Atom net_wm_state_skip_pager;
    Atom net_wm_state_hidden;
    Atom net_wm_state_fullscreen;
    Atom net_wm_state_above;
    Atom net_wm_state_below;

    Atom net_wm_state_add;
    Atom net_wm_state_remove;
    Atom net_wm_state_toggle;

    Atom net_wm_orientation_horz;
    Atom net_wm_orientation_vert;
    Atom net_wm_topleft;
    Atom net_wm_topright;
    Atom net_wm_bottomright;
    Atom net_wm_bottomleft;

    /* Extra atoms */

    Atom kde_net_system_tray_windows;
    Atom kde_net_wm_system_tray_window_for;
    Atom kde_net_wm_window_type_override;

    Atom kwm_win_icon;

    Atom rootpmapid;
    Atom esetrootid;

    /* Openbox specific atoms */
     
    Atom openbox_pid;
    Atom openbox_premax;
} Atoms;
Atoms prop_atoms;

void prop_startup();

gboolean prop_get32(Window win, Atom prop, Atom type,
                    gulong **data, gulong num);

gboolean prop_get_prealloc(Window win, Atom prop, Atom type, int size,
			   guchar *data, gulong num);

gboolean prop_get_all(Window win, Atom prop, Atom type, int size,
		      guchar **data, gulong *num);

gboolean prop_get_string(Window win, Atom prop, Atom type, guchar **data);
gboolean prop_get_strings(Window win, Atom prop, Atom type,
			  GPtrArray *data);

void prop_set_strings(Window win, Atom prop, Atom type, GPtrArray *data);

void prop_erase(Window win, Atom prop);

void prop_message(Window about, Atom messagetype, long data0, long data1,
		  long data2, long data3);

#define PROP_MSG(about, msgtype, data0, data1, data2, data3) \
  (prop_message(about, prop_atoms.msgtype, data0, data1, data2, data3))

/* Set an 8-bit property from a string */
#define PROP_SETS(win, prop, type, value) \
  (XChangeProperty(ob_display, win, prop_atoms.prop, prop_atoms.type, 8, \
		   PropModeReplace, (guchar*)value, strlen(value)))
/* Set an 8-bit property array from a GPtrArray of strings */
#define PROP_SETSA(win, prop, type, value) \
  (prop_set_strings(win, prop_atoms.prop, prop_atoms.type, value))

/* Set a 32-bit property from a single value */
#define PROP_SET32(win, prop, type, value) \
  (XChangeProperty(ob_display, win, prop_atoms.prop, prop_atoms.type, 32, \
		   PropModeReplace, (guchar*)&value, 1))
/* Set a 32-bit property from an array */
#define PROP_SET32A(win, prop, type, value, num) \
  (XChangeProperty(ob_display, win, prop_atoms.prop, prop_atoms.type, 32, \
		   PropModeReplace, (guchar*)value, num))

/* Get an 8-bit property into a string */
#define PROP_GETS(win, prop, type, value) \
  (prop_get_string(win, prop_atoms.prop, prop_atoms.type, \
       	           (guchar**)&value))
/* Get an 8-bit property into a GPtrArray of strings
   (The strings must be freed, the GPtrArray must already be created.) */
#define PROP_GETSA(win, prop, type, value) \
  (prop_get_strings(win, prop_atoms.prop, prop_atoms.type, \
       	            value))

/* Get an entire 8-bit property into an array (which must be freed) */
#define PROP_GET8U(win, prop, type, value, num) \
  (prop_get_all(win, prop_atoms.prop, prop_atoms.type, 8, \
                (guchar**)&value, &num))

/* Get 1 element of a 32-bit property into a given variable */
#define PROP_GET32(win, prop, type, value) \
  (prop_get_prealloc(win, prop_atoms.prop, prop_atoms.type, 32, \
	             (guchar*)&value, 1))

/* Get an amount of a 32-bit property into an array (which must be freed) */
#define PROP_GET32A(win, prop, type, value, num) \
  (prop_get32(win, prop_atoms.prop, prop_atoms.type, (gulong**)&value, num))

/* Get an entire 32-bit property into an array (which must be freed) */
#define PROP_GET32U(win, prop, type, value, num) \
  (prop_get_all(win, prop_atoms.prop, prop_atoms.type, 32, \
	        (guchar**)&value, &num))

#define PROP_ERASE(win, prop) (prop_erase(win, prop_atoms.prop))

#endif
