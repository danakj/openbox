#ifndef __cwmcc_atom_h
#define __cwmcc_atom_h

#include <X11/Xlib.h>

/*! Atoms for basic data types for properties */
struct TypeAtoms {
    Atom cardinal; /*!< The atom which represents the Cardinal data type */
    Atom window;   /*!< The atom which represents window ids */
    Atom pixmap;   /*!< The atom which represents pixmap ids */
    Atom atom;     /*!< The atom which represents atom values */
    Atom string;   /*!< The atom which represents ascii strings */
    Atom utf8;     /*!< The atom which represents utf8-encoded strings */
};

/*! Atoms for client window properties */
struct ClientAtoms {
    Atom wm_protocols;
    Atom wm_state;
    Atom wm_name;
    Atom wm_icon_name;
    Atom wm_class;
    Atom wm_window_role;
    Atom motif_wm_hints;
    Atom net_wm_name;
    Atom net_wm_visible_name;
    Atom net_wm_icon_name;
    Atom net_wm_visible_icon_name;
    Atom net_wm_desktop;
    Atom net_wm_window_type;
    Atom net_wm_state;
    Atom net_wm_strut;
    Atom net_wm_icon;
    Atom net_wm_allowed_actions;
    Atom kwm_win_icon;
    Atom openbox_premax;
};

/*! Atoms for root window properties */
struct RootAtoms {
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
    Atom openbox_pid;
};

/*! Atoms used for protocols or client messages, or for setting as values of
  properties */
struct DataAtoms {
    /* window hints */
    Atom wm_delete_window;
    Atom wm_take_focus;
    Atom wm_change_state;
    Atom net_close_window;
    Atom net_wm_moveresize;

    Atom net_wm_window_type_desktop;
    Atom net_wm_window_type_dock;
    Atom net_wm_window_type_toolbar;
    Atom net_wm_window_type_menu;
    Atom net_wm_window_type_utility;
    Atom net_wm_window_type_splash;
    Atom net_wm_window_type_dialog;
    Atom net_wm_window_type_normal;
    Atom kde_net_wm_window_type_override;

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
};

extern struct TypeAtoms cwmcc_atom_type;
extern struct ClientAtoms cwmcc_atom_client;
extern struct RootAtoms cwmcc_atom_root;
extern struct DataAtoms cwmcc_atom_data;

#define CWMCC_ATOM(type, name) (cwmcc_atom_##type.name)

void atom_startup();

#endif
