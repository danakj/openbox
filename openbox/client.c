/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   client.c for the Openbox window manager
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

#include "client.h"
#include "debug.h"
#include "startupnotify.h"
#include "dock.h"
#include "xerror.h"
#include "screen.h"
#include "moveresize.h"
#include "place.h"
#include "prop.h"
#include "extensions.h"
#include "frame.h"
#include "session.h"
#include "event.h"
#include "grab.h"
#include "focus.h"
#include "stacking.h"
#include "openbox.h"
#include "group.h"
#include "config.h"
#include "menuframe.h"
#include "keyboard.h"
#include "mouse.h"
#include "render/render.h"
#include "gettext.h"

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#ifdef HAVE_SIGNAL_H
#  include <signal.h> /* for kill() */
#endif

#include <glib.h>
#include <X11/Xutil.h>

/*! The event mask to grab on client windows */
#define CLIENT_EVENTMASK (PropertyChangeMask | StructureNotifyMask | \
                          ColormapChangeMask)

#define CLIENT_NOPROPAGATEMASK (ButtonPressMask | ButtonReleaseMask | \
                                ButtonMotionMask)

typedef struct
{
    ObClientCallback func;
    gpointer data;
} ClientCallback;

GList            *client_list          = NULL;

static GSList *client_destroy_notifies = NULL;

static void client_get_all(ObClient *self, gboolean real);
static void client_get_startup_id(ObClient *self);
static void client_get_session_ids(ObClient *self);
static void client_get_area(ObClient *self);
static void client_get_desktop(ObClient *self);
static void client_get_state(ObClient *self);
static void client_get_shaped(ObClient *self);
static void client_get_mwm_hints(ObClient *self);
static void client_get_colormap(ObClient *self);
static void client_change_allowed_actions(ObClient *self);
static void client_change_state(ObClient *self);
static void client_change_wm_state(ObClient *self);
static void client_apply_startup_state(ObClient *self,
                                       gint x, gint y, gint w, gint h);
static void client_restore_session_state(ObClient *self);
static gboolean client_restore_session_stacking(ObClient *self);
static ObAppSettings *client_get_settings_state(ObClient *self);
static void client_update_transient_tree(ObClient *self,
                                         ObGroup *oldgroup, ObGroup *newgroup,
                                         gboolean oldgtran, gboolean newgtran,
                                         ObClient* oldparent,
                                         ObClient *newparent);
static void client_present(ObClient *self, gboolean here, gboolean raise,
                           gboolean unshade);
static GSList *client_search_all_top_parents_internal(ObClient *self,
                                                      gboolean bylayer,
                                                      ObStackingLayer layer);
static void client_call_notifies(ObClient *self, GSList *list);
static void client_ping_event(ObClient *self, gboolean dead);


void client_startup(gboolean reconfig)
{
    if (reconfig) return;

    client_set_list();
}

void client_shutdown(gboolean reconfig)
{
    if (reconfig) return;
}

static void client_call_notifies(ObClient *self, GSList *list)
{
    GSList *it;

    for (it = list; it; it = g_slist_next(it)) {
        ClientCallback *d = it->data;
        d->func(self, d->data);
    }
}

void client_add_destroy_notify(ObClientCallback func, gpointer data)
{
    ClientCallback *d = g_new(ClientCallback, 1);
    d->func = func;
    d->data = data;
    client_destroy_notifies = g_slist_prepend(client_destroy_notifies, d);
}

void client_remove_destroy_notify(ObClientCallback func)
{
    GSList *it;

    for (it = client_destroy_notifies; it; it = g_slist_next(it)) {
        ClientCallback *d = it->data;
        if (d->func == func) {
            g_free(d);
            client_destroy_notifies =
                g_slist_delete_link(client_destroy_notifies, it);
            break;
        }
    }
}

void client_set_list(void)
{
    Window *windows, *win_it;
    GList *it;
    guint size = g_list_length(client_list);

    /* create an array of the window ids */
    if (size > 0) {
        windows = g_new(Window, size);
        win_it = windows;
        for (it = client_list; it; it = g_list_next(it), ++win_it)
            *win_it = ((ObClient*)it->data)->window;
    } else
        windows = NULL;

    PROP_SETA32(RootWindow(ob_display, ob_screen),
                net_client_list, window, (gulong*)windows, size);

    if (windows)
        g_free(windows);

    stacking_set_list();
}

void client_manage_all(void)
{
    guint i, j, nchild;
    Window w, *children;
    XWMHints *wmhints;
    XWindowAttributes attrib;

    XQueryTree(ob_display, RootWindow(ob_display, ob_screen),
               &w, &w, &children, &nchild);

    /* remove all icon windows from the list */
    for (i = 0; i < nchild; i++) {
        if (children[i] == None) continue;
        wmhints = XGetWMHints(ob_display, children[i]);
        if (wmhints) {
            if ((wmhints->flags & IconWindowHint) &&
                (wmhints->icon_window != children[i]))
                for (j = 0; j < nchild; j++)
                    if (children[j] == wmhints->icon_window) {
                        children[j] = None;
                        break;
                    }
            XFree(wmhints);
        }
    }

    for (i = 0; i < nchild; ++i) {
        if (children[i] == None)
            continue;
        if (XGetWindowAttributes(ob_display, children[i], &attrib)) {
            if (attrib.override_redirect) continue;

            if (attrib.map_state != IsUnmapped)
                client_manage(children[i]);
        }
    }
    XFree(children);
}

void client_manage(Window window)
{
    ObClient *self;
    XEvent e;
    XWindowAttributes attrib;
    XSetWindowAttributes attrib_set;
    XWMHints *wmhint;
    gboolean activate = FALSE;
    ObAppSettings *settings;
    gboolean transient = FALSE;
    Rect place, *monitor;
    Time launch_time, map_time;

    grab_server(TRUE);

    /* check if it has already been unmapped by the time we started
       mapping. the grab does a sync so we don't have to here */
    if (XCheckTypedWindowEvent(ob_display, window, DestroyNotify, &e) ||
        XCheckTypedWindowEvent(ob_display, window, UnmapNotify, &e))
    {
        XPutBackEvent(ob_display, &e);

        ob_debug("Trying to manage unmapped window. Aborting that.\n");
        grab_server(FALSE);
        return; /* don't manage it */
    }

    /* make sure it isn't an override-redirect window */
    if (!XGetWindowAttributes(ob_display, window, &attrib) ||
        attrib.override_redirect)
    {
        grab_server(FALSE);
        return; /* don't manage it */
    }

    /* is the window a docking app */
    if ((wmhint = XGetWMHints(ob_display, window))) {
        if ((wmhint->flags & StateHint) &&
            wmhint->initial_state == WithdrawnState)
        {
            dock_add(window, wmhint);
            grab_server(FALSE);
            XFree(wmhint);
            return;
        }
        XFree(wmhint);
    }

    ob_debug("Managing window: 0x%lx\n", window);

    map_time = event_get_server_time();

    /* choose the events we want to receive on the CLIENT window */
    attrib_set.event_mask = CLIENT_EVENTMASK;
    attrib_set.do_not_propagate_mask = CLIENT_NOPROPAGATEMASK;
    XChangeWindowAttributes(ob_display, window,
                            CWEventMask|CWDontPropagate, &attrib_set);

    /* create the ObClient struct, and populate it from the hints on the
       window */
    self = g_new0(ObClient, 1);
    self->obwin.type = Window_Client;
    self->window = window;

    /* non-zero defaults */
    self->wmstate = WithdrawnState; /* make sure it gets updated first time */
    self->gravity = NorthWestGravity;
    self->desktop = screen_num_desktops; /* always an invalid value */

    /* get all the stuff off the window */
    client_get_all(self, TRUE);

    ob_debug("Window type: %d\n", self->type);
    ob_debug("Window group: 0x%x\n", self->group?self->group->leader:0);

    /* specify that if we exit, the window should not be destroyed and
       should be reparented back to root automatically */
    XChangeSaveSet(ob_display, window, SetModeInsert);

    /* create the decoration frame for the client window */
    self->frame = frame_new(self);

    frame_grab_client(self->frame);

    /* we've grabbed everything and set everything that we need to at mapping
       time now */
    grab_server(FALSE);

    /* per-app settings override stuff from client_get_all, and return the
       settings for other uses too. the returned settings is a shallow copy,
       that needs to be freed with g_free(). */
    settings = client_get_settings_state(self);
    /* the session should get the last say though */
    client_restore_session_state(self);

    /* now we have all of the window's information so we can set this up */
    client_setup_decor_and_functions(self, FALSE);

    /* tell startup notification that this app started */
    launch_time = sn_app_started(self->startup_id, self->class);

    /* do this after we have a frame.. it uses the frame to help determine the
       WM_STATE to apply. */
    client_change_state(self);

    /* add ourselves to the focus order */
    focus_order_add_new(self);

    /* do this to add ourselves to the stacking list in a non-intrusive way */
    client_calc_layer(self);

    /* focus the new window? */
    if (ob_state() != OB_STATE_STARTING &&
        (!self->session || self->session->focused) &&
        /* this means focus=true for window is same as config_focus_new=true */
        ((config_focus_new || (settings && settings->focus == 1)) ||
         client_search_focus_tree_full(self)) &&
        /* this checks for focus=false for the window */
        (!settings || settings->focus != 0) &&
        focus_valid_target(self, FALSE, FALSE, TRUE, FALSE, FALSE))
    {
        activate = TRUE;
    }

    /* remove the client's border */
    XSetWindowBorderWidth(ob_display, self->window, 0);

    /* adjust the frame to the client's size before showing or placing
       the window */
    frame_adjust_area(self->frame, FALSE, TRUE, FALSE);
    frame_adjust_client_area(self->frame);

    /* where the frame was placed is where the window was originally */
    place = self->area;
    monitor = screen_physical_area_monitor(screen_find_monitor(&place));

    /* figure out placement for the window if the window is new */
    if (ob_state() == OB_STATE_RUNNING) {
        ob_debug("Positioned: %s @ %d %d\n",
                 (!self->positioned ? "no" :
                  (self->positioned == PPosition ? "program specified" :
                   (self->positioned == USPosition ? "user specified" :
                    (self->positioned == (PPosition | USPosition) ?
                     "program + user specified" :
                     "BADNESS !?")))), place.x, place.y);

        ob_debug("Sized: %s @ %d %d\n",
                 (!self->sized ? "no" :
                  (self->sized == PSize ? "program specified" :
                   (self->sized == USSize ? "user specified" :
                    (self->sized == (PSize | USSize) ?
                     "program + user specified" :
                     "BADNESS !?")))), place.width, place.height);

        /* splash screens are also returned as TRUE for transient,
           and so will be forced on screen below */
        transient = place_client(self, &place.x, &place.y, settings);

        /* make sure the window is visible. */
        client_find_onscreen(self, &place.x, &place.y,
                             place.width, place.height,
                             /* non-normal clients has less rules, and
                                windows that are being restored from a
                                session do also. we can assume you want
                                it back where you saved it. Clients saying
                                they placed themselves are subjected to
                                harder rules, ones that are placed by
                                place.c or by the user are allowed partially
                                off-screen and on xinerama divides (ie,
                                it is up to the placement routines to avoid
                                the xinerama divides)

                                splash screens get "transient" set to TRUE by
                                the place_client call
                             */
                             ob_state() == OB_STATE_RUNNING &&
                             (transient ||
                              (!((self->positioned & USPosition) ||
                                 (settings && settings->pos_given)) &&
                               client_normal(self) &&
                               !self->session &&
                               /* don't move oldschool fullscreen windows to
                                  fit inside the struts (fixes Acroread, which
                                  makes its fullscreen window fit the screen
                                  but it is not USSize'd or USPosition'd) */
                               !(self->decorations == 0 &&
                                 RECT_EQUAL(place, *monitor)))));
    }

    /* if the window isn't user-sized, then make it fit inside
       the visible screen area on its monitor. Use basically the same rules
       for forcing the window on screen in the client_find_onscreen call.

       do this after place_client, it chooses the monitor!

       splash screens get "transient" set to TRUE by
       the place_client call
    */
    if (ob_state() == OB_STATE_RUNNING &&
        (transient ||
         (!(self->sized & USSize || self->positioned & USPosition) &&
          client_normal(self) &&
          !self->session &&
          /* don't shrink oldschool fullscreen windows to fit inside the
             struts (fixes Acroread, which makes its fullscreen window
             fit the screen but it is not USSize'd or USPosition'd) */
          !(self->decorations == 0 && RECT_EQUAL(place, *monitor)))))
    {
        Rect *a = screen_area(self->desktop, SCREEN_AREA_ONE_MONITOR, &place);

        /* get the size of the frame */
        place.width += self->frame->size.left + self->frame->size.right;
        place.height += self->frame->size.top + self->frame->size.bottom;

        /* fit the window inside the area */
        place.width = MIN(place.width, a->width);
        place.height = MIN(place.height, a->height);

        ob_debug("setting window size to %dx%d\n", place.width, place.height);

        /* get the size of the client back */
        place.width -= self->frame->size.left + self->frame->size.right;
        place.height -= self->frame->size.top + self->frame->size.bottom;

        g_free(a);
    }

    ob_debug("placing window 0x%x at %d, %d with size %d x %d. "
             "some restrictions may apply\n",
             self->window, place.x, place.y, place.width, place.height);
    if (self->session)
        ob_debug("  but session requested %d, %d  %d x %d instead, "
                 "overriding\n",
                 self->session->x, self->session->y,
                 self->session->w, self->session->h);

    /* do this after the window is placed, so the premax/prefullscreen numbers
       won't be all wacko!!

       this also places the window
    */
    client_apply_startup_state(self, place.x, place.y,
                               place.width, place.height);

    g_free(monitor);
    monitor = NULL;

    ob_debug_type(OB_DEBUG_FOCUS, "Going to try activate new window? %s\n",
                  activate ? "yes" : "no");
    if (activate) {
        gboolean raise = FALSE;

        /* This is focus stealing prevention */
        ob_debug_type(OB_DEBUG_FOCUS,
                      "Want to focus new window 0x%x at time %u "
                      "launched at %u (last user interaction time %u)\n",
                      self->window, map_time, launch_time,
                      event_last_user_time);

        if (menu_frame_visible || moveresize_in_progress) {
            activate = FALSE;
            raise = TRUE;
            ob_debug_type(OB_DEBUG_FOCUS,
                          "Not focusing the window because the user is inside "
                          "an Openbox menu or is move/resizing a window and "
                          "we don't want to interrupt them\n");
        }

        /* if it's on another desktop */
        else if (!(self->desktop == screen_desktop ||
                   self->desktop == DESKTOP_ALL) &&
                 /* the timestamp is from before you changed desktops */
                 launch_time && screen_desktop_user_time &&
                 !event_time_after(launch_time, screen_desktop_user_time))
        {
            activate = FALSE;
            raise = TRUE;
            ob_debug_type(OB_DEBUG_FOCUS,
                          "Not focusing the window because its on another "
                          "desktop\n");
        }
        /* If something is focused, and it's not our relative... */
        else if (focus_client && client_search_focus_tree_full(self) == NULL &&
                 client_search_focus_group_full(self) == NULL)
        {
            /* If the user is working in another window right now, then don't
               steal focus */
            if (event_last_user_time && launch_time &&
                event_time_after(event_last_user_time, launch_time) &&
                event_last_user_time != launch_time &&
                event_time_after(event_last_user_time,
                                 map_time - OB_EVENT_USER_TIME_DELAY))
            {
                activate = FALSE;
                ob_debug_type(OB_DEBUG_FOCUS,
                              "Not focusing the window because the user is "
                              "working in another window\n");
            }
            /* If its a transient (and its parents aren't focused) */
            else if (client_has_parent(self)) {
                activate = FALSE;
                ob_debug_type(OB_DEBUG_FOCUS,
                              "Not focusing the window because it is a "
                              "transient, and its relatives aren't focused\n");
            }
            /* Don't steal focus from globally active clients.
               I stole this idea from KWin. It seems nice.
             */
            else if (!(focus_client->can_focus ||
                       focus_client->focus_notify))
            {
                activate = FALSE;
                ob_debug_type(OB_DEBUG_FOCUS,
                              "Not focusing the window because a globally "
                              "active client has focus\n");
            }
            /* Don't move focus if it's not going to go to this window
               anyway */
            else if (client_focus_target(self) != self) {
                activate = FALSE;
                raise = TRUE;
                ob_debug_type(OB_DEBUG_FOCUS,
                              "Not focusing the window because another window "
                              "would get the focus anyway\n");
            }
            else if (!(self->desktop == screen_desktop ||
                       self->desktop == DESKTOP_ALL))
            {
                activate = FALSE;
                raise = TRUE;
                ob_debug_type(OB_DEBUG_FOCUS,
                              "Not focusing the window because it is on "
                              "another desktop and no relatives are focused ");
            }
        }

        if (!activate) {
            ob_debug_type(OB_DEBUG_FOCUS,
                          "Focus stealing prevention activated for %s at "
                          "time %u (last user interactioon time %u)\n",
                          self->title, map_time, event_last_user_time);
            /* if the client isn't focused, then hilite it so the user
               knows it is there */
            client_hilite(self, TRUE);
            /* we may want to raise it even tho we're not activating it */
            if (raise && !client_restore_session_stacking(self))
                stacking_raise(CLIENT_AS_WINDOW(self));
        }
    }
    else {
        /* This may look rather odd. Well it's because new windows are added
           to the stacking order non-intrusively. If we're not going to focus
           the new window or hilite it, then we raise it to the top. This will
           take affect for things that don't get focused like splash screens.
           Also if you don't have focus_new enabled, then it's going to get
           raised to the top. Legacy begets legacy I guess?
        */
        if (!client_restore_session_stacking(self))
            stacking_raise(CLIENT_AS_WINDOW(self));
    }

    mouse_grab_for_client(self, TRUE);

    /* this has to happen before we try focus the window, but we want it to
       happen after the client's stacking has been determined or it looks bad
    */
    {
        gulong ignore_start;
        if (!config_focus_under_mouse)
            ignore_start = event_start_ignore_all_enters();

        client_show(self);

        if (!config_focus_under_mouse)
            event_end_ignore_all_enters(ignore_start);
    }

    if (activate) {
        gboolean stacked = client_restore_session_stacking(self);
        client_present(self, FALSE, !stacked, TRUE);
    }

    /* add to client list/map */
    client_list = g_list_append(client_list, self);
    g_hash_table_insert(window_map, &self->window, self);

    /* this has to happen after we're in the client_list */
    if (STRUT_EXISTS(self->strut))
        screen_update_areas();

    /* update the list hints */
    client_set_list();

    /* free the ObAppSettings shallow copy */
    g_free(settings);

    ob_debug("Managed window 0x%lx plate 0x%x (%s)\n",
             window, self->frame->window, self->class);

    return;
}


