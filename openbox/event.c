/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   event.c for the Openbox window manager
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

#include "event.h"
#include "debug.h"
#include "window.h"
#include "openbox.h"
#include "dock.h"
#include "actions.h"
#include "client.h"
#include "config.h"
#include "screen.h"
#include "frame.h"
#include "grab.h"
#include "menu.h"
#include "prompt.h"
#include "menuframe.h"
#include "keyboard.h"
#include "mouse.h"
#include "focus.h"
#include "focus_cycle.h"
#include "moveresize.h"
#include "group.h"
#include "stacking.h"
#include "ping.h"
#include "obt/display.h"
#include "obt/prop.h"
#include "obt/keyboard.h"

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <glib.h>

#ifdef HAVE_SYS_SELECT_H
#  include <sys/select.h>
#endif
#ifdef HAVE_SIGNAL_H
#  include <signal.h>
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h> /* for usleep() */
#endif
#ifdef XKB
#  include <X11/XKBlib.h>
#endif

#ifdef USE_SM
#include <X11/ICE/ICElib.h>
#endif

typedef struct
{
    gboolean ignored;
} ObEventData;

typedef struct
{
    ObClient *client;
    Time time;
    gulong serial;
} ObFocusDelayData;

typedef struct
{
    gulong start; /* inclusive */
    gulong end;   /* inclusive */
} ObSerialRange;

static void event_process(const XEvent *e, gpointer data);
static void event_handle_root(XEvent *e);
static gboolean event_handle_menu_input(XEvent *e);
static void event_handle_menu(ObMenuFrame *frame, XEvent *e);
static gboolean event_handle_prompt(ObPrompt *p, XEvent *e);
static void event_handle_dock(ObDock *s, XEvent *e);
static void event_handle_dockapp(ObDockApp *app, XEvent *e);
static void event_handle_client(ObClient *c, XEvent *e);
static void event_handle_user_input(ObClient *client, XEvent *e);
static gboolean is_enter_focus_event_ignored(gulong serial);
static void event_ignore_enter_range(gulong start, gulong end);

static void focus_delay_dest(gpointer data);
static gboolean focus_delay_cmp(gconstpointer d1, gconstpointer d2);
static gboolean focus_delay_func(gpointer data);
static void focus_delay_client_dest(ObClient *client, gpointer data);

Time event_curtime = CurrentTime;
Time event_last_user_time = CurrentTime;
/*! The serial of the current X event */

static gulong event_curserial;
static gboolean focus_left_screen = FALSE;
/*! A list of ObSerialRanges which are to be ignored for mouse enter events */
static GSList *ignore_serials = NULL;

#ifdef USE_SM
static void ice_handler(gint fd, gpointer conn)
{
    Bool b;
    IceProcessMessages(conn, NULL, &b);
}

static void ice_watch(IceConn conn, IcePointer data, Bool opening,
                      IcePointer *watch_data)
{
    static gint fd = -1;

    if (opening) {
        fd = IceConnectionNumber(conn);
        obt_main_loop_fd_add(ob_main_loop, fd, ice_handler, conn, NULL);
    } else {
        obt_main_loop_fd_remove(ob_main_loop, fd);
        fd = -1;
    }
}
#endif

void event_startup(gboolean reconfig)
{
    if (reconfig) return;

    obt_main_loop_x_add(ob_main_loop, event_process, NULL, NULL);

#ifdef USE_SM
    IceAddConnectionWatch(ice_watch, NULL);
#endif

    client_add_destroy_notify(focus_delay_client_dest, NULL);
}

void event_shutdown(gboolean reconfig)
{
    if (reconfig) return;

#ifdef USE_SM
    IceRemoveConnectionWatch(ice_watch, NULL);
#endif

    client_remove_destroy_notify(focus_delay_client_dest);
}

static Window event_get_window(XEvent *e)
{
    Window window;

    /* pick a window */
    switch (e->type) {
    case SelectionClear:
        window = obt_root(ob_screen);
        break;
    case CreateNotify:
        window = e->xcreatewindow.window;
        break;
    case MapRequest:
        window = e->xmaprequest.window;
        break;
    case MapNotify:
        window = e->xmap.window;
        break;
    case UnmapNotify:
        window = e->xunmap.window;
        break;
    case DestroyNotify:
        window = e->xdestroywindow.window;
        break;
    case ConfigureRequest:
        window = e->xconfigurerequest.window;
        break;
    case ConfigureNotify:
        window = e->xconfigure.window;
        break;
    default:
#ifdef XKB
        if (obt_display_extension_xkb &&
            e->type == obt_display_extension_xkb_basep)
        {
            switch (((XkbAnyEvent*)e)->xkb_type) {
            case XkbBellNotify:
                window = ((XkbBellNotifyEvent*)e)->window;
            default:
                window = None;
            }
        } else
#endif
#ifdef SYNC
        if (obt_display_extension_sync &&
            e->type == obt_display_extension_sync_basep + XSyncAlarmNotify)
        {
            window = None;
        } else
#endif
            window = e->xany.window;
    }
    return window;
}

static void event_set_curtime(XEvent *e)
{
    Time t = CurrentTime;

    /* grab the lasttime and hack up the state */
    switch (e->type) {
    case ButtonPress:
    case ButtonRelease:
        t = e->xbutton.time;
        break;
    case KeyPress:
        t = e->xkey.time;
        break;
    case KeyRelease:
        t = e->xkey.time;
        break;
    case MotionNotify:
        t = e->xmotion.time;
        break;
    case PropertyNotify:
        t = e->xproperty.time;
        break;
    case EnterNotify:
    case LeaveNotify:
        t = e->xcrossing.time;
        break;
    default:
#ifdef SYNC
        if (obt_display_extension_sync &&
            e->type == obt_display_extension_sync_basep + XSyncAlarmNotify)
        {
            t = ((XSyncAlarmNotifyEvent*)e)->time;
        }
#endif
        /* if more event types are anticipated, get their timestamp
           explicitly */
        break;
    }

    /* watch that if we get an event earlier than the last specified user_time,
       which can happen if the clock goes backwards, we erase the last
       specified user_time */
    if (t && event_last_user_time && event_time_after(event_last_user_time, t))
        event_last_user_time = CurrentTime;

    event_curtime = t;
}

static void event_hack_mods(XEvent *e)
{
#ifdef XKB
    XkbStateRec xkb_state;
#endif

    switch (e->type) {
    case ButtonPress:
    case ButtonRelease:
        e->xbutton.state = obt_keyboard_only_modmasks(e->xbutton.state);
        break;
    case KeyPress:
        e->xkey.state = obt_keyboard_only_modmasks(e->xkey.state);
        break;
    case KeyRelease:
#ifdef XKB
        /* If XKB is present, then the modifiers are all strange from its
           magic.  Our X core protocol stuff won't work, so we use this to
           find what the modifier state is instead. */
        if (XkbGetState(obt_display, XkbUseCoreKbd, &xkb_state) == Success)
            e->xkey.state =
                obt_keyboard_only_modmasks(xkb_state.compat_state);
        else
#endif
        {
            e->xkey.state = obt_keyboard_only_modmasks(e->xkey.state);
            /* remove from the state the mask of the modifier key being
               released, if it is a modifier key being released that is */
            e->xkey.state &= ~obt_keyboard_keycode_to_modmask(e->xkey.keycode);
        }
        break;
    case MotionNotify:
        e->xmotion.state = obt_keyboard_only_modmasks(e->xmotion.state);
        /* compress events */
        {
            XEvent ce;
            while (XCheckTypedWindowEvent(obt_display, e->xmotion.window,
                                          e->type, &ce)) {
                e->xmotion.x = ce.xmotion.x;
                e->xmotion.y = ce.xmotion.y;
                e->xmotion.x_root = ce.xmotion.x_root;
                e->xmotion.y_root = ce.xmotion.y_root;
            }
        }
        break;
    }
}

