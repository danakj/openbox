/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   dock.c for the Openbox window manager
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

#include "debug.h"
#include "dock.h"
#include "mainloop.h"
#include "screen.h"
#include "prop.h"
#include "config.h"
#include "grab.h"
#include "openbox.h"
#include "render/theme.h"

#define DOCK_EVENT_MASK (ButtonPressMask | ButtonReleaseMask | \
                         EnterWindowMask | LeaveWindowMask)
#define DOCKAPP_EVENT_MASK (StructureNotifyMask)

static ObDock *dock;

StrutPartial dock_strut;

static void dock_app_grab_button(ObDockApp *app, gboolean grab)
{
    if (grab) {
        grab_button_full(config_dock_app_move_button,
                         config_dock_app_move_modifiers, app->icon_win,
                         ButtonPressMask | ButtonReleaseMask |
                         ButtonMotionMask,
                         GrabModeAsync, OB_CURSOR_MOVE);
    } else {
        ungrab_button(config_dock_app_move_button,
                      config_dock_app_move_modifiers, app->icon_win);
    }
}

void dock_startup(gboolean reconfig)
{
    XSetWindowAttributes attrib;

    if (reconfig) {
        GList *it;

        XSetWindowBorder(ob_display, dock->frame,
                         RrColorPixel(ob_rr_theme->frame_b_color));
        XSetWindowBorderWidth(ob_display, dock->frame, ob_rr_theme->fbwidth);

        RrAppearanceFree(dock->a_frame);
        dock->a_frame = RrAppearanceCopy(ob_rr_theme->a_focused_title);

        stacking_add(DOCK_AS_WINDOW(dock));

        dock_configure();
        dock_hide(TRUE);

        for (it = dock->dock_apps; it; it = g_list_next(it))
            dock_app_grab_button(it->data, TRUE);
        return;
    }

    STRUT_PARTIAL_SET(dock_strut, 0, 0, 0, 0,
                      0, 0, 0, 0, 0, 0, 0, 0);

    dock = g_new0(ObDock, 1);
    dock->obwin.type = Window_Dock;

    dock->hidden = TRUE;

    attrib.event_mask = DOCK_EVENT_MASK;
    attrib.override_redirect = True;
    dock->frame = XCreateWindow(ob_display, RootWindow(ob_display, ob_screen),
                                0, 0, 1, 1, 0,
                                RrDepth(ob_rr_inst), InputOutput,
                                RrVisual(ob_rr_inst),
                                CWOverrideRedirect | CWEventMask,
                                &attrib);
    dock->a_frame = RrAppearanceCopy(ob_rr_theme->a_focused_title);
    XSetWindowBorder(ob_display, dock->frame,
                     RrColorPixel(ob_rr_theme->frame_b_color));
    XSetWindowBorderWidth(ob_display, dock->frame, ob_rr_theme->fbwidth);

    g_hash_table_insert(window_map, &dock->frame, dock);
    stacking_add(DOCK_AS_WINDOW(dock));
}

void dock_shutdown(gboolean reconfig)
{
    if (reconfig) {
        GList *it;

        stacking_remove(DOCK_AS_WINDOW(dock));

        for (it = dock->dock_apps; it; it = g_list_next(it))
            dock_app_grab_button(it->data, FALSE);
        return;
    }

    XDestroyWindow(ob_display, dock->frame);
    RrAppearanceFree(dock->a_frame);
    g_hash_table_remove(window_map, &dock->frame);
    stacking_remove(dock);
}