ObClient *client_fake_manage(Window window)
{
    ObClient *self;
    ObAppSettings *settings;

    ob_debug("Pretend-managing window: %lx\n", window);

    /* do this minimal stuff to figure out the client's decorations */

    self = g_new0(ObClient, 1);
    self->window = window;

    client_get_all(self, FALSE);
    /* per-app settings override stuff, and return the settings for other
       uses too. this returns a shallow copy that needs to be freed */
    settings = client_get_settings_state(self);

    client_setup_decor_and_functions(self, FALSE);

    /* create the decoration frame for the client window and adjust its size */
    self->frame = frame_new(self);
    frame_adjust_area(self->frame, FALSE, TRUE, TRUE);

    ob_debug("gave extents left %d right %d top %d bottom %d\n",
             self->frame->size.left, self->frame->size.right,
             self->frame->size.top, self->frame->size.bottom);

    /* free the ObAppSettings shallow copy */
    g_free(settings);

    return self;
}

void client_unmanage_all(void)
{
    while (client_list != NULL)
        client_unmanage(client_list->data);
}

void client_unmanage(ObClient *self)
{
    guint j;
    GSList *it;
    gulong ignore_start;

    ob_debug("Unmanaging window: 0x%x plate 0x%x (%s) (%s)\n",
             self->window, self->frame->window,
             self->class, self->title ? self->title : "");

    g_assert(self != NULL);

    /* we dont want events no more. do this before hiding the frame so we
       don't generate more events */
    XSelectInput(ob_display, self->window, NoEventMask);

    /* ignore enter events from the unmap so it doesnt mess with the focus */
    if (!config_focus_under_mouse)
        ignore_start = event_start_ignore_all_enters();

    frame_hide(self->frame);
    /* flush to send the hide to the server quickly */
    XFlush(ob_display);

    if (!config_focus_under_mouse)
        event_end_ignore_all_enters(ignore_start);

    mouse_grab_for_client(self, FALSE);

    /* remove the window from our save set */
    XChangeSaveSet(ob_display, self->window, SetModeDelete);

    /* update the focus lists */
    focus_order_remove(self);
    if (client_focused(self)) {
        /* don't leave an invalid focus_client */
        focus_client = NULL;
    }

    client_list = g_list_remove(client_list, self);
    stacking_remove(self);
    g_hash_table_remove(window_map, &self->window);

    /* once the client is out of the list, update the struts to remove its
       influence */
    if (STRUT_EXISTS(self->strut))
        screen_update_areas();

    client_call_notifies(self, client_destroy_notifies);

    /* tell our parent(s) that we're gone */
    for (it = self->parents; it; it = g_slist_next(it))
        ((ObClient*)it->data)->transients =
            g_slist_remove(((ObClient*)it->data)->transients,self);

    /* tell our transients that we're gone */
    for (it = self->transients; it; it = g_slist_next(it)) {
        ((ObClient*)it->data)->parents =
            g_slist_remove(((ObClient*)it->data)->parents, self);
        /* we could be keeping our children in a higher layer */
        client_calc_layer(it->data);
    }

    /* remove from its group */
    if (self->group) {
        group_remove(self->group, self);
        self->group = NULL;
    }

    /* restore the window's original geometry so it is not lost */
    {
        Rect a;

        a = self->area;

        if (self->fullscreen)
            a = self->pre_fullscreen_area;
        else if (self->max_horz || self->max_vert) {
            if (self->max_horz) {
                a.x = self->pre_max_area.x;
                a.width = self->pre_max_area.width;
            }
            if (self->max_vert) {
                a.y = self->pre_max_area.y;
                a.height = self->pre_max_area.height;
            }
        }

        self->fullscreen = self->max_horz = self->max_vert = FALSE;
        /* let it be moved and resized no matter what */
        self->functions = OB_CLIENT_FUNC_MOVE | OB_CLIENT_FUNC_RESIZE;
        self->decorations = 0; /* unmanaged windows have no decor */

        /* give the client its border back */
        XSetWindowBorderWidth(ob_display, self->window, self->border_width);

        client_move_resize(self, a.x, a.y, a.width, a.height);
    }

    /* reparent the window out of the frame, and free the frame */
    frame_release_client(self->frame);
    frame_free(self->frame);
    self->frame = NULL;

    if (ob_state() != OB_STATE_EXITING) {
        /* these values should not be persisted across a window
           unmapping/mapping */
        PROP_ERASE(self->window, net_wm_desktop);
        PROP_ERASE(self->window, net_wm_state);
        PROP_ERASE(self->window, wm_state);
    } else {
        /* if we're left in an unmapped state, the client wont be mapped.
           this is bad, since we will no longer be managing the window on
           restart */
        XMapWindow(ob_display, self->window);
    }

    /* these should not be left on the window ever.  other window managers
       don't necessarily use them and it will mess them up (like compiz) */
    PROP_ERASE(self->window, net_wm_visible_name);
    PROP_ERASE(self->window, net_wm_visible_icon_name);

    /* update the list hints */
    client_set_list();

    ob_debug("Unmanaged window 0x%lx\n", self->window);

    /* free all data allocated in the client struct */
    g_slist_free(self->transients);
    for (j = 0; j < self->nicons; ++j)
        g_free(self->icons[j].data);
    if (self->nicons > 0)
        g_free(self->icons);
    g_free(self->wm_command);
    g_free(self->title);
    g_free(self->icon_title);
    g_free(self->name);
    g_free(self->class);
    g_free(self->role);
    g_free(self->client_machine);
    g_free(self->sm_client_id);
    g_free(self);
}

void client_fake_unmanage(ObClient *self)
{
    /* this is all that got allocated to get the decorations */

    frame_free(self->frame);
    g_free(self);
}

/*! Returns a new structure containing the per-app settings for this client.
  The returned structure needs to be freed with g_free. */
static ObAppSettings *client_get_settings_state(ObClient *self)
{
    ObAppSettings *settings;
    GSList *it;

    settings = config_create_app_settings();

    for (it = config_per_app_settings; it; it = g_slist_next(it)) {
        ObAppSettings *app = it->data;
        gboolean match = TRUE;

        g_assert(app->name != NULL || app->class != NULL);

        /* we know that either name or class is not NULL so it will have to
           match to use the rule */
        if (app->name &&
            !g_pattern_match(app->name, strlen(self->name), self->name, NULL))
            match = FALSE;
        else if (app->class &&
                !g_pattern_match(app->class,
                                 strlen(self->class), self->class, NULL))
            match = FALSE;
        else if (app->role &&
                 !g_pattern_match(app->role,
                                  strlen(self->role), self->role, NULL))
            match = FALSE;

        if (match) {
            ob_debug("Window matching: %s\n", app->name);

            /* copy the settings to our struct, overriding the existing
               settings if they are not defaults */
            config_app_settings_copy_non_defaults(app, settings);
        }
    }

    if (settings->shade != -1)
        self->shaded = !!settings->shade;
    if (settings->decor != -1)
        self->undecorated = !settings->decor;
    if (settings->iconic != -1)
        self->iconic = !!settings->iconic;
    if (settings->skip_pager != -1)
        self->skip_pager = !!settings->skip_pager;
    if (settings->skip_taskbar != -1)
        self->skip_taskbar = !!settings->skip_taskbar;

    if (settings->max_vert != -1)
        self->max_vert = !!settings->max_vert;
    if (settings->max_horz != -1)
        self->max_horz = !!settings->max_horz;

    if (settings->fullscreen != -1)
        self->fullscreen = !!settings->fullscreen;

    if (settings->desktop) {
        if (settings->desktop == DESKTOP_ALL)
            self->desktop = settings->desktop;
        else if (settings->desktop > 0 &&
                 settings->desktop <= screen_num_desktops)
            self->desktop = settings->desktop - 1;
    }

    if (settings->layer == -1) {
        self->below = TRUE;
        self->above = FALSE;
    }
    else if (settings->layer == 0) {
        self->below = FALSE;
        self->above = FALSE;
    }
    else if (settings->layer == 1) {
        self->below = FALSE;
        self->above = TRUE;
    }
    return settings;
}

static void client_restore_session_state(ObClient *self)
{
    GList *it;

    ob_debug_type(OB_DEBUG_SM,
                  "Restore session for client %s\n", self->title);

    if (!(it = session_state_find(self))) {
        ob_debug_type(OB_DEBUG_SM,
                      "Session data not found for client %s\n", self->title);
        return;
    }

    self->session = it->data;

    ob_debug_type(OB_DEBUG_SM, "Session data loaded for client %s\n",
                  self->title);

    RECT_SET_POINT(self->area, self->session->x, self->session->y);
    self->positioned = USPosition;
    self->sized = USSize;
    if (self->session->w > 0)
        self->area.width = self->session->w;
    if (self->session->h > 0)
        self->area.height = self->session->h;
    XResizeWindow(ob_display, self->window,
                  self->area.width, self->area.height);

    self->desktop = (self->session->desktop == DESKTOP_ALL ?
                     self->session->desktop :
                     MIN(screen_num_desktops - 1, self->session->desktop));
    PROP_SET32(self->window, net_wm_desktop, cardinal, self->desktop);

    self->shaded = self->session->shaded;
    self->iconic = self->session->iconic;
    self->skip_pager = self->session->skip_pager;
    self->skip_taskbar = self->session->skip_taskbar;
    self->fullscreen = self->session->fullscreen;
    self->above = self->session->above;
    self->below = self->session->below;
    self->max_horz = self->session->max_horz;
    self->max_vert = self->session->max_vert;
    self->undecorated = self->session->undecorated;
}

static gboolean client_restore_session_stacking(ObClient *self)
{
    GList *it, *mypos;

    if (!self->session) return FALSE;

    mypos = g_list_find(session_saved_state, self->session);
    if (!mypos) return FALSE;

    /* start above me and look for the first client */
    for (it = g_list_previous(mypos); it; it = g_list_previous(it)) {
        GList *cit;

        for (cit = client_list; cit; cit = g_list_next(cit)) {
            ObClient *c = cit->data;
            /* found a client that was in the session, so go below it */
            if (c->session == it->data) {
                stacking_below(CLIENT_AS_WINDOW(self),
                               CLIENT_AS_WINDOW(cit->data));
                return TRUE;
            }
        }
    }
    return FALSE;
}

void client_move_onscreen(ObClient *self, gboolean rude)
{
    gint x = self->area.x;
    gint y = self->area.y;
    if (client_find_onscreen(self, &x, &y,
                             self->area.width,
                             self->area.height, rude)) {
        client_move(self, x, y);
    }
}

gboolean client_find_onscreen(ObClient *self, gint *x, gint *y, gint w, gint h,
                              gboolean rude)
{
    gint ox = *x, oy = *y;
    gboolean rudel = rude, ruder = rude, rudet = rude, rudeb = rude;
    gint fw, fh;
    Rect desired;
    guint i;

    RECT_SET(desired, *x, *y, w, h);
    frame_rect_to_frame(self->frame, &desired);

    /* get where the frame would be */
    frame_client_gravity(self->frame, x, y);

    /* get the requested size of the window with decorations */
    fw = self->frame->size.left + w + self->frame->size.right;
    fh = self->frame->size.top + h + self->frame->size.bottom;

    /* If rudeness wasn't requested, then still be rude in a given direction
       if the client is not moving, only resizing in that direction */
    if (!rude) {
        Point oldtl, oldtr, oldbl, oldbr;
        Point newtl, newtr, newbl, newbr;
        gboolean stationary_l, stationary_r, stationary_t, stationary_b;

        POINT_SET(oldtl, self->frame->area.x, self->frame->area.y);
        POINT_SET(oldbr, self->frame->area.x + self->frame->area.width - 1,
                  self->frame->area.y + self->frame->area.height - 1);
        POINT_SET(oldtr, oldbr.x, oldtl.y);
        POINT_SET(oldbl, oldtl.x, oldbr.y);

        POINT_SET(newtl, *x, *y);
        POINT_SET(newbr, *x + fw - 1, *y + fh - 1);
        POINT_SET(newtr, newbr.x, newtl.y);
        POINT_SET(newbl, newtl.x, newbr.y);

        /* is it moving or just resizing from some corner? */
        stationary_l = oldtl.x == newtl.x;
        stationary_r = oldtr.x == newtr.x;
        stationary_t = oldtl.y == newtl.y;
        stationary_b = oldbl.y == newbl.y;

        /* if left edge is growing and didnt move right edge */
        if (stationary_r && newtl.x < oldtl.x)
            rudel = TRUE;
        /* if right edge is growing and didnt move left edge */
        if (stationary_l && newtr.x > oldtr.x)
            ruder = TRUE;
        /* if top edge is growing and didnt move bottom edge */
        if (stationary_b && newtl.y < oldtl.y)
            rudet = TRUE;
        /* if bottom edge is growing and didnt move top edge */
        if (stationary_t && newbl.y > oldbl.y)
            rudeb = TRUE;
    }

    for (i = 0; i < screen_num_monitors; ++i) {
        Rect *a;

        if (!screen_physical_area_monitor_contains(i, &desired)) {
            if (i < screen_num_monitors - 1)
                continue;

            /* the window is not inside any monitor! so just use the first
               one */
            a = screen_area(self->desktop, 0, NULL);
        } else
            a = screen_area(self->desktop, SCREEN_AREA_ONE_MONITOR, &desired);

        /* This makes sure windows aren't entirely outside of the screen so you
           can't see them at all.
           It makes sure 10% of the window is on the screen at least. At don't
           let it move itself off the top of the screen, which would hide the
           titlebar on you. (The user can still do this if they want too, it's
           only limiting the application.
        */
        if (client_normal(self)) {
            if (!self->strut.right && *x + fw/10 >= a->x + a->width - 1)
                *x = a->x + a->width - fw/10;
            if (!self->strut.bottom && *y + fh/10 >= a->y + a->height - 1)
                *y = a->y + a->height - fh/10;
            if (!self->strut.left && *x + fw*9/10 - 1 < a->x)
                *x = a->x - fw*9/10;
            if (!self->strut.top && *y + fh*9/10 - 1 < a->y)
                *y = a->y - fh*9/10;
        }

        /* This here doesn't let windows even a pixel outside the
           struts/screen. When called from client_manage, programs placing
           themselves are forced completely onscreen, while things like
           xterm -geometry resolution-width/2 will work fine. Trying to
           place it completely offscreen will be handled in the above code.
           Sorry for this confused comment, i am tired. */
        if (rudel && !self->strut.left && *x < a->x) *x = a->x;
        if (ruder && !self->strut.right && *x + fw > a->x + a->width)
            *x = a->x + MAX(0, a->width - fw);

        if (rudet && !self->strut.top && *y < a->y) *y = a->y;
        if (rudeb && !self->strut.bottom && *y + fh > a->y + a->height)
            *y = a->y + MAX(0, a->height - fh);

        g_free(a);
    }

    /* get where the client should be */
    frame_frame_gravity(self->frame, x, y);

    return ox != *x || oy != *y;
}