static gboolean wanted_focusevent(XEvent *e, gboolean in_client_only)
{
    gint mode = e->xfocus.mode;
    gint detail = e->xfocus.detail;
    Window win = e->xany.window;

    if (e->type == FocusIn) {
        /* These are ones we never want.. */

        /* This means focus was given by a keyboard/mouse grab. */
        if (mode == NotifyGrab)
            return FALSE;
        /* This means focus was given back from a keyboard/mouse grab. */
        if (mode == NotifyUngrab)
            return FALSE;

        /* These are the ones we want.. */

        if (win == obt_root(ob_screen)) {
            /* If looking for a focus in on a client, then always return
               FALSE for focus in's to the root window */
            if (in_client_only)
                return FALSE;
            /* This means focus reverted off of a client */
            else if (detail == NotifyPointerRoot ||
                     detail == NotifyDetailNone ||
                     detail == NotifyInferior ||
                     /* This means focus got here from another screen */
                     detail == NotifyNonlinear)
                return TRUE;
            else
                return FALSE;
        }

        /* It was on a client, was it a valid one?
           It's possible to get a FocusIn event for a client that was managed
           but has disappeared.
        */
        if (in_client_only) {
            ObWindow *w = window_find(e->xfocus.window);
            if (!w || !WINDOW_IS_CLIENT(w))
                return FALSE;
        }
        else {
            /* This means focus reverted to parent from the client (this
               happens often during iconify animation) */
            if (detail == NotifyInferior)
                return TRUE;
        }

        /* This means focus moved from the root window to a client */
        if (detail == NotifyVirtual)
            return TRUE;
        /* This means focus moved from one client to another */
        if (detail == NotifyNonlinearVirtual)
            return TRUE;

        /* Otherwise.. */
        return FALSE;
    } else {
        g_assert(e->type == FocusOut);

        /* These are ones we never want.. */

        /* This means focus was taken by a keyboard/mouse grab. */
        if (mode == NotifyGrab)
            return FALSE;
        /* This means focus was grabbed on a window and it was released. */
        if (mode == NotifyUngrab)
            return FALSE;

        /* Focus left the root window revertedto state */
        if (win == obt_root(ob_screen))
            return FALSE;

        /* These are the ones we want.. */

        /* This means focus moved from a client to the root window */
        if (detail == NotifyVirtual)
            return TRUE;
        /* This means focus moved from one client to another */
        if (detail == NotifyNonlinearVirtual)
            return TRUE;

        /* Otherwise.. */
        return FALSE;
    }
}

static Bool event_look_for_focusin(Display *d, XEvent *e, XPointer arg)
{
    return e->type == FocusIn && wanted_focusevent(e, FALSE);
}

static Bool event_look_for_focusin_client(Display *d, XEvent *e, XPointer arg)
{
    return e->type == FocusIn && wanted_focusevent(e, TRUE);
}

static void print_focusevent(XEvent *e)
{
    gint mode = e->xfocus.mode;
    gint detail = e->xfocus.detail;
    Window win = e->xany.window;
    const gchar *modestr, *detailstr;

    switch (mode) {
    case NotifyNormal:       modestr="NotifyNormal";       break;
    case NotifyGrab:         modestr="NotifyGrab";         break;
    case NotifyUngrab:       modestr="NotifyUngrab";       break;
    case NotifyWhileGrabbed: modestr="NotifyWhileGrabbed"; break;
    }
    switch (detail) {
    case NotifyAncestor:    detailstr="NotifyAncestor";    break;
    case NotifyVirtual:     detailstr="NotifyVirtual";     break;
    case NotifyInferior:    detailstr="NotifyInferior";    break;
    case NotifyNonlinear:   detailstr="NotifyNonlinear";   break;
    case NotifyNonlinearVirtual: detailstr="NotifyNonlinearVirtual"; break;
    case NotifyPointer:     detailstr="NotifyPointer";     break;
    case NotifyPointerRoot: detailstr="NotifyPointerRoot"; break;
    case NotifyDetailNone:  detailstr="NotifyDetailNone";  break;
    }

    if (mode == NotifyGrab || mode == NotifyUngrab)
        return;

    g_assert(modestr);
    g_assert(detailstr);
    ob_debug_type(OB_DEBUG_FOCUS, "Focus%s 0x%x mode=%s detail=%s",
                  (e->xfocus.type == FocusIn ? "In" : "Out"),
                  win,
                  modestr, detailstr);

}

static gboolean event_ignore(XEvent *e, ObClient *client)
{
    switch(e->type) {
    case FocusIn:
        print_focusevent(e);
        if (!wanted_focusevent(e, FALSE))
            return TRUE;
        break;
    case FocusOut:
        print_focusevent(e);
        if (!wanted_focusevent(e, FALSE))
            return TRUE;
        break;
    }
    return FALSE;
}

static void event_process(const XEvent *ec, gpointer data)
{
    XEvent ee, *e;
    ObEventData *ed = data;

    Window window;
    ObClient *client = NULL;
    ObDock *dock = NULL;
    ObDockApp *dockapp = NULL;
    ObWindow *obwin = NULL;
    ObMenuFrame *menu = NULL;
    ObPrompt *prompt = NULL;

    /* make a copy we can mangle */
    ee = *ec;
    e = &ee;

    window = event_get_window(e);
    if (window == obt_root(ob_screen))
        /* don't do any lookups, waste of cpu */;
    else if ((obwin = window_find(window))) {
        switch (obwin->type) {
        case OB_WINDOW_CLASS_DOCK:
            dock = WINDOW_AS_DOCK(obwin);
            break;
        case OB_WINDOW_CLASS_CLIENT:
            client = WINDOW_AS_CLIENT(obwin);
            /* events on clients can be events on prompt windows too */
            prompt = client->prompt;
            break;
        case OB_WINDOW_CLASS_MENUFRAME:
            menu = WINDOW_AS_MENUFRAME(obwin);
            break;
        case OB_WINDOW_CLASS_INTERNAL:
            /* we don't do anything with events directly on these windows */
            break;
        case OB_WINDOW_CLASS_PROMPT:
            prompt = WINDOW_AS_PROMPT(obwin);
            break;
        }
    }
    else
        dockapp = dock_find_dockapp(window);

    event_set_curtime(e);
    event_curserial = e->xany.serial;
    event_hack_mods(e);
    if (event_ignore(e, client)) {
        if (ed)
            ed->ignored = TRUE;
        return;
    } else if (ed)
            ed->ignored = FALSE;

    /* deal with it in the kernel */

    if (e->type == FocusIn) {
        if (client &&
            e->xfocus.detail == NotifyInferior)
        {
            ob_debug_type(OB_DEBUG_FOCUS,
                          "Focus went to the frame window");

            focus_left_screen = FALSE;

            focus_fallback(FALSE, config_focus_under_mouse, TRUE, TRUE);

            /* We don't get a FocusOut for this case, because it's just moving
               from our Inferior up to us. This happens when iconifying a
               window with RevertToParent focus */
            frame_adjust_focus(client->frame, FALSE);
            /* focus_set_client(NULL) has already been called */
        }
        else if (e->xfocus.detail == NotifyPointerRoot ||
                 e->xfocus.detail == NotifyDetailNone ||
                 e->xfocus.detail == NotifyInferior ||
                 e->xfocus.detail == NotifyNonlinear)
        {
            XEvent ce;

            ob_debug_type(OB_DEBUG_FOCUS,
                          "Focus went to root or pointer root/none");

            if (e->xfocus.detail == NotifyInferior ||
                e->xfocus.detail == NotifyNonlinear)
            {
                focus_left_screen = FALSE;
            }

            /* If another FocusIn is in the queue then don't fallback yet. This
               fixes the fun case of:
               window map -> send focusin
               window unmap -> get focusout
               window map -> send focusin
               get first focus out -> fall back to something (new window
                 hasn't received focus yet, so something else) -> send focusin
               which means the "something else" is the last thing to get a
               focusin sent to it, so the new window doesn't end up with focus.

               But if the other focus in is something like PointerRoot then we
               still want to fall back.
            */
            if (XCheckIfEvent(obt_display, &ce, event_look_for_focusin_client,
                              NULL))
            {
                XPutBackEvent(obt_display, &ce);
                ob_debug_type(OB_DEBUG_FOCUS,
                              "  but another FocusIn is coming");
            } else {
                /* Focus has been reverted.

                   FocusOut events come after UnmapNotify, so we don't need to
                   worry about focusing an invalid window
                */

                if (!focus_left_screen)
                    focus_fallback(FALSE, config_focus_under_mouse,
                                   TRUE, TRUE);
            }
        }
        else if (!client)
        {
            ob_debug_type(OB_DEBUG_FOCUS,
                          "Focus went to a window that is already gone");

            /* If you send focus to a window and then it disappears, you can
               get the FocusIn for it, after it is unmanaged.
               Just wait for the next FocusOut/FocusIn pair, but make note that
               the window that was focused no longer is. */
            focus_set_client(NULL);
        }
        else if (client != focus_client) {
            focus_left_screen = FALSE;
            frame_adjust_focus(client->frame, TRUE);
            focus_set_client(client);
            client_calc_layer(client);
            client_bring_helper_windows(client);
        }
    } else if (e->type == FocusOut) {
        XEvent ce;

        /* Look for the followup FocusIn */
        if (!XCheckIfEvent(obt_display, &ce, event_look_for_focusin, NULL)) {
            /* There is no FocusIn, this means focus went to a window that
               is not being managed, or a window on another screen. */
            Window win, root;
            gint i;
            guint u;
            obt_display_ignore_errors(TRUE);
            if (XGetInputFocus(obt_display, &win, &i) &&
                XGetGeometry(obt_display, win, &root, &i,&i,&u,&u,&u,&u) &&
                root != obt_root(ob_screen))
            {
                ob_debug_type(OB_DEBUG_FOCUS,
                              "Focus went to another screen !");
                focus_left_screen = TRUE;
            }
            else
                ob_debug_type(OB_DEBUG_FOCUS,
                              "Focus went to a black hole !");
            obt_display_ignore_errors(FALSE);
            /* nothing is focused */
            focus_set_client(NULL);
        } else {
            /* Focus moved, so process the FocusIn event */
            ObEventData ed = { .ignored = FALSE };
            event_process(&ce, &ed);
            if (ed.ignored) {
                /* The FocusIn was ignored, this means it was on a window
                   that isn't a client. */
                ob_debug_type(OB_DEBUG_FOCUS,
                              "Focus went to an unmanaged window 0x%x !",
                              ce.xfocus.window);
                focus_fallback(TRUE, config_focus_under_mouse, TRUE, TRUE);
            }
        }

        if (client && client != focus_client) {
            frame_adjust_focus(client->frame, FALSE);
            /* focus_set_client(NULL) has already been called in this
               section or by focus_fallback */
        }
    }
    else if (client)
        event_handle_client(client, e);
    else if (dockapp)
        event_handle_dockapp(dockapp, e);
    else if (dock)
        event_handle_dock(dock, e);
    else if (menu)
        event_handle_menu(menu, e);
    else if (window == obt_root(ob_screen))
        event_handle_root(e);
    else if (e->type == MapRequest)
        window_manage(window);
    else if (e->type == MappingNotify) {
        /* keyboard layout changes for modifier mapping changes. reload the
           modifier map, and rebind all the key bindings as appropriate */
        ob_debug("Kepboard map changed. Reloading keyboard bindings.");
        obt_keyboard_reload();
        keyboard_rebind();
    }
    else if (e->type == ClientMessage) {
        /* This is for _NET_WM_REQUEST_FRAME_EXTENTS messages. They come for
           windows that are not managed yet. */
        if (e->xclient.message_type ==
            OBT_PROP_ATOM(NET_REQUEST_FRAME_EXTENTS))
        {
            /* Pretend to manage the client, getting information used to
               determine its decorations */
            ObClient *c = client_fake_manage(e->xclient.window);
            gulong vals[4];

            /* set the frame extents on the window */
            vals[0] = c->frame->size.left;
            vals[1] = c->frame->size.right;
            vals[2] = c->frame->size.top;
            vals[3] = c->frame->size.bottom;
            OBT_PROP_SETA32(e->xclient.window, NET_FRAME_EXTENTS,
                            CARDINAL, vals, 4);

            /* Free the pretend client */
            client_fake_unmanage(c);
        }
    }
    else if (e->type == ConfigureRequest) {
        /* unhandled configure requests must be used to configure the
           window directly */
        XWindowChanges xwc;

        xwc.x = e->xconfigurerequest.x;
        xwc.y = e->xconfigurerequest.y;
        xwc.width = e->xconfigurerequest.width;
        xwc.height = e->xconfigurerequest.height;
        xwc.border_width = e->xconfigurerequest.border_width;
        xwc.sibling = e->xconfigurerequest.above;
        xwc.stack_mode = e->xconfigurerequest.detail;

        /* we are not to be held responsible if someone sends us an
           invalid request! */
        obt_display_ignore_errors(TRUE);
        XConfigureWindow(obt_display, window,
                         e->xconfigurerequest.value_mask, &xwc);
        obt_display_ignore_errors(FALSE);
    }
#ifdef SYNC
    else if (obt_display_extension_sync &&
             e->type == obt_display_extension_sync_basep + XSyncAlarmNotify)
    {
        XSyncAlarmNotifyEvent *se = (XSyncAlarmNotifyEvent*)e;
        if (se->alarm == moveresize_alarm && moveresize_in_progress)
            moveresize_event(e);
    }
#endif

    if (prompt && event_handle_prompt(prompt, e))
        ;
    else if (e->type == ButtonPress || e->type == ButtonRelease) {
        /* If the button press was on some non-root window, or was physically
           on the root window, then process it */
        if (window != obt_root(ob_screen) ||
            e->xbutton.subwindow == None)
        {
            event_handle_user_input(client, e);
        }
        /* Otherwise only process it if it was physically on an openbox
           internal window */
        else {
            ObWindow *w;

            if ((w = window_find(e->xbutton.subwindow)) &&
                WINDOW_IS_INTERNAL(w))
            {
                event_handle_user_input(client, e);
            }
        }
    }
    else if (e->type == KeyPress || e->type == KeyRelease ||
             e->type == MotionNotify)
        event_handle_user_input(client, e);

    /* if something happens and it's not from an XEvent, then we don't know
       the time */
    event_curtime = CurrentTime;
    event_curserial = 0;
}

