#include "prop.h"
#include "openbox.h"
#include <X11/Xatom.h>

Atoms prop_atoms;

#define CREATE(var, name) (prop_atoms.var = \
                           XInternAtom(ob_display, name, FALSE))

void prop_startup()
{
    g_assert(ob_display != NULL);

    CREATE(cardinal, "CARDINAL");
    CREATE(window, "WINDOW");
    CREATE(pixmap, "PIXMAP");
    CREATE(atom, "ATOM");
    CREATE(string, "STRING");
    CREATE(utf8, "UTF8_STRING");
     
    CREATE(wm_colormap_windows, "WM_COLORMAP_WINDOWS");
    CREATE(wm_protocols, "WM_PROTOCOLS");
    CREATE(wm_state, "WM_STATE");
    CREATE(wm_change_state, "WM_CHANGE_STATE");
    CREATE(wm_delete_window, "WM_DELETE_WINDOW");
    CREATE(wm_take_focus, "WM_TAKE_FOCUS");
    CREATE(wm_name, "WM_NAME");
    CREATE(wm_icon_name, "WM_ICON_NAME");
    CREATE(wm_class, "WM_CLASS");
    CREATE(wm_window_role, "WM_WINDOW_ROLE");
    CREATE(motif_wm_hints, "_MOTIF_WM_HINTS");

    CREATE(net_supported, "_NET_SUPPORTED");
    CREATE(net_client_list, "_NET_CLIENT_LIST");
    CREATE(net_client_list_stacking, "_NET_CLIENT_LIST_STACKING");
    CREATE(net_number_of_desktops, "_NET_NUMBER_OF_DESKTOPS");
    CREATE(net_desktop_geometry, "_NET_DESKTOP_GEOMETRY");
    CREATE(net_desktop_viewport, "_NET_DESKTOP_VIEWPORT");
    CREATE(net_current_desktop, "_NET_CURRENT_DESKTOP");
    CREATE(net_desktop_names, "_NET_DESKTOP_NAMES");
    CREATE(net_active_window, "_NET_ACTIVE_WINDOW");
    CREATE(net_workarea, "_NET_WORKAREA");
    CREATE(net_supporting_wm_check, "_NET_SUPPORTING_WM_CHECK");
/*   CREATE(net_virtual_roots, "_NET_VIRTUAL_ROOTS"); */
    CREATE(net_desktop_layout, "_NET_DESKTOP_LAYOUT");
    CREATE(net_showing_desktop, "_NET_SHOWING_DESKTOP");

    CREATE(net_close_window, "_NET_CLOSE_WINDOW");
    CREATE(net_wm_moveresize, "_NET_WM_MOVERESIZE");

/*   CREATE(net_properties, "_NET_PROPERTIES"); */
    CREATE(net_wm_name, "_NET_WM_NAME");
    CREATE(net_wm_visible_name, "_NET_WM_VISIBLE_NAME");
    CREATE(net_wm_icon_name, "_NET_WM_ICON_NAME");
    CREATE(net_wm_visible_icon_name, "_NET_WM_VISIBLE_ICON_NAME");
    CREATE(net_wm_desktop, "_NET_WM_DESKTOP");
    CREATE(net_wm_window_type, "_NET_WM_WINDOW_TYPE");
    CREATE(net_wm_state, "_NET_WM_STATE");
    CREATE(net_wm_strut, "_NET_WM_STRUT");
/*   CREATE(net_wm_icon_geometry, "_NET_WM_ICON_GEOMETRY"); */
    CREATE(net_wm_icon, "_NET_WM_ICON");
/*   CREATE(net_wm_pid, "_NET_WM_PID"); */
/*   CREATE(net_wm_handled_icons, "_NET_WM_HANDLED_ICONS"); */
    CREATE(net_wm_allowed_actions, "_NET_WM_ALLOWED_ACTIONS");

/*   CREATE(net_wm_ping, "_NET_WM_PING"); */
  
    CREATE(net_wm_window_type_desktop, "_NET_WM_WINDOW_TYPE_DESKTOP");
    CREATE(net_wm_window_type_dock, "_NET_WM_WINDOW_TYPE_DOCK");
    CREATE(net_wm_window_type_toolbar, "_NET_WM_WINDOW_TYPE_TOOLBAR");
    CREATE(net_wm_window_type_menu, "_NET_WM_WINDOW_TYPE_MENU");
    CREATE(net_wm_window_type_utility, "_NET_WM_WINDOW_TYPE_UTILITY");
    CREATE(net_wm_window_type_splash, "_NET_WM_WINDOW_TYPE_SPLASH");
    CREATE(net_wm_window_type_dialog, "_NET_WM_WINDOW_TYPE_DIALOG");
    CREATE(net_wm_window_type_normal, "_NET_WM_WINDOW_TYPE_NORMAL");

    CREATE(net_wm_moveresize_size_topleft, "_NET_WM_MOVERESIZE_SIZE_TOPLEFT");
    CREATE(net_wm_moveresize_size_topright,
	   "_NET_WM_MOVERESIZE_SIZE_TOPRIGHT");
    CREATE(net_wm_moveresize_size_bottomleft,
	   "_NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT");
    CREATE(net_wm_moveresize_size_bottomright,
	   "_NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT");
    CREATE(net_wm_moveresize_move, "_NET_WM_MOVERESIZE_MOVE");
 
    CREATE(net_wm_action_move, "_NET_WM_ACTION_MOVE");
    CREATE(net_wm_action_resize, "_NET_WM_ACTION_RESIZE");
    CREATE(net_wm_action_minimize, "_NET_WM_ACTION_MINIMIZE");
    CREATE(net_wm_action_shade, "_NET_WM_ACTION_SHADE");
    CREATE(net_wm_action_stick, "_NET_WM_ACTION_STICK");
    CREATE(net_wm_action_maximize_horz, "_NET_WM_ACTION_MAXIMIZE_HORZ");
    CREATE(net_wm_action_maximize_vert, "_NET_WM_ACTION_MAXIMIZE_VERT");
    CREATE(net_wm_action_fullscreen, "_NET_WM_ACTION_FULLSCREEN");
    CREATE(net_wm_action_change_desktop, "_NET_WM_ACTION_CHANGE_DESKTOP");
    CREATE(net_wm_action_close, "_NET_WM_ACTION_CLOSE");
    CREATE(net_wm_state_modal, "_NET_WM_STATE_MODAL");
    CREATE(net_wm_state_sticky, "_NET_WM_STATE_STICKY");
    CREATE(net_wm_state_maximized_vert, "_NET_WM_STATE_MAXIMIZED_VERT");
    CREATE(net_wm_state_maximized_horz, "_NET_WM_STATE_MAXIMIZED_HORZ");
    CREATE(net_wm_state_shaded, "_NET_WM_STATE_SHADED");
    CREATE(net_wm_state_skip_taskbar, "_NET_WM_STATE_SKIP_TASKBAR");
    CREATE(net_wm_state_skip_pager, "_NET_WM_STATE_SKIP_PAGER");
    CREATE(net_wm_state_hidden, "_NET_WM_STATE_HIDDEN");
    CREATE(net_wm_state_fullscreen, "_NET_WM_STATE_FULLSCREEN");
    CREATE(net_wm_state_above, "_NET_WM_STATE_ABOVE");
    CREATE(net_wm_state_below, "_NET_WM_STATE_BELOW");
  
    prop_atoms.net_wm_state_add = 1;
    prop_atoms.net_wm_state_remove = 0;
    prop_atoms.net_wm_state_toggle = 2;

    prop_atoms.net_wm_orientation_horz = 0;
    prop_atoms.net_wm_orientation_vert = 1;
    prop_atoms.net_wm_topleft = 0;
    prop_atoms.net_wm_topright = 1;
    prop_atoms.net_wm_bottomright = 2;
    prop_atoms.net_wm_bottomleft = 3;

    CREATE(kde_net_system_tray_windows, "_KDE_NET_SYSTEM_TRAY_WINDOWS");
    CREATE(kde_net_wm_system_tray_window_for,
	   "_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR");
    CREATE(kde_net_wm_window_type_override,
	   "_KDE_NET_WM_WINDOW_TYPE_OVERRIDE");

    CREATE(kwm_win_icon, "KWM_WIN_ICON");
  
    CREATE(rootpmapid, "_XROOTPMAP_ID");
    CREATE(esetrootid, "ESETROOT_PMAP_ID");

    CREATE(openbox_pid, "_OPENBOX_PID");
    CREATE(openbox_premax, "_OPENBOX_PREMAX");
}

