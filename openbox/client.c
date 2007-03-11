/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-
   
   client.c for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
   Copyright (c) 2003        Ben Jansens

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

#include <glib.h>
#include <X11/Xutil.h>

/*! The event mask to grab on client windows */
#define CLIENT_EVENTMASK (PropertyChangeMask | FocusChangeMask | \
                          StructureNotifyMask)

#define CLIENT_NOPROPAGATEMASK (ButtonPressMask | ButtonReleaseMask | \
                                ButtonMotionMask)

typedef struct
{
    ObClientDestructor func;
    gpointer data;
} Destructor;

GList         *client_list           = NULL;

static GSList *client_destructors    = NULL;
static Time    client_last_user_time = CurrentTime;

static void client_get_all(ObClient *self);
static void client_toggle_border(ObClient *self, gboolean show);
static void client_get_startup_id(ObClient *self);
static void client_get_area(ObClient *self);
static void client_get_desktop(ObClient *self);
static void client_get_state(ObClient *self);
static void client_get_layer(ObClient *self);
static void client_get_shaped(ObClient *self);
static void client_get_mwm_hints(ObClient *self);
static void client_get_gravity(ObClient *self);
static void client_showhide(ObClient *self);
static void client_change_allowed_actions(ObClient *self);
static void client_change_state(ObClient *self);
static void client_apply_startup_state(ObClient *self);
static void client_restore_session_state(ObClient *self);
static void client_restore_session_stacking(ObClient *self);

void client_startup(gboolean reconfig)
{
    if (reconfig) return;

    client_set_list();
}

void client_shutdown(gboolean reconfig)
{
}

void client_add_destructor(ObClientDestructor func, gpointer data)
{
    Destructor *d = g_new(Destructor, 1);
    d->func = func;
    d->data = data;
    client_destructors = g_slist_prepend(client_destructors, d);
}

