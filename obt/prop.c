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

#define CREATE_NAME(var, name) (prop_atoms[OBT_PROP_##var] = \
                                XInternAtom((obt_display), (name), FALSE))
#define CREATE(var) CREATE_NAME(var, #var)
#define CREATE_(var) CREATE_NAME(var, "_" #var)

void obt_prop_startup(void)
{
    if (prop_started) return;
    prop_started = TRUE;

    g_assert(obt_display);

    CREATE(CARDINAL);
    CREATE(WINDOW);
    CREATE(PIXMAP);
    CREATE(ATOM);
    CREATE(STRING);
    CREATE_NAME(UTF8, "UTF8_STRING");

    CREATE(MANAGER);

    CREATE(WM_COLORMAP_WINDOWS);
    CREATE(WM_PROTOCOLS);
    CREATE(WM_STATE);
    CREATE(WM_CHANGE_STATE);
    CREATE(WM_DELETE_WINDOW);
    CREATE(WM_TAKE_FOCUS);
    CREATE(WM_NAME);
    CREATE(WM_ICON_NAME);
    CREATE(WM_CLASS);
    CREATE(WM_WINDOW_ROLE);
    CREATE(WM_CLIENT_MACHINE);
    CREATE(WM_COMMAND);
    CREATE(WM_CLIENT_LEADER);
    CREATE(WM_TRANSIENT_FOR);
    CREATE_(MOTIF_WM_HINTS);

    CREATE(SM_CLIENT_ID);

    CREATE_(NET_WM_FULL_PLACEMENT);

    CREATE_(NET_SUPPORTED);
    CREATE_(NET_CLIENT_LIST);
    CREATE_(NET_CLIENT_LIST_STACKING);
    CREATE_(NET_NUMBER_OF_DESKTOPS);
    CREATE_(NET_DESKTOP_GEOMETRY);
    CREATE_(NET_DESKTOP_VIEWPORT);
    CREATE_(NET_CURRENT_DESKTOP);
    CREATE_(NET_DESKTOP_NAMES);
    CREATE_(NET_ACTIVE_WINDOW);
/*    CREATE_(NET_RESTACK_WINDOW);*/
    CREATE_(NET_WORKAREA);
    CREATE_(NET_SUPPORTING_WM_CHECK);
    CREATE_(NET_DESKTOP_LAYOUT);
    CREATE_(NET_SHOWING_DESKTOP);

    CREATE_(NET_CLOSE_WINDOW);
    CREATE_(NET_WM_MOVERESIZE);
    CREATE_(NET_MOVERESIZE_WINDOW);
    CREATE_(NET_REQUEST_FRAME_EXTENTS);
    CREATE_(NET_RESTACK_WINDOW);

    CREATE_(NET_STARTUP_ID);

    CREATE_(NET_WM_NAME);
    CREATE_(NET_WM_VISIBLE_NAME);
    CREATE_(NET_WM_ICON_NAME);
    CREATE_(NET_WM_VISIBLE_ICON_NAME);
    CREATE_(NET_WM_DESKTOP);
    CREATE_(NET_WM_WINDOW_TYPE);
    CREATE_(NET_WM_STATE);
    CREATE_(NET_WM_STRUT);
    CREATE_(NET_WM_STRUT_PARTIAL);
    CREATE_(NET_WM_ICON);
    CREATE_(NET_WM_ICON_GEOMETRY);
    CREATE_(NET_WM_PID);
    CREATE_(NET_WM_ALLOWED_ACTIONS);
    CREATE_(NET_WM_USER_TIME);
/*  CREATE_(NET_WM_USER_TIME_WINDOW); */
    CREATE_(KDE_NET_WM_FRAME_STRUT);
    CREATE_(NET_FRAME_EXTENTS);

    CREATE_(NET_WM_PING);
#ifdef SYNC
    CREATE_(NET_WM_SYNC_REQUEST);
    CREATE_(NET_WM_SYNC_REQUEST_COUNTER);
#endif

    CREATE_(NET_WM_WINDOW_TYPE_DESKTOP);
    CREATE_(NET_WM_WINDOW_TYPE_DOCK);
    CREATE_(NET_WM_WINDOW_TYPE_TOOLBAR);
    CREATE_(NET_WM_WINDOW_TYPE_MENU);
    CREATE_(NET_WM_WINDOW_TYPE_UTILITY);
    CREATE_(NET_WM_WINDOW_TYPE_SPLASH);
    CREATE_(NET_WM_WINDOW_TYPE_DIALOG);
    CREATE_(NET_WM_WINDOW_TYPE_NORMAL);
    CREATE_(NET_WM_WINDOW_TYPE_POPUP_MENU);

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

    CREATE_(NET_WM_ACTION_MOVE);
    CREATE_(NET_WM_ACTION_RESIZE);
    CREATE_(NET_WM_ACTION_MINIMIZE);
    CREATE_(NET_WM_ACTION_SHADE);
    CREATE_(NET_WM_ACTION_MAXIMIZE_HORZ);
    CREATE_(NET_WM_ACTION_MAXIMIZE_VERT);
    CREATE_(NET_WM_ACTION_FULLSCREEN);
    CREATE_(NET_WM_ACTION_CHANGE_DESKTOP);
    CREATE_(NET_WM_ACTION_CLOSE);
    CREATE_(NET_WM_ACTION_ABOVE);
    CREATE_(NET_WM_ACTION_BELOW);

    CREATE_(NET_WM_STATE_MODAL);
/*    CREATE_(NET_WM_STATE_STICKY);*/
    CREATE_(NET_WM_STATE_MAXIMIZED_VERT);
    CREATE_(NET_WM_STATE_MAXIMIZED_HORZ);
    CREATE_(NET_WM_STATE_SHADED);
    CREATE_(NET_WM_STATE_SKIP_TASKBAR);
    CREATE_(NET_WM_STATE_SKIP_PAGER);
    CREATE_(NET_WM_STATE_HIDDEN);
    CREATE_(NET_WM_STATE_FULLSCREEN);
    CREATE_(NET_WM_STATE_ABOVE);
    CREATE_(NET_WM_STATE_BELOW);
    CREATE_(NET_WM_STATE_DEMANDS_ATTENTION);

    prop_atoms[OBT_PROP_NET_WM_STATE_ADD] = 1;
    prop_atoms[OBT_PROP_NET_WM_STATE_REMOVE] = 0;
    prop_atoms[OBT_PROP_NET_WM_STATE_TOGGLE] = 2;

    prop_atoms[OBT_PROP_NET_WM_ORIENTATION_HORZ] = 0;
    prop_atoms[OBT_PROP_NET_WM_ORIENTATION_VERT] = 1;
    prop_atoms[OBT_PROP_NET_WM_TOPLEFT] = 0;
    prop_atoms[OBT_PROP_NET_WM_TOPRIGHT] = 1;
    prop_atoms[OBT_PROP_NET_WM_BOTTOMRIGHT] = 2;
    prop_atoms[OBT_PROP_NET_WM_BOTTOMLEFT] = 3;

    CREATE_(KDE_WM_CHANGE_STATE);
    CREATE_(KDE_NET_WM_WINDOW_TYPE_OVERRIDE);

/*
    CREATE_NAME(ROOTPMAPId, "_XROOTPMAP_ID");
    CREATE_NAME(ESETROOTId, "ESETROOT_PMAP_ID");
*/

    CREATE_(OPENBOX_PID);
    CREATE_(OB_THEME);
    CREATE_(OB_CONFIG_FILE);
    CREATE_(OB_WM_ACTION_UNDECORATE);
    CREATE_(OB_WM_STATE_UNDECORATED);
    CREATE_(OB_CONTROL);
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

    if (get_all(win, prop, OBT_PROP_ATOM(STRING), 8,
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

    if (get_all(win, prop, OBT_PROP_ATOM(UTF8), 8,
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

    if (get_all(win, prop, OBT_PROP_ATOM(UTF8), 8,
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

void obt_prop_set_string_locale(Window win, Atom prop, const gchar *val)
{
    gchar const *s[2] = { val, NULL };
    obt_prop_set_strings_locale(win, prop, s);
}

void obt_prop_set_strings_locale(Window win, Atom prop, const gchar **strs)
{
    gint i, count;
    gchar **lstrs;
    XTextProperty tprop;

    /* count the strings in strs, and convert them to the locale format */
    for (count = 0; strs[count]; ++count);
    lstrs = g_new0(char*, count);
    for (i = 0; i < count; ++i) {
        lstrs[i] = g_locale_from_utf8(strs[i], -1, NULL, NULL, NULL);
        if (!lstrs[i]) {
            lstrs[i] = g_strdup(""); /* make it an empty string */
            g_warning("Unable to translate string '%s' from UTF8 to locale "
                      "format", strs[i]);
        }
    }


    XStringListToTextProperty(lstrs, count, &tprop);
    XSetTextProperty(obt_display, win, &tprop, prop);
    XFree(tprop.value);
}

void obt_prop_set_string_utf8(Window win, Atom prop, const gchar *val)
{
    XChangeProperty(obt_display, win, prop, OBT_PROP_ATOM(UTF8), 8,
                    PropModeReplace, (const guchar*)val, strlen(val));
}

void obt_prop_set_strings_utf8(Window win, Atom prop, const gchar **strs)
{
    GString *str;
    gchar const **s;

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
    obt_prop_message_to(obt_root(screen), about, messagetype,
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
