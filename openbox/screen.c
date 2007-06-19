/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   screen.c for the Openbox window manager
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

#include "debug.h"
#include "openbox.h"
#include "dock.h"
#include "xerror.h"
#include "prop.h"
#include "grab.h"
#include "startupnotify.h"
#include "moveresize.h"
#include "config.h"
#include "screen.h"
#include "client.h"
#include "session.h"
#include "frame.h"
#include "event.h"
#include "focus.h"
#include "popup.h"
#include "extensions.h"
#include "render/render.h"
#include "gettext.h"

#include <X11/Xlib.h>
#ifdef HAVE_UNISTD_H
#  include <sys/types.h>
#  include <unistd.h>
#endif
#include <assert.h>

/*! The event mask to grab on the root window */
#define ROOT_EVENTMASK (StructureNotifyMask | PropertyChangeMask | \
                        EnterWindowMask | LeaveWindowMask | \
                        SubstructureRedirectMask | FocusChangeMask | \
                        ButtonPressMask | ButtonReleaseMask | ButtonMotionMask)

static gboolean screen_validate_layout(ObDesktopLayout *l);
static gboolean replace_wm();
static void     screen_tell_ksplash();

guint    screen_num_desktops;
guint    screen_num_monitors;
guint    screen_desktop;
guint    screen_last_desktop;
Size     screen_physical_size;
gboolean screen_showing_desktop;
ObDesktopLayout screen_desktop_layout;
gchar  **screen_desktop_names;
Window   screen_support_win;
Time     screen_desktop_user_time = CurrentTime;

/*! An array of desktops, holding array of areas per monitor */
static Rect  *monitor_area = NULL;
/*! An array of desktops, holding an array of struts */
static GSList *struts_top = NULL;
static GSList *struts_left = NULL;
static GSList *struts_right = NULL;
static GSList *struts_bottom = NULL;

static ObPagerPopup *desktop_cycle_popup;

static gboolean replace_wm()
{
    gchar *wm_sn;
    Atom wm_sn_atom;
    Window current_wm_sn_owner;
    Time timestamp;

    wm_sn = g_strdup_printf("WM_S%d", ob_screen);
    wm_sn_atom = XInternAtom(ob_display, wm_sn, FALSE);
    g_free(wm_sn);

    current_wm_sn_owner = XGetSelectionOwner(ob_display, wm_sn_atom);
    if (current_wm_sn_owner == screen_support_win)
        current_wm_sn_owner = None;
    if (current_wm_sn_owner) {
        if (!ob_replace_wm) {
            g_message(_("A window manager is already running on screen %d"),
                      ob_screen);
            return FALSE;
        }
        xerror_set_ignore(TRUE);
        xerror_occured = FALSE;

        /* We want to find out when the current selection owner dies */
        XSelectInput(ob_display, current_wm_sn_owner, StructureNotifyMask);
        XSync(ob_display, FALSE);

        xerror_set_ignore(FALSE);
        if (xerror_occured)
            current_wm_sn_owner = None;
    }

    {
        /* Generate a timestamp */
        XEvent event;

        XSelectInput(ob_display, screen_support_win, PropertyChangeMask);

        XChangeProperty(ob_display, screen_support_win,
                        prop_atoms.wm_class, prop_atoms.string,
                        8, PropModeAppend, NULL, 0);
        XWindowEvent(ob_display, screen_support_win,
                     PropertyChangeMask, &event);

        XSelectInput(ob_display, screen_support_win, NoEventMask);

        timestamp = event.xproperty.time;
    }

    XSetSelectionOwner(ob_display, wm_sn_atom, screen_support_win,
                       timestamp);

    if (XGetSelectionOwner(ob_display, wm_sn_atom) != screen_support_win) {
        g_message(_("Could not acquire window manager selection on screen %d"),
                  ob_screen);
        return FALSE;
    }

    /* Wait for old window manager to go away */
    if (current_wm_sn_owner) {
      XEvent event;
      gulong wait = 0;
      const gulong timeout = G_USEC_PER_SEC * 15; /* wait for 15s max */

      while (wait < timeout) {
          if (XCheckWindowEvent(ob_display, current_wm_sn_owner,
                                StructureNotifyMask, &event) &&
              event.type == DestroyNotify)
              break;
          g_usleep(G_USEC_PER_SEC / 10);
          wait += G_USEC_PER_SEC / 10;
      }

      if (wait >= timeout) {
          g_message(_("The WM on screen %d is not exiting"), ob_screen);
          return FALSE;
      }
    }

    /* Send client message indicating that we are now the WM */
    prop_message(RootWindow(ob_display, ob_screen), prop_atoms.manager,
                 timestamp, wm_sn_atom, screen_support_win, 0,
                 SubstructureNotifyMask);

    return TRUE;
}