void dock_add(Window win, XWMHints *wmhints)
{
    ObDockApp *app;
    XWindowAttributes attrib;
    gchar **data;

    app = g_new0(ObDockApp, 1);
    app->obwin.type = Window_DockApp;
    app->win = win;
    app->icon_win = (wmhints->flags & IconWindowHint) ?
        wmhints->icon_window : win;

    if (PROP_GETSS(app->win, wm_class, locale, &data)) {
        if (data[0]) {
            app->name = g_strdup(data[0]);
            if (data[1])
                app->class = g_strdup(data[1]);
        }
        g_strfreev(data);     
    }

    if (app->name == NULL) app->name = g_strdup("");
    if (app->class == NULL) app->class = g_strdup("");
    
    if (XGetWindowAttributes(ob_display, app->icon_win, &attrib)) {
        app->w = attrib.width;
        app->h = attrib.height;
    } else {
        app->w = app->h = 64;
    }

    dock->dock_apps = g_list_append(dock->dock_apps, app);
    dock_configure();

    XReparentWindow(ob_display, app->icon_win, dock->frame, app->x, app->y);
    /*
      This is the same case as in frame.c for client windows. When Openbox is
      starting, the window is already mapped so we see unmap events occur for
      it. There are 2 unmap events generated that we see, one with the 'event'
      member set the root window, and one set to the client, but both get
      handled and need to be ignored.
    */
    if (ob_state() == OB_STATE_STARTING)
        app->ignore_unmaps += 2;

    if (app->win != app->icon_win) {
        /* have to map it so that it can be re-managed on a restart */
        XMoveWindow(ob_display, app->win, -1000, -1000);
        XMapWindow(ob_display, app->win);
    }
    XMapWindow(ob_display, app->icon_win);
    XSync(ob_display, False);

    /* specify that if we exit, the window should not be destroyed and should
       be reparented back to root automatically */
    XChangeSaveSet(ob_display, app->icon_win, SetModeInsert);
    XSelectInput(ob_display, app->icon_win, DOCKAPP_EVENT_MASK);

    dock_app_grab_button(app, TRUE);

    g_hash_table_insert(window_map, &app->icon_win, app);

    ob_debug("Managed Dock App: 0x%lx (%s)\n", app->icon_win, app->class);
}

void dock_remove_all()
{
    while (dock->dock_apps)
        dock_remove(dock->dock_apps->data, TRUE);
}

void dock_remove(ObDockApp *app, gboolean reparent)
{
    dock_app_grab_button(app, FALSE);
    XSelectInput(ob_display, app->icon_win, NoEventMask);
    /* remove the window from our save set */
    XChangeSaveSet(ob_display, app->icon_win, SetModeDelete);
    XSync(ob_display, False);

    g_hash_table_remove(window_map, &app->icon_win);

    if (reparent)
        XReparentWindow(ob_display, app->icon_win,
                        RootWindow(ob_display, ob_screen), app->x, app->y);

    dock->dock_apps = g_list_remove(dock->dock_apps, app);
    dock_configure();

    ob_debug("Unmanaged Dock App: 0x%lx (%s)\n", app->icon_win, app->class);

    g_free(app->name);
    g_free(app->class);
    g_free(app);
}