static void client_get_all(ObClient *self, gboolean real)
{
    /* this is needed for the frame to set itself up */
    client_get_area(self);

    /* these things can change the decor and functions of the window */

    client_get_mwm_hints(self);
    /* this can change the mwmhints for special cases */
    client_get_type_and_transientness(self);
    client_get_state(self);
    client_update_normal_hints(self);

    /* get the session related properties, these can change decorations
       from per-app settings */
    client_get_session_ids(self);

    /* now we got everything that can affect the decorations */
    if (!real)
        return;

    /* get this early so we have it for debugging */
    client_update_title(self);

    client_update_protocols(self);

    client_update_wmhints(self);
    /* this may have already been called from client_update_wmhints */
    if (!self->parents && !self->transient_for_group)
        client_update_transient_for(self);

    client_get_startup_id(self);
    client_get_desktop(self);/* uses transient data/group/startup id if a
                                desktop is not specified */
    client_get_shaped(self);

    {
        /* a couple type-based defaults for new windows */

        /* this makes sure that these windows appear on all desktops */
        if (self->type == OB_CLIENT_TYPE_DESKTOP)
            self->desktop = DESKTOP_ALL;
    }

#ifdef SYNC
    client_update_sync_request_counter(self);
#endif

    client_get_colormap(self);
    client_update_strut(self);
    client_update_icons(self);
    client_update_icon_geometry(self);
}

static void client_get_startup_id(ObClient *self)
{
    if (!(PROP_GETS(self->window, net_startup_id, utf8, &self->startup_id)))
        if (self->group)
            PROP_GETS(self->group->leader,
                      net_startup_id, utf8, &self->startup_id);
}

static void client_get_area(ObClient *self)
{
    XWindowAttributes wattrib;
    Status ret;

    ret = XGetWindowAttributes(ob_display, self->window, &wattrib);
    g_assert(ret != BadWindow);

    RECT_SET(self->area, wattrib.x, wattrib.y, wattrib.width, wattrib.height);
    POINT_SET(self->root_pos, wattrib.x, wattrib.y);
    self->border_width = wattrib.border_width;

    ob_debug("client area: %d %d  %d %d  bw %d\n", wattrib.x, wattrib.y,
             wattrib.width, wattrib.height, wattrib.border_width);
}

static void client_get_desktop(ObClient *self)
{
    guint32 d = screen_num_desktops; /* an always-invalid value */

    if (PROP_GET32(self->window, net_wm_desktop, cardinal, &d)) {
        if (d >= screen_num_desktops && d != DESKTOP_ALL)
            self->desktop = screen_num_desktops - 1;
        else
            self->desktop = d;
        ob_debug("client requested desktop 0x%x\n", self->desktop);
    } else {
        GSList *it;
        gboolean first = TRUE;
        guint all = screen_num_desktops; /* not a valid value */

        /* if they are all on one desktop, then open it on the
           same desktop */
        for (it = self->parents; it; it = g_slist_next(it)) {
            ObClient *c = it->data;

            if (c->desktop == DESKTOP_ALL) continue;

            if (first) {
                all = c->desktop;
                first = FALSE;
            }
            else if (all != c->desktop)
                all = screen_num_desktops; /* make it invalid */
        }
        if (all != screen_num_desktops) {
            self->desktop = all;

            ob_debug("client desktop set from parents: 0x%x\n",
                     self->desktop);
        }
        /* try get from the startup-notification protocol */
        else if (sn_get_desktop(self->startup_id, &self->desktop)) {
            if (self->desktop >= screen_num_desktops &&
                self->desktop != DESKTOP_ALL)
                self->desktop = screen_num_desktops - 1;
            ob_debug("client desktop set from startup-notification: 0x%x\n",
                     self->desktop);
        }
        /* defaults to the current desktop */
        else {
            self->desktop = screen_desktop;
            ob_debug("client desktop set to the current desktop: %d\n",
                     self->desktop);
        }
    }
}

static void client_get_state(ObClient *self)
{
    guint32 *state;
    guint num;

    if (PROP_GETA32(self->window, net_wm_state, atom, &state, &num)) {
        gulong i;
        for (i = 0; i < num; ++i) {
            if (state[i] == prop_atoms.net_wm_state_modal)
                self->modal = TRUE;
            else if (state[i] == prop_atoms.net_wm_state_shaded)
                self->shaded = TRUE;
            else if (state[i] == prop_atoms.net_wm_state_hidden)
                self->iconic = TRUE;
            else if (state[i] == prop_atoms.net_wm_state_skip_taskbar)
                self->skip_taskbar = TRUE;
            else if (state[i] == prop_atoms.net_wm_state_skip_pager)
                self->skip_pager = TRUE;
            else if (state[i] == prop_atoms.net_wm_state_fullscreen)
                self->fullscreen = TRUE;
            else if (state[i] == prop_atoms.net_wm_state_maximized_vert)
                self->max_vert = TRUE;
            else if (state[i] == prop_atoms.net_wm_state_maximized_horz)
                self->max_horz = TRUE;
            else if (state[i] == prop_atoms.net_wm_state_above)
                self->above = TRUE;
            else if (state[i] == prop_atoms.net_wm_state_below)
                self->below = TRUE;
            else if (state[i] == prop_atoms.net_wm_state_demands_attention)
                self->demands_attention = TRUE;
            else if (state[i] == prop_atoms.ob_wm_state_undecorated)
                self->undecorated = TRUE;
        }

        g_free(state);
    }
}

static void client_get_shaped(ObClient *self)
{
    self->shaped = FALSE;
#ifdef   SHAPE
    if (extensions_shape) {
        gint foo;
        guint ufoo;
        gint s;

        XShapeSelectInput(ob_display, self->window, ShapeNotifyMask);

        XShapeQueryExtents(ob_display, self->window, &s, &foo,
                           &foo, &ufoo, &ufoo, &foo, &foo, &foo, &ufoo,
                           &ufoo);
        self->shaped = (s != 0);
    }
#endif
}

void client_update_transient_for(ObClient *self)
{
    Window t = None;
    ObClient *target = NULL;
    gboolean trangroup = FALSE;

    if (XGetTransientForHint(ob_display, self->window, &t)) {
        if (t != self->window) { /* cant be transient to itself! */
            target = g_hash_table_lookup(window_map, &t);
            /* if this happens then we need to check for it*/
            g_assert(target != self);
            if (target && !WINDOW_IS_CLIENT(target)) {
                /* this can happen when a dialog is a child of
                   a dockapp, for example */
                target = NULL;
            }
        }

        /* Setting the transient_for to Root is actually illegal, however
           applications from time have done this to specify transient for
           their group */
        if (!target && self->group && t == RootWindow(ob_display, ob_screen))
            trangroup = TRUE;
    } else if (self->group && self->transient)
        trangroup = TRUE;

    client_update_transient_tree(self, self->group, self->group,
                                 self->transient_for_group, trangroup,
                                 client_direct_parent(self), target);
    self->transient_for_group = trangroup;

}

static void client_update_transient_tree(ObClient *self,
                                         ObGroup *oldgroup, ObGroup *newgroup,
                                         gboolean oldgtran, gboolean newgtran,
                                         ObClient* oldparent,
                                         ObClient *newparent)
{
    GSList *it, *next;
    ObClient *c;

    g_assert(!oldgtran || oldgroup);
    g_assert(!newgtran || newgroup);
    g_assert((!oldgtran && !oldparent) ||
             (oldgtran && !oldparent) ||
             (!oldgtran && oldparent));
    g_assert((!newgtran && !newparent) ||
             (newgtran && !newparent) ||
             (!newgtran && newparent));

    /* * *
      Group transient windows are not allowed to have other group
      transient windows as their children.
      * * */


    /* No change has occured */
    if (oldgroup == newgroup &&
        oldgtran == newgtran &&
        oldparent == newparent) return;

    /** Remove the client from the transient tree **/

    for (it = self->transients; it; it = next) {
        next = g_slist_next(it);
        c = it->data;
        self->transients = g_slist_delete_link(self->transients, it);
        c->parents = g_slist_remove(c->parents, self);
    }
    for (it = self->parents; it; it = next) {
        next = g_slist_next(it);
        c = it->data;
        self->parents = g_slist_delete_link(self->parents, it);
        c->transients = g_slist_remove(c->transients, self);
    }

    /** Re-add the client to the transient tree **/

    /* If we're transient for a group then we need to add ourselves to all our
       parents */
    if (newgtran) {
        for (it = newgroup->members; it; it = g_slist_next(it)) {
            c = it->data;
            if (c != self &&
                !client_search_top_direct_parent(c)->transient_for_group &&
                client_normal(c))
            {
                c->transients = g_slist_prepend(c->transients, self);
                self->parents = g_slist_prepend(self->parents, c);
            }
        }
    }

    /* If we are now transient for a single window we need to add ourselves to
       its children

       WARNING: Cyclical transient ness is possible if two windows are
       transient for eachother.
    */
    else if (newparent &&
             /* don't make ourself its child if it is already our child */
             !client_is_direct_child(self, newparent) &&
             client_normal(newparent))
    {
        newparent->transients = g_slist_prepend(newparent->transients, self);
        self->parents = g_slist_prepend(self->parents, newparent);
    }

    /* Add any group transient windows to our children. But if we're transient
       for the group, then other group transients are not our children.

       WARNING: Cyclical transient-ness is possible. For e.g. if:
       A is transient for the group
       B is transient for A
       C is transient for B
       A can't be transient for C or we have a cycle
    */
    if (!newgtran && newgroup &&
        (!newparent ||
         !client_search_top_direct_parent(newparent)->transient_for_group) &&
        client_normal(self))
    {
        for (it = newgroup->members; it; it = g_slist_next(it)) {
            c = it->data;
            if (c != self && c->transient_for_group &&
                /* Don't make it our child if it is already our parent */
                !client_is_direct_child(c, self))
            {
                self->transients = g_slist_prepend(self->transients, c);
                c->parents = g_slist_prepend(c->parents, self);
            }
        }
    }

    /** If we change our group transient-ness, our children change their
        effect group transient-ness, which affects how they relate to other
        group windows **/

    for (it = self->transients; it; it = g_slist_next(it)) {
        c = it->data;
        if (!c->transient_for_group)
            client_update_transient_tree(c, c->group, c->group,
                                         c->transient_for_group,
                                         c->transient_for_group,
                                         client_direct_parent(c),
                                         client_direct_parent(c));
    }
}

static void client_get_mwm_hints(ObClient *self)
{
    guint num;
    guint32 *hints;

    self->mwmhints.flags = 0; /* default to none */

    if (PROP_GETA32(self->window, motif_wm_hints, motif_wm_hints,
                    &hints, &num)) {
        if (num >= OB_MWM_ELEMENTS) {
            self->mwmhints.flags = hints[0];
            self->mwmhints.functions = hints[1];
            self->mwmhints.decorations = hints[2];
        }
        g_free(hints);
    }
}

void client_get_type_and_transientness(ObClient *self)
{
    guint num, i;
    guint32 *val;
    Window t;

    self->type = -1;
    self->transient = FALSE;

    if (PROP_GETA32(self->window, net_wm_window_type, atom, &val, &num)) {
        /* use the first value that we know about in the array */
        for (i = 0; i < num; ++i) {
            if (val[i] == prop_atoms.net_wm_window_type_desktop)
                self->type = OB_CLIENT_TYPE_DESKTOP;
            else if (val[i] == prop_atoms.net_wm_window_type_dock)
                self->type = OB_CLIENT_TYPE_DOCK;
            else if (val[i] == prop_atoms.net_wm_window_type_toolbar)
                self->type = OB_CLIENT_TYPE_TOOLBAR;
            else if (val[i] == prop_atoms.net_wm_window_type_menu)
                self->type = OB_CLIENT_TYPE_MENU;
            else if (val[i] == prop_atoms.net_wm_window_type_utility)
                self->type = OB_CLIENT_TYPE_UTILITY;
            else if (val[i] == prop_atoms.net_wm_window_type_splash)
                self->type = OB_CLIENT_TYPE_SPLASH;
            else if (val[i] == prop_atoms.net_wm_window_type_dialog)
                self->type = OB_CLIENT_TYPE_DIALOG;
            else if (val[i] == prop_atoms.net_wm_window_type_normal)
                self->type = OB_CLIENT_TYPE_NORMAL;
            else if (val[i] == prop_atoms.kde_net_wm_window_type_override) {
                /* prevent this window from getting any decor or
                   functionality */
                self->mwmhints.flags &= (OB_MWM_FLAG_FUNCTIONS |
                                         OB_MWM_FLAG_DECORATIONS);
                self->mwmhints.decorations = 0;
                self->mwmhints.functions = 0;
            }
            if (self->type != (ObClientType) -1)
                break; /* grab the first legit type */
        }
        g_free(val);
    }

    if (XGetTransientForHint(ob_display, self->window, &t))
        self->transient = TRUE;

    if (self->type == (ObClientType) -1) {
        /*the window type hint was not set, which means we either classify
          ourself as a normal window or a dialog, depending on if we are a
          transient. */
        if (self->transient)
            self->type = OB_CLIENT_TYPE_DIALOG;
        else
            self->type = OB_CLIENT_TYPE_NORMAL;
    }

    /* then, based on our type, we can update our transientness.. */
    if (self->type == OB_CLIENT_TYPE_DIALOG ||
        self->type == OB_CLIENT_TYPE_TOOLBAR ||
        self->type == OB_CLIENT_TYPE_MENU ||
        self->type == OB_CLIENT_TYPE_UTILITY)
    {
        self->transient = TRUE;
    }
}

void client_update_protocols(ObClient *self)
{
    guint32 *proto;
    guint num_return, i;

    self->focus_notify = FALSE;
    self->delete_window = FALSE;

    if (PROP_GETA32(self->window, wm_protocols, atom, &proto, &num_return)) {
        for (i = 0; i < num_return; ++i) {
            if (proto[i] == prop_atoms.wm_delete_window)
                /* this means we can request the window to close */
                self->delete_window = TRUE;
            else if (proto[i] == prop_atoms.wm_take_focus)
                /* if this protocol is requested, then the window will be
                   notified whenever we want it to receive focus */
                self->focus_notify = TRUE;
            else if (proto[i] == prop_atoms.net_wm_ping)
                /* if this protocol is requested, then the window will allow
                   pings to determine if it is still alive */
                self->ping = TRUE;
#ifdef SYNC
            else if (proto[i] == prop_atoms.net_wm_sync_request)
                /* if this protocol is requested, then resizing the
                   window will be synchronized between the frame and the
                   client */
                self->sync_request = TRUE;
#endif
        }
        g_free(proto);
    }
}

#ifdef SYNC
void client_update_sync_request_counter(ObClient *self)
{
    guint32 i;

    if (PROP_GET32(self->window, net_wm_sync_request_counter, cardinal, &i)) {
        self->sync_counter = i;
    } else
        self->sync_counter = None;
}
#endif

void client_get_colormap(ObClient *self)
{
    XWindowAttributes wa;

    if (XGetWindowAttributes(ob_display, self->window, &wa))
        client_update_colormap(self, wa.colormap);
}

void client_update_colormap(ObClient *self, Colormap colormap)
{
    if (colormap == self->colormap) return;

    ob_debug("Setting client %s colormap: 0x%x\n", self->title, colormap);

    if (client_focused(self)) {
        screen_install_colormap(self, FALSE); /* uninstall old one */
        self->colormap = colormap;
        screen_install_colormap(self, FALSE); /* install new one */
    } else
        self->colormap = colormap;
}

void client_update_normal_hints(ObClient *self)
{
    XSizeHints size;
    glong ret;

    /* defaults */
    self->min_ratio = 0.0f;
    self->max_ratio = 0.0f;
    SIZE_SET(self->size_inc, 1, 1);
    SIZE_SET(self->base_size, 0, 0);
    SIZE_SET(self->min_size, 0, 0);
    SIZE_SET(self->max_size, G_MAXINT, G_MAXINT);

    /* get the hints from the window */
    if (XGetWMNormalHints(ob_display, self->window, &size, &ret)) {
        /* normal windows can't request placement! har har
        if (!client_normal(self))
        */
        self->positioned = (size.flags & (PPosition|USPosition));
        self->sized = (size.flags & (PSize|USSize));

        if (size.flags & PWinGravity)
            self->gravity = size.win_gravity;

        if (size.flags & PAspect) {
            if (size.min_aspect.y)
                self->min_ratio =
                    (gfloat) size.min_aspect.x / size.min_aspect.y;
            if (size.max_aspect.y)
                self->max_ratio =
                    (gfloat) size.max_aspect.x / size.max_aspect.y;
        }

        if (size.flags & PMinSize)
            SIZE_SET(self->min_size, size.min_width, size.min_height);

        if (size.flags & PMaxSize)
            SIZE_SET(self->max_size, size.max_width, size.max_height);

        if (size.flags & PBaseSize)
            SIZE_SET(self->base_size, size.base_width, size.base_height);

        if (size.flags & PResizeInc && size.width_inc && size.height_inc)
            SIZE_SET(self->size_inc, size.width_inc, size.height_inc);

        ob_debug("Normal hints: min size (%d %d) max size (%d %d)\n   "
                 "size inc (%d %d) base size (%d %d)\n",
                 self->min_size.width, self->min_size.height,
                 self->max_size.width, self->max_size.height,
                 self->size_inc.width, self->size_inc.height,
                 self->base_size.width, self->base_size.height);
    }
    else
        ob_debug("Normal hints: not set\n");
}