static void event_handle_root(XEvent *e)
{
    Atom msgtype;

    switch(e->type) {
    case SelectionClear:
        ob_debug("Another WM has requested to replace us. Exiting.");
        ob_exit_replace();
        break;

    case ClientMessage:
        if (e->xclient.format != 32) break;

        msgtype = e->xclient.message_type;
        if (msgtype == OBT_PROP_ATOM(NET_CURRENT_DESKTOP)) {
            guint d = e->xclient.data.l[0];
            if (d < screen_num_desktops) {
                event_curtime = e->xclient.data.l[1];
                if (event_curtime == 0)
                    ob_debug_type(OB_DEBUG_APP_BUGS,
                                  "_NET_CURRENT_DESKTOP message is missing "
                                  "a timestamp");
                screen_set_desktop(d, TRUE);
            }
        } else if (msgtype == OBT_PROP_ATOM(NET_NUMBER_OF_DESKTOPS)) {
            guint d = e->xclient.data.l[0];
            if (d > 0 && d <= 1000)
                screen_set_num_desktops(d);
        } else if (msgtype == OBT_PROP_ATOM(NET_SHOWING_DESKTOP)) {
            screen_show_desktop(e->xclient.data.l[0] != 0, NULL);
        } else if (msgtype == OBT_PROP_ATOM(OB_CONTROL)) {
            ob_debug("OB_CONTROL: %d", e->xclient.data.l[0]);
            if (e->xclient.data.l[0] == 1)
                ob_reconfigure();
            else if (e->xclient.data.l[0] == 2)
                ob_restart();
            else if (e->xclient.data.l[0] == 3)
                ob_exit(0);
        } else if (msgtype == OBT_PROP_ATOM(WM_PROTOCOLS)) {
            if ((Atom)e->xclient.data.l[0] == OBT_PROP_ATOM(NET_WM_PING))
                ping_got_pong(e->xclient.data.l[1]);
        }
        break;
    case PropertyNotify:
        if (e->xproperty.atom == OBT_PROP_ATOM(NET_DESKTOP_NAMES)) {
            ob_debug("UPDATE DESKTOP NAMES");
            screen_update_desktop_names();
        }
        else if (e->xproperty.atom == OBT_PROP_ATOM(NET_DESKTOP_LAYOUT))
            screen_update_layout();
        break;
    case ConfigureNotify:
#ifdef XRANDR
        XRRUpdateConfiguration(e);
#endif
        screen_resize();
        break;
    default:
        ;
    }
}

void event_enter_client(ObClient *client)
{
    g_assert(config_focus_follow);

    if (is_enter_focus_event_ignored(event_curserial)) {
        ob_debug_type(OB_DEBUG_FOCUS, "Ignoring enter event with serial %lu\n"
                      "on client 0x%x", event_curserial, client->window);
        return;
    }

    if (client_enter_focusable(client) && client_can_focus(client)) {
        if (config_focus_delay) {
            ObFocusDelayData *data;

            obt_main_loop_timeout_remove(ob_main_loop, focus_delay_func);

            data = g_new(ObFocusDelayData, 1);
            data->client = client;
            data->time = event_curtime;
            data->serial = event_curserial;

            obt_main_loop_timeout_add(ob_main_loop,
                                      config_focus_delay * 1000,
                                      focus_delay_func,
                                      data, focus_delay_cmp, focus_delay_dest);
        } else {
            ObFocusDelayData data;
            data.client = client;
            data.time = event_curtime;
            data.serial = event_curserial;
            focus_delay_func(&data);
        }
    }
}

