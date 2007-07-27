/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   obt/prop.c for the Openbox window manager
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

#include "obt/prop.h"
#include "obt/display.h"

#include <X11/Xatom.h>
#ifdef HAVE_STRING_H
#  include <string.h>
#endif

Atom prop_atoms[OBT_PROP_NUM_ATOMS];
gboolean prop_started = FALSE;

#define CREATE(var, name) (prop_atoms[OBT_PROP_##var] = \
                           XInternAtom((obt_display), (name), FALSE))

void obt_prop_startup()
{
    if (prop_started) return;
    prop_started = TRUE;

    g_assert(obt_display);

    CREATE(CARDINAL, "CARDINAL");
    CREATE(WINDOW, "WINDOW");
    CREATE(PIXMAP, "PIXMAP");
    CREATE(ATOM, "ATOM");
    CREATE(STRING, "STRING");
    CREATE(UTF8, "UTF8_STRING");

    CREATE(MANAGER, "MANAGER");

    CREATE(WM_COLORMAP_WINDOWS, "WM_COLORMAP_WINDOWS");
    CREATE(WM_PROTOCOLS, "WM_PROTOCOLS");
    CREATE(WM_STATE, "WM_STATE");
    CREATE(WM_CHANGE_STATE, "WM_CHANGE_STATE");
    CREATE(WM_DELETE_WINDOW, "WM_DELETE_WINDOW");
    CREATE(WM_TAKE_FOCUS, "WM_TAKE_FOCUS");
    CREATE(WM_NAME, "WM_NAME");
    CREATE(WM_ICON_NAME, "WM_ICON_NAME");
    CREATE(WM_CLASS, "WM_CLASS");
    CREATE(WM_WINDOW_ROLE, "WM_WINDOW_ROLE");
    CREATE(WM_CLIENT_MACHINE, "WM_CLIENT_MACHINE");
    CREATE(WM_COMMAND, "WM_COMMAND");
    CREATE(WM_CLIENT_LEADER, "WM_CLIENT_LEADER");
    CREATE(MOTIF_WM_HINTS, "_MOTIF_WM_HINTS");

    CREATE(SM_CLIENT_ID, "SM_CLIENT_ID");

    CREATE(NET_WM_FULL_PLACEMENT, "_NET_WM_FULL_PLACEMENT");

    CREATE(NET_SUPPORTED, "_NET_SUPPORTED");
    CREATE(NET_CLIENT_LIST, "_NET_CLIENT_LIST");
    CREATE(NET_CLIENT_LIST_STACKING, "_NET_CLIENT_LIST_STACKING");
    CREATE(NET_NUMBER_OF_DESKTOPS, "_NET_NUMBER_OF_DESKTOPS");
    CREATE(NET_DESKTOP_GEOMETRY, "_NET_DESKTOP_GEOMETRY");
    CREATE(NET_DESKTOP_VIEWPORT, "_NET_DESKTOP_VIEWPORT");
    CREATE(NET_CURRENT_DESKTOP, "_NET_CURRENT_DESKTOP");
    CREATE(NET_DESKTOP_NAMES, "_NET_DESKTOP_NAMES");
    CREATE(NET_ACTIVE_WINDOW, "_NET_ACTIVE_WINDOW");
/*    CREATE(NET_RESTACK_WINDOW, "_NET_RESTACK_WINDOW");*/
    CREATE(NET_WORKAREA, "_NET_WORKAREA");
    CREATE(NET_SUPPORTING_WM_CHECK, "_NET_SUPPORTING_WM_CHECK");
    CREATE(NET_DESKTOP_LAYOUT, "_NET_DESKTOP_LAYOUT");
    CREATE(NET_SHOWING_DESKTOP, "_NET_SHOWING_DESKTOP");

    CREATE(NET_CLOSE_WINDOW, "_NET_CLOSE_WINDOW");
    CREATE(NET_WM_MOVERESIZE, "_NET_WM_MOVERESIZE");
    CREATE(NET_MOVERESIZE_WINDOW, "_NET_MOVERESIZE_WINDOW");
    CREATE(NET_REQUEST_FRAME_EXTENTS, "_NET_REQUEST_FRAME_EXTENTS");
    CREATE(NET_RESTACK_WINDOW, "_NET_RESTACK_WINDOW");

    CREATE(NET_STARTUP_ID, "_NET_STARTUP_ID");

    CREATE(NET_WM_NAME, "_NET_WM_NAME");
    CREATE(NET_WM_VISIBLE_NAME, "_NET_WM_VISIBLE_NAME");
    CREATE(NET_WM_ICON_NAME, "_NET_WM_ICON_NAME");
    CREATE(NET_WM_VISIBLE_ICON_NAME, "_NET_WM_VISIBLE_ICON_NAME");
    CREATE(NET_WM_DESKTOP, "_NET_WM_DESKTOP");
    CREATE(NET_WM_WINDOW_TYPE, "_NET_WM_WINDOW_TYPE");
    CREATE(NET_WM_STATE, "_NET_WM_STATE");
    CREATE(NET_WM_STRUT, "_NET_WM_STRUT");
    CREATE(NET_WM_STRUT_PARTIAL, "_NET_WM_STRUT_PARTIAL");
    CREATE(NET_WM_ICON, "_NET_WM_ICON");
    CREATE(NET_WM_ICON_GEOMETRY, "_NET_WM_ICON_GEOMETRY");
    CREATE(NET_WM_PID, "_NET_WM_PID");
    CREATE(NET_WM_ALLOWED_ACTIONS, "_NET_WM_ALLOWED_ACTIONS");
    CREATE(NET_WM_USER_TIME, "_NET_WM_USER_TIME");
/*  CREATE(NET_WM_USER_TIME_WINDOW, "_NET_WM_USER_TIME_WINDOW"); */
    CREATE(KDE_NET_WM_FRAME_STRUT, "_KDE_NET_WM_FRAME_STRUT");
    CREATE(NET_FRAME_EXTENTS, "_NET_FRAME_EXTENTS");

    CREATE(NET_WM_PING, "_NET_WM_PING");
#ifdef SYNC
    CREATE(NET_WM_SYNC_REQUEST, "_NET_WM_SYNC_REQUEST");
    CREATE(NET_WM_SYNC_REQUEST_COUNTER, "_NET_WM_SYNC_REQUEST_COUNTER");
#endif

    CREATE(NET_WM_WINDOW_TYPE_DESKTOP, "_NET_WM_WINDOW_TYPE_DESKTOP");
    CREATE(NET_WM_WINDOW_TYPE_DOCK, "_NET_WM_WINDOW_TYPE_DOCK");
    CREATE(NET_WM_WINDOW_TYPE_TOOLBAR, "_NET_WM_WINDOW_TYPE_TOOLBAR");
    CREATE(NET_WM_WINDOW_TYPE_MENU, "_NET_WM_WINDOW_TYPE_MENU");
    CREATE(NET_WM_WINDOW_TYPE_UTILITY, "_NET_WM_WINDOW_TYPE_UTILITY");
    CREATE(NET_WM_WINDOW_TYPE_SPLASH, "_NET_WM_WINDOW_TYPE_SPLASH");
    CREATE(NET_WM_WINDOW_TYPE_DIALOG, "_NET_WM_WINDOW_TYPE_DIALOG");
    CREATE(NET_WM_WINDOW_TYPE_NORMAL, "_NET_WM_WINDOW_TYPE_NORMAL");

    prop_atoms[OBT_PROP_NET_WM_MOVERESIZE_SIZE_TOPLEFT] = 0;
    prop_atoms[OBT_PROP_NET_WM_MOVERESIZE_SIZE_TOP] = 1;
    prop_atoms[OBT_PROP_NET_WM_MOVERESIZE_SIZE_TOPRIGHT] = 2;
    prop_atoms[OBT_PROP_NET_WM_MOVERESIZE_SIZE_RIGHT] = 3;
    prop_atoms[OBT_PROP_NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT] = 4;
    prop_atoms[OBT_PROP_NET_WM_MOVERESIZE_SIZE_BOTTOM] = 5;
    prop_atoms[OBT_PROP_NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT] = 6;
    prop_atoms[OBT_PROP_NET_WM_MOVERESIZE_SIZE_LEFT] = 7;
    prop_atoms[OBT_PROP_NET_WM_MOVERESIZE_MOVE] = 8;
    prop_atoms[OBT_PROP_NET_WM_MOVERESIZE_SIZE_KEYBOARD] = 9;
    prop_atoms[OBT_PROP_NET_WM_MOVERESIZE_MOVE_KEYBOARD] = 10;
    prop_atoms[OBT_PROP_NET_WM_MOVERESIZE_CANCEL] = 11;

    CREATE(NET_WM_ACTION_MOVE, "_NET_WM_ACTION_MOVE");
    CREATE(NET_WM_ACTION_RESIZE, "_NET_WM_ACTION_RESIZE");
    CREATE(NET_WM_ACTION_MINIMIZE, "_NET_WM_ACTION_MINIMIZE");
    CREATE(NET_WM_ACTION_SHADE, "_NET_WM_ACTION_SHADE");
    CREATE(NET_WM_ACTION_MAXIMIZE_HORZ, "_NET_WM_ACTION_MAXIMIZE_HORZ");
    CREATE(NET_WM_ACTION_MAXIMIZE_VERT, "_NET_WM_ACTION_MAXIMIZE_VERT");
    CREATE(NET_WM_ACTION_FULLSCREEN, "_NET_WM_ACTION_FULLSCREEN");
    CREATE(NET_WM_ACTION_CHANGE_DESKTOP, "_NET_WM_ACTION_CHANGE_DESKTOP");
    CREATE(NET_WM_ACTION_CLOSE, "_NET_WM_ACTION_CLOSE");
    CREATE(NET_WM_ACTION_ABOVE, "_NET_WM_ACTION_ABOVE");
    CREATE(NET_WM_ACTION_BELOW, "_NET_WM_ACTION_BELOW");

    CREATE(NET_WM_STATE_MODAL, "_NET_WM_STATE_MODAL");
/*    CREATE(NET_WM_STATE_STICKY, "_NET_WM_STATE_STICKY");*/
    CREATE(NET_WM_STATE_MAXIMIZED_VERT, "_NET_WM_STATE_MAXIMIZED_VERT");
    CREATE(NET_WM_STATE_MAXIMIZED_HORZ, "_NET_WM_STATE_MAXIMIZED_HORZ");
    CREATE(NET_WM_STATE_SHADED, "_NET_WM_STATE_SHADED");
    CREATE(NET_WM_STATE_SKIP_TASKBAR, "_NET_WM_STATE_SKIP_TASKBAR");
    CREATE(NET_WM_STATE_SKIP_PAGER, "_NET_WM_STATE_SKIP_PAGER");
    CREATE(NET_WM_STATE_HIDDEN, "_NET_WM_STATE_HIDDEN");
    CREATE(NET_WM_STATE_FULLSCREEN, "_NET_WM_STATE_FULLSCREEN");
    CREATE(NET_WM_STATE_ABOVE, "_NET_WM_STATE_ABOVE");
    CREATE(NET_WM_STATE_BELOW, "_NET_WM_STATE_BELOW");
    CREATE(NET_WM_STATE_DEMANDS_ATTENTION, "_NET_WM_STATE_DEMANDS_ATTENTION");

    prop_atoms[OBT_PROP_NET_WM_STATE_ADD] = 1;
    prop_atoms[OBT_PROP_NET_WM_STATE_REMOVE] = 0;
    prop_atoms[OBT_PROP_NET_WM_STATE_TOGGLE] = 2;

    prop_atoms[OBT_PROP_NET_WM_ORIENTATION_HORZ] = 0;
    prop_atoms[OBT_PROP_NET_WM_ORIENTATION_VERT] = 1;
    prop_atoms[OBT_PROP_NET_WM_TOPLEFT] = 0;
    prop_atoms[OBT_PROP_NET_WM_TOPRIGHT] = 1;
    prop_atoms[OBT_PROP_NET_WM_BOTTOMRIGHT] = 2;
    prop_atoms[OBT_PROP_NET_WM_BOTTOMLEFT] = 3;

    CREATE(KDE_WM_CHANGE_STATE, "_KDE_WM_CHANGE_STATE");
    CREATE(KDE_NET_WM_WINDOW_TYPE_OVERRIDE,"_KDE_NET_WM_WINDOW_TYPE_OVERRIDE");

/*
    CREATE(ROOTPMAPId, "_XROOTPMAP_ID");
    CREATE(ESETROOTId, "ESETROOT_PMAP_ID");
*/

    CREATE(OPENBOX_PID, "_OPENBOX_PID");
    CREATE(OB_THEME, "_OB_THEME");
    CREATE(OB_WM_ACTION_UNDECORATE, "_OB_WM_ACTION_UNDECORATE");
    CREATE(OB_WM_STATE_UNDECORATED, "_OB_WM_STATE_UNDECORATED");
    CREATE(OB_CONTROL, "_OB_CONTROL");
}

