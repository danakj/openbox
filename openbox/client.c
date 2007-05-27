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
#include "propwin.h"
#include "stacking.h"
#include "openbox.h"
#include "group.h"
#include "config.h"
#include "menuframe.h"
#include "keyboard.h"
#include "mouse.h"
#include "render/render.h"

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
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
static void client_apply_startup_state(ObClient *self);
static void client_restore_session_state(ObClient *self);
static gboolean client_restore_session_stacking(ObClient *self);
static ObAppSettings *client_get_settings_state(ObClient *self);
static void client_update_transient_tree(ObClient *self,
                                         ObGroup *oldgroup, ObGroup *newgroup,
                                         ObClient* oldparent,
                                         ObClient *newparent);
static void client_present(ObClient *self, gboolean here, gboolean raise);
static GSList *client_search_all_top_parents_internal(ObClient *self,
                                                      gboolean bylayer,
                                                      ObStackingLayer layer);
static void client_call_notifies(ObClient *self, GSList *list);

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

void client_set_list()
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

/*
  void client_foreach_transient(ObClient *self, ObClientForeachFunc func, gpointer data)
  {
  GSList *it;

  for (it = self->transients; it; it = g_slist_next(it)) {
  if (!func(it->data, data)) return;
  client_foreach_transient(it->data, func, data);
  }
  }

  void client_foreach_ancestor(ObClient *self, ObClientForeachFunc func, gpointer data)
  {
  if (self->transient_for) {
  if (self->transient_for != OB_TRAN_GROUP) {
  if (!func(self->transient_for, data)) return;
  client_foreach_ancestor(self->transient_for, func, data);
  } else {
  GSList *it;

  for (it = self->group->members; it; it = g_slist_next(it))
  if (it->data != self &&
  !((ObClient*)it->data)->transient_for) {
  if (!func(it->data, data)) return;
  client_foreach_ancestor(it->data, func, data);
  }
  }
  }
  }
*/