static void event_handle_client(ObClient *client, XEvent *e)
{
    XEvent ce;
    Atom msgtype;
    ObFrameContext con;
    static gint px = -1, py = -1;
    static guint pb = 0;

    switch (e->type) {
    case ButtonPress:
        /* save where the press occured for the first button pressed */
        if (!pb) {
            pb = e->xbutton.button;
            px = e->xbutton.x;
            py = e->xbutton.y;
        }
    case ButtonRelease:
        /* Wheel buttons don't draw because they are an instant click, so it
           is a waste of resources to go drawing it.
           if the user is doing an intereactive thing, or has a menu open then
           the mouse is grabbed (possibly) and if we get these events we don't
           want to deal with them
        */
        if (!(e->xbutton.button == 4 || e->xbutton.button == 5) &&
            !grab_on_keyboard())
        {
            /* use where the press occured */
            con = frame_context(client, e->xbutton.window, px, py);
            con = mouse_button_frame_context(con, e->xbutton.button,
                                             e->xbutton.state);

            if (e->type == ButtonRelease && e->xbutton.button == pb)
                pb = 0, px = py = -1;

            switch (con) {
            case OB_FRAME_CONTEXT_MAXIMIZE:
                client->frame->max_press = (e->type == ButtonPress);
                frame_adjust_state(client->frame);
                break;
            case OB_FRAME_CONTEXT_CLOSE:
                client->frame->close_press = (e->type == ButtonPress);
                frame_adjust_state(client->frame);
                break;
            case OB_FRAME_CONTEXT_ICONIFY:
                client->frame->iconify_press = (e->type == ButtonPress);
                frame_adjust_state(client->frame);
                break;
            case OB_FRAME_CONTEXT_ALLDESKTOPS:
                client->frame->desk_press = (e->type == ButtonPress);
                frame_adjust_state(client->frame);
                break;
            case OB_FRAME_CONTEXT_SHADE:
                client->frame->shade_press = (e->type == ButtonPress);
                frame_adjust_state(client->frame);
                break;
            default:
                /* nothing changes with clicks for any other contexts */
                break;
            }
        }
        break;
    case MotionNotify:
        /* when there is a grab on the pointer, we won't get enter/leave
           notifies, but we still get motion events */
        if (grab_on_pointer()) break;

        con = frame_context(client, e->xmotion.window,
                            e->xmotion.x, e->xmotion.y);
        switch (con) {
        case OB_FRAME_CONTEXT_TITLEBAR:
        case OB_FRAME_CONTEXT_TLCORNER:
        case OB_FRAME_CONTEXT_TRCORNER:
            /* we've left the button area inside the titlebar */
            if (client->frame->max_hover || client->frame->desk_hover ||
                client->frame->shade_hover || client->frame->iconify_hover ||
                client->frame->close_hover)
            {
                client->frame->max_hover = FALSE;
                client->frame->desk_hover = FALSE;
                client->frame->shade_hover = FALSE;
                client->frame->iconify_hover = FALSE;
                client->frame->close_hover = FALSE;
                frame_adjust_state(client->frame);
            }
            break;
        case OB_FRAME_CONTEXT_MAXIMIZE:
            if (!client->frame->max_hover) {
                client->frame->max_hover = TRUE;
                frame_adjust_state(client->frame);
            }
            break;
        case OB_FRAME_CONTEXT_ALLDESKTOPS:
            if (!client->frame->desk_hover) {
                client->frame->desk_hover = TRUE;
                frame_adjust_state(client->frame);
            }
            break;
        case OB_FRAME_CONTEXT_SHADE:
            if (!client->frame->shade_hover) {
                client->frame->shade_hover = TRUE;
                frame_adjust_state(client->frame);
            }
            break;
        case OB_FRAME_CONTEXT_ICONIFY:
            if (!client->frame->iconify_hover) {
                client->frame->iconify_hover = TRUE;
                frame_adjust_state(client->frame);
            }
            break;
        case OB_FRAME_CONTEXT_CLOSE:
            if (!client->frame->close_hover) {
                client->frame->close_hover = TRUE;
                frame_adjust_state(client->frame);
            }
            break;
        default:
            break;
        }
        break;
    case LeaveNotify:
        con = frame_context(client, e->xcrossing.window,
                            e->xcrossing.x, e->xcrossing.y);
        switch (con) {
        case OB_FRAME_CONTEXT_TITLEBAR:
        case OB_FRAME_CONTEXT_TLCORNER:
        case OB_FRAME_CONTEXT_TRCORNER:
            /* we've left the button area inside the titlebar */
            if (client->frame->max_hover || client->frame->desk_hover ||
                client->frame->shade_hover || client->frame->iconify_hover ||
                client->frame->close_hover)
            {
                client->frame->max_hover = FALSE;
                client->frame->desk_hover = FALSE;
                client->frame->shade_hover = FALSE;
                client->frame->iconify_hover = FALSE;
                client->frame->close_hover = FALSE;
                frame_adjust_state(client->frame);
            }
            break;
        case OB_FRAME_CONTEXT_MAXIMIZE:
            client->frame->max_hover = FALSE;
            frame_adjust_state(client->frame);
            break;
        case OB_FRAME_CONTEXT_ALLDESKTOPS:
            client->frame->desk_hover = FALSE;
            frame_adjust_state(client->frame);
            break;
        case OB_FRAME_CONTEXT_SHADE:
            client->frame->shade_hover = FALSE;
            frame_adjust_state(client->frame);
            break;
        case OB_FRAME_CONTEXT_ICONIFY:
            client->frame->iconify_hover = FALSE;
            frame_adjust_state(client->frame);
            break;
        case OB_FRAME_CONTEXT_CLOSE:
            client->frame->close_hover = FALSE;
            frame_adjust_state(client->frame);
            break;
        case OB_FRAME_CONTEXT_FRAME:
            /* When the mouse leaves an animating window, don't use the
               corresponding enter events. Pretend like the animating window
               doesn't even exist..! */
            if (frame_iconify_animating(client->frame))
                event_end_ignore_all_enters(event_start_ignore_all_enters());

            ob_debug_type(OB_DEBUG_FOCUS,
                          "%sNotify mode %d detail %d on %lx",
                          (e->type == EnterNotify ? "Enter" : "Leave"),
                          e->xcrossing.mode,
                          e->xcrossing.detail, (client?client->window:0));
            if (grab_on_keyboard())
                break;
            if (config_focus_follow && config_focus_delay &&
                /* leave inferior events can happen when the mouse goes onto
                   the window's border and then into the window before the
                   delay is up */
                e->xcrossing.detail != NotifyInferior)
            {
                obt_main_loop_timeout_remove_data(ob_main_loop,
                                                  focus_delay_func,
                                                  client, FALSE);
            }
            break;
        default:
            break;
        }
        break;
    case EnterNotify:
    {
        con = frame_context(client, e->xcrossing.window,
                            e->xcrossing.x, e->xcrossing.y);
        switch (con) {
        case OB_FRAME_CONTEXT_MAXIMIZE:
            client->frame->max_hover = TRUE;
            frame_adjust_state(client->frame);
            break;
        case OB_FRAME_CONTEXT_ALLDESKTOPS:
            client->frame->desk_hover = TRUE;
            frame_adjust_state(client->frame);
            break;
        case OB_FRAME_CONTEXT_SHADE:
            client->frame->shade_hover = TRUE;
            frame_adjust_state(client->frame);
            break;
        case OB_FRAME_CONTEXT_ICONIFY:
            client->frame->iconify_hover = TRUE;
            frame_adjust_state(client->frame);
            break;
        case OB_FRAME_CONTEXT_CLOSE:
            client->frame->close_hover = TRUE;
            frame_adjust_state(client->frame);
            break;
        case OB_FRAME_CONTEXT_FRAME:
            if (grab_on_keyboard())
                break;
            if (e->xcrossing.mode == NotifyGrab ||
                e->xcrossing.mode == NotifyUngrab ||
                /*ignore enters when we're already in the window */
                e->xcrossing.detail == NotifyInferior)
            {
                ob_debug_type(OB_DEBUG_FOCUS,
                              "%sNotify mode %d detail %d serial %lu on %lx "
                              "IGNORED",
                              (e->type == EnterNotify ? "Enter" : "Leave"),
                              e->xcrossing.mode,
                              e->xcrossing.detail,
                              e->xcrossing.serial,
                              client?client->window:0);
            }
            else {
                ob_debug_type(OB_DEBUG_FOCUS,
                              "%sNotify mode %d detail %d serial %lu on %lx, "
                              "focusing window",
                              (e->type == EnterNotify ? "Enter" : "Leave"),
                              e->xcrossing.mode,
                              e->xcrossing.detail,
                              e->xcrossing.serial,
                              (client?client->window:0));
                if (config_focus_follow)
                    event_enter_client(client);
            }
            break;
        default:
            break;
        }
        break;
    }
    case ConfigureRequest:
    {
        /* dont compress these unless you're going to watch for property
           notifies in between (these can change what the configure would
           do to the window).
           also you can't compress stacking events
        */

        gint x, y, w, h;
        gboolean move = FALSE;
        gboolean resize = FALSE;

        /* get the current area */
        RECT_TO_DIMS(client->area, x, y, w, h);

        ob_debug("ConfigureRequest for \"%s\" desktop %d wmstate %d "
                 "visibile %d",
                 client->title,
                 screen_desktop, client->wmstate, client->frame->visible);
        ob_debug("                     x %d y %d w %d h %d b %d",
                 x, y, w, h, client->border_width);

        if (e->xconfigurerequest.value_mask & CWBorderWidth)
            if (client->border_width != e->xconfigurerequest.border_width) {
                client->border_width = e->xconfigurerequest.border_width;

                /* if the border width is changing then that is the same
                   as requesting a resize, but we don't actually change
                   the client's border, so it will change their root
                   coordiantes (since they include the border width) and
                   we need to a notify then */
                move = TRUE;
            }


        if (e->xconfigurerequest.value_mask & CWStackMode) {
            ObClient *sibling = NULL;
            gulong ignore_start;
            gboolean ok = TRUE;

            /* get the sibling */
            if (e->xconfigurerequest.value_mask & CWSibling) {
                ObWindow *win;
                win = window_find(e->xconfigurerequest.above);
                if (win && WINDOW_IS_CLIENT(win) &&
                    WINDOW_AS_CLIENT(win) != client)
                {
                    sibling = WINDOW_AS_CLIENT(win);
                }
                else
                    /* an invalid sibling was specified so don't restack at
                       all, it won't make sense no matter what we do */
                    ok = FALSE;
            }

            if (ok) {
                if (!config_focus_under_mouse)
                    ignore_start = event_start_ignore_all_enters();
                stacking_restack_request(client, sibling,
                                         e->xconfigurerequest.detail);
                if (!config_focus_under_mouse)
                    event_end_ignore_all_enters(ignore_start);
            }

            /* a stacking change moves the window without resizing */
            move = TRUE;
        }

        if ((e->xconfigurerequest.value_mask & CWX) ||
            (e->xconfigurerequest.value_mask & CWY) ||
            (e->xconfigurerequest.value_mask & CWWidth) ||
            (e->xconfigurerequest.value_mask & CWHeight))
        {
            if (e->xconfigurerequest.value_mask & CWX) {
                /* don't allow clients to move shaded windows (fvwm does this)
                 */
                if (!client->shaded)
                    x = e->xconfigurerequest.x;
                move = TRUE;
            }
            if (e->xconfigurerequest.value_mask & CWY) {
                /* don't allow clients to move shaded windows (fvwm does this)
                 */
                if (!client->shaded)
                    y = e->xconfigurerequest.y;
                move = TRUE;
            }

            if (e->xconfigurerequest.value_mask & CWWidth) {
                w = e->xconfigurerequest.width;
                resize = TRUE;
            }
            if (e->xconfigurerequest.value_mask & CWHeight) {
                h = e->xconfigurerequest.height;
                resize = TRUE;
            }
        }

        ob_debug("ConfigureRequest x(%d) %d y(%d) %d w(%d) %d h(%d) %d "
                 "move %d resize %d",
                 e->xconfigurerequest.value_mask & CWX, x,
                 e->xconfigurerequest.value_mask & CWY, y,
                 e->xconfigurerequest.value_mask & CWWidth, w,
                 e->xconfigurerequest.value_mask & CWHeight, h,
                 move, resize);

        /* check for broken apps moving to their root position

           XXX remove this some day...that would be nice. right now all
           kde apps do this when they try activate themselves on another
           desktop. eg. open amarok window on desktop 1, switch to desktop
           2, click amarok tray icon. it will move by its decoration size.
        */
        if (x != client->area.x &&
            x == (client->frame->area.x + client->frame->size.left -
                  (gint)client->border_width) &&
            y != client->area.y &&
            y == (client->frame->area.y + client->frame->size.top -
                  (gint)client->border_width) &&
            w == client->area.width &&
            h == client->area.height)
        {
            ob_debug_type(OB_DEBUG_APP_BUGS,
                          "Application %s is trying to move via "
                          "ConfigureRequest to it's root window position "
                          "but it is not using StaticGravity",
                          client->title);
            /* don't move it */
            x = client->area.x;
            y = client->area.y;

            /* they still requested a move, so don't change whether a
               notify is sent or not */
        }

        {
            gint lw,lh;

            client_try_configure(client, &x, &y, &w, &h, &lw, &lh, FALSE);

            /* if x was not given, then use gravity to figure out the new
               x.  the reference point should not be moved */
            if ((e->xconfigurerequest.value_mask & CWWidth &&
                 !(e->xconfigurerequest.value_mask & CWX)))
                client_gravity_resize_w(client, &x, client->area.width, w);
            /* if y was not given, then use gravity to figure out the new
               y.  the reference point should not be moved */
            if ((e->xconfigurerequest.value_mask & CWHeight &&
                 !(e->xconfigurerequest.value_mask & CWY)))
                client_gravity_resize_h(client, &y, client->area.height,h);

            client_find_onscreen(client, &x, &y, w, h, FALSE);

            ob_debug("Granting ConfigureRequest x %d y %d w %d h %d",
                     x, y, w, h);
            client_configure(client, x, y, w, h, FALSE, TRUE, TRUE);
        }
        break;
    }
    case UnmapNotify:
        ob_debug("UnmapNotify for window 0x%x eventwin 0x%x sendevent %d "
                 "ignores left %d",
                 client->window, e->xunmap.event, e->xunmap.from_configure,
                 client->ignore_unmaps);
        if (client->ignore_unmaps) {
            client->ignore_unmaps--;
            break;
        }
        client_unmanage(client);
        break;
    case DestroyNotify:
        ob_debug("DestroyNotify for window 0x%x", client->window);
        client_unmanage(client);
        break;
    case ReparentNotify:
        /* this is when the client is first taken captive in the frame */
        if (e->xreparent.parent == client->frame->window) break;

        /*
          This event is quite rare and is usually handled in unmapHandler.
          However, if the window is unmapped when the reparent event occurs,
          the window manager never sees it because an unmap event is not sent
          to an already unmapped window.
        */

        /* we don't want the reparent event, put it back on the stack for the
           X server to deal with after we unmanage the window */
        XPutBackEvent(obt_display, e);

        ob_debug("ReparentNotify for window 0x%x", client->window);
        client_unmanage(client);
        break;
    case MapRequest:
        ob_debug("MapRequest for 0x%lx", client->window);
        if (!client->iconic) break; /* this normally doesn't happen, but if it
                                       does, we don't want it!
                                       it can happen now when the window is on
                                       another desktop, but we still don't
                                       want it! */
        client_activate(client, FALSE, TRUE, TRUE, TRUE);
        break;
    case ClientMessage:
        /* validate cuz we query stuff off the client here */
        if (!client_validate(client)) break;

        if (e->xclient.format != 32) return;

        msgtype = e->xclient.message_type;
        if (msgtype == OBT_PROP_ATOM(WM_CHANGE_STATE)) {
            /* compress changes into a single change */
            while (XCheckTypedWindowEvent(obt_display, client->window,
                                          e->type, &ce)) {
                /* XXX: it would be nice to compress ALL messages of a
                   type, not just messages in a row without other
                   message types between. */
                if (ce.xclient.message_type != msgtype) {
                    XPutBackEvent(obt_display, &ce);
                    break;
                }
                e->xclient = ce.xclient;
            }
            client_set_wm_state(client, e->xclient.data.l[0]);
        } else if (msgtype == OBT_PROP_ATOM(NET_WM_DESKTOP)) {
            /* compress changes into a single change */
            while (XCheckTypedWindowEvent(obt_display, client->window,
                                          e->type, &ce)) {
                /* XXX: it would be nice to compress ALL messages of a
                   type, not just messages in a row without other
                   message types between. */
                if (ce.xclient.message_type != msgtype) {
                    XPutBackEvent(obt_display, &ce);
                    break;
                }
                e->xclient = ce.xclient;
            }
            if ((unsigned)e->xclient.data.l[0] < screen_num_desktops ||
                (unsigned)e->xclient.data.l[0] == DESKTOP_ALL)
                client_set_desktop(client, (unsigned)e->xclient.data.l[0],
                                   FALSE, FALSE);
        } else if (msgtype == OBT_PROP_ATOM(NET_WM_STATE)) {
            gulong ignore_start;

            /* can't compress these */
            ob_debug("net_wm_state %s %ld %ld for 0x%lx",
                     (e->xclient.data.l[0] == 0 ? "Remove" :
                      e->xclient.data.l[0] == 1 ? "Add" :
                      e->xclient.data.l[0] == 2 ? "Toggle" : "INVALID"),
                     e->xclient.data.l[1], e->xclient.data.l[2],
                     client->window);

            /* ignore enter events caused by these like ob actions do */
            if (!config_focus_under_mouse)
                ignore_start = event_start_ignore_all_enters();
            client_set_state(client, e->xclient.data.l[0],
                             e->xclient.data.l[1], e->xclient.data.l[2]);
            if (!config_focus_under_mouse)
                event_end_ignore_all_enters(ignore_start);
        } else if (msgtype == OBT_PROP_ATOM(NET_CLOSE_WINDOW)) {
            ob_debug("net_close_window for 0x%lx", client->window);
            client_close(client);
        } else if (msgtype == OBT_PROP_ATOM(NET_ACTIVE_WINDOW)) {
            ob_debug("net_active_window for 0x%lx source=%s",
                     client->window,
                     (e->xclient.data.l[0] == 0 ? "unknown" :
                      (e->xclient.data.l[0] == 1 ? "application" :
                       (e->xclient.data.l[0] == 2 ? "user" : "INVALID"))));
            /* XXX make use of data.l[2] !? */
            if (e->xclient.data.l[0] == 1 || e->xclient.data.l[0] == 2) {
                /* don't use the user's timestamp for client_focus, cuz if it's
                   an old broken timestamp (happens all the time) then focus
                   won't move even though we're trying to move it
                  event_curtime = e->xclient.data.l[1];*/
                if (e->xclient.data.l[1] == 0)
                    ob_debug_type(OB_DEBUG_APP_BUGS,
                                  "_NET_ACTIVE_WINDOW message for window %s is"
                                  " missing a timestamp", client->title);
            } else
                ob_debug_type(OB_DEBUG_APP_BUGS,
                              "_NET_ACTIVE_WINDOW message for window %s is "
                              "missing source indication");
            client_activate(client, TRUE, TRUE, TRUE,
                            (e->xclient.data.l[0] == 0 ||
                             e->xclient.data.l[0] == 2));
        } else if (msgtype == OBT_PROP_ATOM(NET_WM_MOVERESIZE)) {
            ob_debug("net_wm_moveresize for 0x%lx direction %d",
                     client->window, e->xclient.data.l[2]);
            if ((Atom)e->xclient.data.l[2] ==
                OBT_PROP_ATOM(NET_WM_MOVERESIZE_SIZE_TOPLEFT) ||
                (Atom)e->xclient.data.l[2] ==
                OBT_PROP_ATOM(NET_WM_MOVERESIZE_SIZE_TOP) ||
                (Atom)e->xclient.data.l[2] ==
                OBT_PROP_ATOM(NET_WM_MOVERESIZE_SIZE_TOPRIGHT) ||
                (Atom)e->xclient.data.l[2] ==
                OBT_PROP_ATOM(NET_WM_MOVERESIZE_SIZE_RIGHT) ||
                (Atom)e->xclient.data.l[2] ==
                OBT_PROP_ATOM(NET_WM_MOVERESIZE_SIZE_RIGHT) ||
                (Atom)e->xclient.data.l[2] ==
                OBT_PROP_ATOM(NET_WM_MOVERESIZE_SIZE_BOTTOMRIGHT) ||
                (Atom)e->xclient.data.l[2] ==
                OBT_PROP_ATOM(NET_WM_MOVERESIZE_SIZE_BOTTOM) ||
                (Atom)e->xclient.data.l[2] ==
                OBT_PROP_ATOM(NET_WM_MOVERESIZE_SIZE_BOTTOMLEFT) ||
                (Atom)e->xclient.data.l[2] ==
                OBT_PROP_ATOM(NET_WM_MOVERESIZE_SIZE_LEFT) ||
                (Atom)e->xclient.data.l[2] ==
                OBT_PROP_ATOM(NET_WM_MOVERESIZE_MOVE) ||
                (Atom)e->xclient.data.l[2] ==
                OBT_PROP_ATOM(NET_WM_MOVERESIZE_SIZE_KEYBOARD) ||
                (Atom)e->xclient.data.l[2] ==
                OBT_PROP_ATOM(NET_WM_MOVERESIZE_MOVE_KEYBOARD))
            {
                moveresize_start(client, e->xclient.data.l[0],
                                 e->xclient.data.l[1], e->xclient.data.l[3],
                                 e->xclient.data.l[2]);
            }
            else if ((Atom)e->xclient.data.l[2] ==
                     OBT_PROP_ATOM(NET_WM_MOVERESIZE_CANCEL))
                moveresize_end(TRUE);
        } else if (msgtype == OBT_PROP_ATOM(NET_MOVERESIZE_WINDOW)) {
            gint ograv, x, y, w, h;

            ograv = client->gravity;

            if (e->xclient.data.l[0] & 0xff)
                client->gravity = e->xclient.data.l[0] & 0xff;

            if (e->xclient.data.l[0] & 1 << 8)
                x = e->xclient.data.l[1];
            else
                x = client->area.x;
            if (e->xclient.data.l[0] & 1 << 9)
                y = e->xclient.data.l[2];
            else
                y = client->area.y;

            if (e->xclient.data.l[0] & 1 << 10) {
                w = e->xclient.data.l[3];

                /* if x was not given, then use gravity to figure out the new
                   x.  the reference point should not be moved */
                if (!(e->xclient.data.l[0] & 1 << 8))
                    client_gravity_resize_w(client, &x, client->area.width, w);
            }
            else
                w = client->area.width;

            if (e->xclient.data.l[0] & 1 << 11) {
                h = e->xclient.data.l[4];

                /* if y was not given, then use gravity to figure out the new
                   y.  the reference point should not be moved */
                if (!(e->xclient.data.l[0] & 1 << 9))
                    client_gravity_resize_h(client, &y, client->area.height,h);
            }
            else
                h = client->area.height;

            ob_debug("MOVERESIZE x %d %d y %d %d (gravity %d)",
                     e->xclient.data.l[0] & 1 << 8, x,
                     e->xclient.data.l[0] & 1 << 9, y,
                     client->gravity);

            client_find_onscreen(client, &x, &y, w, h, FALSE);

            client_configure(client, x, y, w, h, FALSE, TRUE, FALSE);

            client->gravity = ograv;
        } else if (msgtype == OBT_PROP_ATOM(NET_RESTACK_WINDOW)) {
            if (e->xclient.data.l[0] != 2) {
                ob_debug_type(OB_DEBUG_APP_BUGS,
                              "_NET_RESTACK_WINDOW sent for window %s with "
                              "invalid source indication %ld",
                              client->title, e->xclient.data.l[0]);
            } else {
                ObClient *sibling = NULL;
                if (e->xclient.data.l[1]) {
                    ObWindow *win = window_find(e->xclient.data.l[1]);
                    if (WINDOW_IS_CLIENT(win) &&
                        WINDOW_AS_CLIENT(win) != client)
                    {
                        sibling = WINDOW_AS_CLIENT(win);
                    }
                    if (sibling == NULL)
                        ob_debug_type(OB_DEBUG_APP_BUGS,
                                      "_NET_RESTACK_WINDOW sent for window %s "
                                      "with invalid sibling 0x%x",
                                 client->title, e->xclient.data.l[1]);
                }
                if (e->xclient.data.l[2] == Below ||
                    e->xclient.data.l[2] == BottomIf ||
                    e->xclient.data.l[2] == Above ||
                    e->xclient.data.l[2] == TopIf ||
                    e->xclient.data.l[2] == Opposite)
                {
                    gulong ignore_start;

                    if (!config_focus_under_mouse)
                        ignore_start = event_start_ignore_all_enters();
                    /* just raise, don't activate */
                    stacking_restack_request(client, sibling,
                                             e->xclient.data.l[2]);
                    if (!config_focus_under_mouse)
                        event_end_ignore_all_enters(ignore_start);

                    /* send a synthetic ConfigureNotify, cuz this is supposed
                       to be like a ConfigureRequest. */
                    client_reconfigure(client, TRUE);
                } else
                    ob_debug_type(OB_DEBUG_APP_BUGS,
                                  "_NET_RESTACK_WINDOW sent for window %s "
                                  "with invalid detail %d",
                                  client->title, e->xclient.data.l[2]);
            }
        }
        break;
    case PropertyNotify:
        /* validate cuz we query stuff off the client here */
        if (!client_validate(client)) break;

        /* compress changes to a single property into a single change */
        while (XCheckTypedWindowEvent(obt_display, client->window,
                                      e->type, &ce)) {
            Atom a, b;

            /* XXX: it would be nice to compress ALL changes to a property,
               not just changes in a row without other props between. */

            a = ce.xproperty.atom;
            b = e->xproperty.atom;

            if (a == b)
                continue;
            if ((a == OBT_PROP_ATOM(NET_WM_NAME) ||
                 a == OBT_PROP_ATOM(WM_NAME) ||
                 a == OBT_PROP_ATOM(NET_WM_ICON_NAME) ||
                 a == OBT_PROP_ATOM(WM_ICON_NAME))
                &&
                (b == OBT_PROP_ATOM(NET_WM_NAME) ||
                 b == OBT_PROP_ATOM(WM_NAME) ||
                 b == OBT_PROP_ATOM(NET_WM_ICON_NAME) ||
                 b == OBT_PROP_ATOM(WM_ICON_NAME))) {
                continue;
            }
            if (a == OBT_PROP_ATOM(NET_WM_ICON) &&
                b == OBT_PROP_ATOM(NET_WM_ICON))
                continue;

            XPutBackEvent(obt_display, &ce);
            break;
        }

        msgtype = e->xproperty.atom;
        if (msgtype == XA_WM_NORMAL_HINTS) {
            ob_debug("Update NORMAL hints");
            client_update_normal_hints(client);
            /* normal hints can make a window non-resizable */
            client_setup_decor_and_functions(client, FALSE);

            /* make sure the client's sizes are within its bounds, but only
               reconfigure the window if it needs to. emacs will update its
               normal hints every time it receives a conigurenotify */
            client_reconfigure(client, FALSE);
        } else if (msgtype == XA_WM_HINTS) {
            client_update_wmhints(client);
        } else if (msgtype == XA_WM_TRANSIENT_FOR) {
            client_update_transient_for(client);
            client_get_type_and_transientness(client);
            /* type may have changed, so update the layer */
            client_calc_layer(client);
            client_setup_decor_and_functions(client, TRUE);
        } else if (msgtype == OBT_PROP_ATOM(NET_WM_NAME) ||
                   msgtype == OBT_PROP_ATOM(WM_NAME) ||
                   msgtype == OBT_PROP_ATOM(NET_WM_ICON_NAME) ||
                   msgtype == OBT_PROP_ATOM(WM_ICON_NAME)) {
            client_update_title(client);
        } else if (msgtype == OBT_PROP_ATOM(WM_PROTOCOLS)) {
            client_update_protocols(client);
            client_setup_decor_and_functions(client, TRUE);
        }
        else if (msgtype == OBT_PROP_ATOM(NET_WM_STRUT)) {
            client_update_strut(client);
        }
        else if (msgtype == OBT_PROP_ATOM(NET_WM_STRUT_PARTIAL)) {
            client_update_strut(client);
        }
        else if (msgtype == OBT_PROP_ATOM(NET_WM_ICON)) {
            client_update_icons(client);
        }
        else if (msgtype == OBT_PROP_ATOM(NET_WM_ICON_GEOMETRY)) {
            client_update_icon_geometry(client);
        }
        else if (msgtype == OBT_PROP_ATOM(NET_WM_USER_TIME)) {
            guint32 t;
            if (client == focus_client &&
                OBT_PROP_GET32(client->window, NET_WM_USER_TIME, CARDINAL, &t)
                && t && !event_time_after(t, e->xproperty.time) &&
                (!event_last_user_time ||
                 event_time_after(t, event_last_user_time)))
            {
                event_last_user_time = t;
            }
        }
#ifdef SYNC
        else if (msgtype == OBT_PROP_ATOM(NET_WM_SYNC_REQUEST_COUNTER)) {
            client_update_sync_request_counter(client);
        }
#endif
        break;
    case ColormapNotify:
        client_update_colormap(client, e->xcolormap.colormap);
        break;
    default:
        ;
#ifdef SHAPE
        if (obt_display_extension_shape &&
            e->type == obt_display_extension_shape_basep)
        {
            client->shaped = ((XShapeEvent*)e)->shaped;
            frame_adjust_shape(client->frame);
        }
#endif
    }
}