gboolean screen_annex()
{
    XSetWindowAttributes attrib;
    pid_t pid;
    gint i, num_support;
    Atom *prop_atoms_start, *wm_supported_pos;
    gulong *supported;

    /* create the netwm support window */
    attrib.override_redirect = TRUE;
    screen_support_win = XCreateWindow(ob_display,
                                       RootWindow(ob_display, ob_screen),
                                       -100, -100, 1, 1, 0,
                                       CopyFromParent, InputOutput,
                                       CopyFromParent,
                                       CWOverrideRedirect, &attrib);
    XMapWindow(ob_display, screen_support_win);
    XLowerWindow(ob_display, screen_support_win);

    if (!replace_wm()) {
        XDestroyWindow(ob_display, screen_support_win);
        return FALSE;
    }

    xerror_set_ignore(TRUE);
    xerror_occured = FALSE;
    XSelectInput(ob_display, RootWindow(ob_display, ob_screen),
                 ROOT_EVENTMASK);
    xerror_set_ignore(FALSE);
    if (xerror_occured) {
        g_message(_("A window manager is already running on screen %d"),
                  ob_screen);

        XDestroyWindow(ob_display, screen_support_win);
        return FALSE;
    }

    screen_set_root_cursor();

    /* set the OPENBOX_PID hint */
    pid = getpid();
    PROP_SET32(RootWindow(ob_display, ob_screen),
               openbox_pid, cardinal, pid);

    /* set supporting window */
    PROP_SET32(RootWindow(ob_display, ob_screen),
               net_supporting_wm_check, window, screen_support_win);

    /* set properties on the supporting window */
    PROP_SETS(screen_support_win, net_wm_name, "Openbox");
    PROP_SET32(screen_support_win, net_supporting_wm_check,
               window, screen_support_win);

    /* set the _NET_SUPPORTED_ATOMS hint */

    /* this is all the atoms after net_supported in the prop_atoms struct */
    prop_atoms_start = (Atom*)&prop_atoms;
    wm_supported_pos = (Atom*)&(prop_atoms.net_supported);
    num_support = sizeof(prop_atoms) / sizeof(Atom) -
        (wm_supported_pos - prop_atoms_start) - 1;
    i = 0;
    supported = g_new(gulong, num_support);
    supported[i++] = prop_atoms.net_supporting_wm_check;
    supported[i++] = prop_atoms.net_wm_full_placement;
    supported[i++] = prop_atoms.net_current_desktop;
    supported[i++] = prop_atoms.net_number_of_desktops;
    supported[i++] = prop_atoms.net_desktop_geometry;
    supported[i++] = prop_atoms.net_desktop_viewport;
    supported[i++] = prop_atoms.net_active_window;
    supported[i++] = prop_atoms.net_workarea;
    supported[i++] = prop_atoms.net_client_list;
    supported[i++] = prop_atoms.net_client_list_stacking;
    supported[i++] = prop_atoms.net_desktop_names;
    supported[i++] = prop_atoms.net_close_window;
    supported[i++] = prop_atoms.net_desktop_layout;
    supported[i++] = prop_atoms.net_showing_desktop;
    supported[i++] = prop_atoms.net_wm_name;
    supported[i++] = prop_atoms.net_wm_visible_name;
    supported[i++] = prop_atoms.net_wm_icon_name;
    supported[i++] = prop_atoms.net_wm_visible_icon_name;
    supported[i++] = prop_atoms.net_wm_desktop;
    supported[i++] = prop_atoms.net_wm_strut;
    supported[i++] = prop_atoms.net_wm_strut_partial;
    supported[i++] = prop_atoms.net_wm_icon;
    supported[i++] = prop_atoms.net_wm_icon_geometry;
    supported[i++] = prop_atoms.net_wm_window_type;
    supported[i++] = prop_atoms.net_wm_window_type_desktop;
    supported[i++] = prop_atoms.net_wm_window_type_dock;
    supported[i++] = prop_atoms.net_wm_window_type_toolbar;
    supported[i++] = prop_atoms.net_wm_window_type_menu;
    supported[i++] = prop_atoms.net_wm_window_type_utility;
    supported[i++] = prop_atoms.net_wm_window_type_splash;
    supported[i++] = prop_atoms.net_wm_window_type_dialog;
    supported[i++] = prop_atoms.net_wm_window_type_normal;
    supported[i++] = prop_atoms.net_wm_allowed_actions;
    supported[i++] = prop_atoms.net_wm_action_move;
    supported[i++] = prop_atoms.net_wm_action_resize;
    supported[i++] = prop_atoms.net_wm_action_minimize;
    supported[i++] = prop_atoms.net_wm_action_shade;
    supported[i++] = prop_atoms.net_wm_action_maximize_horz;
    supported[i++] = prop_atoms.net_wm_action_maximize_vert;
    supported[i++] = prop_atoms.net_wm_action_fullscreen;
    supported[i++] = prop_atoms.net_wm_action_change_desktop;
    supported[i++] = prop_atoms.net_wm_action_close;
    supported[i++] = prop_atoms.net_wm_action_above;
    supported[i++] = prop_atoms.net_wm_action_below;
    supported[i++] = prop_atoms.net_wm_state;
    supported[i++] = prop_atoms.net_wm_state_modal;
    supported[i++] = prop_atoms.net_wm_state_maximized_vert;
    supported[i++] = prop_atoms.net_wm_state_maximized_horz;
    supported[i++] = prop_atoms.net_wm_state_shaded;
    supported[i++] = prop_atoms.net_wm_state_skip_taskbar;
    supported[i++] = prop_atoms.net_wm_state_skip_pager;
    supported[i++] = prop_atoms.net_wm_state_hidden;
    supported[i++] = prop_atoms.net_wm_state_fullscreen;
    supported[i++] = prop_atoms.net_wm_state_above;
    supported[i++] = prop_atoms.net_wm_state_below;
    supported[i++] = prop_atoms.net_wm_state_demands_attention;
    supported[i++] = prop_atoms.net_moveresize_window;
    supported[i++] = prop_atoms.net_wm_moveresize;
    supported[i++] = prop_atoms.net_wm_user_time;
    supported[i++] = prop_atoms.net_wm_user_time_window;
    supported[i++] = prop_atoms.net_frame_extents;
    supported[i++] = prop_atoms.net_request_frame_extents;
    supported[i++] = prop_atoms.net_restack_window;
    supported[i++] = prop_atoms.net_startup_id;
#ifdef SYNC
    supported[i++] = prop_atoms.net_wm_sync_request;
    supported[i++] = prop_atoms.net_wm_sync_request_counter;
#endif

    supported[i++] = prop_atoms.kde_wm_change_state;
    supported[i++] = prop_atoms.kde_net_wm_frame_strut;
    supported[i++] = prop_atoms.kde_net_wm_window_type_override;

    supported[i++] = prop_atoms.ob_wm_action_undecorate;
    supported[i++] = prop_atoms.ob_wm_state_undecorated;
    supported[i++] = prop_atoms.openbox_pid;
    supported[i++] = prop_atoms.ob_theme;
    supported[i++] = prop_atoms.ob_control;
    g_assert(i == num_support);

    PROP_SETA32(RootWindow(ob_display, ob_screen),
                net_supported, atom, supported, num_support);
    g_free(supported);

    screen_tell_ksplash();

    return TRUE;
}

static void screen_tell_ksplash()
{
    XEvent e;
    char **argv;

    argv = g_new(gchar*, 6);
    argv[0] = g_strdup("dcop");
    argv[1] = g_strdup("ksplash");
    argv[2] = g_strdup("ksplash");
    argv[3] = g_strdup("upAndRunning(QString)");
    argv[4] = g_strdup("wm started");
    argv[5] = NULL;

    /* tell ksplash through the dcop server command line interface */
    g_spawn_async(NULL, argv, NULL,
                  G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD |
                  G_SPAWN_STDERR_TO_DEV_NULL | G_SPAWN_STDOUT_TO_DEV_NULL,
                  NULL, NULL, NULL, NULL);
    g_strfreev(argv);

    /* i'm not sure why we do this, kwin does it, but ksplash doesn't seem to
       hear it anyways. perhaps it is for old ksplash. or new ksplash. or
       something. oh well. */
    e.xclient.type = ClientMessage;
    e.xclient.display = ob_display;
    e.xclient.window = RootWindow(ob_display, ob_screen);
    e.xclient.message_type =
        XInternAtom(ob_display, "_KDE_SPLASH_PROGRESS", False );
    e.xclient.format = 8;
    strcpy(e.xclient.data.b, "wm started");
    XSendEvent(ob_display, RootWindow(ob_display, ob_screen),
               False, SubstructureNotifyMask, &e );
}

