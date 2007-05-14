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
#include "client.h"
#include "xerror.h"
#include "prop.h"
#include "config.h"
#include "screen.h"
#include "frame.h"
#include "menu.h"
#include "menuframe.h"
#include "keyboard.h"
#include "modkeys.h"
#include "propwin.h"
#include "mouse.h"
#include "mainloop.h"
#include "framerender.h"
#include "focus.h"
#include "moveresize.h"
#include "group.h"
#include "stacking.h"
#include "extensions.h"
#include "translate.h"

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
} ObFocusDelayData;

static void event_process(const XEvent *e, gpointer data);
static void event_handle_root(XEvent *e);
static gboolean event_handle_menu_keyboard(XEvent *e);
static gboolean event_handle_menu(XEvent *e);
static void event_handle_dock(ObDock *s, XEvent *e);
static void event_handle_dockapp(ObDockApp *app, XEvent *e);
static void event_handle_client(ObClient *c, XEvent *e);
static void event_handle_user_time_window_clients(GSList *l, XEvent *e);
static void event_handle_user_input(ObClient *client, XEvent *e);

static void focus_delay_dest(gpointer data);
static gboolean focus_delay_cmp(gconstpointer d1, gconstpointer d2);
static gboolean focus_delay_func(gpointer data);
static void focus_delay_client_dest(ObClient *client, gpointer data);

static gboolean menu_hide_delay_func(gpointer data);

/* The time for the current event being processed */
Time event_curtime = CurrentTime;

static guint ignore_enter_focus = 0;
static gboolean menu_can_hide;
static gboolean focus_left_screen = FALSE;

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
        ob_main_loop_fd_add(ob_main_loop, fd, ice_handler, conn, NULL);
    } else {
        ob_main_loop_fd_remove(ob_main_loop, fd);
        fd = -1;
    }
}
#endif

void event_startup(gboolean reconfig)
{
    if (reconfig) return;

    ob_main_loop_x_add(ob_main_loop, event_process, NULL, NULL);

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
        window = RootWindow(ob_display, ob_screen);
        break;
    case MapRequest:
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
        if (extensions_xkb && e->type == extensions_xkb_event_basep) {
            switch (((XkbAnyEvent*)e)->xkb_type) {
            case XkbBellNotify:
                window = ((XkbBellNotifyEvent*)e)->window;
            default:
                window = None;
            }
        } else
#endif
#ifdef SYNC
        if (extensions_sync &&
            e->type == extensions_sync_event_basep + XSyncAlarmNotify)
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
        if (extensions_sync &&
            e->type == extensions_sync_event_basep + XSyncAlarmNotify)
        {
            t = ((XSyncAlarmNotifyEvent*)e)->time;
        }
#endif
        /* if more event types are anticipated, get their timestamp
           explicitly */
        break;
    }

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
        e->xbutton.state = modkeys_only_modifier_masks(e->xbutton.state);
        break;
    case KeyPress:
        e->xkey.state = modkeys_only_modifier_masks(e->xkey.state);
        break;
    case KeyRelease:
        e->xkey.state = modkeys_only_modifier_masks(e->xkey.state);
#ifdef XKB
        if (XkbGetState(ob_display, XkbUseCoreKbd, &xkb_state) == Success) {
            e->xkey.state = xkb_state.compat_state;
            break;
        }