void client_remove_destructor(ObClientDestructor func)
{
    GSList *it;

    for (it = client_destructors; it; it = g_slist_next(it)) {
        Destructor *d = it->data;
        if (d->func == func) {
            g_free(d);
            client_destructors = g_slist_delete_link(client_destructors, it);
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

static ObAppSettings *get_settings(ObClient *client)
{
    GSList *a = config_per_app_settings;

    while (a) {
        ObAppSettings *app = (ObAppSettings *) a->data;
        
        if (
            (app->name && !app->class && !strcmp(app->name, client->name))
            || (app->class && !app->name && !strcmp(app->class, client->class))
            || (app->class && app->name && !strcmp(app->class, client->class)
                && !strcmp(app->name, client->name))
            ) {
            ob_debug("Window matching: %s\n", app->name);
            /* Match if no role was specified in the per app setting, or if the
             * string matches the beginning of the role, since apps like to set
             * the role to things like browser-window-23c4b2f */
            if (!app->role
                || !strncmp(app->role, client->role, strlen(app->role)))
                return app;
        }

        a = a->next;
    }
    return NULL;
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

    grab_server(TRUE);

    /* check if it has already been unmapped by the time we started mapping
       the grab does a sync so we don't have to here */
    if (XCheckTypedWindowEvent(ob_display, window, DestroyNotify, &e) ||
        XCheckTypedWindowEvent(ob_display, window, UnmapNotify, &e))
    {
        XPutBackEvent(ob_display, &e);

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
    self->title_count = 1;
    self->wmstate = NormalState;
    self->layer = -1;
    self->desktop = screen_num_desktops; /* always an invalid value */
    self->user_time = ~0; /* maximum value, always newer than the real time */

    client_get_all(self);
    client_restore_session_state(self);

    client_calc_layer(self);

    {
        Time t = sn_app_started(self->startup_id, self->class);
        if (t) self->user_time = t;
    }

    /* update the focus lists, do this before the call to change_state or
       it can end up in the list twice! */
    focus_order_add_new(self);

    client_change_state(self);

    /* remove the client's border (and adjust re gravity) */
    client_toggle_border(self, FALSE);
     
    /* specify that if we exit, the window should not be destroyed and should
       be reparented back to root automatically */
    XChangeSaveSet(ob_display, window, SetModeInsert);

    /* create the decoration frame for the client window */
    self->frame = frame_new(self);

    frame_grab_client(self->frame, self);

    grab_server(FALSE);

    client_apply_startup_state(self);

    /* get and set application level settings */
    settings = get_settings(self);

    stacking_add_nonintrusive(CLIENT_AS_WINDOW(self));
    client_restore_session_stacking(self);

    if (settings) {
        /* Don't worry, we won't actually both shade and undecorate the
         * window when push comes to shove. */
        if (settings->shade != -1)
            client_shade(self, !!settings->shade);
        if (settings->decor != -1)
            client_set_undecorated(self, !settings->decor);
        if (settings->iconic != -1)
            client_iconify(self, !!settings->iconic, FALSE);
        if (settings->skip_pager != -1) {
            self->skip_pager = !!settings->skip_pager;
            client_change_state(self);
        }
        if (settings->skip_taskbar != -1) {
            self->skip_taskbar = !!settings->skip_taskbar;
            client_change_state(self);
        }

        /* 1 && -1 shouldn't be possible by the code in config.c */
        if (settings->max_vert == 1 && settings->max_horz == 1)
            client_maximize(self, TRUE, 0, TRUE);
        else if (settings->max_vert == 0 && settings->max_horz == 0)
            client_maximize(self, FALSE, 0, TRUE);
        else if (settings->max_vert == 1 && settings->max_horz == 0) {
            client_maximize(self, TRUE, 2, TRUE);
            client_maximize(self, FALSE, 1, TRUE);
        } else if (settings->max_vert == 0 && settings->max_horz == 1) {
            client_maximize(self, TRUE, 1, TRUE);
            client_maximize(self, FALSE, 2, TRUE);
        }

        if (settings->fullscreen != -1)
            client_fullscreen(self, !!settings->fullscreen, TRUE);

        if (settings->desktop < screen_num_desktops
            || settings->desktop == DESKTOP_ALL)
            client_set_desktop(self, settings->desktop, TRUE);

        if (settings->layer > -2 && settings->layer < 2)
            client_set_layer(self, settings->layer);

    }

    /* focus the new window? */
    if (ob_state() != OB_STATE_STARTING &&
        /* this means focus=true for window is same as config_focus_new=true */
        ((config_focus_new || (settings && settings->focus == 1)) ||
         client_search_focus_parent(self)) &&
        /* this checks for focus=false for the window */
        (!settings || settings->focus != 0) &&
        /* note the check against Type_Normal/Dialog, not client_normal(self),
           which would also include other types. in this case we want more
           strict rules for focus */
        (self->type == OB_CLIENT_TYPE_NORMAL ||
         self->type == OB_CLIENT_TYPE_DIALOG))
    {
        activate = TRUE;
#if 0
        if (self->desktop != screen_desktop) {
            /* activate the window */
            activate = TRUE;
        } else {
            gboolean group_foc = FALSE;

            if (self->group) {
                GSList *it;

                for (it = self->group->members; it; it = g_slist_next(it))
                {
                    if (client_focused(it->data))
                    {
                        group_foc = TRUE;
                        break;
                    }
                }
            }
            if ((group_foc ||
                 (!self->transient_for && (!self->group ||
                                           !self->group->members->next))) ||
                client_search_focus_tree_full(self) ||
                !focus_client ||
                !client_normal(focus_client))
            {
                /* activate the window */
                activate = TRUE;
            }
        }
#endif
    }

    if (ob_state() == OB_STATE_RUNNING) {
        gint x = self->area.x, ox = x;
        gint y = self->area.y, oy = y;
        gboolean transient;

        transient = place_client(self, &x, &y, settings);

        /* make sure the window is visible. */
        client_find_onscreen(self, &x, &y,
                             self->frame->area.width,
                             self->frame->area.height,
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
        if (x != ox || y != oy) 	 
            client_move(self, x, y);
    }

    keyboard_grab_for_client(self, TRUE);
    mouse_grab_for_client(self, TRUE);

    if (activate) {
        /* This is focus stealing prevention */
        ob_debug("Want to focus new window 0x%x with time %u (last time %u)\n",
                 self->window, self->user_time, client_last_user_time);

        /* If a nothing at all, or a parent was focused, then focus this
           always
        */
        if (client_search_focus_parent(self) != NULL ||
            !focus_client)
        {
            activate = TRUE;
        }
        else
        {
            /* If time stamp is old, don't steal focus */
            if (self->user_time && self->user_time < client_last_user_time)
                activate = FALSE;
            /* Don't steal focus from globally active clients.
               I stole this idea from KWin. It seems nice.
             */
            if (focus_client && focus_client->can_focus != TRUE &&
                focus_client->focus_notify == TRUE)
            {
                activate = FALSE;
            }
        }

        if (activate)
        {
            /* since focus can change the stacking orders, if we focus the
               window then the standard raise it gets is not enough, we need
               to queue one for after the focus change takes place */
            client_raise(self);
        } else {
            ob_debug("Focus stealing prevention activated for %s with time %u "
                     "(last time %u)\n",
                     self->title, self->user_time, client_last_user_time);
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
        client_raise(self);
    }

    /* this has to happen before we try focus the window, but we want it to
       happen after the client's stacking has been determined or it looks bad
    */
    client_showhide(self);

    /* use client_focus instead of client_activate cuz client_activate does
       stuff like switch desktops etc and I'm not interested in all that when
       a window maps since its not based on an action from the user like
       clicking a window to activate it. so keep the new window out of the way
       but do focus it. */
    if (activate) {
        /* if using focus_delay, stop the timer now so that focus doesn't
           go moving on us */
        event_halt_focus_delay();
        client_focus(self);
    }

    /* client_activate does this but we aret using it so we have to do it
       here as well */
    if (screen_showing_desktop)
        screen_show_desktop(FALSE);

    /* add to client list/map */
    client_list = g_list_append(client_list, self);
    g_hash_table_insert(window_map, &self->window, self);

    /* this has to happen after we're in the client_list */
    if (STRUT_EXISTS(self->strut))
        screen_update_areas();

    /* update the list hints */
    client_set_list();

    ob_debug("Managed window 0x%lx (%s)\n", window, self->class);
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

    ob_debug("Unmanaging window: %lx (%s)\n", self->window, self->class);

    g_assert(self != NULL);

    keyboard_grab_for_client(self, FALSE);
    mouse_grab_for_client(self, FALSE);

    /* potentially fix focusLast */
    if (config_focus_last)
        grab_pointer(TRUE, OB_CURSOR_NONE);

    /* remove the window from our save set */
    XChangeSaveSet(ob_display, self->window, SetModeDelete);

    /* we dont want events no more */
    XSelectInput(ob_display, self->window, NoEventMask);

    frame_hide(self->frame);

    client_list = g_list_remove(client_list, self);
    stacking_remove(self);
    g_hash_table_remove(window_map, &self->window);

    /* update the focus lists */
    focus_order_remove(self);

    /* once the client is out of the list, update the struts to remove it's
       influence */
    if (STRUT_EXISTS(self->strut))
        screen_update_areas();

    for (it = client_destructors; it; it = g_slist_next(it)) {
        Destructor *d = it->data;
        d->func(self, d->data);
    }
        
    if (focus_client == self) {
        XEvent e;

        /* focus the last focused window on the desktop, and ignore enter
           events from the unmap so it doesnt mess with the focus */
        while (XCheckTypedEvent(ob_display, EnterNotify, &e));
        /* remove these flags so we don't end up getting focused in the
           fallback! */
        self->can_focus = FALSE;
        self->focus_notify = FALSE;
        self->modal = FALSE;
        client_unfocus(self);
    }

    /* tell our parent(s) that we're gone */
    if (self->transient_for == OB_TRAN_GROUP) { /* transient of group */
        for (it = self->group->members; it; it = g_slist_next(it))
            if (it->data != self)
                ((ObClient*)it->data)->transients =
                    g_slist_remove(((ObClient*)it->data)->transients, self);
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

    /* give the client its border back */
    client_toggle_border(self, TRUE);

    /* reparent the window out of the frame, and free the frame */
    frame_release_client(self->frame, self);
    self->frame = NULL;
     
    if (ob_state() != OB_STATE_EXITING) {
        /* these values should not be persisted across a window
           unmapping/mapping */
        PROP_ERASE(self->window, net_wm_desktop);
        PROP_ERASE(self->window, net_wm_state);
        PROP_ERASE(self->window, wm_state);
    } else {
        /* if we're left in an unmapped state, the client wont be mapped. this
           is bad, since we will no longer be managing the window on restart */
        XMapWindow(ob_display, self->window);
    }


    ob_debug("Unmanaged window 0x%lx\n", self->window);

    /* free all data allocated in the client struct */
    g_slist_free(self->transients);
    for (j = 0; j < self->nicons; ++j)
        g_free(self->icons[j].data);
    if (self->nicons > 0)
        g_free(self->icons);
    g_free(self->title);
    g_free(self->icon_title);
    g_free(self->name);
    g_free(self->class);
    g_free(self->role);
    g_free(self->sm_client_id);
    g_free(self);
     
    /* update the list hints */
    client_set_list();

    if (config_focus_last)
        grab_pointer(FALSE, OB_CURSOR_NONE);
}

static void client_restore_session_state(ObClient *self)
{
    GList *it;

    if (!(it = session_state_find(self)))
        return;

    self->session = it->data;

    RECT_SET_POINT(self->area, self->session->x, self->session->y);
    self->positioned = PPosition;
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
}

static void client_restore_session_stacking(ObClient *self)
{
    GList *it;

    if (!self->session) return;

    it = g_list_find(session_saved_state, self->session);
    for (it = g_list_previous(it); it; it = g_list_previous(it)) {
        GList *cit;

        for (cit = client_list; cit; cit = g_list_next(cit))
            if (session_state_cmp(it->data, cit->data))
                break;
        if (cit) {
            client_calc_layer(self);
            stacking_below(CLIENT_AS_WINDOW(self),
                           CLIENT_AS_WINDOW(cit->data));
            break;
        }
    }
}

void client_move_onscreen(ObClient *self, gboolean rude)
{
    gint x = self->area.x;
    gint y = self->area.y;
    if (client_find_onscreen(self, &x, &y,
                             self->frame->area.width,
                             self->frame->area.height, rude)) {
        client_move(self, x, y);
    }
}

gboolean client_find_onscreen(ObClient *self, gint *x, gint *y, gint w, gint h,
                              gboolean rude)
{
    Rect *a;
    gint ox = *x, oy = *y;

    frame_client_gravity(self->frame, x, y); /* get where the frame
                                                would be */

    /* XXX watch for xinerama dead areas */
    /* This makes sure windows aren't entirely outside of the screen so you
       can't see them at all.
       It makes sure 10% of the window is on the screen at least. At don't let
       it move itself off the top of the screen, which would hide the titlebar
       on you. (The user can still do this if they want too, it's only limiting
       the application.
    */
    if (client_normal(self)) {
        a = screen_area(self->desktop);
        if (!self->strut.right &&
            *x + self->frame->area.width/10 >= a->x + a->width - 1)
            *x = a->x + a->width - self->frame->area.width/10;
        if (!self->strut.bottom &&
            *y + self->frame->area.height/10 >= a->y + a->height - 1)
            *y = a->y + a->height - self->frame->area.height/10;
        if (!self->strut.left && *x + self->frame->area.width*9/10 - 1 < a->x)
            *x = a->x - self->frame->area.width*9/10;
        if (!self->strut.top && *y + self->frame->area.height*9/10 - 1 < a->y)
            *y = a->y - self->frame->area.width*9/10;
    }

    /* This here doesn't let windows even a pixel outside the screen,
     * when called from client_manage, programs placing themselves are
     * forced completely onscreen, while things like
     * xterm -geometry resolution-width/2 will work fine. Trying to
     * place it completely offscreen will be handled in the above code.
     * Sorry for this confused comment, i am tired. */
    if (rude) {
        /* avoid the xinerama monitor divide while we're at it,
         * remember to fix the placement stuff to avoid it also and
         * then remove this XXX */
        a = screen_area_monitor(self->desktop, client_monitor(self));
        /* dont let windows map into the strut unless they
           are bigger than the available area */
        if (w <= a->width) {
            if (!self->strut.left && *x < a->x) *x = a->x;
            if (!self->strut.right && *x + w > a->x + a->width)
                *x = a->x + a->width - w;
        }
        if (h <= a->height) {
            if (!self->strut.top && *y < a->y) *y = a->y;
            if (!self->strut.bottom && *y + h > a->y + a->height)
                *y = a->y + a->height - h;
        }
    }

    frame_frame_gravity(self->frame, x, y); /* get where the client
                                               should be */

    return ox != *x || oy != *y;
}

static void client_toggle_border(ObClient *self, gboolean show)
{
    /* adjust our idea of where the client is, based on its border. When the
       border is removed, the client should now be considered to be in a
       different position.
       when re-adding the border to the client, the same operation needs to be
       reversed. */
    gint oldx = self->area.x, oldy = self->area.y;
    gint x = oldx, y = oldy;
    switch(self->gravity) {
    default:
    case NorthWestGravity:
    case WestGravity:
    case SouthWestGravity:
        break;
    case NorthEastGravity:
    case EastGravity:
    case SouthEastGravity:
        if (show) x -= self->border_width * 2;
        else      x += self->border_width * 2;
        break;
    case NorthGravity:
    case SouthGravity:
    case CenterGravity:
    case ForgetGravity:
    case StaticGravity:
        if (show) x -= self->border_width;
        else      x += self->border_width;
        break;
    }
    switch(self->gravity) {
    default:
    case NorthWestGravity:
    case NorthGravity:
    case NorthEastGravity:
        break;
    case SouthWestGravity:
    case SouthGravity:
    case SouthEastGravity:
        if (show) y -= self->border_width * 2;
        else      y += self->border_width * 2;
        break;
    case WestGravity:
    case EastGravity:
    case CenterGravity:
    case ForgetGravity:
    case StaticGravity:
        if (show) y -= self->border_width;
        else      y += self->border_width;
        break;
    }
    self->area.x = x;
    self->area.y = y;

    if (show) {
        XSetWindowBorderWidth(ob_display, self->window, self->border_width);

        /* move the client so it is back it the right spot _with_ its
           border! */
        if (x != oldx || y != oldy)
            XMoveWindow(ob_display, self->window, x, y);
    } else
        XSetWindowBorderWidth(ob_display, self->window, 0);
}


static void client_get_all(ObClient *self)
{
    client_get_area(self);
    client_get_mwm_hints(self);

    /* The transient hint is used to pick a type, but the type can also affect
       transiency (dialogs are always made transients of their group if they
       have one). This is Havoc's idea, but it is needed to make some apps
       work right (eg tsclient). */
    client_update_transient_for(self);
    client_get_type(self);/* this can change the mwmhints for special cases */
    client_get_state(self);
    client_update_transient_for(self);

    client_update_wmhints(self);
    client_get_startup_id(self);
    client_get_desktop(self);/* uses transient data/group/startup id if a
                                desktop is not specified */
    client_get_shaped(self);

    client_get_layer(self); /* if layer hasn't been specified, get it from
                               other sources if possible */

    {
        /* a couple type-based defaults for new windows */

        /* this makes sure that these windows appear on all desktops */
        if (self->type == OB_CLIENT_TYPE_DESKTOP)
            self->desktop = DESKTOP_ALL;
    }

    client_update_protocols(self);

    client_get_gravity(self); /* get the attribute gravity */
    client_update_normal_hints(self); /* this may override the attribute
                                         gravity */

    /* got the type, the mwmhints, the protocols, and the normal hints
       (min/max sizes), so we're ready to set up the decorations/functions */
    client_setup_decor_and_functions(self);
  
    client_update_title(self);
    client_update_class(self);
    client_update_sm_client_id(self);
    client_update_strut(self);
    client_update_icons(self);
    client_update_user_time(self, FALSE);
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
    self->border_width = wattrib.border_width;
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
                GSList *it;

                for (it = self->group->members; it; it = g_slist_next(it))
                    if (it->data != self &&
                        !((ObClient*)it->data)->transient_for) {
                        self->desktop = ((ObClient*)it->data)->desktop;
                        trdesk = TRUE;
                        break;
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
    if (self->desktop != d) {
        /* set the desktop hint, to make sure that it always exists */
        PROP_SET32(self->window, net_wm_desktop, cardinal, self->desktop);
    }
}

static void client_get_layer(ObClient *self)
{
    if (!(self->above || self->below)) {
        if (self->group) {
            /* apply stuff from the group */
            GSList *it;
            gint layer = -2;

            for (it = self->group->members; it; it = g_slist_next(it)) {
                ObClient *c = it->data;
                if (c != self && !client_search_transient(self, c) &&
                    client_normal(self) && client_normal(c))
                {
                    layer = MAX(layer,
                                (c->above ? 1 : (c->below ? -1 : 0)));
                }
            }
            switch (layer) {
            case -1:
                self->below = TRUE;
                break;
            case -2:
            case 0:
                break;
            case 1:
                self->above = TRUE;
                break;
            default:
                g_assert_not_reached();
                break;
            }
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
        self->transient = TRUE;
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
    } else if (self->type == OB_CLIENT_TYPE_DIALOG && self->group) {
        self->transient = TRUE;
        target = OB_TRAN_GROUP;
    } else
        self->transient = FALSE;

    /* if anything has changed... */
    if (target != self->transient_for) {
        if (self->transient_for == OB_TRAN_GROUP) { /* transient of group */
            GSList *it;

            /* remove from old parents */
            for (it = self->group->members; it; it = g_slist_next(it)) {
                ObClient *c = it->data;
                if (c != self && !c->transient_for)
                    c->transients = g_slist_remove(c->transients, self);
            }
        } else if (self->transient_for != NULL) { /* transient of window */
            /* remove from old parent */
            self->transient_for->transients =
                g_slist_remove(self->transient_for->transients, self);
        }
        self->transient_for = target;
        if (self->transient_for == OB_TRAN_GROUP) { /* transient of group */
            GSList *it;

            /* add to new parents */
            for (it = self->group->members; it; it = g_slist_next(it)) {
                ObClient *c = it->data;
                if (c != self && !c->transient_for)
                    c->transients = g_slist_append(c->transients, self);
            }

            /* remove all transients which are in the group, that causes
               circlular pointer hell of doom */
            for (it = self->group->members; it; it = g_slist_next(it)) {
                GSList *sit, *next;
                for (sit = self->transients; sit; sit = next) {
                    next = g_slist_next(sit);
                    if (sit->data == it->data)
                        self->transients =
                            g_slist_delete_link(self->transients, sit);
                }
            }
        } else if (self->transient_for != NULL) { /* transient of window */
            /* add to new parent */
            self->transient_for->transients =
                g_slist_append(self->transient_for->transients, self);
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

void client_get_type(ObClient *self)
{
    guint num, i;
    guint32 *val;

    self->type = -1;
  
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
    
    if (self->type == (ObClientType) -1) {
        /*the window type hint was not set, which means we either classify
          ourself as a normal window or a dialog, depending on if we are a
          transient. */
        if (self->transient)
            self->type = OB_CLIENT_TYPE_DIALOG;
        else
            self->type = OB_CLIENT_TYPE_NORMAL;
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
            if (proto[i] == prop_atoms.wm_delete_window) {
                /* this means we can request the window to close */
                self->delete_window = TRUE;
            } else if (proto[i] == prop_atoms.wm_take_focus)
                /* if this protocol is requested, then the window will be
                   notified whenever we want it to receive focus */
                self->focus_notify = TRUE;
        }
        g_free(proto);
    }
}

static void client_get_gravity(ObClient *self)
{
    XWindowAttributes wattrib;
    Status ret;

    ret = XGetWindowAttributes(ob_display, self->window, &wattrib);
    g_assert(ret != BadWindow);
    self->gravity = wattrib.win_gravity;
}

void client_update_normal_hints(ObClient *self)
{
    XSizeHints size;
    glong ret;
    gint oldgravity = self->gravity;

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

        if (size.flags & PWinGravity) {
            self->gravity = size.win_gravity;
      
            /* if the client has a frame, i.e. has already been mapped and
               is changing its gravity */
            if (self->frame && self->gravity != oldgravity) {
                /* move our idea of the client's position based on its new
                   gravity */
                self->area.x = self->frame->area.x;
                self->area.y = self->frame->area.y;
                frame_frame_gravity(self->frame, &self->area.x, &self->area.y);
            }
        }

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
         OB_CLIENT_FUNC_CLOSE);

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
        /* these windows cannot be maximized */
        self->functions &= ~OB_CLIENT_FUNC_MAXIMIZE;
        break;

    case OB_CLIENT_TYPE_MENU:
    case OB_CLIENT_TYPE_TOOLBAR:
        /* these windows get less functionality */
        self->functions &= ~(OB_CLIENT_FUNC_ICONIFY | OB_CLIENT_FUNC_RESIZE);
        break;

    case OB_CLIENT_TYPE_DESKTOP:
    case OB_CLIENT_TYPE_DOCK:
    case OB_CLIENT_TYPE_SPLASH:
        /* none of these windows are manipulated by the window manager */
        self->decorations = 0;
        self->functions = 0;
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
        self->decorations &= ~OB_FRAME_DECOR_GRIPS;

    /* can't maximize without moving/resizing */
    if (!((self->functions & OB_CLIENT_FUNC_MAXIMIZE) &&
          (self->functions & OB_CLIENT_FUNC_MOVE) &&
          (self->functions & OB_CLIENT_FUNC_RESIZE))) {
        self->functions &= ~OB_CLIENT_FUNC_MAXIMIZE;
        self->decorations &= ~OB_FRAME_DECOR_MAXIMIZE;
    }

    /* kill the handle on fully maxed windows */
    if (self->max_vert && self->max_horz)
        self->decorations &= ~OB_FRAME_DECOR_HANDLE;

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

    if (self->frame) {
        /* adjust the client's decorations, etc. */
        client_reconfigure(self);
    }
}

static void client_change_allowed_actions(ObClient *self)
{
    gulong actions[9];
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

    PROP_SETA32(self->window, net_wm_allowed_actions, atom, actions, num);

    /* make sure the window isn't breaking any rules now */

    if (!(self->functions & OB_CLIENT_FUNC_SHADE) && self->shaded) {
        if (self->frame) client_shade(self, FALSE);
        else self->shaded = FALSE;
    }
    if (!(self->functions & OB_CLIENT_FUNC_ICONIFY) && self->iconic) {
        if (self->frame) client_iconify(self, FALSE, TRUE);
        else self->iconic = FALSE;
    }
    if (!(self->functions & OB_CLIENT_FUNC_FULLSCREEN) && self->fullscreen) {
        if (self->frame) client_fullscreen(self, FALSE, TRUE);
        else self->fullscreen = FALSE;
    }
    if (!(self->functions & OB_CLIENT_FUNC_MAXIMIZE) && (self->max_horz ||
                                                         self->max_vert)) {
        if (self->frame) client_maximize(self, FALSE, 0, TRUE);
        else self->max_vert = self->max_horz = FALSE;
    }
}

void client_reconfigure(ObClient *self)
{
    /* by making this pass FALSE for user, we avoid the emacs event storm where
       every configurenotify causes an update in its normal hints, i think this
       is generally what we want anyways... */
    client_configure(self, OB_CORNER_TOPLEFT, self->area.x, self->area.y,
                     self->area.width, self->area.height, FALSE, TRUE);
}

void client_update_wmhints(ObClient *self)
{
    XWMHints *hints;
    GSList *it;

    /* assume a window takes input if it doesnt specify */
    self->can_focus = TRUE;
  
    if ((hints = XGetWMHints(ob_display, self->window)) != NULL) {
        if (hints->flags & InputHint)
            self->can_focus = hints->input;

        /* only do this when first managing the window *AND* when we aren't
           starting up! */
        if (ob_state() != OB_STATE_STARTING && self->frame == NULL)
            if (hints->flags & StateHint)
                self->iconic = hints->initial_state == IconicState;

        if (!(hints->flags & WindowGroupHint))
            hints->window_group = None;

        /* did the group state change? */
        if (hints->window_group !=
            (self->group ? self->group->leader : None)) {
            /* remove from the old group if there was one */
            if (self->group != NULL) {
                /* remove transients of the group */
                for (it = self->group->members; it; it = g_slist_next(it))
                    self->transients = g_slist_remove(self->transients,
                                                      it->data);

                /* remove myself from parents in the group */
                if (self->transient_for == OB_TRAN_GROUP) {
                    for (it = self->group->members; it;
                         it = g_slist_next(it))
                    {
                        ObClient *c = it->data;

                        if (c != self && !c->transient_for)
                            c->transients = g_slist_remove(c->transients,
                                                           self);
                    }
                }

                group_remove(self->group, self);
                self->group = NULL;
            }
            if (hints->window_group != None) {
                self->group = group_add(hints->window_group, self);

                /* i can only have transients from the group if i am not
                   transient myself */
                if (!self->transient_for) {
                    /* add other transients of the group that are already
                       set up */
                    for (it = self->group->members; it;
                         it = g_slist_next(it))
                    {
                        ObClient *c = it->data;
                        if (c != self && c->transient_for == OB_TRAN_GROUP)
                            self->transients =
                                g_slist_append(self->transients, c);
                    }
                }
            }

            /* because the self->transient flag wont change from this call,
               we don't need to update the window's type and such, only its
               transient_for, and the transients lists of other windows in
               the group may be affected */
            client_update_transient_for(self);
        }

        /* the WM_HINTS can contain an icon */
        client_update_icons(self);

        XFree(hints);
    }
}

void client_update_title(ObClient *self)
{
    GList *it;
    guint32 nums;
    guint i;
    gchar *data = NULL;
    gboolean read_title;
    gchar *old_title;

    old_title = self->title;
     
    /* try netwm */
    if (!PROP_GETS(self->window, net_wm_name, utf8, &data)) {
        /* try old x stuff */
        if (!(PROP_GETS(self->window, wm_name, locale, &data)
              || PROP_GETS(self->window, wm_name, utf8, &data))) {
            // http://developer.gnome.org/projects/gup/hig/draft_hig_new/windows-alert.html
            if (self->transient) {
                data = g_strdup("");
                goto no_number;
            } else
                data = g_strdup("Unnamed Window");
        }
    }

    if (config_title_number) {

        /* did the title change? then reset the title_count */
        if (old_title && 0 != strncmp(old_title, data, strlen(data)))
            self->title_count = 1;

        /* look for duplicates and append a number */
        nums = 0;
        for (it = client_list; it; it = g_list_next(it))
            if (it->data != self) {
                ObClient *c = it->data;

                if (c->title_count == 1) {
                    if (!strcmp(c->title, data))
                        nums |= 1 << c->title_count;
                } else {
                    size_t len;
                    gchar *end;

                    /* find the beginning of our " - [%u]", this relies on
                     that syntax being used */
                    end = strrchr(c->title, '-') - 1; 
                    len = end - c->title;
                    if (!strncmp(c->title, data, len))
                        nums |= 1 << c->title_count;
                }
            }
        /* find first free number */
        for (i = 1; i <= 32; ++i)
            if (!(nums & (1 << i))) {
                if (self->title_count == 1 || i == 1)
                    self->title_count = i;
                break;
            }
        /* dont display the number for the first window */
        if (self->title_count > 1) {
            gchar *ndata;
            ndata = g_strdup_printf("%s - [%u]", data, self->title_count);
            g_free(data);
            data = ndata;
        }
    } else
        self->title_count = 1;

no_number:
    PROP_SETS(self->window, net_wm_visible_name, data);
    self->title = data;

    if (self->frame)
        frame_adjust_title(self->frame);

    g_free(old_title);

    /* update the icon title */
    data = NULL;
    g_free(self->icon_title);

    read_title = TRUE;
    /* try netwm */
    if (!PROP_GETS(self->window, net_wm_icon_name, utf8, &data))
        /* try old x stuff */
        if (!(PROP_GETS(self->window, wm_icon_name, locale, &data)
              || PROP_GETS(self->window, wm_icon_name, utf8, &data))) {
            data = g_strdup(self->title);
            read_title = FALSE;
        }

    /* append the title count, dont display the number for the first window.
     * We don't need to check for config_title_number here since title_count
     * is not set above 1 then. */
    if (read_title && self->title_count > 1) {
        gchar *newdata;
        newdata = g_strdup_printf("%s - [%u]", data, self->title_count);
        g_free(data);
        data = newdata;
    }

    PROP_SETS(self->window, net_wm_visible_icon_name, data);

    self->icon_title = data;
}

void client_update_class(ObClient *self)
{
    gchar **data;
    gchar *s;

    if (self->name) g_free(self->name);
    if (self->class) g_free(self->class);
    if (self->role) g_free(self->role);

    self->name = self->class = self->role = NULL;

    if (PROP_GETSS(self->window, wm_class, locale, &data)) {
        if (data[0]) {
            self->name = g_strdup(data[0]);
            if (data[1])
                self->class = g_strdup(data[1]);
        }
        g_strfreev(data);     
    }

    if (PROP_GETS(self->window, wm_window_role, locale, &s))
        self->role = s;

    if (self->name == NULL) self->name = g_strdup("");
    if (self->class == NULL) self->class = g_strdup("");
    if (self->role == NULL) self->role = g_strdup("");
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

    if (self->frame)
        frame_adjust_icon(self->frame);
}

void client_update_user_time(ObClient *self, gboolean new_event)
{
    guint32 time;

    if (PROP_GET32(self->window, net_wm_user_time, cardinal, &time)) {
        self->user_time = time;
        /* we set this every time, not just when it grows, because in practice
           sometimes time goes backwards! (ntpdate.. yay....) so.. if it goes
           backward we don't want all windows to stop focusing. we'll just
           assume noone is setting times older than the last one, cuz that
           would be pretty stupid anyways
           However! This is called when a window is mapped to get its user time
           but it's an old number, it's not changing it from new user
           interaction, so in that case, don't change the last user time.
        */
        if (new_event)
            client_last_user_time = time;

        /*ob_debug("window 0x%x user time %u\n", self->window, time);*/
    }
}

static void client_change_state(ObClient *self)
{
    gulong state[2];
    gulong netstate[11];
    guint num;

    state[0] = self->wmstate;
    state[1] = None;
    PROP_SETA32(self->window, wm_state, wm_state, state, 2);

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

    if (self->fullscreen &&
        (client_focused(self) || client_search_focus_tree(self)))
        l = OB_STACKING_LAYER_FULLSCREEN;
    else if (self->type == OB_CLIENT_TYPE_DESKTOP)
        l = OB_STACKING_LAYER_DESKTOP;
    else if (self->type == OB_CLIENT_TYPE_DOCK) {
        if (self->below) l = OB_STACKING_LAYER_NORMAL;
        else l = OB_STACKING_LAYER_ABOVE;
    }
    else if (self->above) l = OB_STACKING_LAYER_ABOVE;
    else if (self->below) l = OB_STACKING_LAYER_BELOW;
    else l = OB_STACKING_LAYER_NORMAL;

    return l;
}

static void client_calc_layer_recursive(ObClient *self, ObClient *orig,
                                        ObStackingLayer min, gboolean raised)
{
    ObStackingLayer old, own;
    GSList *it;

    old = self->layer;
    own = calc_layer(self);
    self->layer = MAX(own, min);

    ob_debug("layer for %s: %d\n", self->title, self->layer);

    for (it = self->transients; it; it = g_slist_next(it))
        client_calc_layer_recursive(it->data, orig,
                                    self->layer,
                                    raised ? raised : self->layer != old);

    if (!raised && self->layer != old)
        if (orig->frame) { /* only restack if the original window is managed */
            stacking_remove(CLIENT_AS_WINDOW(self));
            stacking_add(CLIENT_AS_WINDOW(self));
        }
}

void client_calc_layer(ObClient *self)
{
    ObClient *orig;
    GSList *it;

    orig = self;

    /* transients take on the layer of their parents */
    it = client_search_top_transients(self);

    for (; it; it = g_slist_next(it))
        client_calc_layer_recursive(it->data, orig, 0, FALSE);
}

gboolean client_should_show(ObClient *self)
{
    if (self->iconic)
        return FALSE;
    if (client_normal(self) && screen_showing_desktop)
        return FALSE;
    /*
    if (self->transient_for) {
        if (self->transient_for != OB_TRAN_GROUP)
            return client_should_show(self->transient_for);
        else {
            GSList *it;

            for (it = self->group->members; it; it = g_slist_next(it)) {
                ObClient *c = it->data;
                if (c != self && !c->transient_for) {
                    if (client_should_show(c))
                        return TRUE;
                }
            }
        }
    }
    */
    if (self->desktop == screen_desktop || self->desktop == DESKTOP_ALL)
        return TRUE;
    
    return FALSE;
}

static void client_showhide(ObClient *self)
{

    if (client_should_show(self))
        frame_show(self->frame);
    else
        frame_hide(self->frame);
}

gboolean client_normal(ObClient *self) {
    return ! (self->type == OB_CLIENT_TYPE_DESKTOP ||
              self->type == OB_CLIENT_TYPE_DOCK ||
              self->type == OB_CLIENT_TYPE_SPLASH);
}

static void client_apply_startup_state(ObClient *self)
{
    /* these are in a carefully crafted order.. */

    if (self->iconic) {
        self->iconic = FALSE;
        client_iconify(self, TRUE, FALSE);
    }
    if (self->fullscreen) {
        self->fullscreen = FALSE;
        client_fullscreen(self, TRUE, FALSE);
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
        client_maximize(self, TRUE, 0, FALSE);
    } else if (self->max_vert) {
        self->max_vert = FALSE;
        client_maximize(self, TRUE, 2, FALSE);
    } else if (self->max_horz) {
        self->max_horz = FALSE;
        client_maximize(self, TRUE, 1, FALSE);
    }

    /* nothing to do for the other states:
       skip_taskbar
       skip_pager
       modal
       above
       below
    */
}

void client_configure_full(ObClient *self, ObCorner anchor,
                           gint x, gint y, gint w, gint h,
                           gboolean user, gboolean final,
                           gboolean force_reply)
{
    gint oldw, oldh;
    gboolean send_resize_client;
    gboolean moved = FALSE, resized = FALSE;
    guint fdecor = self->frame->decorations;
    gboolean fhorz = self->frame->max_horz;
    Rect desired_area = {x, y, w, h};

    /* make the frame recalculate its dimentions n shit without changing
       anything visible for real, this way the constraints below can work with
       the updated frame dimensions. */
    frame_adjust_area(self->frame, TRUE, TRUE, TRUE);

    /* gets the frame's position */
    frame_client_gravity(self->frame, &x, &y);

    /* these positions are frame positions, not client positions */

    /* set the size and position if fullscreen */
    if (self->fullscreen) {
        Rect *a;
        guint i;

        i = screen_find_monitor(&desired_area);
        a = screen_physical_area_monitor(i);

        x = a->x;
        y = a->y;
        w = a->width;
        h = a->height;

        user = FALSE; /* ignore that increment etc shit when in fullscreen */
    } else {
        Rect *a;
        guint i;

        i = screen_find_monitor(&desired_area);
        a = screen_area_monitor(self->desktop, i);

        /* set the size and position if maximized */
        if (self->max_horz) {
            x = a->x;
            w = a->width - self->frame->size.left - self->frame->size.right;
        }
        if (self->max_vert) {
            y = a->y;
            h = a->height - self->frame->size.top - self->frame->size.bottom;
        }
    }

    /* gets the client's position */
    frame_frame_gravity(self->frame, &x, &y);

    /* these override the above states! if you cant move you can't move! */
    if (user) {
        if (!(self->functions & OB_CLIENT_FUNC_MOVE)) {
            x = self->area.x;
            y = self->area.y;
        }
        if (!(self->functions & OB_CLIENT_FUNC_RESIZE)) {
            w = self->area.width;
            h = self->area.height;
        }
    }

    if (!(w == self->area.width && h == self->area.height)) {
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
        if (w > self->max_size.width) w = self->max_size.width;
        if (w < minw) w = minw;
        if (h > self->max_size.height) h = self->max_size.height;
        if (h < minh) h = minh;

        w -= basew;
        h -= baseh;

        /* keep to the increments */
        w /= self->size_inc.width;
        h /= self->size_inc.height;

        /* you cannot resize to nothing */
        if (basew + w < 1) w = 1 - basew;
        if (baseh + h < 1) h = 1 - baseh;
  
        /* store the logical size */
        SIZE_SET(self->logical_size,
                 self->size_inc.width > 1 ? w : w + basew,
                 self->size_inc.height > 1 ? h : h + baseh);

        w *= self->size_inc.width;
        h *= self->size_inc.height;

        w += basew;
        h += baseh;

        /* adjust the height to match the width for the aspect ratios.
           for this, min size is not substituted for base size ever. */
        w -= self->base_size.width;
        h -= self->base_size.height;

        if (!self->fullscreen) {
            if (self->min_ratio)
                if (h * self->min_ratio > w) {
                    h = (gint)(w / self->min_ratio);

                    /* you cannot resize to nothing */
                    if (h < 1) {
                        h = 1;
                        w = (gint)(h * self->min_ratio);
                    }
                }
            if (self->max_ratio)
                if (h * self->max_ratio < w) {
                    h = (gint)(w / self->max_ratio);

                    /* you cannot resize to nothing */
                    if (h < 1) {
                        h = 1;
                        w = (gint)(h * self->min_ratio);
                    }
                }
        }

        w += self->base_size.width;
        h += self->base_size.height;
    }

    g_assert(w > 0);
    g_assert(h > 0);

    switch (anchor) {
    case OB_CORNER_TOPLEFT:
        break;
    case OB_CORNER_TOPRIGHT:
        x -= w - self->area.width;
        break;
    case OB_CORNER_BOTTOMLEFT:
        y -= h - self->area.height;
        break;
    case OB_CORNER_BOTTOMRIGHT:
        x -= w - self->area.width;
        y -= h - self->area.height;
        break;
    }

    moved = x != self->area.x || y != self->area.y;
    resized = w != self->area.width || h != self->area.height;

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
    if (send_resize_client && user && (w > oldw || h > oldh))
        XResizeWindow(ob_display, self->window, MAX(w, oldw), MAX(h, oldh));

    /* move/resize the frame to match the request */
    if (self->frame) {
        if (self->decorations != fdecor || self->max_horz != fhorz)
            moved = resized = TRUE;

        if (moved || resized)
            frame_adjust_area(self->frame, moved, resized, FALSE);

        if (!resized && (force_reply || ((!user && moved) || (user && final))))
        {
            XEvent event;
            event.type = ConfigureNotify;
            event.xconfigure.display = ob_display;
            event.xconfigure.event = self->window;
            event.xconfigure.window = self->window;

            /* root window real coords */
            event.xconfigure.x = self->frame->area.x + self->frame->size.left -
                self->border_width;
            event.xconfigure.y = self->frame->area.y + self->frame->size.top -
                self->border_width;
            event.xconfigure.width = w;
            event.xconfigure.height = h;
            event.xconfigure.border_width = 0;
            event.xconfigure.above = self->frame->plate;
            event.xconfigure.override_redirect = FALSE;
            XSendEvent(event.xconfigure.display, event.xconfigure.window,
                       FALSE, StructureNotifyMask, &event);
        }
    }

    /* if the client is shrinking, then resize the frame before the client */
    if (send_resize_client && (!user || (w <= oldw || h <= oldh)))
        XResizeWindow(ob_display, self->window, w, h);

    XFlush(ob_display);
}

void client_fullscreen(ObClient *self, gboolean fs, gboolean savearea)
{
    gint x, y, w, h;

    if (!(self->functions & OB_CLIENT_FUNC_FULLSCREEN) || /* can't */
        self->fullscreen == fs) return;                   /* already done */

    self->fullscreen = fs;
    client_change_state(self); /* change the state hints on the client */
    client_calc_layer(self);   /* and adjust out layer/stacking */

    if (fs) {
        if (savearea)
            self->pre_fullscreen_area = self->area;

        /* these are not actually used cuz client_configure will set them
           as appropriate when the window is fullscreened */
        x = y = w = h = 0;
    } else {
        Rect *a;

        if (self->pre_fullscreen_area.width > 0 &&
            self->pre_fullscreen_area.height > 0)
        {
            x = self->pre_fullscreen_area.x;
            y = self->pre_fullscreen_area.y;
            w = self->pre_fullscreen_area.width;
            h = self->pre_fullscreen_area.height;
            RECT_SET(self->pre_fullscreen_area, 0, 0, 0, 0);
        } else {
            /* pick some fallbacks... */
            a = screen_area_monitor(self->desktop, 0);
            x = a->x + a->width / 4;
            y = a->y + a->height / 4;
            w = a->width / 2;
            h = a->height / 2;
        }
    }

    client_setup_decor_and_functions(self);

    client_move_resize(self, x, y, w, h);

    /* try focus us when we go into fullscreen mode */
    client_focus(self);
}

static void client_iconify_recursive(ObClient *self,
                                     gboolean iconic, gboolean curdesk)
{
    GSList *it;
    gboolean changed = FALSE;


    if (self->iconic != iconic) {
        ob_debug("%sconifying window: 0x%lx\n", (iconic ? "I" : "Uni"),
                 self->window);

        self->iconic = iconic;

        if (iconic) {
            if (self->functions & OB_CLIENT_FUNC_ICONIFY) {
                glong old;

                old = self->wmstate;
                self->wmstate = IconicState;
                if (old != self->wmstate)
                    PROP_MSG(self->window, kde_wm_change_state,
                             self->wmstate, 1, 0, 0);

                /* update the focus lists.. iconic windows go to the bottom of
                   the list, put the new iconic window at the 'top of the
                   bottom'. */
                focus_order_to_top(self);

                changed = TRUE;
            }
        } else {
            glong old;

            if (curdesk)
                client_set_desktop(self, screen_desktop, FALSE);

            old = self->wmstate;
            self->wmstate = self->shaded ? IconicState : NormalState;
            if (old != self->wmstate)
                PROP_MSG(self->window, kde_wm_change_state,
                         self->wmstate, 1, 0, 0);

            /* this puts it after the current focused window */
            focus_order_remove(self);
            focus_order_add_new(self);

            changed = TRUE;
        }
    }

    if (changed) {
        client_change_state(self);
        client_showhide(self);
        if (STRUT_EXISTS(self->strut))
            screen_update_areas();
    }

    /* iconify all transients */
    for (it = self->transients; it; it = g_slist_next(it))
        if (it->data != self) client_iconify_recursive(it->data,
                                                       iconic, curdesk);
}

void client_iconify(ObClient *self, gboolean iconic, gboolean curdesk)
{
    GSList *it;

    /* move up the transient chain as far as possible first */
    it = client_search_top_transients(self);

    for (; it; it = g_slist_next(it))
    client_iconify_recursive(it->data, iconic, curdesk);
}

void client_maximize(ObClient *self, gboolean max, gint dir, gboolean savearea)
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

    /* we just tell it to configure in the same place and client_configure
       worries about filling the screen with the window */
    x = self->area.x;
    y = self->area.y;
    w = self->area.width;
    h = self->area.height;

    if (max) {
        if (savearea) {
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
        }
    } else {
        Rect *a;

        a = screen_area_monitor(self->desktop, 0);
        if ((dir == 0 || dir == 1) && self->max_horz) { /* horz */
            if (self->pre_max_area.width > 0) {
                x = self->pre_max_area.x;
                w = self->pre_max_area.width;

                RECT_SET(self->pre_max_area, 0, self->pre_max_area.y,
                         0, self->pre_max_area.height);
            } else {
                /* pick some fallbacks... */
                x = a->x + a->width / 4;
                w = a->width / 2;
            }
        }
        if ((dir == 0 || dir == 2) && self->max_vert) { /* vert */
            if (self->pre_max_area.height > 0) {
                y = self->pre_max_area.y;
                h = self->pre_max_area.height;

                RECT_SET(self->pre_max_area, self->pre_max_area.x, 0,
                         self->pre_max_area.width, 0);
            } else {
                /* pick some fallbacks... */
                y = a->y + a->height / 4;
                h = a->height / 2;
            }
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

    /* when we're iconic, don't change the wmstate */
    if (!self->iconic) {
        glong old;

        old = self->wmstate;
        self->wmstate = shade ? IconicState : NormalState;
        if (old != self->wmstate)
            PROP_MSG(self->window, kde_wm_change_state,
                     self->wmstate, 1, 0, 0);
    }

    self->shaded = shade;
    client_change_state(self);
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
    /* don't allow focused windows to hilite */
    self->demands_attention = hilite && !client_focused(self);
    if (self->demands_attention)
        frame_flash_start(self->frame);
    else
        frame_flash_stop(self->frame);
    client_change_state(self);
}

void client_set_desktop_recursive(ObClient *self,
                                  guint target, gboolean donthide)
{
    guint old;
    GSList *it;

    if (target != self->desktop) {

        ob_debug("Setting desktop %u\n", target+1);

        g_assert(target < screen_num_desktops || target == DESKTOP_ALL);

        /* remove from the old desktop(s) */
        focus_order_remove(self);

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
            client_raise(self);
        if (STRUT_EXISTS(self->strut))
            screen_update_areas();

        /* add to the new desktop(s) */
        if (config_focus_new)
            focus_order_to_top(self);
        else
            focus_order_to_bottom(self);
    }

    /* move all transients */
    for (it = self->transients; it; it = g_slist_next(it))
        if (it->data != self) client_set_desktop_recursive(it->data,
                                                           target, donthide);
}

void client_set_desktop(ObClient *self, guint target, gboolean donthide)
{
    GSList *it;

    it = client_search_top_transients(self);

    for(; it; it = g_slist_next(it))
        client_set_desktop_recursive(it->data, target, donthide);
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
        client_iconify(self, TRUE, TRUE);
        break;
    case NormalState:
        client_iconify(self, FALSE, TRUE);
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
                self->above = TRUE;
                self->below = FALSE;
            } else if (state == prop_atoms.net_wm_state_below) {
                self->above = FALSE;
                self->below = TRUE;
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
                self->above = FALSE;
            } else if (state == prop_atoms.net_wm_state_below) {
                self->below = FALSE;
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
                client_maximize(self, max_horz, 0, TRUE);
            } else {
                client_maximize(self, max_horz, 1, TRUE);
                client_maximize(self, max_vert, 2, TRUE);
            }
        } else {
            /* toggling one */
            if (max_horz != self->max_horz)
                client_maximize(self, max_horz, 1, TRUE);
            else
                client_maximize(self, max_vert, 2, TRUE);
        }
    }
    /* change fullscreen state before shading, as it will affect if the window
       can shade or not */
    if (fullscreen != self->fullscreen)
        client_fullscreen(self, fullscreen, TRUE);
    if (shaded != self->shaded)
        client_shade(self, shaded);
    if (undecorated != self->undecorated)
        client_set_undecorated(self, undecorated);
    if (modal != self->modal) {
        self->modal = modal;
        /* when a window changes modality, then its stacking order with its
           transients needs to change */
        client_raise(self);
    }
    if (iconic != self->iconic)
        client_iconify(self, iconic, FALSE);

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
    XEvent ev;

    /* choose the correct target */
    self = client_focus_target(self);

    if (!self->frame->visible)
        return FALSE;

    if (!(self->can_focus || self->focus_notify))
        return FALSE;

    /* do a check to see if the window has already been unmapped or destroyed
       do this intelligently while watching out for unmaps we've generated
       (ignore_unmaps > 0) */
    if (XCheckTypedWindowEvent(ob_display, self->window,
                               DestroyNotify, &ev)) {
        XPutBackEvent(ob_display, &ev);
        return FALSE;
    }
    while (XCheckTypedWindowEvent(ob_display, self->window,
                                  UnmapNotify, &ev)) {
        if (self->ignore_unmaps) {
            self->ignore_unmaps--;
        } else {
            XPutBackEvent(ob_display, &ev);
            return FALSE;
        }
    }

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

    if (self->can_focus) {
        /* RevertToPointerRoot causes much more headache than RevertToNone, so
           I choose to use it always, hopefully to find errors quicker, if any
           are left. (I hate X. I hate focus events.)
           
           Update: Changing this to RevertToNone fixed a bug with mozilla (bug
           #799. So now it is RevertToNone again.
        */
        XSetInputFocus(ob_display, self->window, RevertToNone,
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

#ifdef DEBUG_FOCUS
    ob_debug("%sively focusing %lx at %d\n",
             (self->can_focus ? "act" : "pass"),
             self->window, (gint) event_curtime);
#endif

    /* Cause the FocusIn to come back to us. Important for desktop switches,
       since otherwise we'll have no FocusIn on the queue and send it off to
       the focus_backup. */
    XSync(ob_display, FALSE);
    return TRUE;
}

/* Used when the current client is closed, focus_last will then prevent
 * focus from going to the mouse pointer */
void client_unfocus(ObClient *self)
{
    if (focus_client == self) {
#ifdef DEBUG_FOCUS
        ob_debug("client_unfocus for %lx\n", self->window);
#endif
        focus_fallback(OB_FOCUS_FALLBACK_CLOSED);
    }
}

void client_activate(ObClient *self, gboolean here, gboolean user, Time time)
{
    /* XXX do some stuff here if user is false to determine if we really want
       to activate it or not (a parent or group member is currently
       active)?
    */
    ob_debug("Want to activate window 0x%x with time %u (last time %u), "
             "source=%s\n",
             self->window, time, client_last_user_time,
             (user ? "user" : "application"));
    if (!user && time && time < client_last_user_time)
        client_hilite(self, TRUE);
    else {
        if (client_normal(self) && screen_showing_desktop)
            screen_show_desktop(FALSE);
        if (self->iconic)
            client_iconify(self, FALSE, here);
        if (self->desktop != DESKTOP_ALL &&
            self->desktop != screen_desktop) {
            if (here)
                client_set_desktop(self, screen_desktop, FALSE);
            else
                screen_set_desktop(self->desktop);
        } else if (!self->frame->visible)
            /* if its not visible for other reasons, then don't mess
               with it */
            return;
        if (self->shaded)
            client_shade(self, FALSE);

        client_focus(self);

        /* we do this an action here. this is rather important. this is because
           we want the results from the focus change to take place BEFORE we go
           about raising the window. when a fullscreen window loses focus, we
           need this or else the raise wont be able to raise above the
           to-lose-focus fullscreen window. */
        client_raise(self);
    }
}

void client_raise(ObClient *self)
{
    action_run_string("Raise", self, CurrentTime);
}

void client_lower(ObClient *self)
{
    action_run_string("Lower", self, CurrentTime);
}

gboolean client_focused(ObClient *self)
{
    return self == focus_client;
}

static ObClientIcon* client_icon_recursive(ObClient *self, gint w, gint h)
{
    guint i;
    /* si is the smallest image >= req */
    /* li is the largest image < req */
    gulong size, smallest = 0xffffffff, largest = 0, si = 0, li = 0;

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

    for (i = 0; i < self->nicons; ++i) {
        size = self->icons[i].width * self->icons[i].height;
        if (size < smallest && size >= (unsigned)(w * h)) {
            smallest = size;
            si = i;
        }
        if (size > largest && size <= (unsigned)(w * h)) {
            largest = size;
            li = i;
        }
    }
    if (largest == 0) /* didnt find one smaller than the requested size */
        return &self->icons[si];
    return &self->icons[li];
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

/* this be mostly ripped from fvwm */
ObClient *client_find_directional(ObClient *c, ObDirection dir) 
{
    gint my_cx, my_cy, his_cx, his_cy;
    gint offset = 0;
    gint distance = 0;
    gint score, best_score;
    ObClient *best_client, *cur;
    GList *it;

    if(!client_list)
        return NULL;

    /* first, find the centre coords of the currently focused window */
    my_cx = c->frame->area.x + c->frame->area.width / 2;
    my_cy = c->frame->area.y + c->frame->area.height / 2;

    best_score = -1;
    best_client = NULL;

    for(it = g_list_first(client_list); it; it = g_list_next(it)) {
        cur = it->data;

        /* the currently selected window isn't interesting */
        if(cur == c)
            continue;
        if (!client_normal(cur))
            continue;
        /* using c->desktop instead of screen_desktop doesn't work if the
         * current window was omnipresent, hope this doesn't have any other
         * side effects */
        if(screen_desktop != cur->desktop && cur->desktop != DESKTOP_ALL)
            continue;
        if(cur->iconic)
            continue;
        if(!(client_focus_target(cur) == cur &&
             client_can_focus(cur)))
            continue;

        /* find the centre coords of this window, from the
         * currently focused window's point of view */
        his_cx = (cur->frame->area.x - my_cx)
            + cur->frame->area.width / 2;
        his_cy = (cur->frame->area.y - my_cy)
            + cur->frame->area.height / 2;

        if(dir == OB_DIRECTION_NORTHEAST || dir == OB_DIRECTION_SOUTHEAST ||
           dir == OB_DIRECTION_SOUTHWEST || dir == OB_DIRECTION_NORTHWEST) {
            gint tx;
            /* Rotate the diagonals 45 degrees counterclockwise.
             * To do this, multiply the matrix /+h +h\ with the
             * vector (x y).                   \-h +h/
             * h = sqrt(0.5). We can set h := 1 since absolute
             * distance doesn't matter here. */
            tx = his_cx + his_cy;
            his_cy = -his_cx + his_cy;
            his_cx = tx;
        }

        switch(dir) {
        case OB_DIRECTION_NORTH:
        case OB_DIRECTION_SOUTH:
        case OB_DIRECTION_NORTHEAST:
        case OB_DIRECTION_SOUTHWEST:
            offset = (his_cx < 0) ? -his_cx : his_cx;
            distance = ((dir == OB_DIRECTION_NORTH ||
                         dir == OB_DIRECTION_NORTHEAST) ?
                        -his_cy : his_cy);
            break;
        case OB_DIRECTION_EAST:
        case OB_DIRECTION_WEST:
        case OB_DIRECTION_SOUTHEAST:
        case OB_DIRECTION_NORTHWEST:
            offset = (his_cy < 0) ? -his_cy : his_cy;
            distance = ((dir == OB_DIRECTION_WEST ||
                         dir == OB_DIRECTION_NORTHWEST) ?
                        -his_cx : his_cx);
            break;
        }

        /* the target must be in the requested direction */
        if(distance <= 0)
            continue;

        /* Calculate score for this window.  The smaller the better. */
        score = distance + offset;

        /* windows more than 45 degrees off the direction are
         * heavily penalized and will only be chosen if nothing
         * else within a million pixels */
        if(offset > distance)
            score += 1000000;

        if(best_score == -1 || score < best_score)
            best_client = cur,
                best_score = score;
    }

    return best_client;
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
    if (self->undecorated != undecorated) {
        self->undecorated = undecorated;
        client_setup_decor_and_functions(self);
        /* Make sure the client knows it might have moved. Maybe there is a
         * better way of doing this so only one client_configure is sent, but
         * since 125 of these are sent per second when moving the window (with
         * user = FALSE) i doubt it matters much.
         */
        client_configure(self, OB_CORNER_TOPLEFT, self->area.x, self->area.y,
                         self->area.width, self->area.height, TRUE, TRUE);
        client_change_state(self); /* reflect this in the state hints */
    }
}

guint client_monitor(ObClient *self)
{
    return screen_find_monitor(&self->frame->area);
}

GSList *client_search_top_transients(ObClient *self)
{
    GSList *ret = NULL;

    /* move up the direct transient chain as far as possible */
    while (self->transient_for && self->transient_for != OB_TRAN_GROUP)
        self = self->transient_for;

    if (!self->transient_for)
        ret = g_slist_prepend(ret, self);
    else {
            GSList *it;

            g_assert(self->group);

            for (it = self->group->members; it; it = g_slist_next(it)) {
                ObClient *c = it->data;

                if (!c->transient_for)
                    ret = g_slist_prepend(ret, c);
            }

            if (ret == NULL) /* no group parents */
                ret = g_slist_prepend(ret, self);
    }

    return ret;
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

void client_update_sm_client_id(ObClient *self)
{
    g_free(self->sm_client_id);
    self->sm_client_id = NULL;

    if (!PROP_GETS(self->window, sm_client_id, locale, &self->sm_client_id) &&
        self->group)
        PROP_GETS(self->group->leader, sm_client_id, locale,
                  &self->sm_client_id);
}

#define WANT_EDGE(cur, c) \
            if(cur == c)                                                      \
                continue;                                                     \
            if(!client_normal(cur))                                   \
                continue;                                                     \
            if(screen_desktop != cur->desktop && cur->desktop != DESKTOP_ALL) \
                continue;                                                     \
            if(cur->iconic)                                                   \
                continue;                                                     \
            if(cur->layer < c->layer && !config_resist_layers_below)          \
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
                    RECT_CONTAINS(c->frame->area, x, y)) {
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