void dock_configure()
{
    GList *it;
    gint spot;
    gint gravity;
    gint minw, minh;
    gint strw, strh;
    Rect *a;

    RrMinsize(dock->a_frame, &minw, &minh);

    dock->w = dock->h = 0;

    /* get the size */
    for (it = dock->dock_apps; it; it = g_list_next(it)) {
        ObDockApp *app = it->data;
        switch (config_dock_orient) {
        case OB_ORIENTATION_HORZ:
            dock->w += app->w;
            dock->h = MAX(dock->h, app->h);
            break;
        case OB_ORIENTATION_VERT:
            dock->w = MAX(dock->w, app->w);
            dock->h += app->h;
            break;
        }
    }

    spot = (config_dock_orient == OB_ORIENTATION_HORZ ? minw : minh) / 2;

    /* position the apps */
    for (it = dock->dock_apps; it; it = g_list_next(it)) {
        ObDockApp *app = it->data;
        switch (config_dock_orient) {
        case OB_ORIENTATION_HORZ:
            app->x = spot;
            app->y = (dock->h - app->h) / 2;
            spot += app->w;
            break;
        case OB_ORIENTATION_VERT:
            app->x = (dock->w - app->w) / 2;
            app->y = spot;
            spot += app->h;
            break;
        }

        XMoveWindow(ob_display, app->icon_win, app->x, app->y);
    }

    /* used for calculating offsets */
    dock->w += ob_rr_theme->fbwidth * 2;
    dock->h += ob_rr_theme->fbwidth * 2;

    a = screen_physical_area();

    /* calculate position */
    if (config_dock_floating) {
        dock->x = config_dock_x;
        dock->y = config_dock_y;
        gravity = NorthWestGravity;
    } else {
        switch (config_dock_pos) {
        case OB_DIRECTION_NORTHWEST:
            dock->x = 0;
            dock->y = 0;
            gravity = NorthWestGravity;
            break;
        case OB_DIRECTION_NORTH:
            dock->x = a->width / 2;
            dock->y = 0;
            gravity = NorthGravity;
            break;
        case OB_DIRECTION_NORTHEAST:
            dock->x = a->width;
            dock->y = 0;
            gravity = NorthEastGravity;
            break;
        case OB_DIRECTION_WEST:
            dock->x = 0;
            dock->y = a->height / 2;
            gravity = WestGravity;
            break;
        case OB_DIRECTION_EAST:
            dock->x = a->width;
            dock->y = a->height / 2;
            gravity = EastGravity;
            break;
        case OB_DIRECTION_SOUTHWEST:
            dock->x = 0;
            dock->y = a->height;
            gravity = SouthWestGravity;
            break;
        case OB_DIRECTION_SOUTH:
            dock->x = a->width / 2;
            dock->y = a->height;
            gravity = SouthGravity;
            break;
        case OB_DIRECTION_SOUTHEAST:
            dock->x = a->width;
            dock->y = a->height;
            gravity = SouthEastGravity;
            break;
        default:
            g_assert_not_reached();
        }
    }

    switch(gravity) {
    case NorthGravity:
    case CenterGravity:
    case SouthGravity:
        dock->x -= dock->w / 2;
        break;
    case NorthEastGravity:
    case EastGravity:
    case SouthEastGravity:
        dock->x -= dock->w;
        break;
    }
    switch(gravity) {
    case WestGravity:
    case CenterGravity:
    case EastGravity:
        dock->y -= dock->h / 2;
        break;
    case SouthWestGravity:
    case SouthGravity:
    case SouthEastGravity:
        dock->y -= dock->h;
        break;
    }

    if (config_dock_hide && dock->hidden) {
        if (!config_dock_floating) {
            switch (config_dock_pos) {
            case OB_DIRECTION_NORTHWEST:
                switch (config_dock_orient) {
                case OB_ORIENTATION_HORZ:
                    dock->y -= dock->h - ob_rr_theme->fbwidth;
                    break;
                case OB_ORIENTATION_VERT:
                    dock->x -= dock->w - ob_rr_theme->fbwidth;
                    break;
                }
                break;
            case OB_DIRECTION_NORTH:
                dock->y -= dock->h - ob_rr_theme->fbwidth;
                break;
            case OB_DIRECTION_NORTHEAST:
                switch (config_dock_orient) {
                case OB_ORIENTATION_HORZ:
                    dock->y -= dock->h - ob_rr_theme->fbwidth;
                    break;
                case OB_ORIENTATION_VERT:
                    dock->x += dock->w - ob_rr_theme->fbwidth;
                    break;
                }
                break;
            case OB_DIRECTION_WEST:
                dock->x -= dock->w - ob_rr_theme->fbwidth;
                break;
            case OB_DIRECTION_EAST:
                dock->x += dock->w - ob_rr_theme->fbwidth;
                break;
            case OB_DIRECTION_SOUTHWEST:
                switch (config_dock_orient) {
                case OB_ORIENTATION_HORZ:
                    dock->y += dock->h - ob_rr_theme->fbwidth;
                    break;
                case OB_ORIENTATION_VERT:
                    dock->x -= dock->w - ob_rr_theme->fbwidth;
                    break;
                } break;
            case OB_DIRECTION_SOUTH:
                dock->y += dock->h - ob_rr_theme->fbwidth;
                break;
            case OB_DIRECTION_SOUTHEAST:
                switch (config_dock_orient) {
                case OB_ORIENTATION_HORZ:
                    dock->y += dock->h - ob_rr_theme->fbwidth;
                    break;
                case OB_ORIENTATION_VERT:
                    dock->x += dock->w - ob_rr_theme->fbwidth;
                    break;
                }
                break;
            }    
        }
    }

    if (!config_dock_floating && config_dock_hide) {
        strw = ob_rr_theme->fbwidth;
        strh = ob_rr_theme->fbwidth;
    } else {
        strw = dock->w;
        strh = dock->h;
    }

    /* set the strut */
    if (!dock->dock_apps) {
        STRUT_PARTIAL_SET(dock_strut, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0);
    } else if (config_dock_floating || config_dock_nostrut) {
        STRUT_PARTIAL_SET(dock_strut, 0, 0, 0, 0,
                          0, 0, 0, 0, 0, 0, 0, 0);
    } else {
        switch (config_dock_pos) {
        case OB_DIRECTION_NORTHWEST:
            switch (config_dock_orient) {
            case OB_ORIENTATION_HORZ:
                STRUT_PARTIAL_SET(dock_strut, 0, strh, 0, 0,
                                  0, 0, dock->x, dock->x + dock->w - 1,
                                  0, 0, 0, 0);
                break;
            case OB_ORIENTATION_VERT:
                STRUT_PARTIAL_SET(dock_strut, strw, 0, 0, 0,
                                  dock->y, dock->y + dock->h - 1,
                                  0, 0, 0, 0, 0, 0);
                break;
            }
            break;
        case OB_DIRECTION_NORTH:
            STRUT_PARTIAL_SET(dock_strut, 0, strh, 0, 0,
                              dock->x, dock->x + dock->w - 1,
                              0, 0, 0, 0, 0, 0);
            break;
        case OB_DIRECTION_NORTHEAST:
            switch (config_dock_orient) {
            case OB_ORIENTATION_HORZ:
                STRUT_PARTIAL_SET(dock_strut, 0, strh, 0, 0,
                                  0, 0, dock->x, dock->x + dock->w -1,
                                  0, 0, 0, 0);
                break;
            case OB_ORIENTATION_VERT:
                STRUT_PARTIAL_SET(dock_strut, 0, 0, strw, 0,
                                  0, 0, 0, 0,
                                  dock->y, dock->y + dock->h - 1, 0, 0);
                break;
            }
            break;
        case OB_DIRECTION_WEST:
            STRUT_PARTIAL_SET(dock_strut, strw, 0, 0, 0,
                              dock->y, dock->y + dock->h - 1,
                              0, 0, 0, 0, 0, 0);
            break;
        case OB_DIRECTION_EAST:
            STRUT_PARTIAL_SET(dock_strut, 0, 0, strw, 0,
                              0, 0, 0, 0,
                              dock->y, dock->y + dock->h - 1, 0, 0);
            break;
        case OB_DIRECTION_SOUTHWEST:
            switch (config_dock_orient) {
            case OB_ORIENTATION_HORZ:
                STRUT_PARTIAL_SET(dock_strut, 0, 0, 0, strh,
                                  0, 0, 0, 0, 0, 0,
                                  dock->x, dock->x + dock->w - 1);
                break;
            case OB_ORIENTATION_VERT:
                STRUT_PARTIAL_SET(dock_strut, strw, 0, 0, 0,
                                  dock->y, dock->y + dock->h - 1,
                                  0, 0, 0, 0, 0, 0);
                break;
            }
            break;
        case OB_DIRECTION_SOUTH:
            STRUT_PARTIAL_SET(dock_strut, 0, 0, 0, strh,
                              0, 0, 0, 0, 0, 0,
                              dock->x, dock->x + dock->w - 1);
            break;
        case OB_DIRECTION_SOUTHEAST:
            switch (config_dock_orient) {
            case OB_ORIENTATION_HORZ:
                STRUT_PARTIAL_SET(dock_strut, 0, 0, 0, strh,
                                  0, 0, 0, 0, 0, 0,
                                  dock->x, dock->x + dock->w - 1);
                break;
            case OB_ORIENTATION_VERT:
                STRUT_PARTIAL_SET(dock_strut, 0, 0, strw, 0,
                                  0, 0, 0, 0,
                                  dock->y, dock->y + dock->h - 1, 0, 0);
                break;
            }
            break;
        }
    }

    dock->w += minw;
    dock->h += minh;

    /* not used for actually sizing shit */
    dock->w -= ob_rr_theme->fbwidth * 2;
    dock->h -= ob_rr_theme->fbwidth * 2;

    if (dock->dock_apps) {
        g_assert(dock->w > 0);
        g_assert(dock->h > 0);

        XMoveResizeWindow(ob_display, dock->frame,
                          dock->x, dock->y, dock->w, dock->h);

        RrPaint(dock->a_frame, dock->frame, dock->w, dock->h);
        XMapWindow(ob_display, dock->frame);
    } else
        XUnmapWindow(ob_display, dock->frame);

    /* but they are useful outside of this function! */
    dock->w += ob_rr_theme->fbwidth * 2;
    dock->h += ob_rr_theme->fbwidth * 2;

    screen_update_areas();
}