static void event_handle_dock(ObDock *s, XEvent *e)
{
    switch (e->type) {
    case ButtonPress:
        if (e->xbutton.button == 1)
            stacking_raise(DOCK_AS_WINDOW(s));
        else if (e->xbutton.button == 2)
            stacking_lower(DOCK_AS_WINDOW(s));
        break;
    case EnterNotify:
        dock_hide(FALSE);
        break;
    case LeaveNotify:
        /* don't hide when moving into a dock app */
        if (e->xcrossing.detail != NotifyInferior)
            dock_hide(TRUE);
        break;
    }
}

static void event_handle_dockapp(ObDockApp *app, XEvent *e)
{
    switch (e->type) {
    case MotionNotify:
        dock_app_drag(app, &e->xmotion);
        break;
    case UnmapNotify:
        if (app->ignore_unmaps) {
            app->ignore_unmaps--;
            break;
        }
        dock_unmanage(app, TRUE);
        break;
    case DestroyNotify:
        dock_unmanage(app, FALSE);
        break;
    case ReparentNotify:
        dock_unmanage(app, FALSE);
        break;
    case ConfigureNotify:
        dock_app_configure(app, e->xconfigure.width, e->xconfigure.height);
        break;
    }
}

static ObMenuFrame* find_active_menu(void)
{
    GList *it;
    ObMenuFrame *ret = NULL;

    for (it = menu_frame_visible; it; it = g_list_next(it)) {
        ret = it->data;
        if (ret->selected)
            break;
        ret = NULL;
    }
    return ret;
}