void client_setup_decor_and_functions(ObClient *self, gboolean reconfig)
{
    /* start with everything (cept fullscreen) */
    self->decorations =
        (OB_FRAME_DECOR_TITLEBAR |
         OB_FRAME_DECOR_HANDLE |
         OB_FRAME_DECOR_GRIPS |
         OB_FRAME_DECOR_BORDER |
         OB_FRAME_DECOR_ICON |
         OB_FRAME_DECOR_ALLDESKTOPS |
         OB_FRAME_DECOR_ICONIFY |
         OB_FRAME_DECOR_MAXIMIZE |
         OB_FRAME_DECOR_SHADE |
         OB_FRAME_DECOR_CLOSE);
    self->functions =
        (OB_CLIENT_FUNC_RESIZE |
         OB_CLIENT_FUNC_MOVE |
         OB_CLIENT_FUNC_ICONIFY |
         OB_CLIENT_FUNC_MAXIMIZE |
         OB_CLIENT_FUNC_SHADE |
         OB_CLIENT_FUNC_CLOSE |
         OB_CLIENT_FUNC_BELOW |
         OB_CLIENT_FUNC_ABOVE |
         OB_CLIENT_FUNC_UNDECORATE);

    if (!(self->min_size.width < self->max_size.width ||
          self->min_size.height < self->max_size.height))
        self->functions &= ~OB_CLIENT_FUNC_RESIZE;

    switch (self->type) {
    case OB_CLIENT_TYPE_NORMAL:
        /* normal windows retain all of the possible decorations and
           functionality, and can be fullscreen */
        self->functions |= OB_CLIENT_FUNC_FULLSCREEN;
        break;

    case OB_CLIENT_TYPE_DIALOG:
        /* sometimes apps make dialog windows fullscreen for some reason (for
           e.g. kpdf does this..) */
        self->functions |= OB_CLIENT_FUNC_FULLSCREEN;
        break;

    case OB_CLIENT_TYPE_UTILITY:
        /* these windows don't have anything added or removed by default */
        break;

    case OB_CLIENT_TYPE_MENU:
    case OB_CLIENT_TYPE_TOOLBAR:
        /* these windows can't iconify or maximize */
        self->decorations &= ~(OB_FRAME_DECOR_ICONIFY |
                               OB_FRAME_DECOR_MAXIMIZE);
        self->functions &= ~(OB_CLIENT_FUNC_ICONIFY |
                             OB_CLIENT_FUNC_MAXIMIZE);
        break;

    case OB_CLIENT_TYPE_SPLASH:
        /* these don't get get any decorations, and the only thing you can
           do with them is move them */
        self->decorations = 0;
        self->functions = OB_CLIENT_FUNC_MOVE;
        break;

    case OB_CLIENT_TYPE_DESKTOP:
        /* these windows are not manipulated by the window manager */
        self->decorations = 0;
        self->functions = 0;
        break;

    case OB_CLIENT_TYPE_DOCK:
        /* these windows are not manipulated by the window manager, but they
           can set below layer which has a special meaning */
        self->decorations = 0;
        self->functions = OB_CLIENT_FUNC_BELOW;
        break;
    }

    /* Mwm Hints are applied subtractively to what has already been chosen for
       decor and functionality */
    if (self->mwmhints.flags & OB_MWM_FLAG_DECORATIONS) {
        if (! (self->mwmhints.decorations & OB_MWM_DECOR_ALL)) {
            if (! ((self->mwmhints.decorations & OB_MWM_DECOR_HANDLE) ||
                   (self->mwmhints.decorations & OB_MWM_DECOR_TITLE)))
            {
                /* if the mwm hints request no handle or title, then all
                   decorations are disabled, but keep the border if that's
                   specified */
                if (self->mwmhints.decorations & OB_MWM_DECOR_BORDER)
                    self->decorations = OB_FRAME_DECOR_BORDER;
                else
                    self->decorations = 0;
            }
        }
    }

    if (self->mwmhints.flags & OB_MWM_FLAG_FUNCTIONS) {
        if (! (self->mwmhints.functions & OB_MWM_FUNC_ALL)) {
            if (! (self->mwmhints.functions & OB_MWM_FUNC_RESIZE))
                self->functions &= ~OB_CLIENT_FUNC_RESIZE;
            if (! (self->mwmhints.functions & OB_MWM_FUNC_MOVE))
                self->functions &= ~OB_CLIENT_FUNC_MOVE;
            /* dont let mwm hints kill any buttons
               if (! (self->mwmhints.functions & OB_MWM_FUNC_ICONIFY))
               self->functions &= ~OB_CLIENT_FUNC_ICONIFY;
               if (! (self->mwmhints.functions & OB_MWM_FUNC_MAXIMIZE))
               self->functions &= ~OB_CLIENT_FUNC_MAXIMIZE;
            */
            /* dont let mwm hints kill the close button
               if (! (self->mwmhints.functions & MwmFunc_Close))
               self->functions &= ~OB_CLIENT_FUNC_CLOSE; */
        }
    }

    if (!(self->functions & OB_CLIENT_FUNC_SHADE))
        self->decorations &= ~OB_FRAME_DECOR_SHADE;
    if (!(self->functions & OB_CLIENT_FUNC_ICONIFY))
        self->decorations &= ~OB_FRAME_DECOR_ICONIFY;
    if (!(self->functions & OB_CLIENT_FUNC_RESIZE))
        self->decorations &= ~(OB_FRAME_DECOR_GRIPS | OB_FRAME_DECOR_HANDLE);

    /* can't maximize without moving/resizing */
    if (!((self->functions & OB_CLIENT_FUNC_MAXIMIZE) &&
          (self->functions & OB_CLIENT_FUNC_MOVE) &&
          (self->functions & OB_CLIENT_FUNC_RESIZE))) {
        self->functions &= ~OB_CLIENT_FUNC_MAXIMIZE;
        self->decorations &= ~OB_FRAME_DECOR_MAXIMIZE;
    }

    if (self->max_horz && self->max_vert) {
        /* you can't resize fully maximized windows */
        self->functions &= ~OB_CLIENT_FUNC_RESIZE;
        /* kill the handle on fully maxed windows */
        self->decorations &= ~(OB_FRAME_DECOR_HANDLE | OB_FRAME_DECOR_GRIPS);
    }

    /* If there are no decorations to remove, don't allow the user to try
       toggle the state */
    if (self->decorations == 0)
        self->functions &= ~OB_CLIENT_FUNC_UNDECORATE;

    /* finally, the user can have requested no decorations, which overrides
       everything (but doesnt give it a border if it doesnt have one) */
    if (self->undecorated)
        self->decorations = 0;

    /* if we don't have a titlebar, then we cannot shade! */
    if (!(self->decorations & OB_FRAME_DECOR_TITLEBAR))
        self->functions &= ~OB_CLIENT_FUNC_SHADE;

    /* now we need to check against rules for the client's current state */
    if (self->fullscreen) {
        self->functions &= (OB_CLIENT_FUNC_CLOSE |
                            OB_CLIENT_FUNC_FULLSCREEN |
                            OB_CLIENT_FUNC_ICONIFY);
        self->decorations = 0;
    }

    client_change_allowed_actions(self);

    if (reconfig)
        /* force reconfigure to make sure decorations are updated */
        client_reconfigure(self, TRUE);
}

static void client_change_allowed_actions(ObClient *self)
{
    gulong actions[12];
    gint num = 0;

    /* desktop windows are kept on all desktops */
    if (self->type != OB_CLIENT_TYPE_DESKTOP)
        actions[num++] = prop_atoms.net_wm_action_change_desktop;

    if (self->functions & OB_CLIENT_FUNC_SHADE)
        actions[num++] = prop_atoms.net_wm_action_shade;
    if (self->functions & OB_CLIENT_FUNC_CLOSE)
        actions[num++] = prop_atoms.net_wm_action_close;
    if (self->functions & OB_CLIENT_FUNC_MOVE)
        actions[num++] = prop_atoms.net_wm_action_move;
    if (self->functions & OB_CLIENT_FUNC_ICONIFY)
        actions[num++] = prop_atoms.net_wm_action_minimize;
    if (self->functions & OB_CLIENT_FUNC_RESIZE)
        actions[num++] = prop_atoms.net_wm_action_resize;
    if (self->functions & OB_CLIENT_FUNC_FULLSCREEN)
        actions[num++] = prop_atoms.net_wm_action_fullscreen;
    if (self->functions & OB_CLIENT_FUNC_MAXIMIZE) {
        actions[num++] = prop_atoms.net_wm_action_maximize_horz;
        actions[num++] = prop_atoms.net_wm_action_maximize_vert;
    }
    if (self->functions & OB_CLIENT_FUNC_ABOVE)
        actions[num++] = prop_atoms.net_wm_action_above;
    if (self->functions & OB_CLIENT_FUNC_BELOW)
        actions[num++] = prop_atoms.net_wm_action_below;
    if (self->functions & OB_CLIENT_FUNC_UNDECORATE)
        actions[num++] = prop_atoms.ob_wm_action_undecorate;

    PROP_SETA32(self->window, net_wm_allowed_actions, atom, actions, num);

    /* make sure the window isn't breaking any rules now */

    if (!(self->functions & OB_CLIENT_FUNC_SHADE) && self->shaded) {
        if (self->frame) client_shade(self, FALSE);
        else self->shaded = FALSE;
    }
    if (!(self->functions & OB_CLIENT_FUNC_ICONIFY) && self->iconic) {
        if (self->frame) client_iconify(self, FALSE, TRUE, FALSE);
        else self->iconic = FALSE;
    }
    if (!(self->functions & OB_CLIENT_FUNC_FULLSCREEN) && self->fullscreen) {
        if (self->frame) client_fullscreen(self, FALSE);
        else self->fullscreen = FALSE;
    }
    if (!(self->functions & OB_CLIENT_FUNC_MAXIMIZE) && (self->max_horz ||
                                                         self->max_vert)) {
        if (self->frame) client_maximize(self, FALSE, 0);
        else self->max_vert = self->max_horz = FALSE;
    }
}

void client_update_wmhints(ObClient *self)
{
    XWMHints *hints;

    /* assume a window takes input if it doesnt specify */
    self->can_focus = TRUE;

    if ((hints = XGetWMHints(ob_display, self->window)) != NULL) {
        gboolean ur;

        if (hints->flags & InputHint)
            self->can_focus = hints->input;

        /* only do this when first managing the window *AND* when we aren't
           starting up! */
        if (ob_state() != OB_STATE_STARTING && self->frame == NULL)
            if (hints->flags & StateHint)
                self->iconic = hints->initial_state == IconicState;

        ur = self->urgent;
        self->urgent = (hints->flags & XUrgencyHint);
        if (self->urgent && !ur)
            client_hilite(self, TRUE);
        else if (!self->urgent && ur && self->demands_attention)
            client_hilite(self, FALSE);

        if (!(hints->flags & WindowGroupHint))
            hints->window_group = None;

        /* did the group state change? */
        if (hints->window_group !=
            (self->group ? self->group->leader : None))
        {
            ObGroup *oldgroup = self->group;

            /* remove from the old group if there was one */
            if (self->group != NULL) {
                group_remove(self->group, self);
                self->group = NULL;
            }

            /* add ourself to the group if we have one */
            if (hints->window_group != None) {
                self->group = group_add(hints->window_group, self);
            }

            /* Put ourselves into the new group's transient tree, and remove
               ourselves from the old group's */
            client_update_transient_tree(self, oldgroup, self->group,
                                         self->transient_for_group,
                                         self->transient_for_group,
                                         client_direct_parent(self),
                                         client_direct_parent(self));

            /* Lastly, being in a group, or not, can change if the window is
               transient for anything.

               The logic for this is:
               self->transient = TRUE always if the window wants to be
               transient for something, even if transient_for was NULL because
               it wasn't in a group before.

               If parents was NULL and oldgroup was NULL we can assume
               that when we add the new group, it will become transient for
               something.

               If transient_for_group is TRUE, then it must have already
               had a group. If it is getting a new group, the above call to
               client_update_transient_tree has already taken care of
               everything ! If it is losing all group status then it will
               no longer be transient for anything and that needs to be
               updated.
            */
            if (self->transient &&
                ((self->parents == NULL && oldgroup == NULL) ||
                 (self->transient_for_group && !self->group)))
                client_update_transient_for(self);
        }

        /* the WM_HINTS can contain an icon */
        if (hints->flags & IconPixmapHint)
            client_update_icons(self);

        XFree(hints);
    }
}

void client_update_title(ObClient *self)
{
    gchar *data = NULL;
    gchar *visible = NULL;

    g_free(self->title);

    /* try netwm */
    if (!PROP_GETS(self->window, net_wm_name, utf8, &data)) {
        /* try old x stuff */
        if (!(PROP_GETS(self->window, wm_name, locale, &data)
              || PROP_GETS(self->window, wm_name, utf8, &data))) {
            if (self->transient) {
    /*
    GNOME alert windows are not given titles:
    http://developer.gnome.org/projects/gup/hig/draft_hig_new/windows-alert.html
    */
                data = g_strdup("");
            } else
                data = g_strdup("Unnamed Window");
        }
    }

    if (self->client_machine) {
        visible = g_strdup_printf("%s (%s)", data, self->client_machine);
        g_free(data);
    } else
        visible = data;

    if (self->not_responding) {
        data = visible;
        visible = g_strdup_printf("%s - [%s]", data, _("Not Responding"));
        g_free(data);
    }

    PROP_SETS(self->window, net_wm_visible_name, visible);
    self->title = visible;

    if (self->frame)
        frame_adjust_title(self->frame);

    /* update the icon title */
    data = NULL;
    g_free(self->icon_title);

    /* try netwm */
    if (!PROP_GETS(self->window, net_wm_icon_name, utf8, &data))
        /* try old x stuff */
        if (!(PROP_GETS(self->window, wm_icon_name, locale, &data) ||
              PROP_GETS(self->window, wm_icon_name, utf8, &data)))
            data = g_strdup(self->title);

    if (self->client_machine) {
        visible = g_strdup_printf("%s (%s)", data, self->client_machine);
        g_free(data);
    } else
        visible = data;

    if (self->not_responding) {
        data = visible;
        visible = g_strdup_printf("%s - [%s]", data, _("Not Responding"));
        g_free(data);
    }

    PROP_SETS(self->window, net_wm_visible_icon_name, visible);
    self->icon_title = visible;
}

void client_update_strut(ObClient *self)
{
    guint num;
    guint32 *data;
    gboolean got = FALSE;
    StrutPartial strut;

    if (PROP_GETA32(self->window, net_wm_strut_partial, cardinal,
                    &data, &num)) {
        if (num == 12) {
            got = TRUE;
            STRUT_PARTIAL_SET(strut,
                              data[0], data[2], data[1], data[3],
                              data[4], data[5], data[8], data[9],
                              data[6], data[7], data[10], data[11]);
        }
        g_free(data);
    }

    if (!got &&
        PROP_GETA32(self->window, net_wm_strut, cardinal, &data, &num)) {
        if (num == 4) {
            Rect *a;

            got = TRUE;

            /* use the screen's width/height */
            a = screen_physical_area_all_monitors();

            STRUT_PARTIAL_SET(strut,
                              data[0], data[2], data[1], data[3],
                              a->y, a->y + a->height - 1,
                              a->x, a->x + a->width - 1,
                              a->y, a->y + a->height - 1,
                              a->x, a->x + a->width - 1);
            g_free(a);
        }
        g_free(data);
    }

    if (!got)
        STRUT_PARTIAL_SET(strut, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0);

    if (!STRUT_EQUAL(strut, self->strut)) {
        self->strut = strut;

        /* updating here is pointless while we're being mapped cuz we're not in
           the client list yet */
        if (self->frame)
            screen_update_areas();
    }
}