void dock_app_configure(ObDockApp *app, gint w, gint h)
{
    app->w = w;
    app->h = h;
    dock_configure();
}

void dock_app_drag(ObDockApp *app, XMotionEvent *e)
{
    ObDockApp *over = NULL;
    GList *it;
    gint x, y;
    gboolean after;
    gboolean stop;

    x = e->x_root;
    y = e->y_root;

    /* are we on top of the dock? */
    if (!(x >= dock->x &&
          y >= dock->y &&
          x < dock->x + dock->w &&
          y < dock->y + dock->h))
        return;

    x -= dock->x;
    y -= dock->y;

    /* which dock app are we on top of? */
    stop = FALSE;
    for (it = dock->dock_apps; it; it = g_list_next(it)) {
        over = it->data;
        switch (config_dock_orient) {
        case OB_ORIENTATION_HORZ:
            if (x >= over->x && x < over->x + over->w)
                stop = TRUE;
            break;
        case OB_ORIENTATION_VERT:
            if (y >= over->y && y < over->y + over->h)
                stop = TRUE;
            break;
        }
        /* dont go to it->next! */
        if (stop) break;
    }
    if (!it || app == over) return;

    x -= over->x;
    y -= over->y;

    switch (config_dock_orient) {
    case OB_ORIENTATION_HORZ:
        after = (x > over->w / 2);
        break;
    case OB_ORIENTATION_VERT:
        after = (y > over->h / 2);
        break;
    default:
        g_assert_not_reached();
    }

    /* remove before doing the it->next! */
    dock->dock_apps = g_list_remove(dock->dock_apps, app);

    if (after) it = it->next;

    dock->dock_apps = g_list_insert_before(dock->dock_apps, it, app);
    dock_configure();
}

static gboolean hide_timeout(gpointer data)
{
    /* hide */
    dock->hidden = TRUE;
    dock_configure();

    return FALSE; /* don't repeat */
}

static gboolean show_timeout(gpointer data)
{
    /* hide */
    dock->hidden = FALSE;
    dock_configure();

    return FALSE; /* don't repeat */
}

void dock_hide(gboolean hide)
{
    if (!hide) {
        if (dock->hidden && config_dock_hide) {
            ob_main_loop_timeout_add(ob_main_loop, config_dock_show_delay,
                                 show_timeout, NULL, g_direct_equal, NULL);
        } else if (!dock->hidden && config_dock_hide) {
            ob_main_loop_timeout_remove(ob_main_loop, hide_timeout);
        }
    } else {
        if (!dock->hidden && config_dock_hide) {
            ob_main_loop_timeout_add(ob_main_loop, config_dock_hide_delay,
                                 hide_timeout, NULL, g_direct_equal, NULL);
        } else if (dock->hidden && config_dock_hide) {
            ob_main_loop_timeout_remove(ob_main_loop, show_timeout);
        }
    }
}
