#include "cwmcc_internal.h"
#include "atom.h"
#include <glib.h>

struct TypeAtoms cwmcc_atom_type;
struct ClientAtoms cwmcc_atom_client;
struct RootAtoms cwmcc_atom_root;
struct DataAtoms cwmcc_atom_data;

#define CREATE(type, var, name) (cwmcc_atom_##type.var = \
                                 XInternAtom(cwmcc_display, name, FALSE))
#define SETVALUE(type, var, value) (cwmcc_atom_##type.var = value)

void atom_startup()
{
    CREATE(type, cardinal, "CARDINAL");
    CREATE(type, window, "WINDOW");
    CREATE(type, pixmap, "PIXMAP");
    CREATE(type, atom, "ATOM");
    CREATE(type, string, "STRING");
    CREATE(type, utf8, "UTF8_STRING");
     
    CREATE(client, wm_protocols, "WM_PROTOCOLS");
    CREATE(client, wm_state, "WM_STATE");
    CREATE(client, wm_name, "WM_NAME");
    CREATE(client, wm_icon_name, "WM_ICON_NAME");
    CREATE(client, wm_class, "WM_CLASS");
    CREATE(client, wm_window_role, "WM_WINDOW_ROLE");
    CREATE(client, motif_wm_hints, "_MOTIF_WM_HINTS");
    CREATE(client, net_wm_name, "_NET_WM_NAME");
    CREATE(client, net_wm_visible_name, "_NET_WM_VISIBLE_NAME");
    CREATE(client, net_wm_icon_name, "_NET_WM_ICON_NAME");
    CREATE(client, net_wm_visible_icon_name, "_NET_WM_VISIBLE_ICON_NAME");
    CREATE(client, net_wm_desktop, "_NET_WM_DESKTOP");
    CREATE(client, net_wm_window_type, "_NET_WM_WINDOW_TYPE");
    CREATE(client, net_wm_state, "_NET_WM_STATE");
    CREATE(client, net_wm_strut, "_NET_WM_STRUT");
    CREATE(client, net_wm_icon, "_NET_WM_ICON");
    CREATE(client, net_wm_allowed_actions, "_NET_WM_ALLOWED_ACTIONS");
    CREATE(client, kwm_win_icon, "KWM_WIN_ICON");
    CREATE(client, openbox_premax, "_OPENBOX_PREMAX");
    
    CREATE(root, net_supported, "_NET_SUPPORTED");
    CREATE(root, net_client_list, "_NET_CLIENT_LIST");
    CREATE(root, net_client_list_stacking, "_NET_CLIENT_LIST_STACKING");
    CREATE(root, net_number_of_desktops, "_NET_NUMBER_OF_DESKTOPS");
    CREATE(root, net_desktop_geometry, "_NET_DESKTOP_GEOMETRY");
    CREATE(root, net_desktop_viewport, "_NET_DESKTOP_VIEWPORT");
    CREATE(root, net_current_desktop, "_NET_CURRENT_DESKTOP");
    CREATE(root, net_desktop_names, "_NET_DESKTOP_NAMES");
    CREATE(root, net_active_window, "_NET_ACTIVE_WINDOW");
    CREATE(root, net_workarea, "_NET_WORKAREA");
    CREATE(root, net_supporting_wm_check, "_NET_SUPPORTING_WM_CHECK");
    CREATE(root, net_desktop_layout, "_NET_DESKTOP_LAYOUT");
    CREATE(root, net_showing_desktop, "_NET_SHOWING_DESKTOP");
    CREATE(root, openbox_pid, "_OPENBOX_PID");

    CREATE(data, wm_delete_window, "WM_DELETE_WINDOW");
    CREATE(data, wm_take_focus, "WM_TAKE_FOCUS");
    CREATE(data, wm_change_state, "WM_CHANGE_STATE");
    CREATE(data, net_close_window, "_NET_CLOSE_WINDOW");
    CREATE(data, net_wm_moveresize, "_NET_WM_MOVERESIZE");

    CREATE(data, net_wm_window_type_desktop, "_NET_WM_WINDOW_TYPE_DESKTOP");
    CREATE(data, net_wm_window_type_dock, "_NET_WM_WINDOW_TYPE_DOCK");
    CREATE(data, net_wm_window_type_toolbar, "_NET_WM_WINDOW_TYPE_TOOLBAR");
    CREATE(data, net_wm_window_type_menu, "_NET_WM_WINDOW_TYPE_MENU");
    CREATE(data, net_wm_window_type_utility, "_NET_WM_WINDOW_TYPE_UTILITY");
    CREATE(data, net_wm_window_type_splash, "_NET_WM_WINDOW_TYPE_SPLASH");
    CREATE(data, net_wm_window_type_dialog, "_NET_WM_WINDOW_TYPE_DIALOG");
    CREATE(data, net_wm_window_type_normal, "_NET_WM_WINDOW_TYPE_NORMAL");
    CREATE(data, kde_net_wm_window_type_override,
           "_KDE_NET_WM_WINDOW_TYPE_OVERRIDE");

    SETVALUE(data, net_wm_moveresize_size_topleft, 0);
    SETVALUE(data, net_wm_moveresize_size_topright, 2);
    SETVALUE(data, net_wm_moveresize_size_bottomright, 4);
    SETVALUE(data, net_wm_moveresize_size_bottomleft, 6);
    SETVALUE(data, net_wm_moveresize_move, 8);

    CREATE(data, net_wm_action_move, "_NET_WM_ACTION_MOVE");
    CREATE(data, net_wm_action_resize, "_NET_WM_ACTION_RESIZE");
    CREATE(data, net_wm_action_minimize, "_NET_WM_ACTION_MINIMIZE");
    CREATE(data, net_wm_action_shade, "_NET_WM_ACTION_SHADE");
    CREATE(data, net_wm_action_stick, "_NET_WM_ACTION_STICK");
    CREATE(data, net_wm_action_maximize_horz, "_NET_WM_ACTION_MAXIMIZE_HORZ");
    CREATE(data, net_wm_action_maximize_vert, "_NET_WM_ACTION_MAXIMIZE_VERT");
    CREATE(data, net_wm_action_fullscreen, "_NET_WM_ACTION_FULLSCREEN");
    CREATE(data, net_wm_action_change_desktop,"_NET_WM_ACTION_CHANGE_DESKTOP");
    CREATE(data, net_wm_action_close, "_NET_WM_ACTION_CLOSE");

    CREATE(data, net_wm_state_modal, "_NET_WM_STATE_MODAL");
    CREATE(data, net_wm_state_sticky, "_NET_WM_STATE_STICKY");
    CREATE(data, net_wm_state_maximized_vert, "_NET_WM_STATE_MAXIMIZED_VERT");
    CREATE(data, net_wm_state_maximized_horz, "_NET_WM_STATE_MAXIMIZED_HORZ");
    CREATE(data, net_wm_state_shaded, "_NET_WM_STATE_SHADED");
    CREATE(data, net_wm_state_skip_taskbar, "_NET_WM_STATE_SKIP_TASKBAR");
    CREATE(data, net_wm_state_skip_pager, "_NET_WM_STATE_SKIP_PAGER");
    CREATE(data, net_wm_state_hidden, "_NET_WM_STATE_HIDDEN");
    CREATE(data, net_wm_state_fullscreen, "_NET_WM_STATE_FULLSCREEN");
    CREATE(data, net_wm_state_above, "_NET_WM_STATE_ABOVE");
    CREATE(data, net_wm_state_below, "_NET_WM_STATE_BELOW");

    SETVALUE(data, net_wm_state_remove, 0);
    SETVALUE(data, net_wm_state_add, 1);
    SETVALUE(data, net_wm_state_toggle, 2);

    SETVALUE(data, net_wm_orientation_horz, 0);
    SETVALUE(data, net_wm_orientation_vert, 1);
    SETVALUE(data, net_wm_topleft, 0);
    SETVALUE(data, net_wm_topright, 1);
    SETVALUE(data, net_wm_bottomright, 2);
    SETVALUE(data, net_wm_bottomleft, 3);
}
