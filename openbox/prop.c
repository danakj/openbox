/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   prop.c for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
   Copyright (c) 2003-2007   Dana Jansens

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   See the COPYING file for a copy of the GNU General Public License.
*/

#include "prop.h"
#include "openbox.h"

#include <X11/Xatom.h>

Atoms prop_atoms;

#define CREATE(var, name) (prop_atoms.var = \
                           XInternAtom(ob_display, name, FALSE))

void prop_startup(void)
{
    CREATE(cardinal, "CARDINAL");
    CREATE(window, "WINDOW");
    CREATE(pixmap, "PIXMAP");
    CREATE(atom, "ATOM");
    CREATE(string, "STRING");
    CREATE(utf8, "UTF8_STRING");

    CREATE(manager, "MANAGER");

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
    CREATE(wm_client_machine, "WM_CLIENT_MACHINE");
    CREATE(wm_command, "WM_COMMAND");
    CREATE(wm_client_leader, "WM_CLIENT_LEADER");
    CREATE(wm_transient_for, "WM_TRANSIENT_FOR");
    CREATE(motif_wm_hints, "_MOTIF_WM_HINTS");

    CREATE(sm_client_id, "SM_CLIENT_ID");

    CREATE(net_wm_full_placement, "_NET_WM_FULL_PLACEMENT");

    CREATE(net_supported, "_NET_SUPPORTED");
    CREATE(net_client_list, "_NET_CLIENT_LIST");
    CREATE(net_client_list_stacking, "_NET_CLIENT_LIST_STACKING");
    CREATE(net_number_of_desktops, "_NET_NUMBER_OF_DESKTOPS");
    CREATE(net_desktop_geometry, "_NET_DESKTOP_GEOMETRY");
    CREATE(net_desktop_viewport, "_NET_DESKTOP_VIEWPORT");
    CREATE(net_current_desktop, "_NET_CURRENT_DESKTOP");
    CREATE(net_desktop_names, "_NET_DESKTOP_NAMES");
    CREATE(net_active_window, "_NET_ACTIVE_WINDOW");
/*    CREATE(net_restack_window, "_NET_RESTACK_WINDOW");*/
    CREATE(net_workarea, "_NET_WORKAREA");
    CREATE(net_supporting_wm_check, "_NET_SUPPORTING_WM_CHECK");
    CREATE(net_desktop_layout, "_NET_DESKTOP_LAYOUT");
    CREATE(net_showing_desktop, "_NET_SHOWING_DESKTOP");

    CREATE(net_close_window, "_NET_CLOSE_WINDOW");
    CREATE(net_wm_moveresize, "_NET_WM_MOVERESIZE");
    CREATE(net_moveresize_window, "_NET_MOVERESIZE_WINDOW");
    CREATE(net_request_frame_extents, "_NET_REQUEST_FRAME_EXTENTS");
    CREATE(net_restack_window, "_NET_RESTACK_WINDOW");

    CREATE(net_startup_id, "_NET_STARTUP_ID");

    CREATE(net_wm_name, "_NET_WM_NAME");
    CREATE(net_wm_visible_name, "_NET_WM_VISIBLE_NAME");
    CREATE(net_wm_icon_name, "_NET_WM_ICON_NAME");
    CREATE(net_wm_visible_icon_name, "_NET_WM_VISIBLE_ICON_NAME");
    CREATE(net_wm_desktop, "_NET_WM_DESKTOP");
    CREATE(net_wm_window_type, "_NET_WM_WINDOW_TYPE");
    CREATE(net_wm_state, "_NET_WM_STATE");
    CREATE(net_wm_strut, "_NET_WM_STRUT");
    CREATE(net_wm_strut_partial, "_NET_WM_STRUT_PARTIAL");
    CREATE(net_wm_icon, "_NET_WM_ICON");
    CREATE(net_wm_icon_geometry, "_NET_WM_ICON_GEOMETRY");
    CREATE(net_wm_pid, "_NET_WM_PID");
    CREATE(net_wm_allowed_actions, "_NET_WM_ALLOWED_ACTIONS");
    CREATE(net_wm_user_time, "_NET_WM_USER_TIME");
/*    CREATE(net_wm_user_time_window, "_NET_WM_USER_TIME_WINDOW"); */
    CREATE(kde_net_wm_frame_strut, "_KDE_NET_WM_FRAME_STRUT");
    CREATE(net_frame_extents, "_NET_FRAME_EXTENTS");

    CREATE(net_wm_ping, "_NET_WM_PING");
#ifdef SYNC
    CREATE(net_wm_sync_request, "_NET_WM_SYNC_REQUEST");
    CREATE(net_wm_sync_request_counter, "_NET_WM_SYNC_REQUEST_COUNTER");
#endif

    CREATE(net_wm_window_type_desktop, "_NET_WM_WINDOW_TYPE_DESKTOP");
    CREATE(net_wm_window_type_dock, "_NET_WM_WINDOW_TYPE_DOCK");
    CREATE(net_wm_window_type_toolbar, "_NET_WM_WINDOW_TYPE_TOOLBAR");
    CREATE(net_wm_window_type_menu, "_NET_WM_WINDOW_TYPE_MENU");
    CREATE(net_wm_window_type_utility, "_NET_WM_WINDOW_TYPE_UTILITY");
    CREATE(net_wm_window_type_splash, "_NET_WM_WINDOW_TYPE_SPLASH");
    CREATE(net_wm_window_type_dialog, "_NET_WM_WINDOW_TYPE_DIALOG");
    CREATE(net_wm_window_type_normal, "_NET_WM_WINDOW_TYPE_NORMAL");
    CREATE(net_wm_window_type_popup_menu, "_NET_WM_WINDOW_TYPE_POPUP_MENU");

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
    prop_atoms.net_wm_moveresize_cancel = 11;

    CREATE(net_wm_action_move, "_NET_WM_ACTION_MOVE");
    CREATE(net_wm_action_resize, "_NET_WM_ACTION_RESIZE");
    CREATE(net_wm_action_minimize, "_NET_WM_ACTION_MINIMIZE");
    CREATE(net_wm_action_shade, "_NET_WM_ACTION_SHADE");
    CREATE(net_wm_action_maximize_horz, "_NET_WM_ACTION_MAXIMIZE_HORZ");
    CREATE(net_wm_action_maximize_vert, "_NET_WM_ACTION_MAXIMIZE_VERT");
    CREATE(net_wm_action_fullscreen, "_NET_WM_ACTION_FULLSCREEN");
    CREATE(net_wm_action_change_desktop, "_NET_WM_ACTION_CHANGE_DESKTOP");
    CREATE(net_wm_action_close, "_NET_WM_ACTION_CLOSE");
    CREATE(net_wm_action_above, "_NET_WM_ACTION_ABOVE");
    CREATE(net_wm_action_below, "_NET_WM_ACTION_BELOW");

    CREATE(net_wm_state_modal, "_NET_WM_STATE_MODAL");
/*    CREATE(net_wm_state_sticky, "_NET_WM_STATE_STICKY");*/
    CREATE(net_wm_state_maximized_vert, "_NET_WM_STATE_MAXIMIZED_VERT");
    CREATE(net_wm_state_maximized_horz, "_NET_WM_STATE_MAXIMIZED_HORZ");
    CREATE(net_wm_state_shaded, "_NET_WM_STATE_SHADED");
    CREATE(net_wm_state_skip_taskbar, "_NET_WM_STATE_SKIP_TASKBAR");
    CREATE(net_wm_state_skip_pager, "_NET_WM_STATE_SKIP_PAGER");
    CREATE(net_wm_state_hidden, "_NET_WM_STATE_HIDDEN");
    CREATE(net_wm_state_fullscreen, "_NET_WM_STATE_FULLSCREEN");
    CREATE(net_wm_state_above, "_NET_WM_STATE_ABOVE");
    CREATE(net_wm_state_below, "_NET_WM_STATE_BELOW");
    CREATE(net_wm_state_demands_attention, "_NET_WM_STATE_DEMANDS_ATTENTION");

    prop_atoms.net_wm_state_add = 1;
    prop_atoms.net_wm_state_remove = 0;
    prop_atoms.net_wm_state_toggle = 2;

    prop_atoms.net_wm_orientation_horz = 0;
    prop_atoms.net_wm_orientation_vert = 1;
    prop_atoms.net_wm_topleft = 0;
    prop_atoms.net_wm_topright = 1;
    prop_atoms.net_wm_bottomright = 2;
    prop_atoms.net_wm_bottomleft = 3;

    CREATE(kde_wm_change_state, "_KDE_WM_CHANGE_STATE");
    CREATE(kde_net_wm_window_type_override,"_KDE_NET_WM_WINDOW_TYPE_OVERRIDE");

/*
    CREATE(rootpmapid, "_XROOTPMAP_ID");
    CREATE(esetrootid, "ESETROOT_PMAP_ID");
*/

    CREATE(openbox_pid, "_OPENBOX_PID");
    CREATE(ob_theme, "_OB_THEME");
    CREATE(ob_config_file, "_OB_CONFIG_FILE");
    CREATE(ob_wm_action_undecorate, "_OB_WM_ACTION_UNDECORATE");
    CREATE(ob_wm_state_undecorated, "_OB_WM_STATE_UNDECORATED");
    CREATE(ob_control, "_OB_CONTROL");
}