static ObMenuFrame* find_active_or_last_menu(void)
{
    ObMenuFrame *ret = NULL;

    ret = find_active_menu();
    if (!ret && menu_frame_visible)
        ret = menu_frame_visible->data;
    return ret;
}

static gboolean event_handle_prompt(ObPrompt *p, XEvent *e)
{
    switch (e->type) {
    case ButtonPress:
    case ButtonRelease:
    case MotionNotify:
        return prompt_mouse_event(p, e);
        break;
    case KeyPress:
        return prompt_key_event(p, e);
        break;
    }
    return FALSE;
}

static gboolean event_handle_menu_input(XEvent *ev)
{
    gboolean ret = FALSE;

    if (ev->type == ButtonRelease || ev->type == ButtonPress) {
        ObMenuEntryFrame *e;

        if (menu_hide_delay_reached() &&
            (ev->xbutton.button < 4 || ev->xbutton.button > 5))
        {
            if ((e = menu_entry_frame_under(ev->xbutton.x_root,
                                            ev->xbutton.y_root)))
            {
                if (ev->type == ButtonPress && e->frame->child)
                    menu_frame_select(e->frame->child, NULL, TRUE);
                menu_frame_select(e->frame, e, TRUE);
                if (ev->type == ButtonRelease)
                    menu_entry_frame_execute(e, ev->xbutton.state);
            }
            else if (ev->type == ButtonRelease)
                menu_frame_hide_all();
        }
        ret = TRUE;
    }
    else if (ev->type == MotionNotify) {
        ObMenuFrame *f;
        ObMenuEntryFrame *e;

        if ((e = menu_entry_frame_under(ev->xmotion.x_root,
                                        ev->xmotion.y_root)))
            if (!(f = find_active_menu()) ||
                f == e->frame ||
                f->parent == e->frame ||
                f->child == e->frame)
                menu_frame_select(e->frame, e, FALSE);
    }
    else if (ev->type == KeyPress || ev->type == KeyRelease) {
        guint keycode, state;
        gunichar unikey;
        ObMenuFrame *frame;

        keycode = ev->xkey.keycode;
        state = ev->xkey.state;
        unikey = obt_keyboard_keycode_to_unichar(keycode);

        frame = find_active_or_last_menu();
        if (frame == NULL)
            g_assert_not_reached(); /* there is no active menu */

        /* Allow control while going thru the menu */
        else if (ev->type == KeyPress && (state & ~ControlMask) == 0) {
            frame->got_press = TRUE;

            if (keycode == ob_keycode(OB_KEY_ESCAPE)) {
                menu_frame_hide_all();
                ret = TRUE;
            }

            else if (keycode == ob_keycode(OB_KEY_LEFT)) {
                /* Left goes to the parent menu */
                menu_frame_select(frame, NULL, TRUE);
                ret = TRUE;
            }

            else if (keycode == ob_keycode(OB_KEY_RIGHT)) {
                /* Right goes to the selected submenu */
                if (frame->child) menu_frame_select_next(frame->child);
                ret = TRUE;
            }

            else if (keycode == ob_keycode(OB_KEY_UP)) {
                menu_frame_select_previous(frame);
                ret = TRUE;
            }

            else if (keycode == ob_keycode(OB_KEY_DOWN)) {
                menu_frame_select_next(frame);
                ret = TRUE;
            }
        }

        /* Use KeyRelease events for running things so that the key release
           doesn't get sent to the focused application.

           Allow ControlMask only, and don't bother if the menu is empty */
        else if (ev->type == KeyRelease && (state & ~ControlMask) == 0 &&
                 frame->entries && frame->got_press)
        {
            if (keycode == ob_keycode(OB_KEY_RETURN)) {
                /* Enter runs the active item or goes into the submenu.
                   Control-Enter runs it without closing the menu. */
                if (frame->child)
                    menu_frame_select_next(frame->child);
                else if (frame->selected)
                    menu_entry_frame_execute(frame->selected, state);

                ret = TRUE;
            }

            /* keyboard accelerator shortcuts. (if it was a valid key) */
            else if (unikey != 0) {
                GList *start;
                GList *it;
                ObMenuEntryFrame *found = NULL;
                guint num_found = 0;

                /* start after the selected one */
                start = frame->entries;
                if (frame->selected) {
                    for (it = start; frame->selected != it->data;
                         it = g_list_next(it))
                        g_assert(it != NULL); /* nothing was selected? */
                    /* next with wraparound */
                    start = g_list_next(it);
                    if (start == NULL) start = frame->entries;
                }

                it = start;
                do {
                    ObMenuEntryFrame *e = it->data;
                    gunichar entrykey = 0;

                    if (e->entry->type == OB_MENU_ENTRY_TYPE_NORMAL)
                        entrykey = e->entry->data.normal.shortcut;
                    else if (e->entry->type == OB_MENU_ENTRY_TYPE_SUBMENU)
                        entrykey = e->entry->data.submenu.submenu->shortcut;

                    if (unikey == entrykey) {
                        if (found == NULL) found = e;
                        ++num_found;
                    }

                    /* next with wraparound */
                    it = g_list_next(it);
                    if (it == NULL) it = frame->entries;
                } while (it != start);

                if (found) {
                    if (found->entry->type == OB_MENU_ENTRY_TYPE_NORMAL &&
                        num_found == 1)
                    {
                        menu_frame_select(frame, found, TRUE);
                        usleep(50000); /* highlight the item for a short bit so
                                          the user can see what happened */
                        menu_entry_frame_execute(found, state);
                    } else {
                        menu_frame_select(frame, found, TRUE);
                        if (num_found == 1)
                            menu_frame_select_next(frame->child);
                    }

                    ret = TRUE;
                }
            }
        }
    }

    return ret;
}