#endif
        /* remove from the state the mask of the modifier key being released,
           if it is a modifier key being released that is */
        e->xkey.state &= ~modkeys_keycode_to_mask(e->xkey.keycode);
        break;
    case MotionNotify:
        e->xmotion.state = modkeys_only_modifier_masks(e->xmotion.state);
        /* compress events */
        {
            XEvent ce;
            while (XCheckTypedWindowEvent(ob_display, e->xmotion.window,
                                          e->type, &ce)) {
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

        if (win == RootWindow(ob_display, ob_screen)) {
            /* If looking for a focus in on a client, then always return
               FALSE for focus in's to the root window */
            if (in_client_only)
                return FALSE;
            /* This means focus reverted off of a client */
            else if (detail == NotifyPointerRoot ||
                     detail == NotifyDetailNone ||
                     detail == NotifyInferior)
                return TRUE;
            else
                return FALSE;
        }

        /* This means focus moved to the frame window */
        if (detail == NotifyInferior && !in_client_only)
            return TRUE;

        /* It was on a client, was it a valid one?
           It's possible to get a FocusIn event for a client that was managed
           but has disappeared.
        */
        if (in_client_only) {
            ObWindow *w = g_hash_table_lookup(window_map, &e->xfocus.window);
            if (!w || !WINDOW_IS_CLIENT(w))
                return FALSE;
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

        /* Focus left the root window revertedto state */
        if (win == RootWindow(ob_display, ob_screen))
            return FALSE;

        /* These are the ones we want.. */

        /* This means focus moved from a client to the root window */
        if (detail == NotifyVirtual)
            return TRUE;
        /* This means focus moved from one client to another */
        if (detail == NotifyNonlinearVirtual)
            return TRUE;
        /* This means focus had moved to our frame window and now moved off */
        if (detail == NotifyNonlinear)
            return TRUE;

        /* Otherwise.. */
        return FALSE;
    }
}

static Bool event_look_for_focusin(Display *d, XEvent *e, XPointer arg)
{
    return e->type == FocusIn && wanted_focusevent(e, FALSE);
}

Bool event_look_for_focusin_client(Display *d, XEvent *e, XPointer arg)
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

    g_assert(modestr);
    g_assert(detailstr);
    ob_debug_type(OB_DEBUG_FOCUS, "Focus%s 0x%x mode=%s detail=%s\n",
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
    Window window;
    ObClient *client = NULL;
    ObDock *dock = NULL;
    ObDockApp *dockapp = NULL;
    ObWindow *obwin = NULL;
    GSList *timewinclients = NULL;
    XEvent ee, *e;
    ObEventData *ed = data;

    /* make a copy we can mangle */
    ee = *ec;
    e = &ee;

    window = event_get_window(e);
    if (e->type != PropertyNotify ||
        !(timewinclients = propwin_get_clients(window,
                                               OB_PROPWIN_USER_TIME)))
        if ((obwin = g_hash_table_lookup(window_map, &window))) {
            switch (obwin->type) {
            case Window_Dock:
                dock = WINDOW_AS_DOCK(obwin);
                break;
            case Window_DockApp:
                dockapp = WINDOW_AS_DOCKAPP(obwin);
                break;
            case Window_Client:
                client = WINDOW_AS_CLIENT(obwin);
                break;
            case Window_Menu:
            case Window_Internal:
                /* not to be used for events */
                g_assert_not_reached();
                break;
            }
        }

    event_set_curtime(e);
    event_hack_mods(e);
    if (event_ignore(e, client)) {
        if (ed)
            ed->ignored = TRUE;
        return;
    } else if (ed)
            ed->ignored = FALSE;

    /* deal with it in the kernel */

    if (menu_frame_visible &&
        (e->type == EnterNotify || e->type == LeaveNotify))
    {
        /* crossing events for menu */
        event_handle_menu(e);
    } else if (e->type == FocusIn) {
        if (e->xfocus.detail == NotifyPointerRoot ||
            e->xfocus.detail == NotifyDetailNone ||
            e->xfocus.detail == NotifyInferior)
        {
            XEvent ce;

            ob_debug_type(OB_DEBUG_FOCUS,
                          "Focus went to pointer root/none or to our frame "
                          "window\n");

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
            if (XCheckIfEvent(ob_display, &ce, event_look_for_focusin_client,
                              NULL))
            {
                XPutBackEvent(ob_display, &ce);
                ob_debug_type(OB_DEBUG_FOCUS,
                              "  but another FocusIn is coming\n");
            } else {
                /* Focus has been reverted to the root window, nothing, or to
                   our frame window.

                   FocusOut events come after UnmapNotify, so we don't need to
                   worry about focusing an invalid window
                */

                /* In this case we know focus is in our screen */
                if (e->xfocus.detail == NotifyInferior)
                    focus_left_screen = FALSE;

                if (!focus_left_screen)
                    focus_fallback(TRUE);
            }
        }
        else if (!client)
        {
            XEvent ce;

            ob_debug_type(OB_DEBUG_FOCUS,
                          "Focus went to a window that is already gone\n");

            /* If you send focus to a window and then it disappears, you can
               get the FocusIn FocusOut for it, after it is unmanaged.
            */
            if (XCheckIfEvent(ob_display, &ce, event_look_for_focusin_client,
                              NULL))
            {
                XPutBackEvent(ob_display, &ce);
                ob_debug_type(OB_DEBUG_FOCUS,
                              "  but another FocusIn is coming\n");
            } else {
                focus_fallback(TRUE);
            }
        }
        else if (client != focus_client) {
            focus_left_screen = FALSE;
            frame_adjust_focus(client->frame, TRUE);
            focus_set_client(client);
            client_calc_layer(client);
            client_bring_helper_windows(client);
        }
    } else if (e->type == FocusOut) {
        gboolean nomove = FALSE;
        XEvent ce;

        /* Look for the followup FocusIn */
        if (!XCheckIfEvent(ob_display, &ce, event_look_for_focusin, NULL)) {
            /* There is no FocusIn, this means focus went to a window that
               is not being managed, or a window on another screen. */
            Window win, root;
            gint i;
            guint u;
            xerror_set_ignore(TRUE);
            if (XGetInputFocus(ob_display, &win, &i) != 0 &&
                XGetGeometry(ob_display, win, &root, &i,&i,&u,&u,&u,&u) != 0 &&
                root != RootWindow(ob_display, ob_screen))
            {
                ob_debug_type(OB_DEBUG_FOCUS,
                              "Focus went to another screen !\n");
                focus_left_screen = TRUE;
            }
            else
                ob_debug_type(OB_DEBUG_FOCUS,
                              "Focus went to a black hole !\n");
            xerror_set_ignore(FALSE);
            /* nothing is focused */
            focus_set_client(NULL);
        } else if (ce.xany.window == e->xany.window) {
            ob_debug_type(OB_DEBUG_FOCUS, "Focus didn't go anywhere\n");
            /* If focus didn't actually move anywhere, there is nothing to do*/
            nomove = TRUE;
        } else {
            /* Focus did move, so process the FocusIn event */
            ObEventData ed = { .ignored = FALSE };
            event_process(&ce, &ed);
            if (ed.ignored) {
                /* The FocusIn was ignored, this means it was on a window
                   that isn't a client. */
                ob_debug_type(OB_DEBUG_FOCUS,
                              "Focus went to an unmanaged window 0x%x !\n",
                              ce.xfocus.window);
                focus_fallback(TRUE);
            }
        }

        if (client && !nomove) {
            frame_adjust_focus(client->frame, FALSE);
            if (client == focus_client)
                focus_set_client(NULL);
            /* focus_set_client has already been called for sure */
            client_calc_layer(client);
        }
    } else if (timewinclients)
        event_handle_user_time_window_clients(timewinclients, e);
    else if (client)
        event_handle_client(client, e);
    else if (dockapp)
        event_handle_dockapp(dockapp, e);
    else if (dock)
        event_handle_dock(dock, e);
    else if (window == RootWindow(ob_display, ob_screen))
        event_handle_root(e);
    else if (e->type == MapRequest)
        client_manage(window);
    else if (e->type == ClientMessage) {
        /* This is for _NET_WM_REQUEST_FRAME_EXTENTS messages. They come for
           windows that are not managed yet. */
        if (e->xclient.message_type == prop_atoms.net_request_frame_extents) {
            /* Pretend to manage the client, getting information used to
               determine its decorations */
            ObClient *c = client_fake_manage(e->xclient.window);
            gulong vals[4];

            /* set the frame extents on the window */
            vals[0] = c->frame->size.left;
            vals[1] = c->frame->size.right;
            vals[2] = c->frame->size.top;
            vals[3] = c->frame->size.bottom;
            PROP_SETA32(e->xclient.window, net_frame_extents,
                        cardinal, vals, 4);

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
        xerror_set_ignore(TRUE);
        XConfigureWindow(ob_display, window,
                         e->xconfigurerequest.value_mask, &xwc);
        xerror_set_ignore(FALSE);
    }
#ifdef SYNC
    else if (extensions_sync &&
        e->type == extensions_sync_event_basep + XSyncAlarmNotify)
    {
        XSyncAlarmNotifyEvent *se = (XSyncAlarmNotifyEvent*)e;
        if (se->alarm == moveresize_alarm && moveresize_in_progress)
            moveresize_event(e);
    }
#endif

    if (e->type == ButtonPress || e->type == ButtonRelease ||
        e->type == MotionNotify || e->type == KeyPress ||
        e->type == KeyRelease)
    {
        event_handle_user_input(client, e);
    }

    /* if something happens and it's not from an XEvent, then we don't know
       the time */
    event_curtime = CurrentTime;
}

static void event_handle_root(XEvent *e)
{
    Atom msgtype;
     
    switch(e->type) {
    case SelectionClear:
        ob_debug("Another WM has requested to replace us. Exiting.\n");
        ob_exit_replace();
        break;

    case ClientMessage:
        if (e->xclient.format != 32) break;

        msgtype = e->xclient.message_type;
        if (msgtype == prop_atoms.net_current_desktop) {
            guint d = e->xclient.data.l[0];
            if (d < screen_num_desktops) {
                event_curtime = e->xclient.data.l[1];
                if (event_curtime == 0)
                    ob_debug_type(OB_DEBUG_APP_BUGS,
                                  "_NET_CURRENT_DESKTOP message is missing "
                                  "a timestamp\n");
                screen_set_desktop(d, TRUE);
            }
        } else if (msgtype == prop_atoms.net_number_of_desktops) {
            guint d = e->xclient.data.l[0];
            if (d > 0)
                screen_set_num_desktops(d);
        } else if (msgtype == prop_atoms.net_showing_desktop) {
            screen_show_desktop(e->xclient.data.l[0] != 0, NULL);
        } else if (msgtype == prop_atoms.ob_control) {
            if (e->xclient.data.l[0] == 1)
                ob_reconfigure();
            else if (e->xclient.data.l[0] == 2)
                ob_restart();
        }
        break;
    case PropertyNotify:
        if (e->xproperty.atom == prop_atoms.net_desktop_names)
            screen_update_desktop_names();
        else if (e->xproperty.atom == prop_atoms.net_desktop_layout)
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

    if (client_enter_focusable(client) && client_can_focus(client)) {
        if (config_focus_delay) {
            ObFocusDelayData *data;

            ob_main_loop_timeout_remove(ob_main_loop, focus_delay_func);

            data = g_new(ObFocusDelayData, 1);
            data->client = client;
            data->time = event_curtime;

            ob_main_loop_timeout_add(ob_main_loop,
                                     config_focus_delay,
                                     focus_delay_func,
                                     data, focus_delay_cmp, focus_delay_dest);
        } else {
            ObFocusDelayData data;
            data.client = client;
            data.time = event_curtime;
            focus_delay_func(&data);
        }
    }
}

static void event_handle_user_time_window_clients(GSList *l, XEvent *e)
{
    g_assert(e->type == PropertyNotify);
    if (e->xproperty.atom == prop_atoms.net_wm_user_time) {
        for (; l; l = g_slist_next(l))
            client_update_user_time(l->data);
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
            !keyboard_interactively_grabbed() &&
            !menu_frame_visible)
        {
            /* use where the press occured */
            con = frame_context(client, e->xbutton.window, px, py);
            con = mouse_button_frame_context(con, e->xbutton.button);

            if (e->type == ButtonRelease && e->xbutton.button == pb)
                pb = 0, px = py = -1;

            switch (con) {
            case OB_FRAME_CONTEXT_MAXIMIZE:
                client->frame->max_press = (e->type == ButtonPress);
                framerender_frame(client->frame);
                break;
            case OB_FRAME_CONTEXT_CLOSE:
                client->frame->close_press = (e->type == ButtonPress);
                framerender_frame(client->frame);
                break;
            case OB_FRAME_CONTEXT_ICONIFY:
                client->frame->iconify_press = (e->type == ButtonPress);
                framerender_frame(client->frame);
                break;
            case OB_FRAME_CONTEXT_ALLDESKTOPS:
                client->frame->desk_press = (e->type == ButtonPress);
                framerender_frame(client->frame);
                break; 
            case OB_FRAME_CONTEXT_SHADE:
                client->frame->shade_press = (e->type == ButtonPress);
                framerender_frame(client->frame);
                break;
            default:
                /* nothing changes with clicks for any other contexts */
                break;
            }
        }
        break;
    case MotionNotify:
        con = frame_context(client, e->xmotion.window,
                            e->xmotion.x, e->xmotion.y);
        switch (con) {
        case OB_FRAME_CONTEXT_TITLEBAR:
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
                event_ignore_queued_enters();

            ob_debug_type(OB_DEBUG_FOCUS,
                          "%sNotify mode %d detail %d on %lx\n",
                          (e->type == EnterNotify ? "Enter" : "Leave"),
                          e->xcrossing.mode,
                          e->xcrossing.detail, (client?client->window:0));
            if (keyboard_interactively_grabbed())
                break;
            if (config_focus_follow && config_focus_delay &&
                /* leave inferior events can happen when the mouse goes onto
                   the window's border and then into the window before the
                   delay is up */
                e->xcrossing.detail != NotifyInferior)
            {
                ob_main_loop_timeout_remove_data(ob_main_loop,
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
        gboolean nofocus = FALSE;

        if (ignore_enter_focus) {
            ignore_enter_focus--;
            nofocus = TRUE;
        }

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
            if (keyboard_interactively_grabbed())
                break;
            if (e->xcrossing.mode == NotifyGrab ||
                e->xcrossing.mode == NotifyUngrab ||
                /*ignore enters when we're already in the window */
                e->xcrossing.detail == NotifyInferior)
            {
                ob_debug_type(OB_DEBUG_FOCUS,
                              "%sNotify mode %d detail %d on %lx IGNORED\n",
                              (e->type == EnterNotify ? "Enter" : "Leave"),
                              e->xcrossing.mode,
                              e->xcrossing.detail, client?client->window:0);
            } else {
                ob_debug_type(OB_DEBUG_FOCUS,
                              "%sNotify mode %d detail %d on %lx, "
                              "focusing window: %d\n",
                              (e->type == EnterNotify ? "Enter" : "Leave"),
                              e->xcrossing.mode,
                              e->xcrossing.detail, (client?client->window:0),
                              !nofocus);
                if (!nofocus && config_focus_follow)
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

        /* if nothing is changed, then a configurenotify is needed */
        gboolean config = TRUE;

        x = client->area.x;
        y = client->area.y;
        w = client->area.width;
        h = client->area.height;

        ob_debug("ConfigureRequest desktop %d wmstate %d visibile %d\n",
                 screen_desktop, client->wmstate, client->frame->visible);

        if (e->xconfigurerequest.value_mask & CWBorderWidth)
            if (client->border_width != e->xconfigurerequest.border_width) {
                client->border_width = e->xconfigurerequest.border_width;
                /* if only the border width is changing, then it's not needed*/
                config = FALSE;
            }


        if (e->xconfigurerequest.value_mask & CWStackMode) {
            ObClient *sibling = NULL;

            /* get the sibling */
            if (e->xconfigurerequest.value_mask & CWSibling) {
                ObWindow *win;
                win = g_hash_table_lookup(window_map,
                                          &e->xconfigurerequest.above);
                if (WINDOW_IS_CLIENT(win) && WINDOW_AS_CLIENT(win) != client)
                    sibling = WINDOW_AS_CLIENT(win);
            }

            /* activate it rather than just focus it */
            stacking_restack_request(client, sibling,
                                     e->xconfigurerequest.detail, TRUE);

            /* if a stacking change is requested then it is needed */
            config = TRUE;
        }

        /* don't allow clients to move shaded windows (fvwm does this) */
        if (client->shaded && (e->xconfigurerequest.value_mask & CWX ||
                               e->xconfigurerequest.value_mask & CWY))
        {
            e->xconfigurerequest.value_mask &= ~CWX;
            e->xconfigurerequest.value_mask &= ~CWY;

            /* if the client tried to move and we aren't letting it then a
               synthetic event is needed */
            config = TRUE;
        }

        if (e->xconfigurerequest.value_mask & CWX ||
            e->xconfigurerequest.value_mask & CWY ||
            e->xconfigurerequest.value_mask & CWWidth ||
            e->xconfigurerequest.value_mask & CWHeight)
        {
            if (e->xconfigurerequest.value_mask & CWX)
                x = e->xconfigurerequest.x;
            if (e->xconfigurerequest.value_mask & CWY)
                y = e->xconfigurerequest.y;
            if (e->xconfigurerequest.value_mask & CWWidth)
                w = e->xconfigurerequest.width;
            if (e->xconfigurerequest.value_mask & CWHeight)
                h = e->xconfigurerequest.height;

            /* if a new position or size is requested, then a configure is
               needed */
            config = TRUE;
        }

        ob_debug("ConfigureRequest x(%d) %d y(%d) %d w(%d) %d h(%d) %d\n",
                 e->xconfigurerequest.value_mask & CWX, x,
                 e->xconfigurerequest.value_mask & CWY, y,
                 e->xconfigurerequest.value_mask & CWWidth, w,
                 e->xconfigurerequest.value_mask & CWHeight, h);

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
                  (gint)client->border_width))
        {
            ob_debug_type(OB_DEBUG_APP_BUGS,
                          "Application %s is trying to move via "
                          "ConfigureRequest to it's root window position "
                          "but it is not using StaticGravity\n",
                          client->title);
            /* don't move it */
            x = client->area.x;
            y = client->area.y;
        }

        if (config) {
            client_find_onscreen(client, &x, &y, w, h, FALSE);
            client_configure_full(client, x, y, w, h, FALSE, TRUE);
        }
        break;
    }
    case UnmapNotify:
        if (client->ignore_unmaps) {
            client->ignore_unmaps--;
            break;
        }
        ob_debug("UnmapNotify for window 0x%x eventwin 0x%x sendevent %d "
                 "ignores left %d\n",
                 client->window, e->xunmap.event, e->xunmap.from_configure,
                 client->ignore_unmaps);
        client_unmanage(client);
        break;
    case DestroyNotify:
        ob_debug("DestroyNotify for window 0x%x\n", client->window);
        client_unmanage(client);
        break;
    case ReparentNotify:
        /* this is when the client is first taken captive in the frame */
        if (e->xreparent.parent == client->frame->plate) break;

        /*
          This event is quite rare and is usually handled in unmapHandler.
          However, if the window is unmapped when the reparent event occurs,
          the window manager never sees it because an unmap event is not sent
          to an already unmapped window.
        */

        /* we don't want the reparent event, put it back on the stack for the
           X server to deal with after we unmanage the window */
        XPutBackEvent(ob_display, e);
     
        ob_debug("ReparentNotify for window 0x%x\n", client->window);
        client_unmanage(client);
        break;
    case MapRequest:
        ob_debug("MapRequest for 0x%lx\n", client->window);
        if (!client->iconic) break; /* this normally doesn't happen, but if it
                                       does, we don't want it!
                                       it can happen now when the window is on
                                       another desktop, but we still don't
                                       want it! */
        client_activate(client, FALSE, TRUE);
        break;
    case ClientMessage:
        /* validate cuz we query stuff off the client here */
        if (!client_validate(client)) break;

        if (e->xclient.format != 32) return;

        msgtype = e->xclient.message_type;
        if (msgtype == prop_atoms.wm_change_state) {
            /* compress changes into a single change */
            while (XCheckTypedWindowEvent(ob_display, client->window,
                                          e->type, &ce)) {
                /* XXX: it would be nice to compress ALL messages of a
                   type, not just messages in a row without other
                   message types between. */
                if (ce.xclient.message_type != msgtype) {
                    XPutBackEvent(ob_display, &ce);
                    break;
                }
                e->xclient = ce.xclient;
            }
            client_set_wm_state(client, e->xclient.data.l[0]);
        } else if (msgtype == prop_atoms.net_wm_desktop) {
            /* compress changes into a single change */
            while (XCheckTypedWindowEvent(ob_display, client->window,
                                          e->type, &ce)) {
                /* XXX: it would be nice to compress ALL messages of a
                   type, not just messages in a row without other
                   message types between. */
                if (ce.xclient.message_type != msgtype) {
                    XPutBackEvent(ob_display, &ce);
                    break;
                }
                e->xclient = ce.xclient;
            }
            if ((unsigned)e->xclient.data.l[0] < screen_num_desktops ||
                (unsigned)e->xclient.data.l[0] == DESKTOP_ALL)
                client_set_desktop(client, (unsigned)e->xclient.data.l[0],
                                   FALSE);
        } else if (msgtype == prop_atoms.net_wm_state) {
            /* can't compress these */
            ob_debug("net_wm_state %s %ld %ld for 0x%lx\n",
                     (e->xclient.data.l[0] == 0 ? "Remove" :
                      e->xclient.data.l[0] == 1 ? "Add" :
                      e->xclient.data.l[0] == 2 ? "Toggle" : "INVALID"),
                     e->xclient.data.l[1], e->xclient.data.l[2],
                     client->window);
            client_set_state(client, e->xclient.data.l[0],
                             e->xclient.data.l[1], e->xclient.data.l[2]);
        } else if (msgtype == prop_atoms.net_close_window) {
            ob_debug("net_close_window for 0x%lx\n", client->window);
            client_close(client);
        } else if (msgtype == prop_atoms.net_active_window) {
            ob_debug("net_active_window for 0x%lx source=%s\n",
                     client->window,
                     (e->xclient.data.l[0] == 0 ? "unknown" :
                      (e->xclient.data.l[0] == 1 ? "application" :
                       (e->xclient.data.l[0] == 2 ? "user" : "INVALID"))));
            /* XXX make use of data.l[2] !? */
            event_curtime = e->xclient.data.l[1];
            if (event_curtime == 0)
                ob_debug_type(OB_DEBUG_APP_BUGS,
                              "_NET_ACTIVE_WINDOW message for window %s is "
                              "missing a timestamp\n", client->title);
            client_activate(client, FALSE,
                            (e->xclient.data.l[0] == 0 ||
                             e->xclient.data.l[0] == 2));
        } else if (msgtype == prop_atoms.net_wm_moveresize) {
            ob_debug("net_wm_moveresize for 0x%lx direction %d\n",
                     client->window, e->xclient.data.l[2]);
            if ((Atom)e->xclient.data.l[2] ==
                prop_atoms.net_wm_moveresize_size_topleft ||
                (Atom)e->xclient.data.l[2] ==
                prop_atoms.net_wm_moveresize_size_top ||
                (Atom)e->xclient.data.l[2] ==
                prop_atoms.net_wm_moveresize_size_topright ||
                (Atom)e->xclient.data.l[2] ==
                prop_atoms.net_wm_moveresize_size_right ||
                (Atom)e->xclient.data.l[2] ==
                prop_atoms.net_wm_moveresize_size_right ||
                (Atom)e->xclient.data.l[2] ==
                prop_atoms.net_wm_moveresize_size_bottomright ||
                (Atom)e->xclient.data.l[2] ==
                prop_atoms.net_wm_moveresize_size_bottom ||
                (Atom)e->xclient.data.l[2] ==
                prop_atoms.net_wm_moveresize_size_bottomleft ||
                (Atom)e->xclient.data.l[2] ==
                prop_atoms.net_wm_moveresize_size_left ||
                (Atom)e->xclient.data.l[2] ==
                prop_atoms.net_wm_moveresize_move ||
                (Atom)e->xclient.data.l[2] ==
                prop_atoms.net_wm_moveresize_size_keyboard ||
                (Atom)e->xclient.data.l[2] ==
                prop_atoms.net_wm_moveresize_move_keyboard) {

                moveresize_start(client, e->xclient.data.l[0],
                                 e->xclient.data.l[1], e->xclient.data.l[3],
                                 e->xclient.data.l[2]);
            }
            else if ((Atom)e->xclient.data.l[2] ==
                     prop_atoms.net_wm_moveresize_cancel)
                moveresize_end(TRUE);
        } else if (msgtype == prop_atoms.net_moveresize_window) {
            gint grav, x, y, w, h;

            if (e->xclient.data.l[0] & 0xff)
                grav = e->xclient.data.l[0] & 0xff;
            else 
                grav = client->gravity;

            if (e->xclient.data.l[0] & 1 << 8)
                x = e->xclient.data.l[1];
            else
                x = client->area.x;
            if (e->xclient.data.l[0] & 1 << 9)
                y = e->xclient.data.l[2];
            else
                y = client->area.y;
            if (e->xclient.data.l[0] & 1 << 10)
                w = e->xclient.data.l[3];
            else
                w = client->area.width;
            if (e->xclient.data.l[0] & 1 << 11)
                h = e->xclient.data.l[4];
            else
                h = client->area.height;

            ob_debug("MOVERESIZE x %d %d y %d %d\n",
                     e->xclient.data.l[0] & 1 << 8, x,
                     e->xclient.data.l[0] & 1 << 9, y);
            client_convert_gravity(client, grav, &x, &y, w, h);
            client_find_onscreen(client, &x, &y, w, h, FALSE);
            client_configure(client, x, y, w, h, FALSE, TRUE);
        } else if (msgtype == prop_atoms.net_restack_window) {
            if (e->xclient.data.l[0] != 2) {
                ob_debug_type(OB_DEBUG_APP_BUGS,
                              "_NET_RESTACK_WINDOW sent for window %s with "
                              "invalid source indication %ld\n",
                              client->title, e->xclient.data.l[0]);
            } else {
                ObClient *sibling = NULL;
                if (e->xclient.data.l[1]) {
                    ObWindow *win = g_hash_table_lookup(window_map,
                                                        &e->xclient.data.l[1]);
                    if (WINDOW_IS_CLIENT(win) &&
                        WINDOW_AS_CLIENT(win) != client)
                    {
                        sibling = WINDOW_AS_CLIENT(win);
                    }
                    if (sibling == NULL)
                        ob_debug_type(OB_DEBUG_APP_BUGS,
                                      "_NET_RESTACK_WINDOW sent for window %s "
                                      "with invalid sibling 0x%x\n",
                                 client->title, e->xclient.data.l[1]);
                }
                if (e->xclient.data.l[2] == Below ||
                    e->xclient.data.l[2] == BottomIf ||
                    e->xclient.data.l[2] == Above ||
                    e->xclient.data.l[2] == TopIf ||
                    e->xclient.data.l[2] == Opposite)
                {
                    /* just raise, don't activate */
                    stacking_restack_request(client, sibling,
                                             e->xclient.data.l[2], FALSE);
                    /* send a synthetic ConfigureNotify, cuz this is supposed
                       to be like a ConfigureRequest. */
                    client_configure_full(client, client->area.x,
                                          client->area.y,
                                          client->area.width,
                                          client->area.height,
                                          FALSE, TRUE);
                } else
                    ob_debug_type(OB_DEBUG_APP_BUGS,
                                  "_NET_RESTACK_WINDOW sent for window %s "
                                  "with invalid detail %d\n",
                                  client->title, e->xclient.data.l[2]);
            }
        }
        break;
    case PropertyNotify:
        /* validate cuz we query stuff off the client here */
        if (!client_validate(client)) break;
  
        /* compress changes to a single property into a single change */
        while (XCheckTypedWindowEvent(ob_display, client->window,
                                      e->type, &ce)) {
            Atom a, b;

            /* XXX: it would be nice to compress ALL changes to a property,
               not just changes in a row without other props between. */

            a = ce.xproperty.atom;
            b = e->xproperty.atom;

            if (a == b)
                continue;
            if ((a == prop_atoms.net_wm_name ||
                 a == prop_atoms.wm_name ||
                 a == prop_atoms.net_wm_icon_name ||
                 a == prop_atoms.wm_icon_name)
                &&
                (b == prop_atoms.net_wm_name ||
                 b == prop_atoms.wm_name ||
                 b == prop_atoms.net_wm_icon_name ||
                 b == prop_atoms.wm_icon_name)) {
                continue;
            }
            if (a == prop_atoms.net_wm_icon &&
                b == prop_atoms.net_wm_icon)
                continue;

            XPutBackEvent(ob_display, &ce);
            break;
        }

        msgtype = e->xproperty.atom;
        if (msgtype == XA_WM_NORMAL_HINTS) {
            client_update_normal_hints(client);
            /* normal hints can make a window non-resizable */
            client_setup_decor_and_functions(client);
        } else if (msgtype == XA_WM_HINTS) {
            client_update_wmhints(client);
        } else if (msgtype == XA_WM_TRANSIENT_FOR) {
            client_update_transient_for(client);
            client_get_type_and_transientness(client);
            /* type may have changed, so update the layer */
            client_calc_layer(client);
            client_setup_decor_and_functions(client);
        } else if (msgtype == prop_atoms.net_wm_name ||
                   msgtype == prop_atoms.wm_name ||
                   msgtype == prop_atoms.net_wm_icon_name ||
                   msgtype == prop_atoms.wm_icon_name) {
            client_update_title(client);
        } else if (msgtype == prop_atoms.wm_protocols) {
            client_update_protocols(client);
            client_setup_decor_and_functions(client);
        }
        else if (msgtype == prop_atoms.net_wm_strut) {
            client_update_strut(client);
        }
        else if (msgtype == prop_atoms.net_wm_icon) {
            client_update_icons(client);
        }
        else if (msgtype == prop_atoms.net_wm_icon_geometry) {
            client_update_icon_geometry(client);
        }
        else if (msgtype == prop_atoms.net_wm_user_time) {
            client_update_user_time(client);
        }
        else if (msgtype == prop_atoms.net_wm_user_time_window) {
            client_update_user_time_window(client);
        }
#ifdef SYNC
        else if (msgtype == prop_atoms.net_wm_sync_request_counter) {
            client_update_sync_request_counter(client);
        }
#endif
    case ColormapNotify:
        client_update_colormap(client, e->xcolormap.colormap);
        break;
    default:
        ;
#ifdef SHAPE
        if (extensions_shape && e->type == extensions_shape_event_basep) {
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
        dock_remove(app, TRUE);
        break;
    case DestroyNotify:
        dock_remove(app, FALSE);
        break;
    case ReparentNotify:
        dock_remove(app, FALSE);
        break;
    case ConfigureNotify:
        dock_app_configure(app, e->xconfigure.width, e->xconfigure.height);
        break;
    }
}

static ObMenuFrame* find_active_menu()
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

static ObMenuFrame* find_active_or_last_menu()
{
    ObMenuFrame *ret = NULL;

    ret = find_active_menu();
    if (!ret && menu_frame_visible)
        ret = menu_frame_visible->data;
    return ret;
}

static gboolean event_handle_menu_keyboard(XEvent *ev)
{
    guint keycode, state;
    gunichar unikey;
    ObMenuFrame *frame;
    gboolean ret = TRUE;

    keycode = ev->xkey.keycode;
    state = ev->xkey.state;
    unikey = translate_unichar(keycode);

    frame = find_active_or_last_menu();
    if (frame == NULL)
        ret = FALSE;

    else if (keycode == ob_keycode(OB_KEY_ESCAPE) && state == 0) {
        /* Escape closes the active menu */
        menu_frame_hide(frame);
    }

    else if (keycode == ob_keycode(OB_KEY_RETURN) && (state == 0 ||
                                                      state == ControlMask))
    {
        /* Enter runs the active item or goes into the submenu.
           Control-Enter runs it without closing the menu. */
        if (frame->child)
            menu_frame_select_next(frame->child);
        else
            menu_entry_frame_execute(frame->selected, state, ev->xkey.time);
    }

    else if (keycode == ob_keycode(OB_KEY_LEFT) && ev->xkey.state == 0) {
        /* Left goes to the parent menu */
        menu_frame_select(frame, NULL, TRUE);
    }

    else if (keycode == ob_keycode(OB_KEY_RIGHT) && ev->xkey.state == 0) {
        /* Right goes to the selected submenu */
        if (frame->child) menu_frame_select_next(frame->child);
    }

    else if (keycode == ob_keycode(OB_KEY_UP) && state == 0) {
        menu_frame_select_previous(frame);
    }

    else if (keycode == ob_keycode(OB_KEY_DOWN) && state == 0) {
        menu_frame_select_next(frame);
    }

    /* keyboard accelerator shortcuts. */
    else if (ev->xkey.state == 0 &&
             /* was it a valid key? */
             unikey != 0 &&
             /* don't bother if the menu is empty. */
             frame->entries)
    {
        GList *start;
        GList *it;
        ObMenuEntryFrame *found = NULL;
        guint num_found = 0;

        /* start after the selected one */
        start = frame->entries;
        if (frame->selected) {
            for (it = start; frame->selected != it->data; it = g_list_next(it))
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
                usleep(50000); /* highlight the item for a short bit so the
                                  user can see what happened */
                menu_entry_frame_execute(found, state, ev->xkey.time);
            } else {
                menu_frame_select(frame, found, TRUE);
                if (num_found == 1)
                    menu_frame_select_next(frame->child);
            }
        } else
            ret = FALSE;
    }
    else
        ret = FALSE;

    return ret;
}

static gboolean event_handle_menu(XEvent *ev)
{
    ObMenuFrame *f;
    ObMenuEntryFrame *e;
    gboolean ret = TRUE;

    switch (ev->type) {
    case ButtonRelease:
        if ((ev->xbutton.button < 4 || ev->xbutton.button > 5)
            && menu_can_hide)
        {
            if ((e = menu_entry_frame_under(ev->xbutton.x_root,
                                            ev->xbutton.y_root)))
                menu_entry_frame_execute(e, ev->xbutton.state,
                                         ev->xbutton.time);
            else
                menu_frame_hide_all();
        }
        break;
    case EnterNotify:
        if ((e = g_hash_table_lookup(menu_frame_map, &ev->xcrossing.window))) {
            if (e->ignore_enters)
                --e->ignore_enters;
            else
                menu_frame_select(e->frame, e, FALSE);
        }
        break;
    case LeaveNotify:
        if ((e = g_hash_table_lookup(menu_frame_map, &ev->xcrossing.window)) &&
            (f = find_active_menu()) && f->selected == e &&
            e->entry->type != OB_MENU_ENTRY_TYPE_SUBMENU)
        {
            menu_frame_select(e->frame, NULL, FALSE);
        }
        break;
    case MotionNotify:   
        if ((e = menu_entry_frame_under(ev->xmotion.x_root,   
                                        ev->xmotion.y_root)))
            menu_frame_select(e->frame, e, FALSE);
        break;
    case KeyPress:
        ret = event_handle_menu_keyboard(ev);
        break;
    }
    return ret;
}

static void event_handle_user_input(ObClient *client, XEvent *e)
{
    g_assert(e->type == ButtonPress || e->type == ButtonRelease ||
             e->type == MotionNotify || e->type == KeyPress ||
             e->type == KeyRelease);

    if (menu_frame_visible) {
        if (event_handle_menu(e))
            /* don't use the event if the menu used it, but if the menu
               didn't use it and it's a keypress that is bound, it will
               close the menu and be used */
            return;
    }

    /* if the keyboard interactive action uses the event then dont
       use it for bindings. likewise is moveresize uses the event. */
    if (!keyboard_process_interactive_grab(e, &client) &&
        !(moveresize_in_progress && moveresize_event(e)))
    {
        if (moveresize_in_progress)
            /* make further actions work on the client being
               moved/resized */
            client = moveresize_client;

        menu_can_hide = FALSE;
        ob_main_loop_timeout_add(ob_main_loop,
                                 config_menu_hide_delay * 1000,
                                 menu_hide_delay_func,
                                 NULL, g_direct_equal, NULL);

        if (e->type == ButtonPress ||
            e->type == ButtonRelease ||
            e->type == MotionNotify)
        {
            /* the frame may not be "visible" but they can still click on it
               in the case where it is animating before disappearing */
            if (!client || !frame_iconify_animating(client->frame))
                mouse_event(client, e);
        } else if (e->type == KeyPress) {
            keyboard_event((focus_cycle_target ? focus_cycle_target :
                            (client ? client : focus_client)), e);
        }
    }
}

static gboolean menu_hide_delay_func(gpointer data)
{
    menu_can_hide = TRUE;
    return FALSE; /* no repeat */
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

    event_curtime = d->time;
    if (focus_client != d->client) {
        if (client_focus(d->client) && config_focus_raise)
            stacking_raise(CLIENT_AS_WINDOW(d->client));
    }
    event_curtime = old;
    return FALSE; /* no repeat */
}

static void focus_delay_client_dest(ObClient *client, gpointer data)
{
    ob_main_loop_timeout_remove_data(ob_main_loop, focus_delay_func,
                                     client, FALSE);
}

void event_halt_focus_delay()
{
    ob_main_loop_timeout_remove(ob_main_loop, focus_delay_func);
}

void event_ignore_queued_enters()
{
    GSList *saved = NULL, *it;
    XEvent *e;
                
    XSync(ob_display, FALSE);

    /* count the events */
    while (TRUE) {
        e = g_new(XEvent, 1);
        if (XCheckTypedEvent(ob_display, EnterNotify, e)) {
            ObWindow *win;
            
            win = g_hash_table_lookup(window_map, &e->xany.window);
            if (win && WINDOW_IS_CLIENT(win))
                ++ignore_enter_focus;
            
            saved = g_slist_append(saved, e);
        } else {
            g_free(e);
            break;
        }
    }
    /* put the events back */
    for (it = saved; it; it = g_slist_next(it)) {
        XPutBackEvent(ob_display, it->data);
        g_free(it->data);
    }
    g_slist_free(saved);
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