void client_update_icons(ObClient *self)
{
    guint num;
    guint32 *data;
    guint w, h, i, j;

    for (i = 0; i < self->nicons; ++i)
        g_free(self->icons[i].data);
    if (self->nicons > 0)
        g_free(self->icons);
    self->nicons = 0;

    if (PROP_GETA32(self->window, net_wm_icon, cardinal, &data, &num)) {
        /* figure out how many valid icons are in here */
        i = 0;
        while (num - i > 2) {
            w = data[i++];
            h = data[i++];
            i += w * h;
            if (i > num || w*h == 0) break;
            ++self->nicons;
        }

        self->icons = g_new(ObClientIcon, self->nicons);

        /* store the icons */
        i = 0;
        for (j = 0; j < self->nicons; ++j) {
            guint x, y, t;

            w = self->icons[j].width = data[i++];
            h = self->icons[j].height = data[i++];

            if (w*h == 0) continue;

            self->icons[j].data = g_new(RrPixel32, w * h);
            for (x = 0, y = 0, t = 0; t < w * h; ++t, ++x, ++i) {
                if (x >= w) {
                    x = 0;
                    ++y;
                }
                self->icons[j].data[t] =
                    (((data[i] >> 24) & 0xff) << RrDefaultAlphaOffset) +
                    (((data[i] >> 16) & 0xff) << RrDefaultRedOffset) +
                    (((data[i] >> 8) & 0xff) << RrDefaultGreenOffset) +
                    (((data[i] >> 0) & 0xff) << RrDefaultBlueOffset);
            }
            g_assert(i <= num);
        }

        g_free(data);
    } else {
        XWMHints *hints;

        if ((hints = XGetWMHints(ob_display, self->window))) {
            if (hints->flags & IconPixmapHint) {
                self->nicons = 1;
                self->icons = g_new(ObClientIcon, self->nicons);
                xerror_set_ignore(TRUE);
                if (!RrPixmapToRGBA(ob_rr_inst,
                                    hints->icon_pixmap,
                                    (hints->flags & IconMaskHint ?
                                     hints->icon_mask : None),
                                    &self->icons[0].width,
                                    &self->icons[0].height,
                                    &self->icons[0].data))
                {
                    g_free(self->icons);
                    self->nicons = 0;
                }
                xerror_set_ignore(FALSE);
            }
            XFree(hints);
        }
    }

    /* set the default icon onto the window
       in theory, this could be a race, but if a window doesn't set an icon
       or removes it entirely, it's not very likely it is going to set one
       right away afterwards

       if it has parents, then one of them will have an icon already
    */
    if (self->nicons == 0 && !self->parents) {
        RrPixel32 *icon = ob_rr_theme->def_win_icon;
        gulong *data;

        data = g_new(gulong, 48*48+2);
        data[0] = data[1] =  48;
        for (i = 0; i < 48*48; ++i)
            data[i+2] = (((icon[i] >> RrDefaultAlphaOffset) & 0xff) << 24) +
                (((icon[i] >> RrDefaultRedOffset) & 0xff) << 16) +
                (((icon[i] >> RrDefaultGreenOffset) & 0xff) << 8) +
                (((icon[i] >> RrDefaultBlueOffset) & 0xff) << 0);
        PROP_SETA32(self->window, net_wm_icon, cardinal, data, 48*48+2);
        g_free(data);
    } else if (self->frame)
        /* don't draw the icon empty if we're just setting one now anyways,
           we'll get the property change any second */
        frame_adjust_icon(self->frame);
}

void client_update_icon_geometry(ObClient *self)
{
    guint num;
    guint32 *data;

    RECT_SET(self->icon_geometry, 0, 0, 0, 0);

    if (PROP_GETA32(self->window, net_wm_icon_geometry, cardinal, &data, &num)
        && num == 4)
    {
        /* don't let them set it with an area < 0 */
        RECT_SET(self->icon_geometry, data[0], data[1],
                 MAX(data[2],0), MAX(data[3],0));
    }
}

static void client_get_session_ids(ObClient *self)
{
    guint32 leader;
    gboolean got;
    gchar *s;
    gchar **ss;

    if (!PROP_GET32(self->window, wm_client_leader, window, &leader))
        leader = None;

    /* get the SM_CLIENT_ID */
    got = FALSE;
    if (leader)
        got = PROP_GETS(leader, sm_client_id, locale, &self->sm_client_id);
    if (!got)
        PROP_GETS(self->window, sm_client_id, locale, &self->sm_client_id);

    /* get the WM_CLASS (name and class). make them "" if they are not
       provided */
    got = FALSE;
    if (leader)
        got = PROP_GETSS(leader, wm_class, locale, &ss);
    if (!got)
        got = PROP_GETSS(self->window, wm_class, locale, &ss);

    if (got) {
        if (ss[0]) {
            self->name = g_strdup(ss[0]);
            if (ss[1])
                self->class = g_strdup(ss[1]);
        }
        g_strfreev(ss);
    }

    if (self->name == NULL) self->name = g_strdup("");
    if (self->class == NULL) self->class = g_strdup("");

    /* get the WM_WINDOW_ROLE. make it "" if it is not provided */
    got = FALSE;
    if (leader)
        got = PROP_GETS(leader, wm_window_role, locale, &s);
    if (!got)
        got = PROP_GETS(self->window, wm_window_role, locale, &s);

    if (got)
        self->role = s;
    else
        self->role = g_strdup("");

    /* get the WM_COMMAND */
    got = FALSE;

    if (leader)
        got = PROP_GETSS(leader, wm_command, locale, &ss);
    if (!got)
        got = PROP_GETSS(self->window, wm_command, locale, &ss);

    if (got) {
        /* merge/mash them all together */
        gchar *merge = NULL;
        gint i;

        for (i = 0; ss[i]; ++i) {
            gchar *tmp = merge;
            if (merge)
                merge = g_strconcat(merge, ss[i], NULL);
            else
                merge = g_strconcat(ss[i], NULL);
            g_free(tmp);
        }
        g_strfreev(ss);

        self->wm_command = merge;
    }

    /* get the WM_CLIENT_MACHINE */
    got = FALSE;
    if (leader)
        got = PROP_GETS(leader, wm_client_machine, locale, &s);
    if (!got)
        got = PROP_GETS(self->window, wm_client_machine, locale, &s);

    if (got) {
        gchar localhost[128];
        guint32 pid;

        gethostname(localhost, 127);
        localhost[127] = '\0';
        if (strcmp(localhost, s) != 0)
            self->client_machine = s;
        else
            g_free(s);

        /* see if it has the PID set too (the PID requires that the
           WM_CLIENT_MACHINE be set) */
        if (PROP_GET32(self->window, net_wm_pid, cardinal, &pid))
            self->pid = pid;
    }
}

static void client_change_wm_state(ObClient *self)
{
    gulong state[2];
    glong old;

    old = self->wmstate;

    if (self->shaded || self->iconic ||
        (self->desktop != DESKTOP_ALL && self->desktop != screen_desktop))
    {
        self->wmstate = IconicState;
    } else
        self->wmstate = NormalState;

    if (old != self->wmstate) {
        PROP_MSG(self->window, kde_wm_change_state,
                 self->wmstate, 1, 0, 0);

        state[0] = self->wmstate;
        state[1] = None;
        PROP_SETA32(self->window, wm_state, wm_state, state, 2);
    }
}

static void client_change_state(ObClient *self)
{
    gulong netstate[12];
    guint num;

    num = 0;
    if (self->modal)
        netstate[num++] = prop_atoms.net_wm_state_modal;
    if (self->shaded)
        netstate[num++] = prop_atoms.net_wm_state_shaded;
    if (self->iconic)
        netstate[num++] = prop_atoms.net_wm_state_hidden;
    if (self->skip_taskbar)
        netstate[num++] = prop_atoms.net_wm_state_skip_taskbar;
    if (self->skip_pager)
        netstate[num++] = prop_atoms.net_wm_state_skip_pager;
    if (self->fullscreen)
        netstate[num++] = prop_atoms.net_wm_state_fullscreen;
    if (self->max_vert)
        netstate[num++] = prop_atoms.net_wm_state_maximized_vert;
    if (self->max_horz)
        netstate[num++] = prop_atoms.net_wm_state_maximized_horz;
    if (self->above)
        netstate[num++] = prop_atoms.net_wm_state_above;
    if (self->below)
        netstate[num++] = prop_atoms.net_wm_state_below;
    if (self->demands_attention)
        netstate[num++] = prop_atoms.net_wm_state_demands_attention;
    if (self->undecorated)
        netstate[num++] = prop_atoms.ob_wm_state_undecorated;
    PROP_SETA32(self->window, net_wm_state, atom, netstate, num);

    if (self->frame)
        frame_adjust_state(self->frame);
}

ObClient *client_search_focus_tree(ObClient *self)
{
    GSList *it;
    ObClient *ret;

    for (it = self->transients; it; it = g_slist_next(it)) {
        if (client_focused(it->data)) return it->data;
        if ((ret = client_search_focus_tree(it->data))) return ret;
    }
    return NULL;
}

ObClient *client_search_focus_tree_full(ObClient *self)
{
    if (self->parents) {
        GSList *it;

        for (it = self->parents; it; it = g_slist_next(it)) {
            ObClient *c = it->data;
            if ((c = client_search_focus_tree_full(it->data))) return c;
        }

        return NULL;
    }
    else {
        /* this function checks the whole tree, the client_search_focus_tree
           does not, so we need to check this window */
        if (client_focused(self))
            return self;
        return client_search_focus_tree(self);
    }
}

ObClient *client_search_focus_group_full(ObClient *self)
{
    GSList *it;

    if (self->group) {
        for (it = self->group->members; it; it = g_slist_next(it)) {
            ObClient *c = it->data;

            if (client_focused(c)) return c;
            if ((c = client_search_focus_tree(it->data))) return c;
        }
    } else
        if (client_focused(self)) return self;
    return NULL;
}

gboolean client_has_parent(ObClient *self)
{
    return self->parents != NULL;
}

static ObStackingLayer calc_layer(ObClient *self)
{
    ObStackingLayer l;
    Rect *monitor;

    monitor = screen_physical_area_monitor(client_monitor(self));

    if (self->type == OB_CLIENT_TYPE_DESKTOP)
        l = OB_STACKING_LAYER_DESKTOP;
    else if (self->type == OB_CLIENT_TYPE_DOCK) {
        if (self->below) l = OB_STACKING_LAYER_NORMAL;
        else l = OB_STACKING_LAYER_ABOVE;
    }
    else if ((self->fullscreen ||
              /* No decorations and fills the monitor = oldskool fullscreen.
                 But not for maximized windows.
              */
              (self->decorations == 0 &&
               !(self->max_horz && self->max_vert) &&
               RECT_EQUAL(self->area, *monitor))) &&
             (client_focused(self) || client_search_focus_tree(self)))
        l = OB_STACKING_LAYER_FULLSCREEN;
    else if (self->above) l = OB_STACKING_LAYER_ABOVE;
    else if (self->below) l = OB_STACKING_LAYER_BELOW;
    else l = OB_STACKING_LAYER_NORMAL;

    g_free(monitor);

    return l;
}

static void client_calc_layer_recursive(ObClient *self, ObClient *orig,
                                        ObStackingLayer min)
{
    ObStackingLayer old, own;
    GSList *it;

    old = self->layer;
    own = calc_layer(self);
    self->layer = MAX(own, min);

    if (self->layer != old) {
        stacking_remove(CLIENT_AS_WINDOW(self));
        stacking_add_nonintrusive(CLIENT_AS_WINDOW(self));
    }

    for (it = self->transients; it; it = g_slist_next(it))
        client_calc_layer_recursive(it->data, orig,
                                    self->layer);
}

void client_calc_layer(ObClient *self)
{
    ObClient *orig;
    GSList *it;

    orig = self;

    /* transients take on the layer of their parents */
    it = client_search_all_top_parents(self);

    for (; it; it = g_slist_next(it))
        client_calc_layer_recursive(it->data, orig, 0);
}

gboolean client_should_show(ObClient *self)
{
    if (self->iconic)
        return FALSE;
    if (client_normal(self) && screen_showing_desktop)
        return FALSE;
    if (self->desktop == screen_desktop || self->desktop == DESKTOP_ALL)
        return TRUE;

    return FALSE;
}

gboolean client_show(ObClient *self)
{
    gboolean show = FALSE;

    if (client_should_show(self)) {
        frame_show(self->frame);
        show = TRUE;

        /* According to the ICCCM (sec 4.1.3.1) when a window is not visible,
           it needs to be in IconicState. This includes when it is on another
           desktop!
        */
        client_change_wm_state(self);
    }
    return show;
}

gboolean client_hide(ObClient *self)
{
    gboolean hide = FALSE;

    if (!client_should_show(self)) {
        if (self == focus_client) {
            /* if there is a grab going on, then we need to cancel it. if we
               move focus during the grab, applications will get
               NotifyWhileGrabbed events and ignore them !

               actions should not rely on being able to move focus during an
               interactive grab.
            */
            event_cancel_all_key_grabs();
        }

        /* We don't need to ignore enter events here.
           The window can hide/iconify in 3 different ways:
           1 - through an x message. in this case we ignore all enter events
               caused by responding to the x message (unless underMouse)
           2 - by a keyboard action. in this case we ignore all enter events
               caused by the action
           3 - by a mouse action. in this case they are doing stuff with the
               mouse and focus _should_ move.

           Also in action_end, we simulate an enter event that can't be ignored
           so trying to ignore them is futile in case 3 anyways
        */

        frame_hide(self->frame);
        hide = TRUE;

        /* According to the ICCCM (sec 4.1.3.1) when a window is not visible,
           it needs to be in IconicState. This includes when it is on another
           desktop!
        */
        client_change_wm_state(self);
    }
    return hide;
}

void client_showhide(ObClient *self)
{
    if (!client_show(self))
        client_hide(self);
}

gboolean client_normal(ObClient *self) {
    return ! (self->type == OB_CLIENT_TYPE_DESKTOP ||
              self->type == OB_CLIENT_TYPE_DOCK ||
              self->type == OB_CLIENT_TYPE_SPLASH);
}

gboolean client_helper(ObClient *self)
{
    return (self->type == OB_CLIENT_TYPE_UTILITY ||
            self->type == OB_CLIENT_TYPE_MENU ||
            self->type == OB_CLIENT_TYPE_TOOLBAR);
}

gboolean client_mouse_focusable(ObClient *self)
{
    return !(self->type == OB_CLIENT_TYPE_MENU ||
             self->type == OB_CLIENT_TYPE_TOOLBAR ||
             self->type == OB_CLIENT_TYPE_SPLASH ||
             self->type == OB_CLIENT_TYPE_DOCK);
}

gboolean client_enter_focusable(ObClient *self)
{
    /* you can focus desktops but it shouldn't on enter */
    return (client_mouse_focusable(self) &&
            self->type != OB_CLIENT_TYPE_DESKTOP);
}


static void client_apply_startup_state(ObClient *self,
                                       gint x, gint y, gint w, gint h)
{
    /* save the states that we are going to apply */
    gboolean iconic = self->iconic;
    gboolean fullscreen = self->fullscreen;
    gboolean undecorated = self->undecorated;
    gboolean shaded = self->shaded;
    gboolean demands_attention = self->demands_attention;
    gboolean max_horz = self->max_horz;
    gboolean max_vert = self->max_vert;
    Rect oldarea;
    gint l;

    /* turn them all off in the client, so they won't affect the window
       being placed */
    self->iconic = self->fullscreen = self->undecorated = self->shaded =
        self->demands_attention = self->max_horz = self->max_vert = FALSE;

    /* move the client to its placed position, or it it's already there,
       generate a ConfigureNotify telling the client where it is.

       do this after adjusting the frame. otherwise it gets all weird and
       clients don't work right

       do this before applying the states so they have the correct
       pre-max/pre-fullscreen values
    */
    client_try_configure(self, &x, &y, &w, &h, &l, &l, FALSE);
    ob_debug("placed window 0x%x at %d, %d with size %d x %d\n",
             self->window, x, y, w, h);
    /* save the area, and make it where it should be for the premax stuff */
    oldarea = self->area;
    RECT_SET(self->area, x, y, w, h);

    /* apply the states. these are in a carefully crafted order.. */

    if (iconic)
        client_iconify(self, TRUE, FALSE, TRUE);
    if (fullscreen)
        client_fullscreen(self, TRUE);
    if (undecorated)
        client_set_undecorated(self, TRUE);
    if (shaded)
        client_shade(self, TRUE);
    if (demands_attention)
        client_hilite(self, TRUE);

    if (max_vert && max_horz)
        client_maximize(self, TRUE, 0);
    else if (max_vert)
        client_maximize(self, TRUE, 2);
    else if (max_horz)
        client_maximize(self, TRUE, 1);

    /* if the window hasn't been configured yet, then do so now, in fact the
       x,y,w,h may _not_ be the same as the area rect, which can end up
       meaning that the client isn't properly moved/resized by the fullscreen
       function
       pho can cause this because it maps at size of the screen but not 0,0
       so openbox moves it on screen to 0,0 (thus x,y=0,0 and area.x,y don't).
       then fullscreen'ing makes it go to 0,0 which it thinks it already is at
       cuz thats where the pre-fullscreen will be. however the actual area is
       not, so this needs to be called even if we have fullscreened/maxed
    */
    self->area = oldarea;
    client_configure(self, x, y, w, h, FALSE, TRUE, FALSE);

    /* set the desktop hint, to make sure that it always exists */
    PROP_SET32(self->window, net_wm_desktop, cardinal, self->desktop);

    /* nothing to do for the other states:
       skip_taskbar
       skip_pager
       modal
       above
       below
    */
}