gboolean prop_get(Window win, Atom prop, Atom type, int size,
		  guchar **data, gulong num)
{
    gboolean ret = FALSE;
    int res;
    guchar *xdata = NULL;
    Atom ret_type;
    int ret_size;
    gulong ret_items, bytes_left;
    long num32 = 32 / size * num; /* num in 32-bit elements */

    res = XGetWindowProperty(ob_display, win, prop, 0l, num32,
			     FALSE, type, &ret_type, &ret_size,
			     &ret_items, &bytes_left, &xdata);
    if (res == Success && ret_items && xdata) {
	if (ret_size == size && ret_items >= num) {
	    *data = g_memdup(xdata, num * (size / 8));
	    ret = TRUE;
	}
	XFree(xdata);
    }
    return ret;
}

gboolean prop_get_prealloc(Window win, Atom prop, Atom type, int size,
			   guchar *data, gulong num)
{
    gboolean ret = FALSE;
    int res;
    guchar *xdata = NULL;
    Atom ret_type;
    int ret_size;
    gulong ret_items, bytes_left;
    long num32 = 32 / size * num; /* num in 32-bit elements */

    res = XGetWindowProperty(ob_display, win, prop, 0l, num32,
			     FALSE, type, &ret_type, &ret_size,
			     &ret_items, &bytes_left, &xdata);
    if (res == Success && ret_items && xdata) {
	if (ret_size == size && ret_items >= num) {
	    gulong i;
	    for (i = 0; i < num; ++i)
		switch (size) {
		case 8:
		    data[i] = xdata[i];
		    break;
		case 16:
		    ((guint16*)data)[i] = ((guint16*)xdata)[i];
		    break;
		case 32:
		    ((guint32*)data)[i] = ((guint32*)xdata)[i];
		    break;
		default:
		    g_assert_not_reached(); /* unhandled size */
		}
	    ret = TRUE;
	}
	XFree(xdata);
    }
    return ret;
}