void screen_startup(gboolean reconfig)
{
    gchar **names = NULL;
    guint32 d;
    gboolean namesexist = FALSE;

    desktop_cycle_popup = pager_popup_new(FALSE);
    pager_popup_height(desktop_cycle_popup, POPUP_HEIGHT);

    if (reconfig) {
        /* update the pager popup's width */
        pager_popup_text_width_to_strings(desktop_cycle_popup,
                                          screen_desktop_names,
                                          screen_num_desktops);
        return;
    }

#ifdef USE_XCOMPOSITE
    if (extensions_comp) {
        /* Redirect window contents to offscreen pixmaps */
        XCompositeRedirectSubwindows(ob_display,
                                     RootWindow(ob_display, ob_screen),
                                     CompositeRedirectAutomatic);
    }
#endif

    /* get the initial size */
    screen_resize();

    /* have names already been set for the desktops? */
    if (PROP_GETSS(RootWindow(ob_display, ob_screen),
                   net_desktop_names, utf8, &names))
    {
        g_strfreev(names);
        namesexist = TRUE;
    }

    /* if names don't exist and we have session names, set those.
       do this stuff BEFORE setting the number of desktops, because that
       will create default names for them
    */
    if (!namesexist && session_desktop_names != NULL) {
        guint i, numnames;
        GSList *it;

        /* get the desktop names */
        numnames = g_slist_length(session_desktop_names);
        names = g_new(gchar*, numnames + 1);
        names[numnames] = NULL;
        for (i = 0, it = session_desktop_names; it; ++i, it = g_slist_next(it))
            names[i] = g_strdup(it->data);

        /* set the root window property */
        PROP_SETSS(RootWindow(ob_display, ob_screen), net_desktop_names,names);

        g_strfreev(names);
    }

    /* set the number of desktops, if it's not already set.

       this will also set the default names from the config file up for
       desktops that don't have names yet */
    screen_num_desktops = 0;
    if (PROP_GET32(RootWindow(ob_display, ob_screen),
                   net_number_of_desktops, cardinal, &d))
        screen_set_num_desktops(d);
    /* restore from session if possible */
    else if (session_num_desktops)
        screen_set_num_desktops(session_num_desktops);
    else
        screen_set_num_desktops(config_desktops_num);

    screen_desktop = screen_num_desktops;  /* something invalid */
    /* start on the current desktop when a wm was already running */
    if (PROP_GET32(RootWindow(ob_display, ob_screen),
                   net_current_desktop, cardinal, &d) &&
        d < screen_num_desktops)
    {
        screen_set_desktop(d, FALSE);
    } else if (session_desktop >= 0)
        screen_set_desktop(MIN((guint)session_desktop,
                               screen_num_desktops), FALSE);
    else
        screen_set_desktop(MIN(config_screen_firstdesk,
                               screen_num_desktops) - 1, FALSE);
    screen_last_desktop = screen_desktop;

    /* don't start in showing-desktop mode */
    screen_showing_desktop = FALSE;
    PROP_SET32(RootWindow(ob_display, ob_screen),
               net_showing_desktop, cardinal, screen_showing_desktop);

    if (session_desktop_layout_present &&
        screen_validate_layout(&session_desktop_layout))
    {
        screen_desktop_layout = session_desktop_layout;
    }
    else
        screen_update_layout();
}

void screen_shutdown(gboolean reconfig)
{
    pager_popup_free(desktop_cycle_popup);

    if (reconfig)
        return;

    XSelectInput(ob_display, RootWindow(ob_display, ob_screen),
                 NoEventMask);

    /* we're not running here no more! */
    PROP_ERASE(RootWindow(ob_display, ob_screen), openbox_pid);
    /* not without us */
    PROP_ERASE(RootWindow(ob_display, ob_screen), net_supported);
    /* don't keep this mode */
    PROP_ERASE(RootWindow(ob_display, ob_screen), net_showing_desktop);

    XDestroyWindow(ob_display, screen_support_win);

    g_strfreev(screen_desktop_names);
    screen_desktop_names = NULL;
}

void screen_resize()
{
    static gint oldw = 0, oldh = 0;
    gint w, h;
    GList *it;
    gulong geometry[2];

    w = WidthOfScreen(ScreenOfDisplay(ob_display, ob_screen));
    h = HeightOfScreen(ScreenOfDisplay(ob_display, ob_screen));

    if (w == oldw && h == oldh) return;

    oldw = w; oldh = h;

    /* Set the _NET_DESKTOP_GEOMETRY hint */
    screen_physical_size.width = geometry[0] = w;
    screen_physical_size.height = geometry[1] = h;
    PROP_SETA32(RootWindow(ob_display, ob_screen),
                net_desktop_geometry, cardinal, geometry, 2);

    if (ob_state() == OB_STATE_STARTING)
        return;

    screen_update_areas();
    dock_configure();

    for (it = client_list; it; it = g_list_next(it))
        client_move_onscreen(it->data, FALSE);
}

void screen_set_num_desktops(guint num)
{
    guint old;
    gulong *viewport;
    GList *it, *stacking_copy;

    g_assert(num > 0);

    if (screen_num_desktops == num) return;

    old = screen_num_desktops;
    screen_num_desktops = num;
    PROP_SET32(RootWindow(ob_display, ob_screen),
               net_number_of_desktops, cardinal, num);

    /* set the viewport hint */
    viewport = g_new0(gulong, num * 2);
    PROP_SETA32(RootWindow(ob_display, ob_screen),
                net_desktop_viewport, cardinal, viewport, num * 2);
    g_free(viewport);

    /* the number of rows/columns will differ */
    screen_update_layout();

    /* move windows on desktops that will no longer exist!
       make a copy of the list cuz we're changing it */
    stacking_copy = g_list_copy(stacking_list);
    for (it = g_list_last(stacking_copy); it; it = g_list_previous(it)) {
        if (WINDOW_IS_CLIENT(it->data)) {
            ObClient *c = it->data;
            if (c->desktop != DESKTOP_ALL && c->desktop >= num)
                client_set_desktop(c, num - 1, FALSE, TRUE);
            /* raise all the windows that are on the current desktop which
               is being merged */
            else if (screen_desktop == num - 1 &&
                     (c->desktop == DESKTOP_ALL ||
                      c->desktop == screen_desktop))
                stacking_raise(WINDOW_AS_CLIENT(c));
        }
    }
 
    /* change our struts/area to match (after moving windows) */
    screen_update_areas();

    /* may be some unnamed desktops that we need to fill in with names
     (after updating the areas so the popup can resize) */
    screen_update_desktop_names();

    /* change our desktop if we're on one that no longer exists! */
    if (screen_desktop >= screen_num_desktops)
        screen_set_desktop(num - 1, TRUE);
}