#include <X11/Xutil.h>
#include <glib.h>
#include <string.h>

/* this just isn't used... and it also breaks on 64bit, watch out
static gboolean get(Window win, Atom prop, Atom type, gint size,
                    guchar **data, gulong num)
{
    gboolean ret = FALSE;
    gint res;
    guchar *xdata = NULL;
    Atom ret_type;
    gint ret_size;
    gulong ret_items, bytes_left;
    glong num32 = 32 / size * num; /\* num in 32-bit elements *\/

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

static gboolean get_prealloc(Window win, Atom prop, Atom type, gint size,
                             guchar *data, gulong num)
{
    gboolean ret = FALSE;
    gint res;
    guchar *xdata = NULL;
    Atom ret_type;
    gint ret_size;
    gulong ret_items, bytes_left;
    glong num32 = 32 / size * num; /* num in 32-bit elements */

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
                    ((guint16*)data)[i] = ((gushort*)xdata)[i];
                    break;
                case 32:
                    ((guint32*)data)[i] = ((gulong*)xdata)[i];
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

static gboolean get_all(Window win, Atom prop, Atom type, gint size,
                        guchar **data, guint *num)
{
    gboolean ret = FALSE;
    gint res;
    guchar *xdata = NULL;
    Atom ret_type;
    gint ret_size;
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
                    ((guint16*)*data)[i] = ((gushort*)xdata)[i];
                    break;
                case 32:
                    ((guint32*)*data)[i] = ((gulong*)xdata)[i];
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

static gboolean get_stringlist(Window win, Atom prop, gchar ***list, gint *nstr)
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

gboolean prop_get_string_locale(Window win, Atom prop, gchar **ret)
{
    gchar **list;
    gint nstr;
    gchar *s;

    if (get_stringlist(win, prop, &list, &nstr) && nstr) {
        s = g_locale_to_utf8(list[0], -1, NULL, NULL, NULL);
        XFreeStringList(list);
        if (s) {
            *ret = s;
            return TRUE;
        }
    }
    return FALSE;
}

gboolean prop_get_strings_locale(Window win, Atom prop, gchar ***ret)
{
    GSList *strs = NULL, *it;
    gchar *raw, *p;
    guint num, i, count = 0;

    if (get_all(win, prop, prop_atoms.string, 8, (guchar**)&raw, &num)) {

        p = raw;
        while (p < raw + num) {
            ++count;
            strs = g_slist_append(strs, p);
            p += strlen(p) + 1; /* next string */
        }

        *ret = g_new0(gchar*, count + 1);
        (*ret)[count] = NULL; /* null terminated list */

        for (i = 0, it = strs; it; ++i, it = g_slist_next(it)) {
            (*ret)[i] = g_locale_to_utf8(it->data, -1, NULL, NULL, NULL);
            /* make sure translation did not fail */
            if (!(*ret)[i])
                (*ret)[i] = g_strdup("");
        }
        g_free(raw);
        g_slist_free(strs);
        return TRUE;
    }
    return FALSE;
}

gboolean prop_get_string_utf8(Window win, Atom prop, gchar **ret)
{
    gchar *raw;
    gchar *str;
    guint num;

    if (get_all(win, prop, prop_atoms.utf8, 8, (guchar**)&raw, &num)) {
        str = g_strndup(raw, num); /* grab the first string from the list */
        g_free(raw);
        if (g_utf8_validate(str, -1, NULL)) {
            *ret = str;
            return TRUE;
        }
        g_free(str);
    }
    return FALSE;
}

gboolean prop_get_strings_utf8(Window win, Atom prop, gchar ***ret)
{
    GSList *strs = NULL, *it;
    gchar *raw, *p;
    guint num, i, count = 0;

    if (get_all(win, prop, prop_atoms.utf8, 8, (guchar**)&raw, &num)) {

        p = raw;
        while (p < raw + num) {
            ++count;
            strs = g_slist_append(strs, p);
            p += strlen(p) + 1; /* next string */
        }

        *ret = g_new0(gchar*, count + 1);

        for (i = 0, it = strs; it; ++i, it = g_slist_next(it)) {
            if (g_utf8_validate(it->data, -1, NULL))
                (*ret)[i] = g_strdup(it->data);
            else
                (*ret)[i] = g_strdup("");
        }
        g_free(raw);
        g_slist_free(strs);
        return TRUE;
    }
    return FALSE;
}

void prop_set32(Window win, Atom prop, Atom type, gulong val)
{
    XChangeProperty(ob_display, win, prop, type, 32, PropModeReplace,
                    (guchar*)&val, 1);
}

void prop_set_array32(Window win, Atom prop, Atom type, gulong *val,
                      guint num)
{
    XChangeProperty(ob_display, win, prop, type, 32, PropModeReplace,
                    (guchar*)val, num);
}

void prop_set_string_utf8(Window win, Atom prop, const gchar *val)
{
    XChangeProperty(ob_display, win, prop, prop_atoms.utf8, 8,
                    PropModeReplace, (const guchar*)val, strlen(val));
}

void prop_set_strings_utf8(Window win, Atom prop, gchar **strs)
{
    GString *str;
    gchar **s;

    str = g_string_sized_new(0);
    for (s = strs; *s; ++s) {
        str = g_string_append(str, *s);
        str = g_string_append_c(str, '\0');
    }
    XChangeProperty(ob_display, win, prop, prop_atoms.utf8, 8,
                    PropModeReplace, (guchar*)str->str, str->len);
    g_string_free(str, TRUE);
}

void prop_erase(Window win, Atom prop)
{
    XDeleteProperty(ob_display, win, prop);
}

void prop_message(Window about, Atom messagetype, glong data0, glong data1,
                  glong data2, glong data3, glong mask)
{
    prop_message_to(RootWindow(ob_display, ob_screen), about, messagetype,
                    data0, data1, data2, data3, 0, mask);
}

void prop_message_to(Window to, Window about, Atom messagetype,
                     glong data0, glong data1, glong data2,
                     glong data3, glong data4, glong mask)
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
    ce.xclient.data.l[4] = data4;
    XSendEvent(ob_display, to, FALSE, mask, &ce);
}