void client_gravity_resize_w(ObClient *self, gint *x, gint oldw, gint neww)
{
    /* these should be the current values. this is for when you're not moving,
       just resizing */
    g_assert(*x == self->area.x);
    g_assert(oldw == self->area.width);

    /* horizontal */
    switch (self->gravity) {
    default:
    case NorthWestGravity:
    case WestGravity:
    case SouthWestGravity:
    case StaticGravity:
    case ForgetGravity:
        break;
    case NorthGravity:
    case CenterGravity:
    case SouthGravity:
        *x -= (neww - oldw) / 2;
        break;
    case NorthEastGravity:
    case EastGravity:
    case SouthEastGravity:
        *x -= neww - oldw;
        break;
    }
}

void client_gravity_resize_h(ObClient *self, gint *y, gint oldh, gint newh)
{
    /* these should be the current values. this is for when you're not moving,
       just resizing */
    g_assert(*y == self->area.y);
    g_assert(oldh == self->area.height);

    /* vertical */
    switch (self->gravity) {
    default:
    case NorthWestGravity:
    case NorthGravity:
    case NorthEastGravity:
    case StaticGravity:
    case ForgetGravity:
        break;
    case WestGravity:
    case CenterGravity:
    case EastGravity:
        *y -= (newh - oldh) / 2;
        break;
    case SouthWestGravity:
    case SouthGravity:
    case SouthEastGravity:
        *y -= newh - oldh;
        break;
    }
}

void client_try_configure(ObClient *self, gint *x, gint *y, gint *w, gint *h,
                          gint *logicalw, gint *logicalh,
                          gboolean user)
{
    Rect desired = {*x, *y, *w, *h};
    frame_rect_to_frame(self->frame, &desired);

    /* make the frame recalculate its dimentions n shit without changing
       anything visible for real, this way the constraints below can work with
       the updated frame dimensions. */
    frame_adjust_area(self->frame, FALSE, TRUE, TRUE);

    /* gets the frame's position */
    frame_client_gravity(self->frame, x, y);

    /* these positions are frame positions, not client positions */

    /* set the size and position if fullscreen */
    if (self->fullscreen) {
        Rect *a;
        guint i;

        i = screen_find_monitor(&desired);
        a = screen_physical_area_monitor(i);

        *x = a->x;
        *y = a->y;
        *w = a->width;
        *h = a->height;

        user = FALSE; /* ignore if the client can't be moved/resized when it
                         is fullscreening */

        g_free(a);
    } else if (self->max_horz || self->max_vert) {
        Rect *a;
        guint i;

        /* use all possible struts when maximizing to the full screen */
        i = screen_find_monitor(&desired);
        a = screen_area(self->desktop, i,
                        (self->max_horz && self->max_vert ? NULL : &desired));

        /* set the size and position if maximized */
        if (self->max_horz) {
            *x = a->x;
            *w = a->width - self->frame->size.left - self->frame->size.right;
        }
        if (self->max_vert) {
            *y = a->y;
            *h = a->height - self->frame->size.top - self->frame->size.bottom;
        }

        user = FALSE; /* ignore if the client can't be moved/resized when it
                         is maximizing */

        g_free(a);
    }

    /* gets the client's position */
    frame_frame_gravity(self->frame, x, y);

    /* work within the prefered sizes given by the window */
    if (!(*w == self->area.width && *h == self->area.height)) {
        gint basew, baseh, minw, minh;
        gint incw, inch;
        gfloat minratio, maxratio;

        incw = self->fullscreen || self->max_horz ? 1 : self->size_inc.width;
        inch = self->fullscreen || self->max_vert ? 1 : self->size_inc.height;
        minratio = self->fullscreen || (self->max_horz && self->max_vert) ?
            0 : self->min_ratio;
        maxratio = self->fullscreen || (self->max_horz && self->max_vert) ?
            0 : self->max_ratio;

        /* base size is substituted with min size if not specified */
        if (self->base_size.width || self->base_size.height) {
            basew = self->base_size.width;
            baseh = self->base_size.height;
        } else {
            basew = self->min_size.width;
            baseh = self->min_size.height;
        }
        /* min size is substituted with base size if not specified */
        if (self->min_size.width || self->min_size.height) {
            minw = self->min_size.width;
            minh = self->min_size.height;
        } else {
            minw = self->base_size.width;
            minh = self->base_size.height;
        }

        /* if this is a user-requested resize, then check against min/max
           sizes */

        /* smaller than min size or bigger than max size? */
        if (*w > self->max_size.width) *w = self->max_size.width;
        if (*w < minw) *w = minw;
        if (*h > self->max_size.height) *h = self->max_size.height;
        if (*h < minh) *h = minh;

        *w -= basew;
        *h -= baseh;

        /* keep to the increments */
        *w /= incw;
        *h /= inch;

        /* you cannot resize to nothing */
        if (basew + *w < 1) *w = 1 - basew;
        if (baseh + *h < 1) *h = 1 - baseh;

        /* save the logical size */
        *logicalw = incw > 1 ? *w : *w + basew;
        *logicalh = inch > 1 ? *h : *h + baseh;

        *w *= incw;
        *h *= inch;

        *w += basew;
        *h += baseh;

        /* adjust the height to match the width for the aspect ratios.
           for this, min size is not substituted for base size ever. */
        *w -= self->base_size.width;
        *h -= self->base_size.height;

        if (minratio)
            if (*h * minratio > *w) {
                *h = (gint)(*w / minratio);

                /* you cannot resize to nothing */
                if (*h < 1) {
                    *h = 1;
                    *w = (gint)(*h * minratio);
                }
            }
        if (maxratio)
            if (*h * maxratio < *w) {
                *h = (gint)(*w / maxratio);

                /* you cannot resize to nothing */
                if (*h < 1) {
                    *h = 1;
                    *w = (gint)(*h * minratio);
                }
            }

        *w += self->base_size.width;
        *h += self->base_size.height;
    }

    /* these override the above states! if you cant move you can't move! */
    if (user) {
        if (!(self->functions & OB_CLIENT_FUNC_MOVE)) {
            *x = self->area.x;
            *y = self->area.y;
        }
        if (!(self->functions & OB_CLIENT_FUNC_RESIZE)) {
            *w = self->area.width;
            *h = self->area.height;
        }
    }

    g_assert(*w > 0);
    g_assert(*h > 0);
}


void client_configure(ObClient *self, gint x, gint y, gint w, gint h,
                      gboolean user, gboolean final, gboolean force_reply)
{
    gint oldw, oldh;
    gboolean send_resize_client;
    gboolean moved = FALSE, resized = FALSE, rootmoved = FALSE;
    gboolean fmoved, fresized;
    guint fdecor = self->frame->decorations;
    gboolean fhorz = self->frame->max_horz;
    gboolean fvert = self->frame->max_vert;
    gint logicalw, logicalh;

    /* find the new x, y, width, and height (and logical size) */
    client_try_configure(self, &x, &y, &w, &h, &logicalw, &logicalh, user);

    /* set the logical size if things changed */
    if (!(w == self->area.width && h == self->area.height))
        SIZE_SET(self->logical_size, logicalw, logicalh);

    /* figure out if we moved or resized or what */
    moved = (x != self->area.x || y != self->area.y);
    resized = (w != self->area.width || h != self->area.height);

    oldw = self->area.width;
    oldh = self->area.height;
    RECT_SET(self->area, x, y, w, h);

    /* for app-requested resizes, always resize if 'resized' is true.
       for user-requested ones, only resize if final is true, or when
       resizing in redraw mode */
    send_resize_client = ((!user && resized) ||
                          (user && (final ||
                                    (resized && config_resize_redraw))));

    /* if the client is enlarging, then resize the client before the frame */
    if (send_resize_client && (w > oldw || h > oldh)) {
        XMoveResizeWindow(ob_display, self->window,
                          self->frame->size.left, self->frame->size.top,
                          MAX(w, oldw), MAX(h, oldh));
        frame_adjust_client_area(self->frame);
    }

    /* find the frame's dimensions and move/resize it */
    fmoved = moved;
    fresized = resized;

    /* if decorations changed, then readjust everything for the frame */
    if (self->decorations != fdecor ||
        self->max_horz != fhorz || self->max_vert != fvert)
    {
        fmoved = fresized = TRUE;
    }

    /* adjust the frame */
    if (fmoved || fresized) {
        gulong ignore_start;
        if (!user)
            ignore_start = event_start_ignore_all_enters();

        frame_adjust_area(self->frame, fmoved, fresized, FALSE);

        if (!user)
            event_end_ignore_all_enters(ignore_start);
    }

    if (!user || final) {
        gint oldrx = self->root_pos.x;
        gint oldry = self->root_pos.y;
        /* we have reset the client to 0 border width, so don't include
           it in these coords */
        POINT_SET(self->root_pos,
                  self->frame->area.x + self->frame->size.left -
                  self->border_width,
                  self->frame->area.y + self->frame->size.top -
                  self->border_width);
        if (self->root_pos.x != oldrx || self->root_pos.y != oldry)
            rootmoved = TRUE;
    }

    /* This is kinda tricky and should not be changed.. let me explain!

       When user = FALSE, then the request is coming from the application
       itself, and we are more strict about when to send a synthetic
       ConfigureNotify.  We strictly follow the rules of the ICCCM sec 4.1.5
       in this case (if force_reply is true)

       When user = TRUE, then the request is coming from "us", like when we
       maximize a window or something.  In this case we are more lenient.  We
       used to follow the same rules as above, but _Java_ Swing can't handle
       this. So just to appease Swing, when user = TRUE, we always send
       a synthetic ConfigureNotify to give the window its root coordinates.
    */
    if ((!user && !resized && (rootmoved || force_reply)) ||
        (user && final && rootmoved))
    {
        XEvent event;

        event.type = ConfigureNotify;
        event.xconfigure.display = ob_display;
        event.xconfigure.event = self->window;
        event.xconfigure.window = self->window;

        ob_debug("Sending ConfigureNotify to %s for %d,%d %dx%d\n",
                 self->title, self->root_pos.x, self->root_pos.y, w, h);

        /* root window real coords */
        event.xconfigure.x = self->root_pos.x;
        event.xconfigure.y = self->root_pos.y;
        event.xconfigure.width = w;
        event.xconfigure.height = h;
        event.xconfigure.border_width = self->border_width;
        event.xconfigure.above = None;
        event.xconfigure.override_redirect = FALSE;
        XSendEvent(event.xconfigure.display, event.xconfigure.window,
                   FALSE, StructureNotifyMask, &event);
    }

    /* if the client is shrinking, then resize the frame before the client.

       both of these resize sections may run, because the top one only resizes
       in the direction that is growing
     */
    if (send_resize_client && (w <= oldw || h <= oldh)) {
        frame_adjust_client_area(self->frame);
        XMoveResizeWindow(ob_display, self->window,
                          self->frame->size.left, self->frame->size.top, w, h);
    }

    XFlush(ob_display);
}

void client_fullscreen(ObClient *self, gboolean fs)
{
    gint x, y, w, h;

    if (!(self->functions & OB_CLIENT_FUNC_FULLSCREEN) || /* can't */
        self->fullscreen == fs) return;                   /* already done */

    self->fullscreen = fs;
    client_change_state(self); /* change the state hints on the client */

    if (fs) {
        self->pre_fullscreen_area = self->area;
        /* if the window is maximized, its area isn't all that meaningful.
           save it's premax area instead. */
        if (self->max_horz) {
            self->pre_fullscreen_area.x = self->pre_max_area.x;
            self->pre_fullscreen_area.width = self->pre_max_area.width;
        }
        if (self->max_vert) {
            self->pre_fullscreen_area.y = self->pre_max_area.y;
            self->pre_fullscreen_area.height = self->pre_max_area.height;
        }

        /* these will help configure_full figure out where to fullscreen
           the window */
        x = self->area.x;
        y = self->area.y;
        w = self->area.width;
        h = self->area.height;
    } else {
        g_assert(self->pre_fullscreen_area.width > 0 &&
                 self->pre_fullscreen_area.height > 0);

        x = self->pre_fullscreen_area.x;
        y = self->pre_fullscreen_area.y;
        w = self->pre_fullscreen_area.width;
        h = self->pre_fullscreen_area.height;
        RECT_SET(self->pre_fullscreen_area, 0, 0, 0, 0);
    }

    ob_debug("Window %s going fullscreen (%d)\n",
             self->title, self->fullscreen);

    client_setup_decor_and_functions(self, FALSE);
    client_move_resize(self, x, y, w, h);

    /* and adjust our layer/stacking. do this after resizing the window,
       and applying decorations, because windows which fill the screen are
       considered "fullscreen" and it affects their layer */
    client_calc_layer(self);

    if (fs) {
        /* try focus us when we go into fullscreen mode */
        client_focus(self);
    }
}

static void client_iconify_recursive(ObClient *self,
                                     gboolean iconic, gboolean curdesk,
                                     gboolean hide_animation)
{
    GSList *it;
    gboolean changed = FALSE;


    if (self->iconic != iconic) {
        ob_debug("%sconifying window: 0x%lx\n", (iconic ? "I" : "Uni"),
                 self->window);

        if (iconic) {
            /* don't let non-normal windows iconify along with their parents
               or whatever */
            if (client_normal(self)) {
                self->iconic = iconic;

                /* update the focus lists.. iconic windows go to the bottom of
                   the list */
                focus_order_to_bottom(self);

                changed = TRUE;
            }
        } else {
            self->iconic = iconic;

            if (curdesk && self->desktop != screen_desktop &&
                self->desktop != DESKTOP_ALL)
                client_set_desktop(self, screen_desktop, FALSE, FALSE);

            /* this puts it after the current focused window */
            focus_order_remove(self);
            focus_order_add_new(self);

            changed = TRUE;
        }
    }

    if (changed) {
        client_change_state(self);
        if (config_animate_iconify && !hide_animation)
            frame_begin_iconify_animation(self->frame, iconic);
        /* do this after starting the animation so it doesn't flash */
        client_showhide(self);
    }

    /* iconify all direct transients, and deiconify all transients
       (non-direct too) */
    for (it = self->transients; it; it = g_slist_next(it))
        if (it->data != self)
            if (client_is_direct_child(self, it->data) || !iconic)
                client_iconify_recursive(it->data, iconic, curdesk,
                                         hide_animation);
}

void client_iconify(ObClient *self, gboolean iconic, gboolean curdesk,
                    gboolean hide_animation)
{
    if (self->functions & OB_CLIENT_FUNC_ICONIFY || !iconic) {
        /* move up the transient chain as far as possible first */
        self = client_search_top_direct_parent(self);
        client_iconify_recursive(self, iconic, curdesk, hide_animation);
    }
}

void client_maximize(ObClient *self, gboolean max, gint dir)
{
    gint x, y, w, h;

    g_assert(dir == 0 || dir == 1 || dir == 2);
    if (!(self->functions & OB_CLIENT_FUNC_MAXIMIZE)) return; /* can't */

    /* check if already done */
    if (max) {
        if (dir == 0 && self->max_horz && self->max_vert) return;
        if (dir == 1 && self->max_horz) return;
        if (dir == 2 && self->max_vert) return;
    } else {
        if (dir == 0 && !self->max_horz && !self->max_vert) return;
        if (dir == 1 && !self->max_horz) return;
        if (dir == 2 && !self->max_vert) return;
    }

    /* these will help configure_full figure out which screen to fill with
       the window */
    x = self->area.x;
    y = self->area.y;
    w = self->area.width;
    h = self->area.height;

    if (max) {
        if ((dir == 0 || dir == 1) && !self->max_horz) { /* horz */
            RECT_SET(self->pre_max_area,
                     self->area.x, self->pre_max_area.y,
                     self->area.width, self->pre_max_area.height);
        }
        if ((dir == 0 || dir == 2) && !self->max_vert) { /* vert */
            RECT_SET(self->pre_max_area,
                     self->pre_max_area.x, self->area.y,
                     self->pre_max_area.width, self->area.height);
        }
    } else {
        if ((dir == 0 || dir == 1) && self->max_horz) { /* horz */
            g_assert(self->pre_max_area.width > 0);

            x = self->pre_max_area.x;
            w = self->pre_max_area.width;

            RECT_SET(self->pre_max_area, 0, self->pre_max_area.y,
                     0, self->pre_max_area.height);
        }
        if ((dir == 0 || dir == 2) && self->max_vert) { /* vert */
            g_assert(self->pre_max_area.height > 0);

            y = self->pre_max_area.y;
            h = self->pre_max_area.height;

            RECT_SET(self->pre_max_area, self->pre_max_area.x, 0,
                     self->pre_max_area.width, 0);
        }
    }

    if (dir == 0 || dir == 1) /* horz */
        self->max_horz = max;
    if (dir == 0 || dir == 2) /* vert */
        self->max_vert = max;

    client_change_state(self); /* change the state hints on the client */

    client_setup_decor_and_functions(self, FALSE);
    client_move_resize(self, x, y, w, h);
}

