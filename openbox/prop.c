#include "prop.h"
#include "openbox.h"

#include <X11/Xatom.h>

Atoms prop_atoms;

#define CREATE(var, name) (prop_atoms.var = \
                           XInternAtom(ob_display, name, FALSE))

void prop_startup()
{
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
    CREATE(net_desktop_layout, "_NET_DESKTOP_LAYOUT");
    CREATE(net_showing_desktop, "_NET_SHOWING_DESKTOP");

    CREATE(net_close_window, "_NET_CLOSE_WINDOW");
    CREATE(net_wm_moveresize, "_NET_WM_MOVERESIZE");

    CREATE(net_wm_name, "_NET_WM_NAME");
    CREATE(net_wm_visible_name, "_NET_WM_VISIBLE_NAME");
    CREATE(net_wm_icon_name, "_NET_WM_ICON_NAME");
    CREATE(net_wm_visible_icon_name, "_NET_WM_VISIBLE_ICON_NAME");
    CREATE(net_wm_desktop, "_NET_WM_DESKTOP");
    CREATE(net_wm_window_type, "_NET_WM_WINDOW_TYPE");
    CREATE(net_wm_state, "_NET_WM_STATE");
    CREATE(net_wm_strut, "_NET_WM_STRUT");
    CREATE(net_wm_icon, "_NET_WM_ICON");
/*   CREATE(net_wm_pid, "_NET_WM_PID"); */
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

    prop_atoms.net_wm_moveresize_size_topleft = 0;
    prop_atoms.net_wm_moveresize_size_top = 1;
    prop_atoms.net_wm_moveresize_size_topright = 2;
    prop_atoms.net_wm_moveresize_size_right = 3;
    prop_atoms.net_wm_moveresize_size_bottomright = 4;
    prop_atoms.net_wm_moveresize_size_bottom = 5;
    prop_atoms.net_wm_moveresize_size_bottomleft = 6;
    prop_atoms.net_wm_moveresize_size_left = 7;
    prop_atoms.net_wm_moveresize_move = 8;
    prop_atoms.net_wm_moveresize_size_keyboard = 9;
    prop_atoms.net_wm_moveresize_move_keyboard = 10;

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

#include <X11/Xutil.h>
#include <glib.h>
#include <string.h>

/* this just isn't used... and it also breaks on 64bit, watch out
static gboolean get(Window win, Atom prop, Atom type, int size,
                    guchar **data, gulong num)
{
    gboolean ret = FALSE;
    int res;
    guchar *xdata = NULL;
    Atom ret_type;
    int ret_size;
    gulong ret_items, bytes_left;
    long num32 = 32 / size * num; /\* num in 32-bit elements *\/

    res = XGetWindowProperty(display, win, prop, 0l, num32,
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
*/

static gboolean get_prealloc(Window win, Atom prop, Atom type, int size,
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
	    guint i;
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

static gboolean get_all(Window win, Atom prop, Atom type, int size,
                        guchar **data, guint *num)
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
	    guint i;

	    *data = g_malloc(ret_items * (size / 8));
	    for (i = 0; i < ret_items; ++i)
		switch (size) {
		case 8:
		    (*data)[i] = xdata[i];
		    break;
		case 16:
		    ((guint16*)*data)[i] = ((guint16*)xdata)[i];
		    break;
		case 32:
		    ((guint32*)*data)[i] = ((guint32*)xdata)[i];
		    break;
		default:
		    g_assert_not_reached(); /* unhandled size */
		}
	    *num = ret_items;
	    ret = TRUE;
	}
	XFree(xdata);
    }
    return ret;
}

static gboolean get_stringlist(Window win, Atom prop, char ***list, int *nstr)
{
    XTextProperty tprop;
    gboolean ret = FALSE;

    if (XGetTextProperty(ob_display, win, &tprop, prop) && tprop.nitems) {
        if (XTextPropertyToStringList(&tprop, list, nstr))
            ret = TRUE;
        XFree(tprop.value);
    }
    return ret;
}

gboolean prop_get32(Window win, Atom prop, Atom type, guint32 *ret)
{
    return get_prealloc(win, prop, type, 32, (guchar*)ret, 1);
}

gboolean prop_get_array32(Window win, Atom prop, Atom type, guint32 **ret,
                          guint *nret)
{
    return get_all(win, prop, type, 32, (guchar**)ret, nret);
}

gboolean prop_get_string_locale(Window win, Atom prop, char **ret)
{
    char **list;
    int nstr;

    if (get_stringlist(win, prop, &list, &nstr) && nstr) {
        *ret = g_locale_to_utf8(list[0], -1, NULL, NULL, NULL);
        XFreeStringList(list);
        if (ret) return TRUE;
    }
    return FALSE;
}

gboolean prop_get_strings_locale(Window win, Atom prop, char ***ret)
{
    char *raw, *p;
    guint num, i;

    if (get_all(win, prop, prop_atoms.string, 8, (guchar**)&raw, &num)){
        *ret = g_new(char*, num + 1);
        (*ret)[num] = NULL; /* null terminated list */

        p = raw;
        for (i = 0; i < num; ++i) {
            (*ret)[i] = g_locale_to_utf8(p, -1, NULL, NULL, NULL);
            /* make sure translation did not fail */
            if (!(*ret)[i]) {
                g_strfreev(*ret); /* free what we did so far */
                break; /* the force is not strong with us */
            }
            p = strchr(p, '\0');
        }
	g_free(raw);
        if (i == num)
            return TRUE;
    }
    return FALSE;
}

gboolean prop_get_string_utf8(Window win, Atom prop, char **ret)
{
    char *raw;
    guint num;
     
    if (get_all(win, prop, prop_atoms.utf8, 8, (guchar**)&raw, &num)) {
	*ret = g_strndup(raw, num); /* grab the first string from the list */
	g_free(raw);
	return TRUE;
    }
    return FALSE;
}

gboolean prop_get_strings_utf8(Window win, Atom prop, char ***ret)
{
    char *raw, *p;
    guint num, i;

    if (get_all(win, prop, prop_atoms.utf8, 8, (guchar**)&raw, &num)) {
        *ret = g_new(char*, num + 1);
        (*ret)[num] = NULL; /* null terminated list */

        p = raw;
        for (i = 0; i < num; ++i) {
            (*ret)[i] = g_strdup(p);
            p = strchr(p, '\0');
        }
	g_free(raw);
	return TRUE;
    }
    return FALSE;
}

void prop_set32(Window win, Atom prop, Atom type, guint32 val)
{
    XChangeProperty(ob_display, win, prop, type, 32, PropModeReplace,
                    (guchar*)&val, 1);
}

void prop_set_array32(Window win, Atom prop, Atom type, guint32 *val,
                      guint num)
{
    XChangeProperty(ob_display, win, prop, type, 32, PropModeReplace,
                    (guchar*)val, num);
}

void prop_set_string_utf8(Window win, Atom prop, char *val)
{
    XChangeProperty(ob_display, win, prop, prop_atoms.utf8, 8,
                    PropModeReplace, (guchar*)val, strlen(val));
}

void prop_set_strings_utf8(Window win, Atom prop, char **strs)
{
    GString *str;
    char **s;

    str = g_string_sized_new(0);
    for (s = strs; *s; ++s) {
        str = g_string_append(str, *s);
        str = g_string_append_c(str, '\0');
    }
    XChangeProperty(ob_display, win, prop, prop_atoms.utf8, 8,
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