void screen_set_desktop(guint num, gboolean dofocus)
{
    ObClient *c;
    GList *it;
    guint old;
    gulong ignore_start;
    gboolean allow_omni;
     
    g_assert(num < screen_num_desktops);

    old = screen_desktop;
    screen_desktop = num;

    if (old == num) return;

    PROP_SET32(RootWindow(ob_display, ob_screen),
               net_current_desktop, cardinal, num);

    screen_last_desktop = old;

    ob_debug("Moving to desktop %d\n", num+1);

    /* ignore enter events caused by the move */
    ignore_start = event_start_ignore_all_enters();

    if (moveresize_client)
        client_set_desktop(moveresize_client, num, TRUE, FALSE);

    /* show windows before hiding the rest to lessen the enter/leave events */

    /* show windows from top to bottom */
    for (it = stacking_list; it; it = g_list_next(it)) {
        if (WINDOW_IS_CLIENT(it->data)) {
            ObClient *c = it->data;
            client_show(c);
        }
    }

    /* only allow omnipresent windows to get focus on desktop change if
       an omnipresent window is already focused (it'll keep focus probably, but
       maybe not depending on mouse-focus options) */
    allow_omni = focus_client && (client_normal(focus_client) &&
                                  focus_client->desktop == DESKTOP_ALL);

    /* the client moved there already so don't move focus. prevent flicker
       on sendtodesktop + follow */
    if (focus_client && focus_client->desktop == screen_desktop)
        dofocus = FALSE;

    /* have to try focus here because when you leave an empty desktop
       there is no focus out to watch for. also, we have different rules
       here. we always allow it to look under the mouse pointer if
       config_focus_last is FALSE

       do this before hiding the windows so if helper windows are coming
       with us, they don't get hidden
    */
    if (dofocus && (c = focus_fallback(TRUE, !config_focus_last, allow_omni)))
    {
        /* only do the flicker reducing stuff ahead of time if we are going
           to call xsetinputfocus on the window ourselves. otherwise there is
           no guarantee the window will actually take focus.. */
        if (c->can_focus) {
            /* reduce flicker by hiliting now rather than waiting for the
               server FocusIn event */
            frame_adjust_focus(c->frame, TRUE);
            /* do this here so that if you switch desktops to a window with
               helper windows then the helper windows won't flash */
            client_bring_helper_windows(c);
        }
    }

    /* hide windows from bottom to top */
    for (it = g_list_last(stacking_list); it; it = g_list_previous(it)) {
        if (WINDOW_IS_CLIENT(it->data)) {
            ObClient *c = it->data;
            client_hide(c);
        }
    }

    event_end_ignore_all_enters(ignore_start);

    if (event_curtime != CurrentTime)
        screen_desktop_user_time = event_curtime;
}

static void get_row_col(guint d, guint *r, guint *c)
{
    switch (screen_desktop_layout.orientation) {
    case OB_ORIENTATION_HORZ:
        switch (screen_desktop_layout.start_corner) {
        case OB_CORNER_TOPLEFT:
            *r = d / screen_desktop_layout.columns;
            *c = d % screen_desktop_layout.columns;
            break;
        case OB_CORNER_BOTTOMLEFT:
            *r = screen_desktop_layout.rows - 1 -
                d / screen_desktop_layout.columns;
            *c = d % screen_desktop_layout.columns;
            break;
        case OB_CORNER_TOPRIGHT:
            *r = d / screen_desktop_layout.columns;
            *c = screen_desktop_layout.columns - 1 -
                d % screen_desktop_layout.columns;
            break;
        case OB_CORNER_BOTTOMRIGHT:
            *r = screen_desktop_layout.rows - 1 -
                d / screen_desktop_layout.columns;
            *c = screen_desktop_layout.columns - 1 -
                d % screen_desktop_layout.columns;
            break;
        }
        break;
    case OB_ORIENTATION_VERT:
        switch (screen_desktop_layout.start_corner) {
        case OB_CORNER_TOPLEFT:
            *r = d % screen_desktop_layout.rows;
            *c = d / screen_desktop_layout.rows;
            break;
        case OB_CORNER_BOTTOMLEFT:
            *r = screen_desktop_layout.rows - 1 -
                d % screen_desktop_layout.rows;
            *c = d / screen_desktop_layout.rows;
            break;
        case OB_CORNER_TOPRIGHT:
            *r = d % screen_desktop_layout.rows;
            *c = screen_desktop_layout.columns - 1 -
                d / screen_desktop_layout.rows;
            break;
        case OB_CORNER_BOTTOMRIGHT:
            *r = screen_desktop_layout.rows - 1 -
                d % screen_desktop_layout.rows;
            *c = screen_desktop_layout.columns - 1 -
                d / screen_desktop_layout.rows;
            break;
        }
        break;
    }
}

static guint translate_row_col(guint r, guint c)
{
    switch (screen_desktop_layout.orientation) {
    case OB_ORIENTATION_HORZ:
        switch (screen_desktop_layout.start_corner) {
        case OB_CORNER_TOPLEFT:
            return r % screen_desktop_layout.rows *
                screen_desktop_layout.columns +
                c % screen_desktop_layout.columns;
        case OB_CORNER_BOTTOMLEFT:
            return (screen_desktop_layout.rows - 1 -
                    r % screen_desktop_layout.rows) *
                screen_desktop_layout.columns +
                c % screen_desktop_layout.columns;
        case OB_CORNER_TOPRIGHT:
            return r % screen_desktop_layout.rows *
                screen_desktop_layout.columns +
                (screen_desktop_layout.columns - 1 -
                 c % screen_desktop_layout.columns);
        case OB_CORNER_BOTTOMRIGHT:
            return (screen_desktop_layout.rows - 1 -
                    r % screen_desktop_layout.rows) *
                screen_desktop_layout.columns +
                (screen_desktop_layout.columns - 1 -
                 c % screen_desktop_layout.columns);
        }
    case OB_ORIENTATION_VERT:
        switch (screen_desktop_layout.start_corner) {
        case OB_CORNER_TOPLEFT:
            return c % screen_desktop_layout.columns *
                screen_desktop_layout.rows +
                r % screen_desktop_layout.rows;
        case OB_CORNER_BOTTOMLEFT:
            return c % screen_desktop_layout.columns *
                screen_desktop_layout.rows +
                (screen_desktop_layout.rows - 1 -
                 r % screen_desktop_layout.rows);
        case OB_CORNER_TOPRIGHT:
            return (screen_desktop_layout.columns - 1 -
                    c % screen_desktop_layout.columns) *
                screen_desktop_layout.rows +
                r % screen_desktop_layout.rows;
        case OB_CORNER_BOTTOMRIGHT:
            return (screen_desktop_layout.columns - 1 -
                    c % screen_desktop_layout.columns) *
                screen_desktop_layout.rows +
                (screen_desktop_layout.rows - 1 -
                 r % screen_desktop_layout.rows);
        }
    }
    g_assert_not_reached();
    return 0;
}