static void event_handle_menu(ObMenuFrame *frame, XEvent *ev)
{
    ObMenuFrame *f;
    ObMenuEntryFrame *e;

    switch (ev->type) {
    case EnterNotify:
        if ((e = g_hash_table_lookup(menu_frame_map, &ev->xcrossing.window))) {
            if (e->ignore_enters)
                --e->ignore_enters;
            else if (!(f = find_active_menu()) ||
                     f == e->frame ||
                     f->parent == e->frame ||
                     f->child == e->frame)
                menu_frame_select(e->frame, e, FALSE);
        }
        break;
    case LeaveNotify:
        /*ignore leaves when we're already in the window */
        if (ev->xcrossing.detail == NotifyInferior)
            break;

        if ((e = g_hash_table_lookup(menu_frame_map, &ev->xcrossing.window)) &&
            (f = find_active_menu()) && f->selected == e &&
            e->entry->type != OB_MENU_ENTRY_TYPE_SUBMENU)
        {
            menu_frame_select(e->frame, NULL, FALSE);
        }
        break;
    }
}

static void event_handle_user_input(ObClient *client, XEvent *e)
{
    g_assert(e->type == ButtonPress || e->type == ButtonRelease ||
             e->type == MotionNotify || e->type == KeyPress ||
             e->type == KeyRelease);

    if (menu_frame_visible) {
        if (event_handle_menu_input(e))
            /* don't use the event if the menu used it, but if the menu
               didn't use it and it's a keypress that is bound, it will
               close the menu and be used */
            return;
    }

    /* if the keyboard interactive action uses the event then dont
       use it for bindings. likewise is moveresize uses the event. */
    if (!actions_interactive_input_event(e) && !moveresize_event(e)) {
        if (moveresize_in_progress)
            /* make further actions work on the client being
               moved/resized */
            client = moveresize_client;

        if (e->type == ButtonPress ||
            e->type == ButtonRelease ||
            e->type == MotionNotify)
        {
            /* the frame may not be "visible" but they can still click on it
               in the case where it is animating before disappearing */
            if (!client || !frame_iconify_animating(client->frame))
                mouse_event(client, e);
        } else
            keyboard_event((focus_cycle_target ? focus_cycle_target :
                            (client ? client : focus_client)), e);
    }
}