void client_shade(ObClient *self, gboolean shade)
{
    if ((!(self->functions & OB_CLIENT_FUNC_SHADE) &&
         shade) ||                         /* can't shade */
        self->shaded == shade) return;     /* already done */

    self->shaded = shade;
    client_change_state(self);
    client_change_wm_state(self); /* the window is being hidden/shown */
    /* resize the frame to just the titlebar */
    frame_adjust_area(self->frame, FALSE, TRUE, FALSE);
}

static void client_ping_event(ObClient *self, gboolean dead)
{
    self->not_responding = dead;
    client_update_title(self);
}

void client_close(ObClient *self)
{
    if (!(self->functions & OB_CLIENT_FUNC_CLOSE)) return;

    /* in the case that the client provides no means to requesting that it
       close, we just kill it */
    if (!self->delete_window)
        /* don't use client_kill(), we should only kill based on PID in
           response to a lack of PING replies */
        XKillClient(ob_display, self->window);
    else if (self->not_responding)
        client_kill(self);
    else {
        PROP_MSG_TO(self->window, self->window, wm_protocols,
                    prop_atoms.wm_delete_window, event_curtime, 0, 0, 0,
                    NoEventMask);

        if (self->ping) {
            /* may have tried to kill it earlier but the window is still
               around and started responding again */
            self->kill_tried_term = FALSE;

            ping_start(self, client_ping_event);
        }
    }
}

void client_kill(ObClient *self)
{
    if (!self->client_machine && self->pid) {
        /* running on the local host */
        if (!self->kill_tried_term) {
            kill(self->pid, SIGTERM);
            self->kill_tried_term = TRUE;
        }
        else
            kill(self->pid, SIGKILL); /* kill -9 */
    }
    else
        XKillClient(ob_display, self->window);
}

void client_hilite(ObClient *self, gboolean hilite)
{
    if (self->demands_attention == hilite)
        return; /* no change */

    /* don't allow focused windows to hilite */
    self->demands_attention = hilite && !client_focused(self);
    if (self->frame != NULL) { /* if we're mapping, just set the state */
        if (self->demands_attention)
            frame_flash_start(self->frame);
        else
            frame_flash_stop(self->frame);
        client_change_state(self);
    }
}

void client_set_desktop_recursive(ObClient *self,
                                  guint target,
                                  gboolean donthide,
                                  gboolean dontraise)
{
    guint old;
    GSList *it;

    if (target != self->desktop && self->type != OB_CLIENT_TYPE_DESKTOP) {

        ob_debug("Setting desktop %u\n", target+1);

        g_assert(target < screen_num_desktops || target == DESKTOP_ALL);

        old = self->desktop;
        self->desktop = target;
        PROP_SET32(self->window, net_wm_desktop, cardinal, target);
        /* the frame can display the current desktop state */
        frame_adjust_state(self->frame);
        /* 'move' the window to the new desktop */
        if (!donthide)
            client_hide(self);
        client_show(self);
        /* raise if it was not already on the desktop */
        if (old != DESKTOP_ALL && !dontraise)
            stacking_raise(CLIENT_AS_WINDOW(self));
        if (STRUT_EXISTS(self->strut))
            screen_update_areas();
        else
            /* the new desktop's geometry may be different, so we may need to
               resize, for example if we are maximized */
            client_reconfigure(self, FALSE);
    }

    /* move all transients */
    for (it = self->transients; it; it = g_slist_next(it))
        if (it->data != self)
            if (client_is_direct_child(self, it->data))
                client_set_desktop_recursive(it->data, target,
                                             donthide, dontraise);
}

void client_set_desktop(ObClient *self, guint target,
                        gboolean donthide, gboolean dontraise)
{
    self = client_search_top_direct_parent(self);
    client_set_desktop_recursive(self, target, donthide, dontraise);
}

gboolean client_is_direct_child(ObClient *parent, ObClient *child)
{
    while (child != parent && (child = client_direct_parent(child)));
    return child == parent;
}

ObClient *client_search_modal_child(ObClient *self)
{
    GSList *it;
    ObClient *ret;

    for (it = self->transients; it; it = g_slist_next(it)) {
        ObClient *c = it->data;
        if ((ret = client_search_modal_child(c))) return ret;
        if (c->modal) return c;
    }
    return NULL;
}

gboolean client_validate(ObClient *self)
{
    XEvent e;

    XSync(ob_display, FALSE); /* get all events on the server */

    if (XCheckTypedWindowEvent(ob_display, self->window, DestroyNotify, &e) ||
        XCheckTypedWindowEvent(ob_display, self->window, UnmapNotify, &e)) {
        XPutBackEvent(ob_display, &e);
        return FALSE;
    }

    return TRUE;
}

void client_set_wm_state(ObClient *self, glong state)
{
    if (state == self->wmstate) return; /* no change */

    switch (state) {
    case IconicState:
        client_iconify(self, TRUE, TRUE, FALSE);
        break;
    case NormalState:
        client_iconify(self, FALSE, TRUE, FALSE);
        break;
    }
}

void client_set_state(ObClient *self, Atom action, glong data1, glong data2)
{
    gboolean shaded = self->shaded;
    gboolean fullscreen = self->fullscreen;
    gboolean undecorated = self->undecorated;
    gboolean max_horz = self->max_horz;
    gboolean max_vert = self->max_vert;
    gboolean modal = self->modal;
    gboolean iconic = self->iconic;
    gboolean demands_attention = self->demands_attention;
    gboolean above = self->above;
    gboolean below = self->below;
    gint i;

    if (!(action == prop_atoms.net_wm_state_add ||
          action == prop_atoms.net_wm_state_remove ||
          action == prop_atoms.net_wm_state_toggle))
        /* an invalid action was passed to the client message, ignore it */
        return;

    for (i = 0; i < 2; ++i) {
        Atom state = i == 0 ? data1 : data2;

        if (!state) continue;

        /* if toggling, then pick whether we're adding or removing */
        if (action == prop_atoms.net_wm_state_toggle) {
            if (state == prop_atoms.net_wm_state_modal)
                action = modal ? prop_atoms.net_wm_state_remove :
                    prop_atoms.net_wm_state_add;
            else if (state == prop_atoms.net_wm_state_maximized_vert)
                action = self->max_vert ? prop_atoms.net_wm_state_remove :
                    prop_atoms.net_wm_state_add;
            else if (state == prop_atoms.net_wm_state_maximized_horz)
                action = self->max_horz ? prop_atoms.net_wm_state_remove :
                    prop_atoms.net_wm_state_add;
            else if (state == prop_atoms.net_wm_state_shaded)
                action = shaded ? prop_atoms.net_wm_state_remove :
                    prop_atoms.net_wm_state_add;
            else if (state == prop_atoms.net_wm_state_skip_taskbar)
                action = self->skip_taskbar ?
                    prop_atoms.net_wm_state_remove :
                    prop_atoms.net_wm_state_add;
            else if (state == prop_atoms.net_wm_state_skip_pager)
                action = self->skip_pager ?
                    prop_atoms.net_wm_state_remove :
                    prop_atoms.net_wm_state_add;
            else if (state == prop_atoms.net_wm_state_hidden)
                action = self->iconic ?
                    prop_atoms.net_wm_state_remove :
                    prop_atoms.net_wm_state_add;
            else if (state == prop_atoms.net_wm_state_fullscreen)
                action = fullscreen ?
                    prop_atoms.net_wm_state_remove :
                    prop_atoms.net_wm_state_add;
            else if (state == prop_atoms.net_wm_state_above)
                action = self->above ? prop_atoms.net_wm_state_remove :
                    prop_atoms.net_wm_state_add;
            else if (state == prop_atoms.net_wm_state_below)
                action = self->below ? prop_atoms.net_wm_state_remove :
                    prop_atoms.net_wm_state_add;
            else if (state == prop_atoms.net_wm_state_demands_attention)
                action = self->demands_attention ?
                    prop_atoms.net_wm_state_remove :
                    prop_atoms.net_wm_state_add;
            else if (state == prop_atoms.ob_wm_state_undecorated)
                action = undecorated ? prop_atoms.net_wm_state_remove :
                    prop_atoms.net_wm_state_add;
        }

        if (action == prop_atoms.net_wm_state_add) {
            if (state == prop_atoms.net_wm_state_modal) {
                modal = TRUE;
            } else if (state == prop_atoms.net_wm_state_maximized_vert) {
                max_vert = TRUE;
            } else if (state == prop_atoms.net_wm_state_maximized_horz) {
                max_horz = TRUE;
            } else if (state == prop_atoms.net_wm_state_shaded) {
                shaded = TRUE;
            } else if (state == prop_atoms.net_wm_state_skip_taskbar) {
                self->skip_taskbar = TRUE;
            } else if (state == prop_atoms.net_wm_state_skip_pager) {
                self->skip_pager = TRUE;
            } else if (state == prop_atoms.net_wm_state_hidden) {
                iconic = TRUE;
            } else if (state == prop_atoms.net_wm_state_fullscreen) {
                fullscreen = TRUE;
            } else if (state == prop_atoms.net_wm_state_above) {
                above = TRUE;
                below = FALSE;
            } else if (state == prop_atoms.net_wm_state_below) {
                above = FALSE;
                below = TRUE;
            } else if (state == prop_atoms.net_wm_state_demands_attention) {
                demands_attention = TRUE;
            } else if (state == prop_atoms.ob_wm_state_undecorated) {
                undecorated = TRUE;
            }

        } else { /* action == prop_atoms.net_wm_state_remove */
            if (state == prop_atoms.net_wm_state_modal) {
                modal = FALSE;
            } else if (state == prop_atoms.net_wm_state_maximized_vert) {
                max_vert = FALSE;
            } else if (state == prop_atoms.net_wm_state_maximized_horz) {
                max_horz = FALSE;
            } else if (state == prop_atoms.net_wm_state_shaded) {
                shaded = FALSE;
            } else if (state == prop_atoms.net_wm_state_skip_taskbar) {
                self->skip_taskbar = FALSE;
            } else if (state == prop_atoms.net_wm_state_skip_pager) {
                self->skip_pager = FALSE;
            } else if (state == prop_atoms.net_wm_state_hidden) {
                iconic = FALSE;
            } else if (state == prop_atoms.net_wm_state_fullscreen) {
                fullscreen = FALSE;
            } else if (state == prop_atoms.net_wm_state_above) {
                above = FALSE;
            } else if (state == prop_atoms.net_wm_state_below) {
                below = FALSE;
            } else if (state == prop_atoms.net_wm_state_demands_attention) {
                demands_attention = FALSE;
            } else if (state == prop_atoms.ob_wm_state_undecorated) {
                undecorated = FALSE;
            }
        }
    }

    if (max_horz != self->max_horz || max_vert != self->max_vert) {
        if (max_horz != self->max_horz && max_vert != self->max_vert) {
            /* toggling both */
            if (max_horz == max_vert) { /* both going the same way */
                client_maximize(self, max_horz, 0);
            } else {
                client_maximize(self, max_horz, 1);
                client_maximize(self, max_vert, 2);
            }
        } else {
            /* toggling one */
            if (max_horz != self->max_horz)
                client_maximize(self, max_horz, 1);
            else
                client_maximize(self, max_vert, 2);
        }
    }
    /* change fullscreen state before shading, as it will affect if the window
       can shade or not */
    if (fullscreen != self->fullscreen)
        client_fullscreen(self, fullscreen);
    if (shaded != self->shaded)
        client_shade(self, shaded);
    if (undecorated != self->undecorated)
        client_set_undecorated(self, undecorated);
    if (above != self->above || below != self->below) {
        self->above = above;
        self->below = below;
        client_calc_layer(self);
    }

    if (modal != self->modal) {
        self->modal = modal;
        /* when a window changes modality, then its stacking order with its
           transients needs to change */
        stacking_raise(CLIENT_AS_WINDOW(self));

        /* it also may get focused. if something is focused that shouldn't
           be focused anymore, then move the focus */
        if (focus_client && client_focus_target(focus_client) != focus_client)
            client_focus(focus_client);
    }

    if (iconic != self->iconic)
        client_iconify(self, iconic, FALSE, FALSE);

    if (demands_attention != self->demands_attention)
        client_hilite(self, demands_attention);

    client_change_state(self); /* change the hint to reflect these changes */
}

ObClient *client_focus_target(ObClient *self)
{
    ObClient *child = NULL;

    child = client_search_modal_child(self);
    if (child) return child;
    return self;
}

gboolean client_can_focus(ObClient *self)
{
    /* choose the correct target */
    self = client_focus_target(self);

    if (!self->frame->visible)
        return FALSE;

    if (!(self->can_focus || self->focus_notify))
        return FALSE;

    return TRUE;
}

gboolean client_focus(ObClient *self)
{
    /* we might not focus this window, so if we have modal children which would
       be focused instead, bring them to this desktop */
    client_bring_modal_windows(self);

    /* choose the correct target */
    self = client_focus_target(self);

    if (!client_can_focus(self)) {
        ob_debug_type(OB_DEBUG_FOCUS,
                      "Client %s can't be focused\n", self->title);
        return FALSE;
    }

    ob_debug_type(OB_DEBUG_FOCUS,
                  "Focusing client \"%s\" (0x%x) at time %u\n",
                  self->title, self->window, event_curtime);

    /* if using focus_delay, stop the timer now so that focus doesn't
       go moving on us */
    event_halt_focus_delay();

    /* if there is a grab going on, then we need to cancel it. if we move
       focus during the grab, applications will get NotifyWhileGrabbed events
       and ignore them !

       actions should not rely on being able to move focus during an
       interactive grab.
    */
    event_cancel_all_key_grabs();

    xerror_set_ignore(TRUE);
    xerror_occured = FALSE;

    if (self->can_focus) {
        /* This can cause a BadMatch error with CurrentTime, or if an app
           passed in a bad time for _NET_WM_ACTIVE_WINDOW. */
        XSetInputFocus(ob_display, self->window, RevertToPointerRoot,
                       event_curtime);
    }

    if (self->focus_notify) {
        XEvent ce;
        ce.xclient.type = ClientMessage;
        ce.xclient.message_type = prop_atoms.wm_protocols;
        ce.xclient.display = ob_display;
        ce.xclient.window = self->window;
        ce.xclient.format = 32;
        ce.xclient.data.l[0] = prop_atoms.wm_take_focus;
        ce.xclient.data.l[1] = event_curtime;
        ce.xclient.data.l[2] = 0l;
        ce.xclient.data.l[3] = 0l;
        ce.xclient.data.l[4] = 0l;
        XSendEvent(ob_display, self->window, FALSE, NoEventMask, &ce);
    }

    xerror_set_ignore(FALSE);

    ob_debug_type(OB_DEBUG_FOCUS, "Error focusing? %d\n", xerror_occured);
    return !xerror_occured;
}

static void client_present(ObClient *self, gboolean here, gboolean raise,
                           gboolean unshade)
{
    if (client_normal(self) && screen_showing_desktop)
        screen_show_desktop(FALSE, self);
    if (self->iconic)
        client_iconify(self, FALSE, here, FALSE);
    if (self->desktop != DESKTOP_ALL &&
        self->desktop != screen_desktop)
    {
        if (here)
            client_set_desktop(self, screen_desktop, FALSE, TRUE);
        else
            screen_set_desktop(self->desktop, FALSE);
    } else if (!self->frame->visible)
        /* if its not visible for other reasons, then don't mess
           with it */
        return;
    if (self->shaded && unshade)
        client_shade(self, FALSE);
    if (raise)
        stacking_raise(CLIENT_AS_WINDOW(self));

    client_focus(self);
}

void client_activate(ObClient *self, gboolean here, gboolean raise,
                     gboolean unshade, gboolean user)
{
    client_present(self, here, raise, unshade);
}