void screen_desktop_popup(guint d, gboolean show)
{
    Rect *a;

    if (!show) {
        pager_popup_hide(desktop_cycle_popup);
    } else {
        a = screen_physical_area_active();
        pager_popup_position(desktop_cycle_popup, CenterGravity,
                             a->x + a->width / 2, a->y + a->height / 2);
        pager_popup_icon_size_multiplier(desktop_cycle_popup,
                                         (screen_desktop_layout.columns /
                                          screen_desktop_layout.rows) / 2,
                                         (screen_desktop_layout.rows/
                                          screen_desktop_layout.columns) / 2);
        pager_popup_max_width(desktop_cycle_popup,
                              MAX(a->width/3, POPUP_WIDTH));
        pager_popup_show(desktop_cycle_popup, screen_desktop_names[d], d);
    }
}

guint screen_cycle_desktop(ObDirection dir, gboolean wrap, gboolean linear,
                           gboolean dialog, gboolean done, gboolean cancel)
{
    guint r, c;
    static guint d = (guint)-1;
    guint ret, oldd;

    if (d == (guint)-1)
        d = screen_desktop;

    if ((cancel || done) && dialog)
        goto show_cycle_dialog;

    oldd = d;
    get_row_col(d, &r, &c);

    if (linear) {
        switch (dir) {
        case OB_DIRECTION_EAST:
            if (d < screen_num_desktops - 1)
                ++d;
            else if (wrap)
                d = 0;
            break;
        case OB_DIRECTION_WEST:
            if (d > 0)
                --d;
            else if (wrap)
                d = screen_num_desktops - 1;
            break;
        default:
            assert(0);
            return screen_desktop;
        }
    } else {
        switch (dir) {
        case OB_DIRECTION_EAST:
            ++c;
            if (c >= screen_desktop_layout.columns) {
                if (wrap)
                    c = 0;
                else
                    goto show_cycle_dialog;
            }
            d = translate_row_col(r, c);
            if (d >= screen_num_desktops) {
                if (wrap) {
                    ++c;
                } else {
                    d = oldd;
                    goto show_cycle_dialog;
                }
            }
            break;
        case OB_DIRECTION_WEST:
            --c;
            if (c >= screen_desktop_layout.columns) {
                if (wrap)
                    c = screen_desktop_layout.columns - 1;
                else
                    goto show_cycle_dialog;
            }
            d = translate_row_col(r, c);
            if (d >= screen_num_desktops) {
                if (wrap) {
                    --c;
                } else {
                    d = oldd;
                    goto show_cycle_dialog;
                }
            }
            break;
        case OB_DIRECTION_SOUTH:
            ++r;
            if (r >= screen_desktop_layout.rows) {
                if (wrap)
                    r = 0;
                else
                    goto show_cycle_dialog;
            }
            d = translate_row_col(r, c);
            if (d >= screen_num_desktops) {
                if (wrap) {
                    ++r;
                } else {
                    d = oldd;
                    goto show_cycle_dialog;
                }
            }
            break;
        case OB_DIRECTION_NORTH:
            --r;
            if (r >= screen_desktop_layout.rows) {
                if (wrap)
                    r = screen_desktop_layout.rows - 1;
                else
                    goto show_cycle_dialog;
            }
            d = translate_row_col(r, c);
            if (d >= screen_num_desktops) {
                if (wrap) {
                    --r;
                } else {
                    d = oldd;
                    goto show_cycle_dialog;
                }
            }
            break;
        default:
            assert(0);
            return d = screen_desktop;
        }

        d = translate_row_col(r, c);
    }

show_cycle_dialog:
    if (dialog && !cancel && !done) {
        screen_desktop_popup(d, TRUE);
    } else
        screen_desktop_popup(0, FALSE);
    ret = d;

    if (!dialog || cancel || done)
        d = (guint)-1;

    return ret;
}

static gboolean screen_validate_layout(ObDesktopLayout *l)
{
    if (l->columns == 0 && l->rows == 0) /* both 0's is bad data.. */
        return FALSE;

    /* fill in a zero rows/columns */
    if (l->columns == 0) {
        l->columns = screen_num_desktops / l->rows;
        if (l->rows * l->columns < screen_num_desktops)
            l->columns++;
        if (l->rows * l->columns >= screen_num_desktops + l->columns)
            l->rows--;
    } else if (l->rows == 0) {
        l->rows = screen_num_desktops / l->columns;
        if (l->columns * l->rows < screen_num_desktops)
            l->rows++;
        if (l->columns * l->rows >= screen_num_desktops + l->rows)
            l->columns--;
    }

    /* bounds checking */
    if (l->orientation == OB_ORIENTATION_HORZ) {
        l->columns = MIN(screen_num_desktops, l->columns);
        l->rows = MIN(l->rows,
                      (screen_num_desktops + l->columns - 1) / l->columns);
        l->columns = screen_num_desktops / l->rows +
            !!(screen_num_desktops % l->rows);
    } else {
        l->rows = MIN(screen_num_desktops, l->rows);
        l->columns = MIN(l->columns,
                         (screen_num_desktops + l->rows - 1) / l->rows);
        l->rows = screen_num_desktops / l->columns +
            !!(screen_num_desktops % l->columns);
    }
    return TRUE;
}

void screen_update_layout()

{
    ObDesktopLayout l;
    guint32 *data;
    guint num;

    screen_desktop_layout.orientation = OB_ORIENTATION_HORZ;
    screen_desktop_layout.start_corner = OB_CORNER_TOPLEFT;
    screen_desktop_layout.rows = 1;
    screen_desktop_layout.columns = screen_num_desktops;

    if (PROP_GETA32(RootWindow(ob_display, ob_screen),
                    net_desktop_layout, cardinal, &data, &num)) {
        if (num == 3 || num == 4) {
            
            if (data[0] == prop_atoms.net_wm_orientation_vert)
                l.orientation = OB_ORIENTATION_VERT;
            else if (data[0] == prop_atoms.net_wm_orientation_horz)
                l.orientation = OB_ORIENTATION_HORZ;
            else
                return;

            if (num < 4)
                l.start_corner = OB_CORNER_TOPLEFT;
            else {
                if (data[3] == prop_atoms.net_wm_topleft)
                    l.start_corner = OB_CORNER_TOPLEFT;
                else if (data[3] == prop_atoms.net_wm_topright)
                    l.start_corner = OB_CORNER_TOPRIGHT;
                else if (data[3] == prop_atoms.net_wm_bottomright)
                    l.start_corner = OB_CORNER_BOTTOMRIGHT;
                else if (data[3] == prop_atoms.net_wm_bottomleft)
                    l.start_corner = OB_CORNER_BOTTOMLEFT;
                else
                    return;
            }

            l.columns = data[1];
            l.rows = data[2];

            if (screen_validate_layout(&l))
                screen_desktop_layout = l;

            g_free(data);
        }
    }
}