static void focus_delay_dest(gpointer data)
{
    g_free(data);
}

static gboolean focus_delay_cmp(gconstpointer d1, gconstpointer d2)
{
    const ObFocusDelayData *f1 = d1;
    return f1->client == d2;
}

static gboolean focus_delay_func(gpointer data)
{
    ObFocusDelayData *d = data;
    Time old = event_curtime;

    /* don't move focus and kill the menu or the move/resize */
    if (menu_frame_visible || moveresize_in_progress) return FALSE;

    event_curtime = d->time;
    event_curserial = d->serial;
    if (client_focus(d->client) && config_focus_raise)
        stacking_raise(CLIENT_AS_WINDOW(d->client));
    event_curtime = old;
    return FALSE; /* no repeat */
}

static void focus_delay_client_dest(ObClient *client, gpointer data)
{
    obt_main_loop_timeout_remove_data(ob_main_loop, focus_delay_func,
                                      client, FALSE);
}

void event_halt_focus_delay(void)
{
    /* ignore all enter events up till the event which caused this to occur */
    if (event_curserial) event_ignore_enter_range(1, event_curserial);
    obt_main_loop_timeout_remove(ob_main_loop, focus_delay_func);
}

gulong event_start_ignore_all_enters(void)
{
    return NextRequest(obt_display);
}

static void event_ignore_enter_range(gulong start, gulong end)
{
    ObSerialRange *r;

    g_assert(start != 0);
    g_assert(end != 0);

    r = g_new(ObSerialRange, 1);
    r->start = start;
    r->end = end;
    ignore_serials = g_slist_prepend(ignore_serials, r);

    ob_debug_type(OB_DEBUG_FOCUS, "ignoring enters from %lu until %lu",
                  r->start, r->end);

    /* increment the serial so we don't ignore events we weren't meant to */
    OBT_PROP_ERASE(screen_support_win, MOTIF_WM_HINTS);
}

void event_end_ignore_all_enters(gulong start)
{
    /* Use (NextRequest-1) so that we ignore up to the current serial only.
       Inside event_ignore_enter_range, we increment the serial by one, but if
       we ignore that serial too, then any enter events generated by mouse
       movement will be ignored until we create some further network traffic.
       Instead ignore up to NextRequest-1, then when we increment the serial,
       we will be *past* the range of ignored serials */
    event_ignore_enter_range(start, NextRequest(obt_display)-1);
}

static gboolean is_enter_focus_event_ignored(gulong serial)
{
    GSList *it, *next;

    for (it = ignore_serials; it; it = next) {
        ObSerialRange *r = it->data;

        next = g_slist_next(it);

        if ((glong)(serial - r->end) > 0) {
            /* past the end */
            ignore_serials = g_slist_delete_link(ignore_serials, it);
            g_free(r);
        }
        else if ((glong)(serial - r->start) >= 0)
            return TRUE;
    }
    return FALSE;
}

void event_cancel_all_key_grabs(void)
{
    if (actions_interactive_act_running()) {
        actions_interactive_cancel_act();
        ob_debug("KILLED interactive action");
    }
    else if (menu_frame_visible) {
        menu_frame_hide_all();
        ob_debug("KILLED open menus");
    }
    else if (moveresize_in_progress) {
        moveresize_end(TRUE);
        ob_debug("KILLED interactive moveresize");
    }
    else if (grab_on_keyboard()) {
        ungrab_keyboard();
        ob_debug("KILLED active grab on keyboard");
    }
    else
        ungrab_passive_key();

    XSync(obt_display, FALSE);
}

gboolean event_time_after(Time t1, Time t2)
{
    g_assert(t1 != CurrentTime);
    g_assert(t2 != CurrentTime);

    /*
      Timestamp values wrap around (after about 49.7 days). The server, given
      its current time is represented by timestamp T, always interprets
      timestamps from clients by treating half of the timestamp space as being
      later in time than T.
      - http://tronche.com/gui/x/xlib/input/pointer-grabbing.html
    */

    /* TIME_HALF is half of the number space of a Time type variable */
#define TIME_HALF (Time)(1 << (sizeof(Time)*8-1))

    if (t2 >= TIME_HALF)
        /* t2 is in the second half so t1 might wrap around and be smaller than
           t2 */
        return t1 >= t2 || t1 < (t2 + TIME_HALF);
    else
        /* t2 is in the first half so t1 has to come after it */
        return t1 >= t2 && t1 < (t2 + TIME_HALF);
}

Time event_get_server_time(void)
{
    /* Generate a timestamp */
    XEvent event;

    XChangeProperty(obt_display, screen_support_win,
                    OBT_PROP_ATOM(WM_CLASS), OBT_PROP_ATOM(STRING),
                    8, PropModeAppend, NULL, 0);
    XWindowEvent(obt_display, screen_support_win, PropertyChangeMask, &event);
    return event.xproperty.time;
}