Atom obt_prop_atom(ObtPropAtom a)
{
    g_assert(prop_started);
    g_assert(a < OBT_PROP_NUM_ATOMS);
    return prop_atoms[a];
}

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

    res = XGetWindowProperty(obt_display, win, prop, 0l, num32,
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

    res = XGetWindowProperty(obt_display, win, prop, 0l, G_MAXLONG,
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

    if (XGetTextProperty(obt_display, win, &tprop, prop) && tprop.nitems) {
        if (XTextPropertyToStringList(&tprop, list, nstr))
            ret = TRUE;
        XFree(tprop.value);
    }
    return ret;
}

gboolean obt_prop_get32(Window win, Atom prop, Atom type, guint32 *ret)
{
    return get_prealloc(win, prop, type, 32, (guchar*)ret, 1);
}

gboolean obt_prop_get_array32(Window win, Atom prop, Atom type, guint32 **ret,
                              guint *nret)
{
    return get_all(win, prop, type, 32, (guchar**)ret, nret);
}

gboolean obt_prop_get_string_locale(Window win, Atom prop, gchar **ret)
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

gboolean obt_prop_get_strings_locale(Window win, Atom prop, gchar ***ret)
{
    GSList *strs = NULL, *it;
    gchar *raw, *p;
    guint num, i, count = 0;

    if (get_all(win, prop, obt_prop_atom(OBT_PROP_STRING), 8,
                (guchar**)&raw, &num))
    {
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

gboolean obt_prop_get_string_utf8(Window win, Atom prop, gchar **ret)
{
    gchar *raw;
    gchar *str;
    guint num;

    if (get_all(win, prop, obt_prop_atom(OBT_PROP_UTF8), 8,
                (guchar**)&raw, &num))
    {
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

gboolean obt_prop_get_strings_utf8(Window win, Atom prop, gchar ***ret)
{
    GSList *strs = NULL, *it;
    gchar *raw, *p;
    guint num, i, count = 0;

    if (get_all(win, prop, obt_prop_atom(OBT_PROP_UTF8), 8,
                (guchar**)&raw, &num))
    {
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

void obt_prop_set32(Window win, Atom prop, Atom type, gulong val)
{
    XChangeProperty(obt_display, win, prop, type, 32, PropModeReplace,
                    (guchar*)&val, 1);
}

void obt_prop_set_array32(Window win, Atom prop, Atom type, gulong *val,
                      guint num)
{
    XChangeProperty(obt_display, win, prop, type, 32, PropModeReplace,
                    (guchar*)val, num);
}

void obt_prop_set_string_utf8(Window win, Atom prop, const gchar *val)
{
    XChangeProperty(obt_display, win, prop, obt_prop_atom(OBT_PROP_UTF8), 8,
                    PropModeReplace, (const guchar*)val, strlen(val));
}

void obt_prop_set_strings_utf8(Window win, Atom prop, gchar **strs)
{
    GString *str;
    gchar **s;

    str = g_string_sized_new(0);
    for (s = strs; *s; ++s) {
        str = g_string_append(str, *s);
        str = g_string_append_c(str, '\0');
    }
    XChangeProperty(obt_display, win, prop, obt_prop_atom(OBT_PROP_UTF8), 8,
                    PropModeReplace, (guchar*)str->str, str->len);
    g_string_free(str, TRUE);
}

void obt_prop_erase(Window win, Atom prop)
{
    XDeleteProperty(obt_display, win, prop);
}

void obt_prop_message(gint screen, Window about, Atom messagetype,
                      glong data0, glong data1, glong data2, glong data3,
                      glong data4, glong mask)
{
    obt_prop_message_to(RootWindow(obt_display, screen), about, messagetype,
                        data0, data1, data2, data3, data4, mask);
}

void obt_prop_message_to(Window to, Window about,
                         Atom messagetype,
                         glong data0, glong data1, glong data2, glong data3,
                         glong data4, glong mask)
{
    XEvent ce;
    ce.xclient.type = ClientMessage;
    ce.xclient.message_type = messagetype;
    ce.xclient.display = obt_display;
    ce.xclient.window = about;
    ce.xclient.format = 32;
    ce.xclient.data.l[0] = data0;
    ce.xclient.data.l[1] = data1;
    ce.xclient.data.l[2] = data2;
    ce.xclient.data.l[3] = data3;
    ce.xclient.data.l[4] = data4;
    XSendEvent(obt_display, to, FALSE, mask, &ce);
}