void screen_update_desktop_names()
{
    guint i;

    /* empty the array */
    g_strfreev(screen_desktop_names);
    screen_desktop_names = NULL;

    if (PROP_GETSS(RootWindow(ob_display, ob_screen),
                   net_desktop_names, utf8, &screen_desktop_names))
        for (i = 0; screen_desktop_names[i] && i < screen_num_desktops; ++i);
    else
        i = 0;
    if (i < screen_num_desktops) {
        GSList *it;

        screen_desktop_names = g_renew(gchar*, screen_desktop_names,
                                       screen_num_desktops + 1);
        screen_desktop_names[screen_num_desktops] = NULL;

        it = g_slist_nth(config_desktops_names, i);

        for (; i < screen_num_desktops; ++i) {
            if (it && ((char*)it->data)[0]) /* not empty */
                /* use the names from the config file when possible */
                screen_desktop_names[i] = g_strdup(it->data);
            else
                /* make up a nice name if it's not though */
                screen_desktop_names[i] = g_strdup_printf(_("desktop %i"),
                                                          i + 1);
            if (it) it = g_slist_next(it);
        }

        /* if we changed any names, then set the root property so we can
           all agree on the names */
        PROP_SETSS(RootWindow(ob_display, ob_screen), net_desktop_names,
                   screen_desktop_names);
    }

    /* resize the pager for these names */
    pager_popup_text_width_to_strings(desktop_cycle_popup,
                                      screen_desktop_names,
                                      screen_num_desktops);
}

void screen_show_desktop(gboolean show, ObClient *show_only)
{
    GList *it;
     
    if (show == screen_showing_desktop) return; /* no change */

    screen_showing_desktop = show;

    if (show) {
        /* hide windows bottom to top */
        for (it = g_list_last(stacking_list); it; it = g_list_previous(it)) {
            if (WINDOW_IS_CLIENT(it->data)) {
                ObClient *client = it->data;
                client_showhide(client);
            }
        }
    }
    else {
        /* restore windows top to bottom */
        for (it = stacking_list; it; it = g_list_next(it)) {
            if (WINDOW_IS_CLIENT(it->data)) {
                ObClient *client = it->data;
                if (client_should_show(client)) {
                    if (!show_only || client == show_only)
                        client_show(client);
                    else
                        client_iconify(client, TRUE, FALSE, TRUE);
                }
            }
        }
    }

    if (show) {
        /* focus the desktop */
        for (it = focus_order; it; it = g_list_next(it)) {
            ObClient *c = it->data;
            if (c->type == OB_CLIENT_TYPE_DESKTOP &&
                (c->desktop == screen_desktop || c->desktop == DESKTOP_ALL) &&
                client_focus(it->data))
                break;
        }
    }
    else if (!show_only) {
        ObClient *c;

        if ((c = focus_fallback(TRUE, FALSE, TRUE))) {
            /* only do the flicker reducing stuff ahead of time if we are going
               to call xsetinputfocus on the window ourselves. otherwise there
               is no guarantee the window will actually take focus.. */
            if (c->can_focus) {
                /* reduce flicker by hiliting now rather than waiting for the
                   server FocusIn event */
                frame_adjust_focus(c->frame, TRUE);
            }
        }
    }

    show = !!show; /* make it boolean */
    PROP_SET32(RootWindow(ob_display, ob_screen),
               net_showing_desktop, cardinal, show);
}

void screen_install_colormap(ObClient *client, gboolean install)
{
    if (client == NULL || client->colormap == None) {
        if (install)
            XInstallColormap(RrDisplay(ob_rr_inst), RrColormap(ob_rr_inst));
        else
            XUninstallColormap(RrDisplay(ob_rr_inst), RrColormap(ob_rr_inst));
    } else {
        xerror_set_ignore(TRUE);
        if (install)
            XInstallColormap(RrDisplay(ob_rr_inst), client->colormap);
        else
            XUninstallColormap(RrDisplay(ob_rr_inst), client->colormap);
        xerror_set_ignore(FALSE);
    }
}

#define STRUT_LEFT_ON_MONITOR(s, i) \
    (RANGES_INTERSECT(s->left_start, s->left_end - s->left_start + 1, \
                      monitor_area[i].y, monitor_area[i].height))
#define STRUT_RIGHT_ON_MONITOR(s, i) \
    (RANGES_INTERSECT(s->right_start, s->right_end - s->right_start + 1, \
                      monitor_area[i].y, monitor_area[i].height))
#define STRUT_TOP_ON_MONITOR(s, i) \
    (RANGES_INTERSECT(s->top_start, s->top_end - s->top_start + 1, \
                      monitor_area[i].x, monitor_area[i].width))
#define STRUT_BOTTOM_ON_MONITOR(s, i) \
    (RANGES_INTERSECT(s->bottom_start, s->bottom_end - s->bottom_start + 1, \
                      monitor_area[i].x, monitor_area[i].width))

typedef struct {
    guint desktop;
    StrutPartial *strut;
} ObScreenStrut;

#define RESET_STRUT_LIST(sl) \
    (g_slist_free(sl), sl = NULL)

#define ADD_STRUT_TO_LIST(sl, d, s) \
{ \
    ObScreenStrut *ss = g_new(ObScreenStrut, 1); \
    ss->desktop = d; \
    ss->strut = s;  \
    sl = g_slist_prepend(sl, ss); \
}