void client_manage_all()
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
    gint placex, placey;

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

    ob_debug("Managing window: %lx\n", window);

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
    self->user_time = focus_client ? focus_client->user_time : CurrentTime;

    /* get all the stuff off the window */
    client_get_all(self, TRUE);

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
    /* the session should get the last say thought */
    client_restore_session_state(self);

    /* now we have all of the window's information so we can set this up */
    client_setup_decor_and_functions(self);

    {
        Time t = sn_app_started(self->startup_id, self->class);
        if (t) self->user_time = t;
    }

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
        !self->iconic &&
        /* this means focus=true for window is same as config_focus_new=true */
        ((config_focus_new || (settings && settings->focus == 1)) ||
         client_search_focus_tree_full(self)) &&
        /* this checks for focus=false for the window */
        (!settings || settings->focus != 0) &&
        /* note the check against Type_Normal/Dialog, not client_normal(self),
           which would also include other types. in this case we want more
           strict rules for focus */
        (self->type == OB_CLIENT_TYPE_NORMAL ||
         self->type == OB_CLIENT_TYPE_DIALOG))
    {
        activate = TRUE;
    }

    /* adjust the frame to the client's size before showing or placing
       the window */
    frame_adjust_area(self->frame, FALSE, TRUE, FALSE);
    frame_adjust_client_area(self->frame);

    /* where the frame was placed is where the window was originally */
    placex = self->area.x;
    placey = self->area.y;

    /* figure out placement for the window if the window is new */
    if (ob_state() == OB_STATE_RUNNING) {
        gboolean transient;

        ob_debug("Positioned: %s @ %d %d\n",
                 (!self->positioned ? "no" :
                  (self->positioned == PPosition ? "program specified" :
                   (self->positioned == USPosition ? "user specified" :
                    (self->positioned == (PPosition | USPosition) ?
                     "program + user specified" :
                     "BADNESS !?")))), self->area.x, self->area.y);

        ob_debug("Sized: %s @ %d %d\n",
                 (!self->sized ? "no" :
                  (self->sized == PSize ? "program specified" :
                   (self->sized == USSize ? "user specified" :
                    (self->sized == (PSize | USSize) ?
                     "program + user specified" :
                     "BADNESS !?")))), self->area.width, self->area.height);

        transient = place_client(self, &placex, &placey, settings);

        /* if the window isn't user-positioned, then make it fit inside
           the visible screen area on its monitor.

           the monitor is chosen by place_client! */
        if (!(self->sized & USSize)) {
            /* make a copy to modify */
            Rect a = *screen_area_monitor(self->desktop, client_monitor(self));

            /* shrink by the frame's area */
            a.width -= self->frame->size.left + self->frame->size.right;
            a.height -= self->frame->size.top + self->frame->size.bottom;

            /* fit the window inside the area */
            if (self->area.width > a.width || self->area.height > a.height) {
                self->area.width = MIN(self->area.width, a.width);
                self->area.height = MIN(self->area.height, a.height);

                ob_debug("setting window size to %dx%d\n",
                         self->area.width, self->area.height);

                /* adjust the frame to the client's new size */
                frame_adjust_area(self->frame, FALSE, TRUE, FALSE);
                frame_adjust_client_area(self->frame);
            }
        }

        /* make sure the window is visible. */
        client_find_onscreen(self, &placex, &placey,
                             self->area.width, self->area.height,
                             /* non-normal clients has less rules, and
                                windows that are being restored from a
                                session do also. we can assume you want
                                it back where you saved it. Clients saying
                                they placed themselves are subjected to
                                harder rules, ones that are placed by
                                place.c or by the user are allowed partially
                                off-screen and on xinerama divides (ie,
                                it is up to the placement routines to avoid
                                the xinerama divides) */
                             transient ||
                             (((self->positioned & PPosition) &&
                               !(self->positioned & USPosition)) &&
                              client_normal(self) &&
                              !self->session));
    }

    ob_debug("placing window 0x%x at %d, %d with size %d x %d\n",
             self->window, placex, placey,
             self->area.width, self->area.height);
    if (self->session)
        ob_debug("  but session requested %d %d instead, overriding\n",
                 self->session->x, self->session->y);

    /* do this after the window is placed, so the premax/prefullscreen numbers
       won't be all wacko!!
       also, this moves the window to the position where it has been placed
    */
    client_apply_startup_state(self);

    /* move the client to its placed position, or it it's already there,
       generate a ConfigureNotify telling the client where it is.

       do this after adjusting the frame. otherwise it gets all weird and
       clients don't work right

       also do this after applying the startup state so maximize and fullscreen
       will get the right sizes and positions if the client is starting with
       those states
    */
    client_configure(self, placex, placey,
                     self->area.width, self->area.height,
                     self->border_width,
                     FALSE, TRUE);


    if (activate) {
        guint32 last_time = focus_client ?
            focus_client->user_time : CurrentTime;

        /* This is focus stealing prevention */
        ob_debug_type(OB_DEBUG_FOCUS,
                      "Want to focus new window 0x%x with time %u "
                      "(last time %u)\n",
                      self->window, self->user_time, last_time);

        /* if it's on another desktop */
        if (!(self->desktop == screen_desktop || self->desktop == DESKTOP_ALL)
            && /* the timestamp is from before you changed desktops */
            self->user_time && screen_desktop_user_time &&
            !event_time_after(self->user_time, screen_desktop_user_time))
        {
            activate = FALSE;
            ob_debug_type(OB_DEBUG_FOCUS,
                          "Not focusing the window because its on another "
                          "desktop\n");
        }
        /* If something is focused, and it's not our relative... */
        else if (focus_client && client_search_focus_tree_full(self) == NULL)
        {
            /* If time stamp is old, don't steal focus */
            if (self->user_time && last_time &&
                !event_time_after(self->user_time, last_time))
            {
                activate = FALSE;
                ob_debug_type(OB_DEBUG_FOCUS,
                              "Not focusing the window because the time is "
                              "too old\n");
            }
            /* If its a transient (and parents aren't focused) and the time
               is ambiguous (either the current focus target doesn't have
               a timestamp, or they are the same (we probably inherited it
               from them) */
            else if (self->transient_for != NULL &&
                     (!last_time || self->user_time == last_time))
            {
                activate = FALSE;
                ob_debug_type(OB_DEBUG_FOCUS,
                              "Not focusing the window because it is a "
                              "transient, and the time is very ambiguous\n");
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
                ob_debug_type(OB_DEBUG_FOCUS,
                              "Not focusing the window because another window "
                              "would get the focus anyway\n");
            }
        }

        if (!activate) {
            ob_debug_type(OB_DEBUG_FOCUS,
                          "Focus stealing prevention activated for %s with "
                          "time %u (last time %u)\n",
                          self->title, self->user_time, last_time);
            /* if the client isn't focused, then hilite it so the user
               knows it is there */
            client_hilite(self, TRUE);
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
    client_show(self);

    if (activate) {
        gboolean stacked = client_restore_session_stacking(self);
        client_present(self, FALSE, !stacked);
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
             window, self->frame->plate, self->class);

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

    client_setup_decor_and_functions(self);

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

void client_unmanage_all()
{
    while (client_list != NULL)
        client_unmanage(client_list->data);
}

void client_unmanage(ObClient *self)
{
    guint j;
    GSList *it;

    ob_debug("Unmanaging window: 0x%x plate 0x%x (%s) (%s)\n",
             self->window, self->frame->plate,
             self->class, self->title ? self->title : "");

    g_assert(self != NULL);

    /* we dont want events no more. do this before hiding the frame so we
       don't generate more events */
    XSelectInput(ob_display, self->window, NoEventMask);

    frame_hide(self->frame);
    /* flush to send the hide to the server quickly */
    XFlush(ob_display);

    /* ignore enter events from the unmap so it doesnt mess with the
       focus */
    event_ignore_all_queued_enters();

    mouse_grab_for_client(self, FALSE);

    /* remove the window from our save set */
    XChangeSaveSet(ob_display, self->window, SetModeDelete);

    /* kill the property windows */
    propwin_remove(self->user_time_window, OB_PROPWIN_USER_TIME, self);

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
    if (self->transient_for == OB_TRAN_GROUP) { /* transient of group */
        for (it = self->group->members; it; it = g_slist_next(it))
            if (it->data != self)
                ((ObClient*)it->data)->transients =
                    g_slist_remove(((ObClient*)it->data)->transients,self);
    } else if (self->transient_for) {        /* transient of window */
        self->transient_for->transients =
            g_slist_remove(self->transient_for->transients, self);
    }

    /* tell our transients that we're gone */
    for (it = self->transients; it; it = g_slist_next(it)) {
        if (((ObClient*)it->data)->transient_for != OB_TRAN_GROUP) {
            ((ObClient*)it->data)->transient_for = NULL;
            client_calc_layer(it->data);
        }
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
    Rect *mon_a, *all_a;
    gint ox = *x, oy = *y;
    gboolean rudel = rude, ruder = rude, rudet = rude, rudeb = rude;
    gint fw, fh;
    Rect desired;

    RECT_SET(desired, *x, *y, w, h);
    all_a = screen_area(self->desktop);
    mon_a = screen_area_monitor(self->desktop, screen_find_monitor(&desired));

    /* get where the frame would be */
    frame_client_gravity(self->frame, x, y, w, h);

    /* get the requested size of the window with decorations */
    fw = self->frame->size.left + w + self->frame->size.right;
    fh = self->frame->size.top + h + self->frame->size.bottom;

    /* This makes sure windows aren't entirely outside of the screen so you
       can't see them at all.
       It makes sure 10% of the window is on the screen at least. At don't let
       it move itself off the top of the screen, which would hide the titlebar
       on you. (The user can still do this if they want too, it's only limiting
       the application.

       XXX watch for xinerama dead areas...
    */
    if (client_normal(self)) {
        if (!self->strut.right && *x + fw/10 >= all_a->x + all_a->width - 1)
            *x = all_a->x + all_a->width - fw/10;
        if (!self->strut.bottom && *y + fh/10 >= all_a->y + all_a->height - 1)
            *y = all_a->y + all_a->height - fh/10;
        if (!self->strut.left && *x + fw*9/10 - 1 < all_a->x)
            *x = all_a->x - fw*9/10;
        if (!self->strut.top && *y + fh*9/10 - 1 < all_a->y)
            *y = all_a->y - fw*9/10;
    }

    /* If rudeness wasn't requested, then figure out of the client is currently
       entirely on the screen. If it is, and the position isn't changing by
       request, and it is enlarging, then be rude even though it wasn't
       requested */
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

    /* This here doesn't let windows even a pixel outside the struts/screen.
     * When called from client_manage, programs placing themselves are
     * forced completely onscreen, while things like
     * xterm -geometry resolution-width/2 will work fine. Trying to
     * place it completely offscreen will be handled in the above code.
     * Sorry for this confused comment, i am tired. */
    if (rudel && !self->strut.left && *x < mon_a->x) *x = mon_a->x;
    if (ruder && !self->strut.right && *x + fw > mon_a->x + mon_a->width)
        *x = mon_a->x + MAX(0, mon_a->width - fw);

    if (rudet && !self->strut.top && *y < mon_a->y) *y = mon_a->y;
    if (rudeb && !self->strut.bottom && *y + fh > mon_a->y + mon_a->height)
        *y = mon_a->y + MAX(0, mon_a->height - fh);

    /* get where the client should be */
    frame_frame_gravity(self->frame, x, y, w, h);

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
    if (self->transient_for == NULL)
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
    client_update_user_time_window(self);
    if (!self->user_time_window) /* check if this would have been called */
        client_update_user_time(self);
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

    ob_debug("client area: %d %d  %d %d\n", wattrib.x, wattrib.y,
             wattrib.width, wattrib.height);
}

static void client_get_desktop(ObClient *self)
{
    guint32 d = screen_num_desktops; /* an always-invalid value */

    if (PROP_GET32(self->window, net_wm_desktop, cardinal, &d)) {
        if (d >= screen_num_desktops && d != DESKTOP_ALL)
            self->desktop = screen_num_desktops - 1;
        else
            self->desktop = d;
    } else {
        gboolean trdesk = FALSE;

        if (self->transient_for) {
            if (self->transient_for != OB_TRAN_GROUP) {
                self->desktop = self->transient_for->desktop;
                trdesk = TRUE;
            } else {
                /* if all the group is on one desktop, then open it on the
                   same desktop */
                GSList *it;
                gboolean first = TRUE;
                guint all = screen_num_desktops; /* not a valid value */

                for (it = self->group->members; it; it = g_slist_next(it)) {
                    ObClient *c = it->data;
                    if (c != self) {
                        if (first) {
                            all = c->desktop;
                            first = FALSE;
                        }
                        else if (all != c->desktop)
                            all = screen_num_desktops; /* make it invalid */
                    }
                }
                if (all != screen_num_desktops) {
                    self->desktop = all;
                    trdesk = TRUE;
                }
            }
        }
        if (!trdesk) {
            /* try get from the startup-notification protocol */
            if (sn_get_desktop(self->startup_id, &self->desktop)) {
                if (self->desktop >= screen_num_desktops &&
                    self->desktop != DESKTOP_ALL)
                    self->desktop = screen_num_desktops - 1;
            } else
                /* defaults to the current desktop */
                self->desktop = screen_desktop;
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

            /* THIS IS SO ANNOYING ! ! ! ! Let me explain.... have a seat..

               Setting the transient_for to Root is actually illegal, however
               applications from time have done this to specify transient for
               their group.

               Now you can do that by being a TYPE_DIALOG and not setting
               the transient_for hint at all on your window. But people still
               use Root, and Kwin is very strange in this regard.

               KWin 3.0 will not consider windows with transient_for set to
               Root as transient for their group *UNLESS* they are also modal.
               In that case, it will make them transient for the group. This
               leads to all sorts of weird behavior from KDE apps which are
               only tested in KWin. I'd like to follow their behavior just to
               make this work right with KDE stuff, but that seems wrong.
            */
            if (!target && self->group) {
                /* not transient to a client, see if it is transient for a
                   group */
                if (t == RootWindow(ob_display, ob_screen)) {
                    /* window is a transient for its group! */
                    target = OB_TRAN_GROUP;
                }
            }
        }
    } else if (self->transient && self->group)
        target = OB_TRAN_GROUP;

    client_update_transient_tree(self, self->group, self->group,
                                 self->transient_for, target);
    self->transient_for = target;
                          
}

static void client_update_transient_tree(ObClient *self,
                                         ObGroup *oldgroup, ObGroup *newgroup,
                                         ObClient* oldparent,
                                         ObClient *newparent)
{
    GSList *it, *next;
    ObClient *c;

    /* * *
      Group transient windows are not allowed to have other group
      transient windows as their children.
      * * */


    /* No change has occured */
    if (oldgroup == newgroup && oldparent == newparent) return;

    /** Remove the client from the transient tree wherever it has changed **/

    /* If the window is becoming a direct transient for a window in its group
       then any group transients which were our children and are now becoming
       our parents need to stop being our children.

       Group transients can't be children of group transients already, but
       we could have any number of direct parents above up, any of which could
       be transient for the group, and we need to remove it from our children.
    */
    if (oldparent != newparent &&
        newparent != NULL && newparent != OB_TRAN_GROUP &&
        newgroup != NULL && newgroup == oldgroup)
    {
        ObClient *look = newparent;
        do {
            self->transients = g_slist_remove(self->transients, look);
            look = look->transient_for;
        } while (look != NULL && look != OB_TRAN_GROUP);
    }
            

    /* If the group changed, or if we are just becoming transient for the
       group, then we need to remove any old group transient windows
       from our children. But if we were already transient for the group, then
       other group transients are not our children. */
    if ((oldgroup != newgroup ||
         (newparent == OB_TRAN_GROUP && oldparent != newparent)) &&
        oldgroup != NULL && oldparent != OB_TRAN_GROUP)
    {
        for (it = self->transients; it; it = next) {
            next = g_slist_next(it);
            c = it->data;
            if (c->group == oldgroup)
                self->transients = g_slist_delete_link(self->transients, it);
        }
    }

    /* If we used to be transient for a group and now we are not, or we're
       transient for a new group, then we need to remove ourselves from all
       our ex-parents */
    if (oldparent == OB_TRAN_GROUP && (oldgroup != newgroup ||
                                       oldparent != newparent))
    {
        for (it = oldgroup->members; it; it = g_slist_next(it)) {
            c = it->data;
            if (c != self && (!c->transient_for ||
                              c->transient_for != OB_TRAN_GROUP))
                c->transients = g_slist_remove(c->transients, self);
        }
    }
    /* If we used to be transient for a single window and we are no longer
       transient for it, then we need to remove ourself from its children */
    else if (oldparent != NULL && oldparent != OB_TRAN_GROUP &&
             oldparent != newparent)
        oldparent->transients = g_slist_remove(oldparent->transients, self);


    /** Re-add the client to the transient tree wherever it has changed **/

    /* If we're now transient for a group and we weren't transient for it
       before then we need to add ourselves to all our new parents */
    if (newparent == OB_TRAN_GROUP && (oldgroup != newgroup ||
                                       oldparent != newparent))
    {
        for (it = oldgroup->members; it; it = g_slist_next(it)) {
            c = it->data;
            if (c != self && (!c->transient_for ||
                              c->transient_for != OB_TRAN_GROUP))
                c->transients = g_slist_prepend(c->transients, self);
        }
    }
    /* If we are now transient for a single window which we weren't before,
       we need to add ourselves to its children

       WARNING: Cyclical transient ness is possible if two windows are
       transient for eachother.
    */
    else if (newparent != NULL && newparent != OB_TRAN_GROUP &&
             newparent != oldparent &&
             /* don't make ourself its child if it is already our child */
             !client_is_direct_child(self, newparent))
        newparent->transients = g_slist_prepend(newparent->transients, self);

    /* If the group changed then we need to add any new group transient
       windows to our children. But if we're transient for the group, then
       other group transients are not our children.

       WARNING: Cyclical transient-ness is possible. For e.g. if:
       A is transient for the group
       B is transient for A
       C is transient for B
       A can't be transient for C or we have a cycle
    */
    if (oldgroup != newgroup && newgroup != NULL &&
        newparent != OB_TRAN_GROUP)
    {
        for (it = newgroup->members; it; it = g_slist_next(it)) {
            c = it->data;
            if (c != self && c->transient_for == OB_TRAN_GROUP &&
                /* Don't make it our child if it is already our parent */
                !client_is_direct_child(c, self))
            {
                self->transients = g_slist_prepend(self->transients, c);
            }
        }
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
    }
}

/*! This needs to be followed by a call to client_configure to make
  the changes show */
void client_setup_decor_and_functions(ObClient *self)
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
           functionality, and are the only windows that you can fullscreen */
        self->functions |= OB_CLIENT_FUNC_FULLSCREEN;
        break;

    case OB_CLIENT_TYPE_DIALOG:
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
    if (self->undecorated) {
        if (config_theme_keepborder)
            self->decorations &= OB_FRAME_DECOR_BORDER;
        else
            self->decorations = 0;
    }

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

void client_reconfigure(ObClient *self)
{
    client_configure(self, self->area.x, self->area.y,
                     self->area.width, self->area.height,
                     self->border_width, FALSE, TRUE);
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
                                         self->transient_for,
                                         self->transient_for);

            /* Lastly, being in a group, or not, can change if the window is
               transient for anything.

               The logic for this is:
               self->transient = TRUE always if the window wants to be
               transient for something, even if transient_for was NULL because
               it wasn't in a group before.

               If transient_for was NULL and oldgroup was NULL we can assume
               that when we add the new group, it will become transient for
               something.

               If transient_for was OB_TRAN_GROUP, then it must have already
               had a group. If it is getting a new group, the above call to
               client_update_transient_tree has already taken care of
               everything ! If it is losing all group status then it will
               no longer be transient for anything and that needs to be
               updated.
            */
            if (self->transient &&
                ((self->transient_for == NULL && oldgroup == NULL) ||
                 (self->transient_for == OB_TRAN_GROUP && !self->group)))
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
            const Rect *a;

            got = TRUE;

            /* use the screen's width/height */
            a = screen_physical_area();

            STRUT_PARTIAL_SET(strut,
                              data[0], data[2], data[1], data[3],
                              a->y, a->y + a->height - 1,
                              a->x, a->x + a->width - 1,
                              a->y, a->y + a->height - 1,
                              a->x, a->x + a->width - 1);
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
                self->nicons++;
                self->icons = g_new(ObClientIcon, self->nicons);
                xerror_set_ignore(TRUE);
                if (!RrPixmapToRGBA(ob_rr_inst,
                                    hints->icon_pixmap,
                                    (hints->flags & IconMaskHint ?
                                     hints->icon_mask : None),
                                    &self->icons[self->nicons-1].width,
                                    &self->icons[self->nicons-1].height,
                                    &self->icons[self->nicons-1].data)){
                    g_free(&self->icons[self->nicons-1]);
                    self->nicons--;
                }
                xerror_set_ignore(FALSE);
            }
            XFree(hints);
        }
    }

    /* set the default icon onto the window
       in theory, this could be a race, but if a window doesn't set an icon
       or removes it entirely, it's not very likely it is going to set one
       right away afterwards */
    if (self->nicons == 0) {
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

void client_update_user_time(ObClient *self)
{
    guint32 time;
    gboolean got = FALSE;

    if (self->user_time_window)
        got = PROP_GET32(self->user_time_window,
                         net_wm_user_time, cardinal, &time);
    if (!got)
        got = PROP_GET32(self->window, net_wm_user_time, cardinal, &time);

    if (got) {
        /* we set this every time, not just when it grows, because in practice
           sometimes time goes backwards! (ntpdate.. yay....) so.. if it goes
           backward we don't want all windows to stop focusing. we'll just
           assume noone is setting times older than the last one, cuz that
           would be pretty stupid anyways
        */
        self->user_time = time;

        /*ob_debug("window %s user time %u\n", self->title, time);*/
    }
}

void client_update_user_time_window(ObClient *self)
{
    guint32 w;

    if (!PROP_GET32(self->window, net_wm_user_time_window, window, &w))
        w = None;

    if (w != self->user_time_window) {
        /* remove the old window */
        propwin_remove(self->user_time_window, OB_PROPWIN_USER_TIME, self);
        self->user_time_window = None;

        if (self->group && self->group->leader == w) {
            ob_debug_type(OB_DEBUG_APP_BUGS, "Window is setting its "
                          "_NET_WM_USER_TYPE_WINDOW to its group leader\n");
            /* do it anyways..? */
        }
        else if (w == self->window) {
            ob_debug_type(OB_DEBUG_APP_BUGS, "Window is setting its "
                          "_NET_WM_USER_TIME_WINDOW to itself\n");
            w = None; /* don't do it */
        }

        /* add the new window */
        propwin_add(w, OB_PROPWIN_USER_TIME, self);
        self->user_time_window = w;

        /* and update from it */
        client_update_user_time(self);
    }
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

        gethostname(localhost, 127);
        localhost[127] = '\0';
        if (strcmp(localhost, s) != 0)
            self->client_machine = s;
        else
            g_free(s);
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
    gulong netstate[11];
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
    if (self->transient_for) {
        if (self->transient_for != OB_TRAN_GROUP) {
            return client_search_focus_tree_full(self->transient_for);
        } else {
            GSList *it;
            gboolean recursed = FALSE;
        
            for (it = self->group->members; it; it = g_slist_next(it))
                if (!((ObClient*)it->data)->transient_for) {
                    ObClient *c;
                    if ((c = client_search_focus_tree_full(it->data)))
                        return c;
                    recursed = TRUE;
                }
            if (recursed)
                return NULL;
        }
    }

    /* this function checks the whole tree, the client_search_focus_tree~
       does not, so we need to check this window */
    if (client_focused(self))
        return self;
    return client_search_focus_tree(self);
}

static ObStackingLayer calc_layer(ObClient *self)
{
    ObStackingLayer l;

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
               RECT_EQUAL(self->area,
                          *screen_physical_area_monitor
                          (client_monitor(self))))) &&
             (client_focused(self) || client_search_focus_tree(self)))
        l = OB_STACKING_LAYER_FULLSCREEN;
    else if (self->above) l = OB_STACKING_LAYER_ABOVE;
    else if (self->below) l = OB_STACKING_LAYER_BELOW;
    else l = OB_STACKING_LAYER_NORMAL;

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