static void client_bring_windows_recursive(ObClient *self,
                                           guint desktop,
                                           gboolean helpers,
                                           gboolean modals,
                                           gboolean iconic)
{
    GSList *it;

    for (it = self->transients; it; it = g_slist_next(it))
        client_bring_windows_recursive(it->data, desktop,
                                       helpers, modals, iconic);

    if (((helpers && client_helper(self)) ||
         (modals && self->modal)) &&
        ((self->desktop != desktop && self->desktop != DESKTOP_ALL) ||
         (iconic && self->iconic)))
    {
        if (iconic && self->iconic)
            client_iconify(self, FALSE, TRUE, FALSE);
        else
            client_set_desktop(self, desktop, FALSE, FALSE);
    }
}

void client_bring_helper_windows(ObClient *self)
{
    client_bring_windows_recursive(self, self->desktop, TRUE, FALSE, FALSE);
}

void client_bring_modal_windows(ObClient *self)
{
    client_bring_windows_recursive(self, self->desktop, FALSE, TRUE, TRUE);
}

gboolean client_focused(ObClient *self)
{
    return self == focus_client;
}

static ObClientIcon* client_icon_recursive(ObClient *self, gint w, gint h)
{
    guint i;
    gulong min_diff, min_i;

    if (!self->nicons) {
        ObClientIcon *parent = NULL;
        GSList *it;

        for (it = self->parents; it; it = g_slist_next(it)) {
            ObClient *c = it->data;
            if ((parent = client_icon_recursive(c, w, h)))
                break;
        }

        return parent;
    }

    /* some kind of crappy approximation to find the icon closest in size to
       what we requested, but icons are generally all the same ratio as
       eachother so it's good enough. */

    min_diff = ABS(self->icons[0].width - w) + ABS(self->icons[0].height - h);
    min_i = 0;

    for (i = 1; i < self->nicons; ++i) {
        gulong diff;

        diff = ABS(self->icons[i].width - w) + ABS(self->icons[i].height - h);
        if (diff < min_diff) {
            min_diff = diff;
            min_i = i;
        }
    }
    return &self->icons[min_i];
}

const ObClientIcon* client_icon(ObClient *self, gint w, gint h)
{
    ObClientIcon *ret;
    static ObClientIcon deficon;

    if (!(ret = client_icon_recursive(self, w, h))) {
        deficon.width = deficon.height = 48;
        deficon.data = ob_rr_theme->def_win_icon;
        ret = &deficon;
    }
    return ret;
}

void client_set_layer(ObClient *self, gint layer)
{
    if (layer < 0) {
        self->below = TRUE;
        self->above = FALSE;
    } else if (layer == 0) {
        self->below = self->above = FALSE;
    } else {
        self->below = FALSE;
        self->above = TRUE;
    }
    client_calc_layer(self);
    client_change_state(self); /* reflect this in the state hints */
}

void client_set_undecorated(ObClient *self, gboolean undecorated)
{
    if (self->undecorated != undecorated &&
        /* don't let it undecorate if the function is missing, but let
           it redecorate */
        (self->functions & OB_CLIENT_FUNC_UNDECORATE || !undecorated))
    {
        self->undecorated = undecorated;
        client_setup_decor_and_functions(self, TRUE);
        client_change_state(self); /* reflect this in the state hints */
    }
}

guint client_monitor(ObClient *self)
{
    return screen_find_monitor(&self->frame->area);
}

ObClient *client_direct_parent(ObClient *self)
{
    if (!self->parents) return NULL;
    if (self->transient_for_group) return NULL;
    return self->parents->data;
}

ObClient *client_search_top_direct_parent(ObClient *self)
{
    ObClient *p;
    while ((p = client_direct_parent(self))) self = p;
    return self;
}

static GSList *client_search_all_top_parents_internal(ObClient *self,
                                                      gboolean bylayer,
                                                      ObStackingLayer layer)
{
    GSList *ret;
    ObClient *p;

    /* move up the direct transient chain as far as possible */
    while ((p = client_direct_parent(self)) &&
           (!bylayer || p->layer == layer))
        self = p;

    if (!self->parents)
        ret = g_slist_prepend(NULL, self);
    else
        ret = g_slist_copy(self->parents);

    return ret;
}

GSList *client_search_all_top_parents(ObClient *self)
{
    return client_search_all_top_parents_internal(self, FALSE, 0);
}

GSList *client_search_all_top_parents_layer(ObClient *self)
{
    return client_search_all_top_parents_internal(self, TRUE, self->layer);
}

ObClient *client_search_focus_parent(ObClient *self)
{
    GSList *it;

    for (it = self->parents; it; it = g_slist_next(it))
        if (client_focused(it->data)) return it->data;

    return NULL;
}

ObClient *client_search_parent(ObClient *self, ObClient *search)
{
    GSList *it;

    for (it = self->parents; it; it = g_slist_next(it))
        if (it->data == search) return search;

    return NULL;
}

ObClient *client_search_transient(ObClient *self, ObClient *search)
{
    GSList *sit;

    for (sit = self->transients; sit; sit = g_slist_next(sit)) {
        if (sit->data == search)
            return search;
        if (client_search_transient(sit->data, search))
            return search;
    }
    return NULL;
}

static void detect_edge(Rect area, ObDirection dir,
                        gint my_head, gint my_size,
                        gint my_edge_start, gint my_edge_size,
                        gint *dest, gboolean *near_edge)
{
    gint edge_start, edge_size, head, tail;
    gboolean skip_head = FALSE, skip_tail = FALSE;

    switch (dir) {
        case OB_DIRECTION_NORTH:
        case OB_DIRECTION_SOUTH:
            edge_start = area.x;
            edge_size = area.width;
            break;
        case OB_DIRECTION_EAST:
        case OB_DIRECTION_WEST:
            edge_start = area.y;
            edge_size = area.height;
            break;
        default:
            g_assert_not_reached();
    }

    /* do we collide with this window? */
    if (!RANGES_INTERSECT(my_edge_start, my_edge_size,
                edge_start, edge_size))
        return;

    switch (dir) {
        case OB_DIRECTION_NORTH:
            head = RECT_BOTTOM(area);
            tail = RECT_TOP(area);
            break;
        case OB_DIRECTION_SOUTH:
            head = RECT_TOP(area);
            tail = RECT_BOTTOM(area);
            break;
        case OB_DIRECTION_WEST:
            head = RECT_RIGHT(area);
            tail = RECT_LEFT(area);
            break;
        case OB_DIRECTION_EAST:
            head = RECT_LEFT(area);
            tail = RECT_RIGHT(area);
            break;
        default:
            g_assert_not_reached();
    }
    switch (dir) {
        case OB_DIRECTION_NORTH:
        case OB_DIRECTION_WEST:
            /* check if our window is past the head of this window */
            if (my_head <= head + 1)
                skip_head = TRUE;
            /* check if our window's tail is past the tail of this window */
            if (my_head + my_size - 1 <= tail)
                skip_tail = TRUE;
            /* check if the head of this window is closer than the previously
               chosen edge (take into account that the previously chosen
               edge might have been a tail, not a head) */
            if (head + (*near_edge ? 0 : my_size) < *dest)
                skip_head = TRUE;
            /* check if the tail of this window is closer than the previously
               chosen edge (take into account that the previously chosen
               edge might have been a head, not a tail) */
            if (tail - (!*near_edge ? 0 : my_size) < *dest)
                skip_tail = TRUE;
            break;
        case OB_DIRECTION_SOUTH:
        case OB_DIRECTION_EAST:
            /* check if our window is past the head of this window */
            if (my_head >= head - 1)
                skip_head = TRUE;
            /* check if our window's tail is past the tail of this window */
            if (my_head - my_size + 1 >= tail)
                skip_tail = TRUE;
            /* check if the head of this window is closer than the previously
               chosen edge (take into account that the previously chosen
               edge might have been a tail, not a head) */
            if (head - (*near_edge ? 0 : my_size) > *dest)
                skip_head = TRUE;
            /* check if the tail of this window is closer than the previously
               chosen edge (take into account that the previously chosen
               edge might have been a head, not a tail) */
            if (tail + (!*near_edge ? 0 : my_size) > *dest)
                skip_tail = TRUE;
            break;
        default:
            g_assert_not_reached();
    }

    ob_debug("my head %d size %d\n", my_head, my_size);
    ob_debug("head %d tail %d deest %d\n", head, tail, *dest);
    if (!skip_head) {
        ob_debug("using near edge %d\n", head);
        *dest = head;
        *near_edge = TRUE;
    }
    else if (!skip_tail) {
        ob_debug("using far edge %d\n", tail);
        *dest = tail;
        *near_edge = FALSE;
    }
}

void client_find_edge_directional(ObClient *self, ObDirection dir,
                                  gint my_head, gint my_size,
                                  gint my_edge_start, gint my_edge_size,
                                  gint *dest, gboolean *near_edge)
{
    GList *it;
    Rect *a, *mon;
    Rect dock_area;
    gint edge;

    a = screen_area(self->desktop, SCREEN_AREA_ALL_MONITORS,
                    &self->frame->area);
    mon = screen_area(self->desktop, SCREEN_AREA_ONE_MONITOR,
                      &self->frame->area);

    switch (dir) {
    case OB_DIRECTION_NORTH:
        if (my_head >= RECT_TOP(*mon) + 1)
            edge = RECT_TOP(*mon) - 1;
        else
            edge = RECT_TOP(*a) - 1;
        break;
    case OB_DIRECTION_SOUTH:
        if (my_head <= RECT_BOTTOM(*mon) - 1)
            edge = RECT_BOTTOM(*mon) + 1;
        else
            edge = RECT_BOTTOM(*a) + 1;
        break;
    case OB_DIRECTION_EAST:
        if (my_head <= RECT_RIGHT(*mon) - 1)
            edge = RECT_RIGHT(*mon) + 1;
        else
            edge = RECT_RIGHT(*a) + 1;
        break;
    case OB_DIRECTION_WEST:
        if (my_head >= RECT_LEFT(*mon) + 1)
            edge = RECT_LEFT(*mon) - 1;
        else
            edge = RECT_LEFT(*a) - 1;
        break;
    default:
        g_assert_not_reached();
    }
    /* default to the far edge, then narrow it down */
    *dest = edge;
    *near_edge = TRUE;

    for (it = client_list; it; it = g_list_next(it)) {
        ObClient *cur = it->data;

        /* skip windows to not bump into */
        if (cur == self)
            continue;
        if (cur->iconic)
            continue;
        if (self->desktop != cur->desktop && cur->desktop != DESKTOP_ALL &&
            cur->desktop != screen_desktop)
            continue;

        ob_debug("trying window %s\n", cur->title);

        detect_edge(cur->frame->area, dir, my_head, my_size, my_edge_start,
                    my_edge_size, dest, near_edge);
    }
    dock_get_area(&dock_area);
    detect_edge(dock_area, dir, my_head, my_size, my_edge_start,
                my_edge_size, dest, near_edge);
    g_free(a);
    g_free(mon);
}

void client_find_move_directional(ObClient *self, ObDirection dir,
                                  gint *x, gint *y)
{
    gint head, size;
    gint e, e_start, e_size;
    gboolean near;

    switch (dir) {
    case OB_DIRECTION_EAST:
        head = RECT_RIGHT(self->frame->area);
        size = self->frame->area.width;
        e_start = RECT_TOP(self->frame->area);
        e_size = self->frame->area.height;
        break;
    case OB_DIRECTION_WEST:
        head = RECT_LEFT(self->frame->area);
        size = self->frame->area.width;
        e_start = RECT_TOP(self->frame->area);
        e_size = self->frame->area.height;
        break;
    case OB_DIRECTION_NORTH:
        head = RECT_TOP(self->frame->area);
        size = self->frame->area.height;
        e_start = RECT_LEFT(self->frame->area);
        e_size = self->frame->area.width;
        break;
    case OB_DIRECTION_SOUTH:
        head = RECT_BOTTOM(self->frame->area);
        size = self->frame->area.height;
        e_start = RECT_LEFT(self->frame->area);
        e_size = self->frame->area.width;
        break;
    default:
        g_assert_not_reached();
    }

    client_find_edge_directional(self, dir, head, size,
                                 e_start, e_size, &e, &near);
    *x = self->frame->area.x;
    *y = self->frame->area.y;
    switch (dir) {
    case OB_DIRECTION_EAST:
        if (near) e -= self->frame->area.width;
        else      e++;
        *x = e;
        break;
    case OB_DIRECTION_WEST:
        if (near) e++;
        else      e -= self->frame->area.width;
        *x = e;
        break;
    case OB_DIRECTION_NORTH:
        if (near) e++;
        else      e -= self->frame->area.height;
        *y = e;
        break;
    case OB_DIRECTION_SOUTH:
        if (near) e -= self->frame->area.height;
        else      e++;
        *y = e;
        break;
    default:
        g_assert_not_reached();
    }
    frame_frame_gravity(self->frame, x, y);
}

void client_find_resize_directional(ObClient *self, ObDirection side,
                                    gboolean grow,
                                    gint *x, gint *y, gint *w, gint *h)
{
    gint head;
    gint e, e_start, e_size, delta;
    gboolean near;
    ObDirection dir;

    switch (side) {
    case OB_DIRECTION_EAST:
        head = RECT_RIGHT(self->frame->area) +
            (self->size_inc.width - 1) * (grow ? 1 : -1);
        e_start = RECT_TOP(self->frame->area);
        e_size = self->frame->area.height;
        dir = grow ? OB_DIRECTION_EAST : OB_DIRECTION_WEST;
        break;
    case OB_DIRECTION_WEST:
        head = RECT_LEFT(self->frame->area) -
            (self->size_inc.width - 1) * (grow ? 1 : -1);
        e_start = RECT_TOP(self->frame->area);
        e_size = self->frame->area.height;
        dir = grow ? OB_DIRECTION_WEST : OB_DIRECTION_EAST;
        break;
    case OB_DIRECTION_NORTH:
        head = RECT_TOP(self->frame->area) -
            (self->size_inc.height - 1) * (grow ? 1 : -1);
        e_start = RECT_LEFT(self->frame->area);
        e_size = self->frame->area.width;
        dir = grow ? OB_DIRECTION_NORTH : OB_DIRECTION_SOUTH;
        break;
    case OB_DIRECTION_SOUTH:
        head = RECT_BOTTOM(self->frame->area) +
            (self->size_inc.height - 1) * (grow ? 1 : -1);
        e_start = RECT_LEFT(self->frame->area);
        e_size = self->frame->area.width;
        dir = grow ? OB_DIRECTION_SOUTH : OB_DIRECTION_NORTH;
        break;
    default:
        g_assert_not_reached();
    }

    ob_debug("head %d dir %d\n", head, dir);
    client_find_edge_directional(self, dir, head, 1,
                                 e_start, e_size, &e, &near);
    ob_debug("edge %d\n", e);
    *x = self->frame->area.x;
    *y = self->frame->area.y;
    *w = self->frame->area.width;
    *h = self->frame->area.height;
    switch (side) {
    case OB_DIRECTION_EAST:
        if (grow == near) --e;
        delta = e - RECT_RIGHT(self->frame->area);
        *w += delta;
        break;
    case OB_DIRECTION_WEST:
        if (grow == near) ++e;
        delta = RECT_LEFT(self->frame->area) - e;
        *x -= delta;
        *w += delta;
        break;
    case OB_DIRECTION_NORTH:
        if (grow == near) ++e;
        delta = RECT_TOP(self->frame->area) - e;
        *y -= delta;
        *h += delta;
        break;
    case OB_DIRECTION_SOUTH:
        if (grow == near) --e;
        delta = e - RECT_BOTTOM(self->frame->area);
        *h += delta;
        break;
    default:
        g_assert_not_reached();
    }
    frame_frame_gravity(self->frame, x, y);
    *w -= self->frame->size.left + self->frame->size.right;
    *h -= self->frame->size.top + self->frame->size.bottom;
}

ObClient* client_under_pointer(void)
{
    gint x, y;
    GList *it;
    ObClient *ret = NULL;

    if (screen_pointer_pos(&x, &y)) {
        for (it = stacking_list; it; it = g_list_next(it)) {
            if (WINDOW_IS_CLIENT(it->data)) {
                ObClient *c = WINDOW_AS_CLIENT(it->data);
                if (c->frame->visible &&
                    /* check the desktop, this is done during desktop
                       switching and windows are shown/hidden status is not
                       reliable */
                    (c->desktop == screen_desktop ||
                     c->desktop == DESKTOP_ALL) &&
                    /* ignore all animating windows */
                    !frame_iconify_animating(c->frame) &&
                    RECT_CONTAINS(c->frame->area, x, y))
                {
                    ret = c;
                    break;
                }
            }
        }
    }
    return ret;
}

gboolean client_has_group_siblings(ObClient *self)
{
    return self->group && self->group->members->next;
}