void screen_update_areas()
{
    guint i, j;
    gulong *dims;
    GList *it;
    GSList *sit;

    ob_debug("updating screen areas\n");

    g_free(monitor_area);
    extensions_xinerama_screens(&monitor_area, &screen_num_monitors);

    dims = g_new(gulong, 4 * screen_num_desktops * screen_num_monitors);

    RESET_STRUT_LIST(struts_left);
    RESET_STRUT_LIST(struts_top);
    RESET_STRUT_LIST(struts_right);
    RESET_STRUT_LIST(struts_bottom);

    /* collect the struts */
    for (it = client_list; it; it = g_list_next(it)) {
        ObClient *c = it->data;
        if (c->strut.left)
            ADD_STRUT_TO_LIST(struts_left, c->desktop, &c->strut);
        if (c->strut.top)
            ADD_STRUT_TO_LIST(struts_top, c->desktop, &c->strut);
        if (c->strut.right)
            ADD_STRUT_TO_LIST(struts_right, c->desktop, &c->strut);
        if (c->strut.bottom)
            ADD_STRUT_TO_LIST(struts_bottom, c->desktop, &c->strut);
    }
    if (dock_strut.left)
        ADD_STRUT_TO_LIST(struts_left, DESKTOP_ALL, &dock_strut);
    if (dock_strut.top)
        ADD_STRUT_TO_LIST(struts_top, DESKTOP_ALL, &dock_strut);
    if (dock_strut.right)
        ADD_STRUT_TO_LIST(struts_right, DESKTOP_ALL, &dock_strut);
    if (dock_strut.bottom)
        ADD_STRUT_TO_LIST(struts_bottom, DESKTOP_ALL, &dock_strut);

    /* set up the work areas to be full screen */
    for (i = 0; i < screen_num_monitors; ++i)
        for (j = 0; j < screen_num_desktops; ++j) {
            dims[(i * screen_num_desktops + j) * 4+0] = monitor_area[i].x;
            dims[(i * screen_num_desktops + j) * 4+1] = monitor_area[i].y;
            dims[(i * screen_num_desktops + j) * 4+2] = monitor_area[i].width;
            dims[(i * screen_num_desktops + j) * 4+3] = monitor_area[i].height;
        }

    /* calculate the work areas from the struts */
    for (i = 0; i < screen_num_monitors; ++i)
        for (j = 0; j < screen_num_desktops; ++j) {
            gint l = 0, r = 0, t = 0, b = 0;

            /* only add the strut to the area if it touches the monitor */

            for (sit = struts_left; sit; sit = g_slist_next(sit)) {
                ObScreenStrut *s = sit->data;
                if ((s->desktop == j || s->desktop == DESKTOP_ALL) &&
                    STRUT_LEFT_ON_MONITOR(s->strut, i))
                    l = MAX(l, s->strut->left);
            }
            for (sit = struts_top; sit; sit = g_slist_next(sit)) {
                ObScreenStrut *s = sit->data;
                if ((s->desktop == j || s->desktop == DESKTOP_ALL) &&
                    STRUT_TOP_ON_MONITOR(s->strut, i))
                    t = MAX(t, s->strut->top);
            }
            for (sit = struts_right; sit; sit = g_slist_next(sit)) {
                ObScreenStrut *s = sit->data;
                if ((s->desktop == j || s->desktop == DESKTOP_ALL) &&
                    STRUT_RIGHT_ON_MONITOR(s->strut, i))
                    r = MAX(r, s->strut->right);
            }
            for (sit = struts_bottom; sit; sit = g_slist_next(sit)) {
                ObScreenStrut *s = sit->data;
                if ((s->desktop == j || s->desktop == DESKTOP_ALL) &&
                    STRUT_BOTTOM_ON_MONITOR(s->strut, i))
                    b = MAX(b, s->strut->bottom);
            }

            /* based on these margins, set the work area for the
               monitor/desktop */
            dims[(i * screen_num_desktops + j) * 4 + 0] += l;
            dims[(i * screen_num_desktops + j) * 4 + 1] += t;
            dims[(i * screen_num_desktops + j) * 4 + 2] -= l + r;
            dims[(i * screen_num_desktops + j) * 4 + 3] -= t + b;
        }

    PROP_SETA32(RootWindow(ob_display, ob_screen), net_workarea, cardinal,
                dims, 4 * screen_num_desktops * screen_num_monitors);

    /* the area has changed, adjust all the windows if they need it */
    for (it = client_list; it; it = g_list_next(it))
        client_reconfigure(it->data, FALSE);

    g_free(dims);
}

#if 0
Rect* screen_area_all_monitors(guint desktop)
{
    guint i;
    Rect *a;

    a = screen_area_monitor(desktop, 0);

    /* combine all the monitors together */
    for (i = 1; i < screen_num_monitors; ++i) {
        Rect *m = screen_area_monitor(desktop, i);
        gint l, r, t, b;

        l = MIN(RECT_LEFT(*a), RECT_LEFT(*m));
        t = MIN(RECT_TOP(*a), RECT_TOP(*m));
        r = MAX(RECT_RIGHT(*a), RECT_RIGHT(*m));
        b = MAX(RECT_BOTTOM(*a), RECT_BOTTOM(*m));

        RECT_SET(*a, l, t, r - l + 1, b - t + 1);

        g_free(m);
    }
        
    return a;
}
#endif

#define STRUT_LEFT_IN_SEARCH(s, search) \
    (RANGES_INTERSECT(search->y, search->height, \
                      s->left_start, s->left_end - s->left_start + 1))
#define STRUT_RIGHT_IN_SEARCH(s, search) \
    (RANGES_INTERSECT(search->y, search->height, \
                      s->right_start, s->right_end - s->right_start + 1))
#define STRUT_TOP_IN_SEARCH(s, search) \
    (RANGES_INTERSECT(search->x, search->width, \
                      s->top_start, s->top_end - s->top_start + 1))
#define STRUT_BOTTOM_IN_SEARCH(s, search) \
    (RANGES_INTERSECT(search->x, search->width, \
                      s->bottom_start, s->bottom_end - s->bottom_start + 1))

#define STRUT_LEFT_IGNORE(s, us, search) \
    (head == SCREEN_AREA_ALL_MONITORS && us && \
     RECT_LEFT(monitor_area[i]) + s->left > RECT_LEFT(*search))
#define STRUT_RIGHT_IGNORE(s, us, search) \
    (head == SCREEN_AREA_ALL_MONITORS && us && \
     RECT_RIGHT(monitor_area[i]) - s->right < RECT_RIGHT(*search))
#define STRUT_TOP_IGNORE(s, us, search) \
    (head == SCREEN_AREA_ALL_MONITORS && us && \
     RECT_TOP(monitor_area[i]) + s->top > RECT_TOP(*search))
#define STRUT_BOTTOM_IGNORE(s, us, search) \
    (head == SCREEN_AREA_ALL_MONITORS && us && \
     RECT_BOTTOM(monitor_area[i]) - s->bottom < RECT_BOTTOM(*search))

