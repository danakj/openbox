#ifndef __atoms_h
#define __atoms_h

#include <X11/Xlib.h>
#include <glib.h>
#ifdef HAVE_STRING_H
#  include <string.h>
#endif

/*! The atoms on the X server which this class will cache */
typedef struct Atoms {
    /* types */
    Atom cardinal; /*!< The atom which represents the Cardinal data type */
    Atom window;   /*!< The atom which represents window ids */
    Atom pixmap;   /*!< The atom which represents pixmap ids */
    Atom atom;     /*!< The atom which represents atom values */
    Atom string;   /*!< The atom which represents ascii strings */
    Atom utf8;     /*!< The atom which represents utf8-encoded strings */

    /* selection stuff */
    Atom manager;

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

    /* SM atoms */
    Atom sm_client_id;

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
    Atom net_desktop_layout;
    Atom net_showing_desktop;
    /* root window messages */
    Atom net_close_window;
    Atom net_wm_moveresize;
    Atom net_moveresize_window;

    /* startup-notification extension */
    Atom net_startup_id;

    /* application window properties */
    Atom net_wm_name;
    Atom net_wm_visible_name;
    Atom net_wm_icon_name;
    Atom net_wm_visible_icon_name;
    Atom net_wm_desktop;
    Atom net_wm_window_type;
    Atom net_wm_state;
    Atom net_wm_strut;
    Atom net_wm_strut_partial;
    Atom net_wm_icon;
/*  Atom net_wm_pid; */
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
    Atom net_wm_moveresize_size_top;
    Atom net_wm_moveresize_size_topright;
    Atom net_wm_moveresize_size_right;
    Atom net_wm_moveresize_size_bottomright;
    Atom net_wm_moveresize_size_bottom;
    Atom net_wm_moveresize_size_bottomleft;
    Atom net_wm_moveresize_size_left;
    Atom net_wm_moveresize_move;
    Atom net_wm_moveresize_size_keyboard;
    Atom net_wm_moveresize_move_keyboard;

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

gboolean prop_get32(Window win, Atom prop, Atom type, guint32 *ret);
gboolean prop_get_array32(Window win, Atom prop, Atom type, guint32 **ret,
                          guint *nret);
gboolean prop_get_string_locale(Window win, Atom prop, char **ret);
gboolean prop_get_string_utf8(Window win, Atom prop, char **ret);
gboolean prop_get_strings_locale(Window win, Atom prop, char ***ret);
gboolean prop_get_strings_utf8(Window win, Atom prop, char ***ret);

void prop_set32(Window win, Atom prop, Atom type, guint32 val);
void prop_set_array32(Window win, Atom prop, Atom type, guint32 *val,
                      guint num);
void prop_set_string_utf8(Window win, Atom prop, char *val);
void prop_set_strings_utf8(Window win, Atom prop, char **strs);

void prop_erase(Window win, Atom prop);

void prop_message(Window about, Atom messagetype, long data0, long data1,
		  long data2, long data3, long mask);

#define PROP_GET32(win, prop, type, ret) \
    (prop_get32(win, prop_atoms.prop, prop_atoms.type, ret))
#define PROP_GETA32(win, prop, type, ret, nret) \
    (prop_get_array32(win, prop_atoms.prop, prop_atoms.type, ret, \
                      nret))
#define PROP_GETS(win, prop, type, ret) \
    (prop_get_string_##type(win, prop_atoms.prop, ret))
#define PROP_GETSS(win, prop, type, ret) \
    (prop_get_strings_##type(win, prop_atoms.prop, ret))

#define PROP_SET32(win, prop, type, val) \
    prop_set32(win, prop_atoms.prop, prop_atoms.type, val)
#define PROP_SETA32(win, prop, type, val, num) \
    prop_set_array32(win, prop_atoms.prop, prop_atoms.type, val, num)
#define PROP_SETS(win, prop, val) \
    prop_set_string_utf8(win, prop_atoms.prop, val)
#define PROP_SETSS(win, prop, strs) \
    prop_set_strings_utf8(win, prop_atoms.prop, strs)

#define PROP_ERASE(win, prop) prop_erase(win, prop_atoms.prop)

#define PROP_MSG(about, msgtype, data0, data1, data2, data3) \
  (prop_message(about, prop_atoms.msgtype, data0, data1, data2, data3, \
                SubstructureNotifyMask | SubstructureRedirectMask))

#endif