static void client_apply_startup_state(ObClient *self)
{
    /* set the desktop hint, to make sure that it always exists */
    PROP_SET32(self->window, net_wm_desktop, cardinal, self->desktop);

    /* these are in a carefully crafted order.. */

    if (self->iconic) {
        self->iconic = FALSE;
        client_iconify(self, TRUE, FALSE, TRUE);
    }
    if (self->fullscreen) {
        self->fullscreen = FALSE;
        client_fullscreen(self, TRUE);
    }
    if (self->undecorated) {
        self->undecorated = FALSE;
        client_set_undecorated(self, TRUE);
    }
    if (self->shaded) {
        self->shaded = FALSE;
        client_shade(self, TRUE);
    }
    if (self->demands_attention) {
        self->demands_attention = FALSE;
        client_hilite(self, TRUE);
    }
  
    if (self->max_vert && self->max_horz) {
        self->max_vert = self->max_horz = FALSE;
        client_maximize(self, TRUE, 0);
    } else if (self->max_vert) {
        self->max_vert = FALSE;
        client_maximize(self, TRUE, 2);
    } else if (self->max_horz) {
        self->max_horz = FALSE;
        client_maximize(self, TRUE, 1);
    }

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
    Rect desired_area = {*x, *y, *w, *h};

    /* make the frame recalculate its dimentions n shit without changing
       anything visible for real, this way the constraints below can work with
       the updated frame dimensions. */
    frame_adjust_area(self->frame, FALSE, TRUE, TRUE);

    /* work within the prefered sizes given by the window */
    if (!(*w == self->area.width && *h == self->area.height)) {
        gint basew, baseh, minw, minh;

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
        *w /= self->size_inc.width;
        *h /= self->size_inc.height;

        /* you cannot resize to nothing */
        if (basew + *w < 1) *w = 1 - basew;
        if (baseh + *h < 1) *h = 1 - baseh;
  
        /* save the logical size */
        *logicalw = self->size_inc.width > 1 ? *w : *w + basew;
        *logicalh = self->size_inc.height > 1 ? *h : *h + baseh;

        *w *= self->size_inc.width;
        *h *= self->size_inc.height;

        *w += basew;
        *h += baseh;

        /* adjust the height to match the width for the aspect ratios.
           for this, min size is not substituted for base size ever. */
        *w -= self->base_size.width;
        *h -= self->base_size.height;

        if (!self->fullscreen) {
            if (self->min_ratio)
                if (*h * self->min_ratio > *w) {
                    *h = (gint)(*w / self->min_ratio);

                    /* you cannot resize to nothing */
                    if (*h < 1) {
                        *h = 1;
                        *w = (gint)(*h * self->min_ratio);
                    }
                }
            if (self->max_ratio)
                if (*h * self->max_ratio < *w) {
                    *h = (gint)(*w / self->max_ratio);

                    /* you cannot resize to nothing */
                    if (*h < 1) {
                        *h = 1;
                        *w = (gint)(*h * self->min_ratio);
                    }
                }
        }

        *w += self->base_size.width;
        *h += self->base_size.height;
    }

    /* gets the frame's position */
    frame_client_gravity(self->frame, x, y, *w, *h);

    /* these positions are frame positions, not client positions */

    /* set the size and position if fullscreen */
    if (self->fullscreen) {
        Rect *a;
        guint i;

        i = screen_find_monitor(&desired_area);
        a = screen_physical_area_monitor(i);

        *x = a->x;
        *y = a->y;
        *w = a->width;
        *h = a->height;

        user = FALSE; /* ignore if the client can't be moved/resized when it
                         is fullscreening */
    } else if (self->max_horz || self->max_vert) {
        Rect *a;
        guint i;

        i = screen_find_monitor(&desired_area);
        a = screen_area_monitor(self->desktop, i);

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
    }

    /* gets the client's position */
    frame_frame_gravity(self->frame, x, y, *w, *h);

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


void client_configure(ObClient *self, gint x, gint y, gint w, gint h, gint b,
                      gboolean user, gboolean final)
{
    gint oldw, oldh;
    gboolean send_resize_client;
    gboolean moved = FALSE, resized = FALSE;
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
    moved = x != self->area.x || y != self->area.y;
    resized = w != self->area.width || h != self->area.height ||
        b != self->border_width;

    oldw = self->area.width;
    oldh = self->area.height;
    RECT_SET(self->area, x, y, w, h);
    self->border_width = b;

    /* for app-requested resizes, always resize if 'resized' is true.
       for user-requested ones, only resize if final is true, or when
       resizing in redraw mode */
    send_resize_client = ((!user && resized) ||
                          (user && (final ||
                                    (resized && config_resize_redraw))));

    /* if the client is enlarging, then resize the client before the frame */
    if (send_resize_client && (w > oldw || h > oldh)) {
        XWindowChanges changes;
        changes.x = -self->border_width;
        changes.y = -self->border_width;
        changes.width = MAX(w, oldw);
        changes.height = MAX(h, oldh);
        changes.border_width = self->border_width;
        XConfigureWindow(ob_display, self->window,
                         CWX|CWY|CWWidth|CWHeight|CWBorderWidth,
                         &changes);
        /* resize the plate to show the client padding color underneath */
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
    if (fmoved || fresized)
        frame_adjust_area(self->frame, fmoved, fresized, FALSE);

    if ((!user || (user && final)) && !resized)
    {
        XEvent event;

        /* we have reset the client to 0 border width, so don't include
           it in these coords */
        POINT_SET(self->root_pos,
                  self->frame->area.x + self->frame->size.left -
                  self->border_width,
                  self->frame->area.y + self->frame->size.top -
                  self->border_width);

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
        event.xconfigure.above = self->frame->plate;
        event.xconfigure.override_redirect = FALSE;
        XSendEvent(event.xconfigure.display, event.xconfigure.window,
                   FALSE, StructureNotifyMask, &event);
    }

    /* if the client is shrinking, then resize the frame before the client */
    if (send_resize_client && (w <= oldw && h <= oldh)) {
        /* resize the plate to show the client padding color underneath */
        frame_adjust_client_area(self->frame);

        if (send_resize_client) {
            XWindowChanges changes;
            changes.x = -self->border_width;
            changes.y = -self->border_width;
            changes.width = w;
            changes.height = h;
            changes.border_width = self->border_width;
            XConfigureWindow(ob_display, self->window,
                             CWX|CWY|CWWidth|CWHeight|CWBorderWidth,
                             &changes);
        }
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

    client_setup_decor_and_functions(self);

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
                client_set_desktop(self, screen_desktop, FALSE);

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
        self = client_search_top_normal_parent(self);
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

    client_setup_decor_and_functions(self);

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
    frame_adjust_area(self->frame, FALSE, FALSE, FALSE);
}

void client_close(ObClient *self)
{
    XEvent ce;

    if (!(self->functions & OB_CLIENT_FUNC_CLOSE)) return;

    /* in the case that the client provides no means to requesting that it
       close, we just kill it */
    if (!self->delete_window)
        client_kill(self);
    
    /*
      XXX: itd be cool to do timeouts and shit here for killing the client's
      process off
      like... if the window is around after 5 seconds, then the close button
      turns a nice red, and if this function is called again, the client is
      explicitly killed.
    */

    ce.xclient.type = ClientMessage;
    ce.xclient.message_type =  prop_atoms.wm_protocols;
    ce.xclient.display = ob_display;
    ce.xclient.window = self->window;
    ce.xclient.format = 32;
    ce.xclient.data.l[0] = prop_atoms.wm_delete_window;
    ce.xclient.data.l[1] = event_curtime;
    ce.xclient.data.l[2] = 0l;
    ce.xclient.data.l[3] = 0l;
    ce.xclient.data.l[4] = 0l;
    XSendEvent(ob_display, self->window, FALSE, NoEventMask, &ce);
}

void client_kill(ObClient *self)
{
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
                                  gboolean donthide)
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
            client_showhide(self);
        /* raise if it was not already on the desktop */
        if (old != DESKTOP_ALL)
            stacking_raise(CLIENT_AS_WINDOW(self));
        if (STRUT_EXISTS(self->strut))
            screen_update_areas();
    }

    /* move all transients */
    for (it = self->transients; it; it = g_slist_next(it))
        if (it->data != self)
            if (client_is_direct_child(self, it->data))
                client_set_desktop_recursive(it->data, target, donthide);
}

void client_set_desktop(ObClient *self, guint target,
                        gboolean donthide)
{
    self = client_search_top_normal_parent(self);
    client_set_desktop_recursive(self, target, donthide);
}

gboolean client_is_direct_child(ObClient *parent, ObClient *child)
{
    while (child != parent &&
           child->transient_for && child->transient_for != OB_TRAN_GROUP)
        child = child->transient_for;
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
    /* choose the correct target */
    self = client_focus_target(self);

    if (!client_can_focus(self)) {
        if (!self->frame->visible) {
            /* update the focus lists */
            focus_order_to_top(self);
        }
        return FALSE;
    }

    ob_debug_type(OB_DEBUG_FOCUS,
                  "Focusing client \"%s\" at time %u\n",
                  self->title, event_curtime);

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

    return !xerror_occured;
}

/*! Present the client to the user.
  @param raise If the client should be raised or not. You should only set
               raise to false if you don't care if the window is completely
               hidden.
*/
static void client_present(ObClient *self, gboolean here, gboolean raise)
{
    /* if using focus_delay, stop the timer now so that focus doesn't
       go moving on us */
    event_halt_focus_delay();

    if (client_normal(self) && screen_showing_desktop)
        screen_show_desktop(FALSE, self);
    if (self->iconic)
        client_iconify(self, FALSE, here, FALSE);
    if (self->desktop != DESKTOP_ALL &&
        self->desktop != screen_desktop)
    {
        if (here)
            client_set_desktop(self, screen_desktop, FALSE);
        else
            screen_set_desktop(self->desktop, FALSE);
    } else if (!self->frame->visible)
        /* if its not visible for other reasons, then don't mess
           with it */
        return;
    if (self->shaded)
        client_shade(self, FALSE);
    if (raise)
        stacking_raise(CLIENT_AS_WINDOW(self));

    client_focus(self);
}

void client_activate(ObClient *self, gboolean here, gboolean user)
{
    guint32 last_time = focus_client ? focus_client->user_time : CurrentTime;
    gboolean allow = FALSE;

    /* if the request came from the user, or if nothing is focused, then grant
       the request.
       if the currently focused app doesn't set a user_time, then it can't
       benefit from any focus stealing prevention.
    */
    if (user || !focus_client || !last_time)
        allow = TRUE;
    /* otherwise, if they didn't give a time stamp or if it is too old, they
       don't get focus */
    else
        allow = event_curtime && event_time_after(event_curtime, last_time);

    ob_debug_type(OB_DEBUG_FOCUS,
                  "Want to activate window 0x%x with time %u (last time %u), "
                  "source=%s allowing? %d\n",
                  self->window, event_curtime, last_time,
                  (user ? "user" : "application"), allow);

    if (allow)
        client_present(self, here, TRUE);
    else
        /* don't focus it but tell the user it wants attention */
        client_hilite(self, TRUE);
}

static void client_bring_helper_windows_recursive(ObClient *self,
                                                  guint desktop)
{
    GSList *it;

    for (it = self->transients; it; it = g_slist_next(it))
        client_bring_helper_windows_recursive(it->data, desktop);

    if (client_helper(self) &&
        self->desktop != desktop && self->desktop != DESKTOP_ALL)
    {
        client_set_desktop(self, desktop, FALSE);
    }
}

void client_bring_helper_windows(ObClient *self)
{
    client_bring_helper_windows_recursive(self, self->desktop);
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

        if (self->transient_for) {
            if (self->transient_for != OB_TRAN_GROUP)
                parent = client_icon_recursive(self->transient_for, w, h);
            else {
                GSList *it;
                for (it = self->group->members; it; it = g_slist_next(it)) {
                    ObClient *c = it->data;
                    if (c != self && !c->transient_for) {
                        if ((parent = client_icon_recursive(c, w, h)))
                            break;
                    }
                }
            }
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
        client_setup_decor_and_functions(self);
        client_reconfigure(self); /* show the lack of decorations */
        client_change_state(self); /* reflect this in the state hints */
    }
}

guint client_monitor(ObClient *self)
{
    return screen_find_monitor(&self->frame->area);
}

ObClient *client_search_top_normal_parent(ObClient *self)
{
    while (self->transient_for && self->transient_for != OB_TRAN_GROUP &&
           client_normal(self->transient_for))
        self = self->transient_for;
    return self;
}

static GSList *client_search_all_top_parents_internal(ObClient *self,
                                                      gboolean bylayer,
                                                      ObStackingLayer layer)
{
    GSList *ret = NULL;
    
    /* move up the direct transient chain as far as possible */
    while (self->transient_for && self->transient_for != OB_TRAN_GROUP &&
           (!bylayer || self->transient_for->layer == layer) &&
           client_normal(self->transient_for))
        self = self->transient_for;

    if (!self->transient_for)
        ret = g_slist_prepend(ret, self);
    else {
            GSList *it;

            g_assert(self->group);

            for (it = self->group->members; it; it = g_slist_next(it)) {
                ObClient *c = it->data;

                if (!c->transient_for && client_normal(c) &&
                    (!bylayer || c->layer == layer))
                {
                    ret = g_slist_prepend(ret, c);
                }
            }

            if (ret == NULL) /* no group parents */
                ret = g_slist_prepend(ret, self);
    }

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
    if (self->transient_for) {
        if (self->transient_for != OB_TRAN_GROUP) {
            if (client_focused(self->transient_for))
                return self->transient_for;
        } else {
            GSList *it;

            for (it = self->group->members; it; it = g_slist_next(it)) {
                ObClient *c = it->data;

                /* checking transient_for prevents infinate loops! */
                if (c != self && !c->transient_for)
                    if (client_focused(c))
                        return c;
            }
        }
    }

    return NULL;
}

ObClient *client_search_parent(ObClient *self, ObClient *search)
{
    if (self->transient_for) {
        if (self->transient_for != OB_TRAN_GROUP) {
            if (self->transient_for == search)
                return search;
        } else {
            GSList *it;

            for (it = self->group->members; it; it = g_slist_next(it)) {
                ObClient *c = it->data;

                /* checking transient_for prevents infinate loops! */
                if (c != self && !c->transient_for)
                    if (c == search)
                        return search;
            }
        }
    }

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

#define WANT_EDGE(cur, c) \
            if(cur == c)                                                      \
                continue;                                                     \
            if(!client_normal(cur))                                           \
                continue;                                                     \
            if(screen_desktop != cur->desktop && cur->desktop != DESKTOP_ALL) \
                continue;                                                     \
            if(cur->iconic)                                                   \
                continue;

#define HIT_EDGE(my_edge_start, my_edge_end, his_edge_start, his_edge_end) \
            if ((his_edge_start >= my_edge_start && \
                 his_edge_start <= my_edge_end) ||  \
                (my_edge_start >= his_edge_start && \
                 my_edge_start <= his_edge_end))    \
                dest = his_offset;

/* finds the nearest edge in the given direction from the current client
 * note to self: the edge is the -frame- edge (the actual one), not the
 * client edge.
 */
gint client_directional_edge_search(ObClient *c, ObDirection dir, gboolean hang)
{
    gint dest, monitor_dest;
    gint my_edge_start, my_edge_end, my_offset;
    GList *it;
    Rect *a, *monitor;
    
    if(!client_list)
        return -1;

    a = screen_area(c->desktop);
    monitor = screen_area_monitor(c->desktop, client_monitor(c));

    switch(dir) {
    case OB_DIRECTION_NORTH:
        my_edge_start = c->frame->area.x;
        my_edge_end = c->frame->area.x + c->frame->area.width;
        my_offset = c->frame->area.y + (hang ? c->frame->area.height : 0);
        
        /* default: top of screen */
        dest = a->y + (hang ? c->frame->area.height : 0);
        monitor_dest = monitor->y + (hang ? c->frame->area.height : 0);
        /* if the monitor edge comes before the screen edge, */
        /* use that as the destination instead. (For xinerama) */
        if (monitor_dest != dest && my_offset > monitor_dest)
            dest = monitor_dest; 

        for(it = client_list; it && my_offset != dest; it = g_list_next(it)) {
            gint his_edge_start, his_edge_end, his_offset;
            ObClient *cur = it->data;

            WANT_EDGE(cur, c)

            his_edge_start = cur->frame->area.x;
            his_edge_end = cur->frame->area.x + cur->frame->area.width;
            his_offset = cur->frame->area.y + 
                         (hang ? 0 : cur->frame->area.height);

            if(his_offset + 1 > my_offset)
                continue;

            if(his_offset < dest)
                continue;

            HIT_EDGE(my_edge_start, my_edge_end, his_edge_start, his_edge_end)
        }
        break;
    case OB_DIRECTION_SOUTH:
        my_edge_start = c->frame->area.x;
        my_edge_end = c->frame->area.x + c->frame->area.width;
        my_offset = c->frame->area.y + (hang ? 0 : c->frame->area.height);

        /* default: bottom of screen */
        dest = a->y + a->height - (hang ? c->frame->area.height : 0);
        monitor_dest = monitor->y + monitor->height -
                       (hang ? c->frame->area.height : 0);
        /* if the monitor edge comes before the screen edge, */
        /* use that as the destination instead. (For xinerama) */
        if (monitor_dest != dest && my_offset < monitor_dest)
            dest = monitor_dest; 

        for(it = client_list; it && my_offset != dest; it = g_list_next(it)) {
            gint his_edge_start, his_edge_end, his_offset;
            ObClient *cur = it->data;

            WANT_EDGE(cur, c)

            his_edge_start = cur->frame->area.x;
            his_edge_end = cur->frame->area.x + cur->frame->area.width;
            his_offset = cur->frame->area.y +
                         (hang ? cur->frame->area.height : 0);


            if(his_offset - 1 < my_offset)
                continue;
            
            if(his_offset > dest)
                continue;

            HIT_EDGE(my_edge_start, my_edge_end, his_edge_start, his_edge_end)
        }
        break;
    case OB_DIRECTION_WEST:
        my_edge_start = c->frame->area.y;
        my_edge_end = c->frame->area.y + c->frame->area.height;
        my_offset = c->frame->area.x + (hang ? c->frame->area.width : 0);

        /* default: leftmost egde of screen */
        dest = a->x + (hang ? c->frame->area.width : 0);
        monitor_dest = monitor->x + (hang ? c->frame->area.width : 0);
        /* if the monitor edge comes before the screen edge, */
        /* use that as the destination instead. (For xinerama) */
        if (monitor_dest != dest && my_offset > monitor_dest)
            dest = monitor_dest;            

        for(it = client_list; it && my_offset != dest; it = g_list_next(it)) {
            gint his_edge_start, his_edge_end, his_offset;
            ObClient *cur = it->data;

            WANT_EDGE(cur, c)

            his_edge_start = cur->frame->area.y;
            his_edge_end = cur->frame->area.y + cur->frame->area.height;
            his_offset = cur->frame->area.x +
                         (hang ? 0 : cur->frame->area.width);

            if(his_offset + 1 > my_offset)
                continue;

            if(his_offset < dest)
                continue;

            HIT_EDGE(my_edge_start, my_edge_end, his_edge_start, his_edge_end)
        }
       break;
    case OB_DIRECTION_EAST:
        my_edge_start = c->frame->area.y;
        my_edge_end = c->frame->area.y + c->frame->area.height;
        my_offset = c->frame->area.x + (hang ? 0 : c->frame->area.width);
        
        /* default: rightmost edge of screen */
        dest = a->x + a->width - (hang ? c->frame->area.width : 0);
        monitor_dest = monitor->x + monitor->width -
                       (hang ? c->frame->area.width : 0);
        /* if the monitor edge comes before the screen edge, */
        /* use that as the destination instead. (For xinerama) */
        if (monitor_dest != dest && my_offset < monitor_dest)
            dest = monitor_dest;            

        for(it = client_list; it && my_offset != dest; it = g_list_next(it)) {
            gint his_edge_start, his_edge_end, his_offset;
            ObClient *cur = it->data;

            WANT_EDGE(cur, c)

            his_edge_start = cur->frame->area.y;
            his_edge_end = cur->frame->area.y + cur->frame->area.height;
            his_offset = cur->frame->area.x +
                         (hang ? cur->frame->area.width : 0);

            if(his_offset - 1 < my_offset)
                continue;
            
            if(his_offset > dest)
                continue;

            HIT_EDGE(my_edge_start, my_edge_end, his_edge_start, his_edge_end)
        }
        break;
    case OB_DIRECTION_NORTHEAST:
    case OB_DIRECTION_SOUTHEAST:
    case OB_DIRECTION_NORTHWEST:
    case OB_DIRECTION_SOUTHWEST:
        /* not implemented */
    default:
        g_assert_not_reached();
        dest = 0; /* suppress warning */
    }
    return dest;
}

ObClient* client_under_pointer()
{
    gint x, y;
    GList *it;
    ObClient *ret = NULL;

    if (screen_pointer_pos(&x, &y)) {
        for (it = stacking_list; it; it = g_list_next(it)) {
            if (WINDOW_IS_CLIENT(it->data)) {
                ObClient *c = WINDOW_AS_CLIENT(it->data);
                if (c->frame->visible &&
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