Rect* screen_area(guint desktop, guint head, Rect *search)
{
    Rect *a;
    GSList *it;
    gint l, r, t, b, al, ar, at, ab;
    guint i, d;
    gboolean us = search != NULL; /* user provided search */

    g_assert(desktop < screen_num_desktops || desktop == DESKTOP_ALL);
    g_assert(head < screen_num_monitors || head == SCREEN_AREA_ONE_MONITOR ||
             head == SCREEN_AREA_ALL_MONITORS);
    g_assert(!(head == SCREEN_AREA_ONE_MONITOR && search == NULL));

    /* find any struts for this monitor
       which will be affecting the search area.
    */

    /* search everything if search is null */
    if (!search) {
        if (head < screen_num_monitors) search = &monitor_area[head];
        else search = &monitor_area[screen_num_monitors];
    }
    if (head == SCREEN_AREA_ONE_MONITOR) head = screen_find_monitor(search);

    /* al is "all left" meaning the furthest left you can get, l is our
       "working left" meaning our current strut edge which we're calculating
    */

    /* only include monitors which the search area lines up with */
    if (RECT_INTERSECTS_RECT(monitor_area[screen_num_monitors], *search)) {
        al = l = RECT_RIGHT(monitor_area[screen_num_monitors]);
        at = t = RECT_BOTTOM(monitor_area[screen_num_monitors]);
        ar = r = RECT_LEFT(monitor_area[screen_num_monitors]);
        ab = b = RECT_TOP(monitor_area[screen_num_monitors]);
        for (i = 0; i < screen_num_monitors; ++i) {
            /* add the monitor if applicable */
            if (RANGES_INTERSECT(search->x, search->width,
                                 monitor_area[i].x, monitor_area[i].width))
            {
                at = t = MIN(t, RECT_TOP(monitor_area[i]));
                ab = b = MAX(b, RECT_BOTTOM(monitor_area[i]));
            }
            if (RANGES_INTERSECT(search->y, search->height,
                                 monitor_area[i].y, monitor_area[i].height))
            {
                al = l = MIN(l, RECT_LEFT(monitor_area[i]));
                ar = r = MAX(r, RECT_RIGHT(monitor_area[i]));
            }
        }
    } else {
        al = l = RECT_LEFT(monitor_area[screen_num_monitors]);
        at = t = RECT_TOP(monitor_area[screen_num_monitors]);
        ar = r = RECT_RIGHT(monitor_area[screen_num_monitors]);
        ab = b = RECT_BOTTOM(monitor_area[screen_num_monitors]);
    }

    for (d = 0; d < screen_num_desktops; ++d) {
        if (d != desktop && desktop != DESKTOP_ALL) continue;

        for (i = 0; i < screen_num_monitors; ++i) {
            if (head != SCREEN_AREA_ALL_MONITORS && head != i) continue;

            for (it = struts_left; it; it = g_slist_next(it)) {
                ObScreenStrut *s = it->data;
                if ((s->desktop == d || s->desktop == DESKTOP_ALL) &&
                    STRUT_LEFT_IN_SEARCH(s->strut, search) &&
                    !STRUT_LEFT_IGNORE(s->strut, us, search))
                    l = MAX(l, al + s->strut->left);
            }
            for (it = struts_top; it; it = g_slist_next(it)) {
                ObScreenStrut *s = it->data;
                if ((s->desktop == d || s->desktop == DESKTOP_ALL) &&
                    STRUT_TOP_IN_SEARCH(s->strut, search) &&
                    !STRUT_TOP_IGNORE(s->strut, us, search))
                    t = MAX(t, at + s->strut->top);
            }
            for (it = struts_right; it; it = g_slist_next(it)) {
                ObScreenStrut *s = it->data;
                if ((s->desktop == d || s->desktop == DESKTOP_ALL) &&
                    STRUT_RIGHT_IN_SEARCH(s->strut, search) &&
                    !STRUT_RIGHT_IGNORE(s->strut, us, search))
                    r = MIN(r, ar - s->strut->right);
            }
            for (it = struts_bottom; it; it = g_slist_next(it)) {
                ObScreenStrut *s = it->data;
                if ((s->desktop == d || s->desktop == DESKTOP_ALL) &&
                    STRUT_BOTTOM_IN_SEARCH(s->strut, search) &&
                    !STRUT_BOTTOM_IGNORE(s->strut, us, search))
                    b = MIN(b, ab - s->strut->bottom);
            }

            /* limit to this monitor */
            if (head == i) {
                l = MAX(l, RECT_LEFT(monitor_area[i]));
                t = MAX(t, RECT_TOP(monitor_area[i]));
                r = MIN(r, RECT_RIGHT(monitor_area[i]));
                b = MIN(b, RECT_BOTTOM(monitor_area[i]));
            }
        }
    }

    a = g_new(Rect, 1);
    a->x = l;
    a->y = t;
    a->width = r - l + 1;
    a->height = b - t + 1;
    return a;
}

guint screen_find_monitor(Rect *search)
{
    guint i;
    guint most = 0;
    guint mostv = 0;

    for (i = 0; i < screen_num_monitors; ++i) {
        Rect *area = screen_physical_area_monitor(i);
        if (RECT_INTERSECTS_RECT(*area, *search)) {
            Rect r;
            guint v;

            RECT_SET_INTERSECTION(r, *area, *search);
            v = r.width * r.height;

            if (v > mostv) {
                mostv = v;
                most = i;
            }
        }
        g_free(area);
    }
    return most;
}

Rect* screen_physical_area_all_monitors()
{
    return screen_physical_area_monitor(screen_num_monitors);
}

Rect* screen_physical_area_monitor(guint head)
{
    Rect *a;
    g_assert(head <= screen_num_monitors);

    a = g_new(Rect, 1);
    *a = monitor_area[head];
    return a;
}

gboolean screen_physical_area_monitor_contains(guint head, Rect *search)
{
    g_assert(head <= screen_num_monitors);
    g_assert(search);
    return RECT_INTERSECTS_RECT(monitor_area[head], *search);
}

Rect* screen_physical_area_active()
{
    Rect *a;
    gint x, y;

    if (focus_client)
        a = screen_physical_area_monitor(client_monitor(focus_client));
    else {
        Rect mon;
        if (screen_pointer_pos(&x, &y))
            RECT_SET(mon, x, y, 1, 1);
        else
            RECT_SET(mon, 0, 0, 1, 1);
        a = screen_physical_area_monitor(screen_find_monitor(&mon));
    }
    return a;
}

void screen_set_root_cursor()
{
    if (sn_app_starting())
        XDefineCursor(ob_display, RootWindow(ob_display, ob_screen),
                      ob_cursor(OB_CURSOR_BUSYPOINTER));
    else
        XDefineCursor(ob_display, RootWindow(ob_display, ob_screen),
                      ob_cursor(OB_CURSOR_POINTER));
}

gboolean screen_pointer_pos(gint *x, gint *y)
{
    Window w;
    gint i;
    guint u;
    gboolean ret;

    ret = !!XQueryPointer(ob_display, RootWindow(ob_display, ob_screen),
                          &w, &w, x, y, &i, &i, &u);
    if (!ret) {
        for (i = 0; i < ScreenCount(ob_display); ++i)
            if (i != ob_screen)
                if (XQueryPointer(ob_display, RootWindow(ob_display, i),
                                  &w, &w, x, y, &i, &i, &u))
                    break;
    }
    return ret;
}