gboolean prop_get_all(Window win, Atom prop, Atom type, int size,
		      guchar **data, gulong *num)
{
    gboolean ret = FALSE;
    int res;
    guchar *xdata = NULL;
    Atom ret_type;
    int ret_size;
    gulong ret_items, bytes_left;

    res = XGetWindowProperty(ob_display, win, prop, 0l, G_MAXLONG,
			     FALSE, type, &ret_type, &ret_size,
			     &ret_items, &bytes_left, &xdata);
    if (res == Success) {
	if (ret_size == size && ret_items > 0) {
	    *data = g_memdup(xdata, ret_items * (size / 8));
	    *num = ret_items;
	    ret = TRUE;
	}
	XFree(xdata);
    }
    return ret;
}

gboolean prop_get_string(Window win, Atom prop, Atom type, guchar **data)
{
    guchar *raw;
    gulong num;
    GString *str;
     
    if (prop_get_all(win, prop, type, 8, &raw, &num)) {
	str = g_string_new_len((char*)raw, num);
	g_assert(str->str[num] == '\0');

	g_free(raw);

	*data = (guchar*)g_string_free(str, FALSE);
	return TRUE;
    }
    return FALSE;
}

gboolean prop_get_strings(Window win, Atom prop, Atom type,
			  GPtrArray *data)
{
    guchar *raw;
    gulong num;
    GString *str, *str2;
    guint i, start;
     
    if (prop_get_all(win, prop, type, 8, &raw, &num)) {
	str = g_string_new_len((gchar*)raw, num);
	g_assert(str->str[num] == '\0'); /* assuming this is always true.. */

	g_free(raw);

	/* split it into the list */
	for (start = 0, i = 0; i < str->len; ++i) {
	    if (str->str[i] == '\0') {
		str2 = g_string_new_len(&str->str[start], i - start);
		g_ptr_array_add(data, g_string_free(str2, FALSE));
		start = i + 1;
	    }
	}
	g_string_free(str, TRUE);

	if (data->len > 0)
	    return TRUE;
    }
    return FALSE;
}

void prop_set_strings(Window win, Atom prop, Atom type, GPtrArray *data)
{
    GString *str;
    guint i;

    str = g_string_sized_new(0);
    for (i = 0; i < data->len; ++i) {
        str = g_string_append(str, data->pdata[i]);
        str = g_string_append_c(str, '\0');
    }
    XChangeProperty(ob_display, win, prop, type, 8,
                    PropModeReplace, (guchar*)str->str, str->len);
}

void prop_erase(Window win, Atom prop)
{
    XDeleteProperty(ob_display, win, prop);
}

void prop_message(Window about, Atom messagetype, long data0, long data1,
		  long data2, long data3)
{
    XEvent ce;
    ce.xclient.type = ClientMessage;
    ce.xclient.message_type = messagetype;
    ce.xclient.display = ob_display;
    ce.xclient.window = about;
    ce.xclient.format = 32;
    ce.xclient.data.l[0] = data0;
    ce.xclient.data.l[1] = data1;
    ce.xclient.data.l[2] = data2;
    ce.xclient.data.l[3] = data3;
    XSendEvent(ob_display, ob_root, FALSE,
	       SubstructureNotifyMask | SubstructureRedirectMask, &ce);
}
