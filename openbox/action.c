/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   action.c for the Openbox window manager
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
#include "client.h"
#include "focus.h"
#include "focus_cycle.h"
#include "moveresize.h"
#include "menu.h"
#include "prop.h"
#include "stacking.h"
#include "screen.h"
#include "action.h"
#include "openbox.h"
#include "grab.h"
#include "keyboard.h"
#include "event.h"
#include "dock.h"
#include "config.h"
#include "mainloop.h"
#include "startupnotify.h"
#include "gettext.h"

#include <glib.h>

static gulong ignore_start = 0;

static void client_action_start(union ActionData *data)
{
    ignore_start = event_start_ignore_all_enters();
}

static void client_action_end(union ActionData *data, gboolean allow_enters)
{
    if (config_focus_follow)
        if (data->any.context != OB_FRAME_CONTEXT_CLIENT) {
            if (!data->any.button && data->any.c && !allow_enters) {
                event_end_ignore_all_enters(ignore_start);
            } else {
                ObClient *c;

                /* usually this is sorta redundant, but with a press action
                   that moves windows our from under the cursor, the enter
                   event will come as a GrabNotify which is ignored, so this
                   makes a fake enter event
                */
                if ((c = client_under_pointer()) && c != data->any.c) {
                    ob_debug_type(OB_DEBUG_FOCUS,
                                  "Generating fake enter because we did a "
                                  "mouse-event action");
                    event_enter_client(c);
                }
            }
        }
}

typedef struct
{
    const gchar *name;
    void (*func)(union ActionData *);
    void (*setup)(ObAction **, ObUserAction uact);
} ActionString;

static ObAction *action_new(void (*func)(union ActionData *data))
{
    ObAction *a = g_new0(ObAction, 1);
    a->ref = 1;
    a->func = func;

    return a;
}

void action_ref(ObAction *a)
{
    ++a->ref;
}

void action_unref(ObAction *a)
{
    if (a == NULL) return;

    if (--a->ref > 0) return;

    /* deal with pointers */
    if (a->func == action_execute || a->func == action_restart)
        g_free(a->data.execute.path);
    else if (a->func == action_debug)
        g_free(a->data.debug.string);
    else if (a->func == action_showmenu)
        g_free(a->data.showmenu.name);

    g_free(a);
}

ObAction* action_copy(const ObAction *src)
{
    ObAction *a = action_new(src->func);

    a->data = src->data;

    /* deal with pointers */
    if (a->func == action_execute || a->func == action_restart)
        a->data.execute.path = g_strdup(a->data.execute.path);
    else if (a->func == action_debug)
        a->data.debug.string = g_strdup(a->data.debug.string);
    else if (a->func == action_showmenu)
        a->data.showmenu.name = g_strdup(a->data.showmenu.name);

    return a;
}

void setup_action_directional_focus_north(ObAction **a, ObUserAction uact)
{
    (*a)->data.interdiraction.inter.any.interactive = TRUE;
    (*a)->data.interdiraction.direction = OB_DIRECTION_NORTH;
    (*a)->data.interdiraction.dialog = TRUE;
    (*a)->data.interdiraction.dock_windows = FALSE;
    (*a)->data.interdiraction.desktop_windows = FALSE;
}

void setup_action_directional_focus_east(ObAction **a, ObUserAction uact)
{
    (*a)->data.interdiraction.inter.any.interactive = TRUE;
    (*a)->data.interdiraction.direction = OB_DIRECTION_EAST;
    (*a)->data.interdiraction.dialog = TRUE;
    (*a)->data.interdiraction.dock_windows = FALSE;
    (*a)->data.interdiraction.desktop_windows = FALSE;
}

void setup_action_directional_focus_south(ObAction **a, ObUserAction uact)
{
    (*a)->data.interdiraction.inter.any.interactive = TRUE;
    (*a)->data.interdiraction.direction = OB_DIRECTION_SOUTH;
    (*a)->data.interdiraction.dialog = TRUE;
    (*a)->data.interdiraction.dock_windows = FALSE;
    (*a)->data.interdiraction.desktop_windows = FALSE;
}

void setup_action_directional_focus_west(ObAction **a, ObUserAction uact)
{
    (*a)->data.interdiraction.inter.any.interactive = TRUE;
    (*a)->data.interdiraction.direction = OB_DIRECTION_WEST;
    (*a)->data.interdiraction.dialog = TRUE;
    (*a)->data.interdiraction.dock_windows = FALSE;
    (*a)->data.interdiraction.desktop_windows = FALSE;
}

void setup_action_directional_focus_northeast(ObAction **a, ObUserAction uact)
{
    (*a)->data.interdiraction.inter.any.interactive = TRUE;
    (*a)->data.interdiraction.direction = OB_DIRECTION_NORTHEAST;
    (*a)->data.interdiraction.dialog = TRUE;
    (*a)->data.interdiraction.dock_windows = FALSE;
    (*a)->data.interdiraction.desktop_windows = FALSE;
}

void setup_action_directional_focus_southeast(ObAction **a, ObUserAction uact)
{
    (*a)->data.interdiraction.inter.any.interactive = TRUE;
    (*a)->data.interdiraction.direction = OB_DIRECTION_SOUTHEAST;
    (*a)->data.interdiraction.dialog = TRUE;
    (*a)->data.interdiraction.dock_windows = FALSE;
    (*a)->data.interdiraction.desktop_windows = FALSE;
}

void setup_action_directional_focus_southwest(ObAction **a, ObUserAction uact)
{
    (*a)->data.interdiraction.inter.any.interactive = TRUE;
    (*a)->data.interdiraction.direction = OB_DIRECTION_SOUTHWEST;
    (*a)->data.interdiraction.dialog = TRUE;
    (*a)->data.interdiraction.dock_windows = FALSE;
    (*a)->data.interdiraction.desktop_windows = FALSE;
}

void setup_action_directional_focus_northwest(ObAction **a, ObUserAction uact)
{
    (*a)->data.interdiraction.inter.any.interactive = TRUE;
    (*a)->data.interdiraction.direction = OB_DIRECTION_NORTHWEST;
    (*a)->data.interdiraction.dialog = TRUE;
    (*a)->data.interdiraction.dock_windows = FALSE;
    (*a)->data.interdiraction.desktop_windows = FALSE;
}

void setup_action_send_to_desktop(ObAction **a, ObUserAction uact)
{
    (*a)->data.sendto.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.sendto.follow = TRUE;
}

void setup_action_send_to_desktop_prev(ObAction **a, ObUserAction uact)
{
    (*a)->data.sendtodir.inter.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.sendtodir.inter.any.interactive = TRUE;
    (*a)->data.sendtodir.dir = OB_DIRECTION_WEST;
    (*a)->data.sendtodir.linear = TRUE;
    (*a)->data.sendtodir.wrap = TRUE;
    (*a)->data.sendtodir.follow = TRUE;
}

void setup_action_send_to_desktop_next(ObAction **a, ObUserAction uact)
{
    (*a)->data.sendtodir.inter.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.sendtodir.inter.any.interactive = TRUE;
    (*a)->data.sendtodir.dir = OB_DIRECTION_EAST;
    (*a)->data.sendtodir.linear = TRUE;
    (*a)->data.sendtodir.wrap = TRUE;
    (*a)->data.sendtodir.follow = TRUE;
}

void setup_action_send_to_desktop_left(ObAction **a, ObUserAction uact)
{
    (*a)->data.sendtodir.inter.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.sendtodir.inter.any.interactive = TRUE;
    (*a)->data.sendtodir.dir = OB_DIRECTION_WEST;
    (*a)->data.sendtodir.linear = FALSE;
    (*a)->data.sendtodir.wrap = TRUE;
    (*a)->data.sendtodir.follow = TRUE;
}

void setup_action_send_to_desktop_right(ObAction **a, ObUserAction uact)
{
    (*a)->data.sendtodir.inter.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.sendtodir.inter.any.interactive = TRUE;
    (*a)->data.sendtodir.dir = OB_DIRECTION_EAST;
    (*a)->data.sendtodir.linear = FALSE;
    (*a)->data.sendtodir.wrap = TRUE;
    (*a)->data.sendtodir.follow = TRUE;
}

void setup_action_send_to_desktop_up(ObAction **a, ObUserAction uact)
{
    (*a)->data.sendtodir.inter.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.sendtodir.inter.any.interactive = TRUE;
    (*a)->data.sendtodir.dir = OB_DIRECTION_NORTH;
    (*a)->data.sendtodir.linear = FALSE;
    (*a)->data.sendtodir.wrap = TRUE;
    (*a)->data.sendtodir.follow = TRUE;
}

void setup_action_send_to_desktop_down(ObAction **a, ObUserAction uact)
{
    (*a)->data.sendtodir.inter.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.sendtodir.inter.any.interactive = TRUE;
    (*a)->data.sendtodir.dir = OB_DIRECTION_SOUTH;
    (*a)->data.sendtodir.linear = FALSE;
    (*a)->data.sendtodir.wrap = TRUE;
    (*a)->data.sendtodir.follow = TRUE;
}

void setup_action_desktop(ObAction **a, ObUserAction uact)
{
/*
    (*a)->data.desktop.inter.any.interactive = FALSE;
*/
}

void setup_action_desktop_prev(ObAction **a, ObUserAction uact)
{
    (*a)->data.desktopdir.inter.any.interactive = TRUE;
    (*a)->data.desktopdir.dir = OB_DIRECTION_WEST;
    (*a)->data.desktopdir.linear = TRUE;
    (*a)->data.desktopdir.wrap = TRUE;
}

void setup_action_desktop_next(ObAction **a, ObUserAction uact)
{
    (*a)->data.desktopdir.inter.any.interactive = TRUE;
    (*a)->data.desktopdir.dir = OB_DIRECTION_EAST;
    (*a)->data.desktopdir.linear = TRUE;
    (*a)->data.desktopdir.wrap = TRUE;
}

void setup_action_desktop_left(ObAction **a, ObUserAction uact)
{
    (*a)->data.desktopdir.inter.any.interactive = TRUE;
    (*a)->data.desktopdir.dir = OB_DIRECTION_WEST;
    (*a)->data.desktopdir.linear = FALSE;
    (*a)->data.desktopdir.wrap = TRUE;
}

void setup_action_desktop_right(ObAction **a, ObUserAction uact)
{
    (*a)->data.desktopdir.inter.any.interactive = TRUE;
    (*a)->data.desktopdir.dir = OB_DIRECTION_EAST;
    (*a)->data.desktopdir.linear = FALSE;
    (*a)->data.desktopdir.wrap = TRUE;
}

void setup_action_desktop_up(ObAction **a, ObUserAction uact)
{
    (*a)->data.desktopdir.inter.any.interactive = TRUE;
    (*a)->data.desktopdir.dir = OB_DIRECTION_NORTH;
    (*a)->data.desktopdir.linear = FALSE;
    (*a)->data.desktopdir.wrap = TRUE;
}

void setup_action_desktop_down(ObAction **a, ObUserAction uact)
{
    (*a)->data.desktopdir.inter.any.interactive = TRUE;
    (*a)->data.desktopdir.dir = OB_DIRECTION_SOUTH;
    (*a)->data.desktopdir.linear = FALSE;
    (*a)->data.desktopdir.wrap = TRUE;
}

void setup_action_cycle_windows_next(ObAction **a, ObUserAction uact)
{
    (*a)->data.cycle.inter.any.interactive = TRUE;
    (*a)->data.cycle.linear = FALSE;
    (*a)->data.cycle.forward = TRUE;
    (*a)->data.cycle.dialog = TRUE;
    (*a)->data.cycle.dock_windows = FALSE;
    (*a)->data.cycle.desktop_windows = FALSE;
    (*a)->data.cycle.all_desktops = FALSE;
}

void setup_action_cycle_windows_previous(ObAction **a, ObUserAction uact)
{
    (*a)->data.cycle.inter.any.interactive = TRUE;
    (*a)->data.cycle.linear = FALSE;
    (*a)->data.cycle.forward = FALSE;
    (*a)->data.cycle.dialog = TRUE;
    (*a)->data.cycle.dock_windows = FALSE;
    (*a)->data.cycle.desktop_windows = FALSE;
    (*a)->data.cycle.all_desktops = FALSE;
}

void setup_action_movefromedge_north(ObAction **a, ObUserAction uact)
{
    (*a)->data.diraction.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.diraction.direction = OB_DIRECTION_NORTH;
    (*a)->data.diraction.hang = TRUE;
}

void setup_action_movefromedge_south(ObAction **a, ObUserAction uact)
{
    (*a)->data.diraction.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.diraction.direction = OB_DIRECTION_SOUTH;
    (*a)->data.diraction.hang = TRUE;
}

void setup_action_movefromedge_east(ObAction **a, ObUserAction uact)
{
    (*a)->data.diraction.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.diraction.direction = OB_DIRECTION_EAST;
    (*a)->data.diraction.hang = TRUE;
}

void setup_action_movefromedge_west(ObAction **a, ObUserAction uact)
{
    (*a)->data.diraction.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.diraction.direction = OB_DIRECTION_WEST;
    (*a)->data.diraction.hang = TRUE;
}

void setup_action_movetoedge_north(ObAction **a, ObUserAction uact)
{
    (*a)->data.diraction.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.diraction.direction = OB_DIRECTION_NORTH;
    (*a)->data.diraction.hang = FALSE;
}

void setup_action_movetoedge_south(ObAction **a, ObUserAction uact)
{
    (*a)->data.diraction.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.diraction.direction = OB_DIRECTION_SOUTH;
    (*a)->data.diraction.hang = FALSE;
}

void setup_action_movetoedge_east(ObAction **a, ObUserAction uact)
{
    (*a)->data.diraction.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.diraction.direction = OB_DIRECTION_EAST;
    (*a)->data.diraction.hang = FALSE;
}

void setup_action_movetoedge_west(ObAction **a, ObUserAction uact)
{
    (*a)->data.diraction.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.diraction.direction = OB_DIRECTION_WEST;
    (*a)->data.diraction.hang = FALSE;
}

void setup_action_growtoedge_north(ObAction **a, ObUserAction uact)
{
    (*a)->data.diraction.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.diraction.direction = OB_DIRECTION_NORTH;
}

void setup_action_growtoedge_south(ObAction **a, ObUserAction uact)
{
    (*a)->data.diraction.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.diraction.direction = OB_DIRECTION_SOUTH;
}

void setup_action_growtoedge_east(ObAction **a, ObUserAction uact)
{
    (*a)->data.diraction.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.diraction.direction = OB_DIRECTION_EAST;
}

void setup_action_growtoedge_west(ObAction **a, ObUserAction uact)
{
    (*a)->data.diraction.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.diraction.direction = OB_DIRECTION_WEST;
}

void setup_action_top_layer(ObAction **a, ObUserAction uact)
{
    (*a)->data.layer.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.layer.layer = 1;
}

void setup_action_normal_layer(ObAction **a, ObUserAction uact)
{
    (*a)->data.layer.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.layer.layer = 0;
}

void setup_action_bottom_layer(ObAction **a, ObUserAction uact)
{
    (*a)->data.layer.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.layer.layer = -1;
}

void setup_action_move(ObAction **a, ObUserAction uact)
{
    (*a)->data.moveresize.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.moveresize.keyboard =
        (uact == OB_USER_ACTION_NONE ||
         uact == OB_USER_ACTION_KEYBOARD_KEY ||
         uact == OB_USER_ACTION_MENU_SELECTION);
    (*a)->data.moveresize.corner = 0;
}

void setup_action_resize(ObAction **a, ObUserAction uact)
{
    (*a)->data.moveresize.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.moveresize.keyboard =
        (uact == OB_USER_ACTION_NONE ||
         uact == OB_USER_ACTION_KEYBOARD_KEY ||
         uact == OB_USER_ACTION_MENU_SELECTION);
    (*a)->data.moveresize.corner = 0;
}

void setup_action_showmenu(ObAction **a, ObUserAction uact)
{
    (*a)->data.showmenu.any.client_action = OB_CLIENT_ACTION_OPTIONAL;
    /* you cannot call ShowMenu from inside a menu, cuz the menu code makes
       assumptions that there is only one menu (and submenus) open at
       a time! */
    if (uact == OB_USER_ACTION_MENU_SELECTION) {
        action_unref(*a);
        *a = NULL;
    }
}

void setup_action_addremove_desktop_current(ObAction **a, ObUserAction uact)
{
    (*a)->data.addremovedesktop.current = TRUE;
}

void setup_action_addremove_desktop_last(ObAction **a, ObUserAction uact)
{
    (*a)->data.addremovedesktop.current = FALSE;
}

void setup_action_focus(ObAction **a, ObUserAction uact)
{
    (*a)->data.any.client_action = OB_CLIENT_ACTION_OPTIONAL;
}

void setup_client_action(ObAction **a, ObUserAction uact)
{
    (*a)->data.any.client_action = OB_CLIENT_ACTION_ALWAYS;
}

ActionString actionstrings[] =
{
    {
        "debug", 
        action_debug,
        NULL
    },
    {
        "execute", 
        action_execute,
        NULL
    },
    {
        "directionalfocusnorth", 
        action_directional_focus, 
        setup_action_directional_focus_north
    },
    {
        "directionalfocuseast", 
        action_directional_focus, 
        setup_action_directional_focus_east
    },
    {
        "directionalfocussouth", 
        action_directional_focus, 
        setup_action_directional_focus_south
    },
    {
        "directionalfocuswest",
        action_directional_focus,
        setup_action_directional_focus_west
    },
    {
        "directionalfocusnortheast",
        action_directional_focus,
        setup_action_directional_focus_northeast
    },
    {
        "directionalfocussoutheast",
        action_directional_focus,
        setup_action_directional_focus_southeast
    },
    {
        "directionalfocussouthwest",
        action_directional_focus,
        setup_action_directional_focus_southwest
    },
    {
        "directionalfocusnorthwest",
        action_directional_focus,
        setup_action_directional_focus_northwest
    },
    {
        "activate",
        action_activate,
        setup_action_focus
    },
    {
        "focus",
        action_focus,
        setup_action_focus
    },
    {
        "unfocus",
        action_unfocus,
        setup_client_action
    },
    {
        "iconify",
        action_iconify,
        setup_client_action
    },
    {
        "focustobottom",
        action_focus_order_to_bottom,
        setup_client_action
    },
    {
        "raiselower",
        action_raiselower,
        setup_client_action
    },
    {
        "raise",
        action_raise,
        setup_client_action
    },
    {
        "lower",
        action_lower,
        setup_client_action
    },
    {
        "close",
        action_close,
        setup_client_action
    },
    {
        "kill",
        action_kill,
        setup_client_action
    },
    {
        "shadelower",
        action_shadelower,
        setup_client_action
    },
    {
        "unshaderaise",
        action_unshaderaise,
        setup_client_action
    },
    {
        "shade",
        action_shade,
        setup_client_action
    },
    {
        "unshade",
        action_unshade,
        setup_client_action
    },
    {
        "toggleshade",
        action_toggle_shade,
        setup_client_action
    },
    {
        "toggleomnipresent",
        action_toggle_omnipresent,
        setup_client_action
    },
    {
        "moverelativehorz",
        action_move_relative_horz,
        setup_client_action
    },
    {
        "moverelativevert",
        action_move_relative_vert,
        setup_client_action
    },
    {
        "movetocenter",
        action_move_to_center,
        setup_client_action
    },
    {
        "resizerelativehorz",
        action_resize_relative_horz,
        setup_client_action
    },
    {
        "resizerelativevert",
        action_resize_relative_vert,
        setup_client_action
    },
    {
        "moverelative",
        action_move_relative,
        setup_client_action
    },
    {
        "resizerelative",
        action_resize_relative,
        setup_client_action
    },
    {
        "maximizefull",
        action_maximize_full,
        setup_client_action
    },
    {
        "unmaximizefull",
        action_unmaximize_full,
        setup_client_action
    },
    {
        "togglemaximizefull",
        action_toggle_maximize_full,
        setup_client_action
    },
    {
        "maximizehorz",
        action_maximize_horz,
        setup_client_action
    },
    {
        "unmaximizehorz",
        action_unmaximize_horz,
        setup_client_action
    },
    {
        "togglemaximizehorz",
        action_toggle_maximize_horz,
        setup_client_action
    },
    {
        "maximizevert",
        action_maximize_vert,
        setup_client_action
    },
    {
        "unmaximizevert",
        action_unmaximize_vert,
        setup_client_action
    },
    {
        "togglemaximizevert",
        action_toggle_maximize_vert,
        setup_client_action
    },
    {
        "togglefullscreen",
        action_toggle_fullscreen,
        setup_client_action
    },
    {
        "sendtodesktop",
        action_send_to_desktop,
        setup_action_send_to_desktop
    },
    {
        "sendtodesktopnext",
        action_send_to_desktop_dir,
        setup_action_send_to_desktop_next
    },
    {
        "sendtodesktopprevious",
        action_send_to_desktop_dir,
        setup_action_send_to_desktop_prev
    },
    {
        "sendtodesktopright",
        action_send_to_desktop_dir,
        setup_action_send_to_desktop_right
    },
    {
        "sendtodesktopleft",
        action_send_to_desktop_dir,
        setup_action_send_to_desktop_left
    },
    {
        "sendtodesktopup",
        action_send_to_desktop_dir,
        setup_action_send_to_desktop_up
    },
    {
        "sendtodesktopdown",
        action_send_to_desktop_dir,
        setup_action_send_to_desktop_down
    },
    {
        "desktop",
        action_desktop,
        setup_action_desktop
    },
    {
        "desktopnext",
        action_desktop_dir,
        setup_action_desktop_next
    },
    {
        "desktopprevious",
        action_desktop_dir,
        setup_action_desktop_prev
    },
    {
        "desktopright",
        action_desktop_dir,
        setup_action_desktop_right
    },
    {
        "desktopleft",
        action_desktop_dir,
        setup_action_desktop_left
    },
    {
        "desktopup",
        action_desktop_dir,
        setup_action_desktop_up
    },
    {
        "desktopdown",
        action_desktop_dir,
        setup_action_desktop_down
    },
    {
        "toggledecorations",
        action_toggle_decorations,
        setup_client_action
    },
    {
        "move",
        action_move,
        setup_action_move
    },
    {
        "resize",
        action_resize,
        setup_action_resize
    },
    {
        "toggledockautohide",
        action_toggle_dockautohide,
        NULL
    },
    {
        "toggleshowdesktop",
        action_toggle_show_desktop,
        NULL
    },
    {
        "showdesktop",
        action_show_desktop,
        NULL
    },
    {
        "unshowdesktop",
        action_unshow_desktop,
        NULL
    },
    {
        "desktoplast",
        action_desktop_last,
        NULL
    },
    {
        "reconfigure",
        action_reconfigure,
        NULL
    },
    {
        "restart",
        action_restart,
        NULL
    },
    {
        "exit",
        action_exit,
        NULL
    },
    {
        "showmenu",
        action_showmenu,
        setup_action_showmenu
    },
    {
        "sendtotoplayer",
        action_send_to_layer,
        setup_action_top_layer
    },
    {
        "togglealwaysontop",
        action_toggle_layer,
        setup_action_top_layer
    },
    {
        "sendtonormallayer",
        action_send_to_layer,
        setup_action_normal_layer
    },
    {
        "sendtobottomlayer",
        action_send_to_layer,
        setup_action_bottom_layer
    },
    {
        "togglealwaysonbottom",
        action_toggle_layer,
        setup_action_bottom_layer
    },
    {
        "nextwindow",
        action_cycle_windows,
        setup_action_cycle_windows_next
    },
    {
        "previouswindow",
        action_cycle_windows,
        setup_action_cycle_windows_previous
    },
    {
        "movefromedgenorth",
        action_movetoedge,
        setup_action_movefromedge_north
    },
    {
        "movefromedgesouth",
        action_movetoedge,
        setup_action_movefromedge_south
    },
    {
        "movefromedgewest",
        action_movetoedge,
        setup_action_movefromedge_west
    },
    {
        "movefromedgeeast",
        action_movetoedge,
        setup_action_movefromedge_east
    },
    {
        "movetoedgenorth",
        action_movetoedge,
        setup_action_movetoedge_north
    },
    {
        "movetoedgesouth",
        action_movetoedge,
        setup_action_movetoedge_south
    },
    {
        "movetoedgewest",
        action_movetoedge,
        setup_action_movetoedge_west
    },
    {
        "movetoedgeeast",
        action_movetoedge,
        setup_action_movetoedge_east
    },
    {
        "growtoedgenorth",
        action_growtoedge,
        setup_action_growtoedge_north
    },
    {
        "growtoedgesouth",
        action_growtoedge,
        setup_action_growtoedge_south
    },
    {
        "growtoedgewest",
        action_growtoedge,
        setup_action_growtoedge_west
    },
    {
        "growtoedgeeast",
        action_growtoedge,
        setup_action_growtoedge_east
    },
    {
        "breakchroot",
        action_break_chroot,
        NULL
    },
    {
        "adddesktoplast",
        action_add_desktop,
        setup_action_addremove_desktop_last
    },
    {
        "removedesktoplast",
        action_remove_desktop,
        setup_action_addremove_desktop_last
    },
    {
        "adddesktopcurrent",
        action_add_desktop,
        setup_action_addremove_desktop_current
    },
    {
        "removedesktopcurrent",
        action_remove_desktop,
        setup_action_addremove_desktop_current
    },
    {
        NULL,
        NULL,
        NULL
    }
};

/* only key bindings can be interactive. thus saith the xor.
   because of how the mouse is grabbed, mouse events dont even get
   read during interactive events, so no dice! >:) */
#define INTERACTIVE_LIMIT(a, uact) \
    if (uact != OB_USER_ACTION_KEYBOARD_KEY) \
        a->data.any.interactive = FALSE;

ObAction *action_from_string(const gchar *name, ObUserAction uact)
{
    ObAction *a = NULL;
    gboolean exist = FALSE;
    gint i;

    for (i = 0; actionstrings[i].name; i++)
        if (!g_ascii_strcasecmp(name, actionstrings[i].name)) {
            exist = TRUE;
            a = action_new(actionstrings[i].func);
            if (actionstrings[i].setup)
                actionstrings[i].setup(&a, uact);
            if (a)
                INTERACTIVE_LIMIT(a, uact);
            break;
        }
    if (!exist)
        g_message(_("Invalid action '%s' requested. No such action exists."),
                  name);
    if (!a)
        g_message(_("Invalid use of action '%s'. Action will be ignored."),
                  name);
    return a;
}

ObAction *action_parse(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                       ObUserAction uact)
{
    gchar *actname;
    ObAction *act = NULL;
    xmlNodePtr n;

    if (parse_attr_string("name", node, &actname)) {
        if ((act = action_from_string(actname, uact))) {
            if (act->func == action_execute || act->func == action_restart) {
                if ((n = parse_find_node("execute", node->xmlChildrenNode))) {
                    gchar *s = parse_string(doc, n);
                    act->data.execute.path = parse_expand_tilde(s);
                    g_free(s);
                }
                if ((n = parse_find_node("startupnotify", node->xmlChildrenNode))) {
                    xmlNodePtr m;
                    if ((m = parse_find_node("enabled", n->xmlChildrenNode)))
                        act->data.execute.startupnotify = parse_bool(doc, m);
                    if ((m = parse_find_node("name", n->xmlChildrenNode)))
                        act->data.execute.name = parse_string(doc, m);
                    if ((m = parse_find_node("icon", n->xmlChildrenNode)))
                        act->data.execute.icon_name = parse_string(doc, m);
                }
            } else if (act->func == action_debug) {
                if ((n = parse_find_node("string", node->xmlChildrenNode)))
                    act->data.debug.string = parse_string(doc, n);
            } else if (act->func == action_showmenu) {
                if ((n = parse_find_node("menu", node->xmlChildrenNode)))
                    act->data.showmenu.name = parse_string(doc, n);
            } else if (act->func == action_move_relative_horz ||
                       act->func == action_move_relative_vert ||
                       act->func == action_resize_relative_horz ||
                       act->func == action_resize_relative_vert) {
                if ((n = parse_find_node("delta", node->xmlChildrenNode)))
                    act->data.relative.deltax = parse_int(doc, n);
            } else if (act->func == action_move_relative) {
                if ((n = parse_find_node("x", node->xmlChildrenNode)))
                    act->data.relative.deltax = parse_int(doc, n);
                if ((n = parse_find_node("y", node->xmlChildrenNode)))
                    act->data.relative.deltay = parse_int(doc, n);
            } else if (act->func == action_resize_relative) {
                if ((n = parse_find_node("left", node->xmlChildrenNode)))
                    act->data.relative.deltaxl = parse_int(doc, n);
                if ((n = parse_find_node("up", node->xmlChildrenNode)))
                    act->data.relative.deltayu = parse_int(doc, n);
                if ((n = parse_find_node("right", node->xmlChildrenNode)))
                    act->data.relative.deltax = parse_int(doc, n);
                if ((n = parse_find_node("down", node->xmlChildrenNode)))
                    act->data.relative.deltay = parse_int(doc, n);
            } else if (act->func == action_desktop) {
                if ((n = parse_find_node("desktop", node->xmlChildrenNode)))
                    act->data.desktop.desk = parse_int(doc, n);
                if (act->data.desktop.desk > 0) act->data.desktop.desk--;
/*
                if ((n = parse_find_node("dialog", node->xmlChildrenNode)))
                    act->data.desktop.inter.any.interactive =
                        parse_bool(doc, n);
*/
           } else if (act->func == action_send_to_desktop) {
                if ((n = parse_find_node("desktop", node->xmlChildrenNode)))
                    act->data.sendto.desk = parse_int(doc, n);
                if (act->data.sendto.desk > 0) act->data.sendto.desk--;
                if ((n = parse_find_node("follow", node->xmlChildrenNode)))
                    act->data.sendto.follow = parse_bool(doc, n);
            } else if (act->func == action_desktop_dir) {
                if ((n = parse_find_node("wrap", node->xmlChildrenNode)))
                    act->data.desktopdir.wrap = parse_bool(doc, n); 
                if ((n = parse_find_node("dialog", node->xmlChildrenNode)))
                    act->data.desktopdir.inter.any.interactive =
                        parse_bool(doc, n);
            } else if (act->func == action_send_to_desktop_dir) {
                if ((n = parse_find_node("wrap", node->xmlChildrenNode)))
                    act->data.sendtodir.wrap = parse_bool(doc, n);
                if ((n = parse_find_node("follow", node->xmlChildrenNode)))
                    act->data.sendtodir.follow = parse_bool(doc, n);
                if ((n = parse_find_node("dialog", node->xmlChildrenNode)))
                    act->data.sendtodir.inter.any.interactive =
                        parse_bool(doc, n);
            } else if (act->func == action_activate) {
                if ((n = parse_find_node("here", node->xmlChildrenNode)))
                    act->data.activate.here = parse_bool(doc, n);
            } else if (act->func == action_cycle_windows) {
                if ((n = parse_find_node("linear", node->xmlChildrenNode)))
                    act->data.cycle.linear = parse_bool(doc, n);
                if ((n = parse_find_node("dialog", node->xmlChildrenNode)))
                    act->data.cycle.dialog = parse_bool(doc, n);
                if ((n = parse_find_node("panels", node->xmlChildrenNode)))
                    act->data.cycle.dock_windows = parse_bool(doc, n);
                if ((n = parse_find_node("desktop", node->xmlChildrenNode)))
                    act->data.cycle.desktop_windows = parse_bool(doc, n);
                if ((n = parse_find_node("allDesktops",
                                         node->xmlChildrenNode)))
                    act->data.cycle.all_desktops = parse_bool(doc, n);
            } else if (act->func == action_directional_focus) {
                if ((n = parse_find_node("dialog", node->xmlChildrenNode)))
                    act->data.interdiraction.dialog = parse_bool(doc, n);
                if ((n = parse_find_node("panels", node->xmlChildrenNode)))
                    act->data.interdiraction.dock_windows = parse_bool(doc, n);
                if ((n = parse_find_node("desktop", node->xmlChildrenNode)))
                    act->data.interdiraction.desktop_windows =
                        parse_bool(doc, n);
            } else if (act->func == action_resize) {
                if ((n = parse_find_node("edge", node->xmlChildrenNode))) {
                    gchar *s = parse_string(doc, n);
                    if (!g_ascii_strcasecmp(s, "top"))
                        act->data.moveresize.corner =
                            prop_atoms.net_wm_moveresize_size_top;
                    else if (!g_ascii_strcasecmp(s, "bottom"))
                        act->data.moveresize.corner =
                            prop_atoms.net_wm_moveresize_size_bottom;
                    else if (!g_ascii_strcasecmp(s, "left"))
                        act->data.moveresize.corner =
                            prop_atoms.net_wm_moveresize_size_left;
                    else if (!g_ascii_strcasecmp(s, "right"))
                        act->data.moveresize.corner =
                            prop_atoms.net_wm_moveresize_size_right;
                    else if (!g_ascii_strcasecmp(s, "topleft"))
                        act->data.moveresize.corner =
                            prop_atoms.net_wm_moveresize_size_topleft;
                    else if (!g_ascii_strcasecmp(s, "topright"))
                        act->data.moveresize.corner =
                            prop_atoms.net_wm_moveresize_size_topright;
                    else if (!g_ascii_strcasecmp(s, "bottomleft"))
                        act->data.moveresize.corner =
                            prop_atoms.net_wm_moveresize_size_bottomleft;
                    else if (!g_ascii_strcasecmp(s, "bottomright"))
                        act->data.moveresize.corner =
                            prop_atoms.net_wm_moveresize_size_bottomright;
                    g_free(s);
                }
            } else if (act->func == action_raise ||
                       act->func == action_lower ||
                       act->func == action_raiselower ||
                       act->func == action_shadelower ||
                       act->func == action_unshaderaise) {
            }
            INTERACTIVE_LIMIT(act, uact);
        }
        g_free(actname);
    }
    return act;
}

void action_run_list(GSList *acts, ObClient *c, ObFrameContext context,
                     guint state, guint button, gint x, gint y, Time time,
                     gboolean cancel, gboolean done)
{
    GSList *it;
    ObAction *a;

    if (!acts)
        return;

    if (x < 0 && y < 0)
        screen_pointer_pos(&x, &y);

    for (it = acts; it; it = g_slist_next(it)) {
        a = it->data;

        if (!(a->data.any.client_action == OB_CLIENT_ACTION_ALWAYS && !c)) {
            a->data.any.c = a->data.any.client_action ? c : NULL;
            a->data.any.context = context;
            a->data.any.x = x;
            a->data.any.y = y;

            a->data.any.button = button;

            a->data.any.time = time;

            if (a->data.any.interactive) {
                a->data.inter.cancel = cancel;
                a->data.inter.final = done;
                if (!(cancel || done))
                    if (!keyboard_interactive_grab(state, a->data.any.c, a))
                        continue;
            }

            /* XXX UGLY HACK race with motion event starting a move and the
               button release gettnig processed first. answer: don't queue
               moveresize starts. UGLY HACK XXX

               XXX ALSO don't queue showmenu events, because on button press
               events we need to know if a mouse grab is going to take place,
               and set the button to 0, so that later motion events don't think
               that a drag is going on. since showmenu grabs the pointer..
            */
            if (a->data.any.interactive || a->func == action_move ||
                a->func == action_resize || a->func == action_showmenu)
            {
                /* interactive actions are not queued */
                a->func(&a->data);
            } else if (a->func == action_focus ||
                       a->func == action_activate ||
                       a->func == action_showmenu)
            {
                /* XXX MORE UGLY HACK
                   actions from clicks on client windows are NOT queued.
                   this solves the mysterious click-and-drag-doesnt-work
                   problem. it was because the window gets focused and stuff
                   after the button event has already been passed through. i
                   dont really know why it should care but it does and it makes
                   a difference.

                   however this very bogus ! !
                   we want to send the button press to the window BEFORE
                   we do the action because the action might move the windows
                   (eg change desktops) and then the button press ends up on
                   the completely wrong window !
                   so, this is just for that bug, and it will only NOT queue it
                   if it is a focusing action that can be used with the mouse
                   pointer. ugh.

                   also with the menus, there is a race going on. if the
                   desktop wants to pop up a menu, and we do too, we send them
                   the button before we pop up the menu, so they pop up their
                   menu first. but not always. if we pop up our menu before
                   sending them the button press, then the result is
                   deterministic. yay.

                   XXX further more. focus actions are not queued at all,
                   because if you bind focus->showmenu, the menu will get
                   hidden to do the focusing
                */
                a->func(&a->data);
            } else
                ob_main_loop_queue_action(ob_main_loop, a);
        }
    }
}

void action_run_string(const gchar *name, struct _ObClient *c, Time time)
{
    ObAction *a;
    GSList *l;

    a = action_from_string(name, OB_USER_ACTION_NONE);
    g_assert(a);

    l = g_slist_append(NULL, a);

    action_run(l, c, 0, time);
}

void action_debug(union ActionData *data)
{
    if (data->debug.string)
        g_print("%s\n", data->debug.string);
}

void action_execute(union ActionData *data)
{
    GError *e = NULL;
    gchar *cmd, **argv = 0;
    if (data->execute.path) {
        cmd = g_filename_from_utf8(data->execute.path, -1, NULL, NULL, NULL);
        if (cmd) {
            /* If there is a keyboard grab going on then we need to cancel
               it so the application can grab things */
            event_cancel_all_key_grabs();

            if (!g_shell_parse_argv (cmd, NULL, &argv, &e)) {
                g_message(_("Failed to execute '%s': %s"),
                          cmd, e->message);
                g_error_free(e);
            } else if (data->execute.startupnotify) {
                gchar *program;
                
                program = g_path_get_basename(argv[0]);
                /* sets up the environment */
                sn_setup_spawn_environment(program,
                                           data->execute.name,
                                           data->execute.icon_name,
                                           /* launch it on the current
                                              desktop */
                                           screen_desktop,
                                           data->execute.any.time);
                if (!g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH |
                                   G_SPAWN_DO_NOT_REAP_CHILD,
                                   NULL, NULL, NULL, &e)) {
                    g_message(_("Failed to execute '%s': %s"),
                              cmd, e->message);
                    g_error_free(e);
                    sn_spawn_cancel();
                }
                unsetenv("DESKTOP_STARTUP_ID");
                g_free(program);
                g_strfreev(argv);
            } else {
                if (!g_spawn_async(NULL, argv, NULL, G_SPAWN_SEARCH_PATH |
                                   G_SPAWN_DO_NOT_REAP_CHILD,
                                   NULL, NULL, NULL, &e))
                {
                    g_message(_("Failed to execute '%s': %s"),
                              cmd, e->message);
                    g_error_free(e);
                }
                g_strfreev(argv);
            }
            g_free(cmd);
        } else {
            g_message(_("Failed to convert the path '%s' from utf8"),
                      data->execute.path);
        }
    }
}

void action_activate(union ActionData *data)
{
    if (data->client.any.c) {
        if (!data->any.button || client_mouse_focusable(data->client.any.c) ||
            (data->any.context != OB_FRAME_CONTEXT_CLIENT &&
             data->any.context != OB_FRAME_CONTEXT_FRAME))
        {
            /* if using focus_delay, stop the timer now so that focus doesn't
               go moving on us */
            event_halt_focus_delay();

            client_activate(data->activate.any.c, data->activate.here, TRUE);
        }
    } else {
        /* focus action on something other than a client, make keybindings
           work for this openbox instance, but don't focus any specific client
        */
        focus_nothing();
    }
}

void action_focus(union ActionData *data)
{
    if (data->client.any.c) {
        if (!data->any.button || client_mouse_focusable(data->client.any.c) ||
            (data->any.context != OB_FRAME_CONTEXT_CLIENT &&
             data->any.context != OB_FRAME_CONTEXT_FRAME))
        {
            /* if using focus_delay, stop the timer now so that focus doesn't
               go moving on us */
            event_halt_focus_delay();

            client_focus(data->client.any.c);
        }
    } else {
        /* focus action on something other than a client, make keybindings
           work for this openbox instance, but don't focus any specific client
        */
        focus_nothing();
    }
}

void action_unfocus (union ActionData *data)
{
    if (data->client.any.c == focus_client)
        focus_fallback(FALSE, FALSE, TRUE);
}

void action_iconify(union ActionData *data)
{
    client_action_start(data);
    client_iconify(data->client.any.c, TRUE, TRUE, FALSE);
    client_action_end(data, config_focus_under_mouse);
}

void action_focus_order_to_bottom(union ActionData *data)
{
    focus_order_to_bottom(data->client.any.c);
}

void action_raiselower(union ActionData *data)
{
    ObClient *c = data->client.any.c;

    client_action_start(data);
    stacking_restack_request(c, NULL, Opposite, FALSE);
    client_action_end(data, config_focus_under_mouse);
}

void action_raise(union ActionData *data)
{
    client_action_start(data);
    stacking_raise(CLIENT_AS_WINDOW(data->client.any.c));
    client_action_end(data, config_focus_under_mouse);
}

void action_unshaderaise(union ActionData *data)
{
    if (data->client.any.c->shaded)
        action_unshade(data);
    else
        action_raise(data);
}

void action_shadelower(union ActionData *data)
{
    if (data->client.any.c->shaded)
        action_lower(data);
    else
        action_shade(data);
}

void action_lower(union ActionData *data)
{
    client_action_start(data);
    stacking_lower(CLIENT_AS_WINDOW(data->client.any.c));
    client_action_end(data, config_focus_under_mouse);
}

void action_close(union ActionData *data)
{
    client_close(data->client.any.c);
}

void action_kill(union ActionData *data)
{
    client_kill(data->client.any.c);
}

void action_shade(union ActionData *data)
{
    client_action_start(data);
    client_shade(data->client.any.c, TRUE);
    client_action_end(data, config_focus_under_mouse);
}

void action_unshade(union ActionData *data)
{
    client_action_start(data);
    client_shade(data->client.any.c, FALSE);
    client_action_end(data, config_focus_under_mouse);
}

void action_toggle_shade(union ActionData *data)
{
    client_action_start(data);
    client_shade(data->client.any.c, !data->client.any.c->shaded);
    client_action_end(data, config_focus_under_mouse);
}

void action_toggle_omnipresent(union ActionData *data)
{ 
    client_set_desktop(data->client.any.c,
                       data->client.any.c->desktop == DESKTOP_ALL ?
                       screen_desktop : DESKTOP_ALL, FALSE, TRUE);
}

void action_move_relative_horz(union ActionData *data)
{
    ObClient *c = data->relative.any.c;
    client_action_start(data);
    client_move(c, c->area.x + data->relative.deltax, c->area.y);
    client_action_end(data, FALSE);
}

void action_move_relative_vert(union ActionData *data)
{
    ObClient *c = data->relative.any.c;
    client_action_start(data);
    client_move(c, c->area.x, c->area.y + data->relative.deltax);
    client_action_end(data, FALSE);
}

void action_move_to_center(union ActionData *data)
{
    ObClient *c = data->client.any.c;
    Rect *area;
    area = screen_area(c->desktop, client_monitor(c), NULL);
    client_action_start(data);
    client_move(c, area->width / 2 - c->area.width / 2,
                area->height / 2 - c->area.height / 2);
    client_action_end(data, FALSE);
    g_free(area);
}

void action_resize_relative_horz(union ActionData *data)
{
    ObClient *c = data->relative.any.c;
    client_action_start(data);
    client_resize(c,
                  c->area.width + data->relative.deltax * c->size_inc.width,
                  c->area.height);
    client_action_end(data, FALSE);
}

void action_resize_relative_vert(union ActionData *data)
{
    ObClient *c = data->relative.any.c;
    if (!c->shaded) {
        client_action_start(data);
        client_resize(c, c->area.width, c->area.height +
                      data->relative.deltax * c->size_inc.height);
        client_action_end(data, FALSE);
    }
}

void action_move_relative(union ActionData *data)
{
    ObClient *c = data->relative.any.c;
    client_action_start(data);
    client_move(c, c->area.x + data->relative.deltax, c->area.y +
                data->relative.deltay);
    client_action_end(data, FALSE);
}

void action_resize_relative(union ActionData *data)
{
    ObClient *c = data->relative.any.c;
    gint x, y, ow, xoff, nw, oh, yoff, nh, lw, lh;

    client_action_start(data);

    x = c->area.x;
    y = c->area.y;
    ow = c->area.width;
    xoff = -data->relative.deltaxl * c->size_inc.width;
    nw = ow + data->relative.deltax * c->size_inc.width
        + data->relative.deltaxl * c->size_inc.width;
    oh = c->area.height;
    yoff = -data->relative.deltayu * c->size_inc.height;
    nh = oh + data->relative.deltay * c->size_inc.height
        + data->relative.deltayu * c->size_inc.height;

    g_print("deltax %d %d x %d ow %d xoff %d nw %d\n",
            data->relative.deltax, 
            data->relative.deltaxl, 
            x, ow, xoff, nw);
    
    client_try_configure(c, &x, &y, &nw, &nh, &lw, &lh, TRUE);
    xoff = xoff == 0 ? 0 : (xoff < 0 ? MAX(xoff, ow-nw) : MIN(xoff, ow-nw));
    yoff = yoff == 0 ? 0 : (yoff < 0 ? MAX(yoff, oh-nh) : MIN(yoff, oh-nh));
    client_move_resize(c, x + xoff, y + yoff, nw, nh);
    client_action_end(data, FALSE);
}

void action_maximize_full(union ActionData *data)
{
    client_action_start(data);
    client_maximize(data->client.any.c, TRUE, 0);
    client_action_end(data, config_focus_under_mouse);
}

void action_unmaximize_full(union ActionData *data)
{
    client_action_start(data);
    client_maximize(data->client.any.c, FALSE, 0);
    client_action_end(data, config_focus_under_mouse);
}

void action_toggle_maximize_full(union ActionData *data)
{
    client_action_start(data);
    client_maximize(data->client.any.c,
                    !(data->client.any.c->max_horz ||
                      data->client.any.c->max_vert),
                    0);
    client_action_end(data, config_focus_under_mouse);
}

void action_maximize_horz(union ActionData *data)
{
    client_action_start(data);
    client_maximize(data->client.any.c, TRUE, 1);
    client_action_end(data, config_focus_under_mouse);
}

void action_unmaximize_horz(union ActionData *data)
{
    client_action_start(data);
    client_maximize(data->client.any.c, FALSE, 1);
    client_action_end(data, config_focus_under_mouse);
}

void action_toggle_maximize_horz(union ActionData *data)
{
    client_action_start(data);
    client_maximize(data->client.any.c,
                    !data->client.any.c->max_horz, 1);
    client_action_end(data, config_focus_under_mouse);
}

void action_maximize_vert(union ActionData *data)
{
    client_action_start(data);
    client_maximize(data->client.any.c, TRUE, 2);
    client_action_end(data, config_focus_under_mouse);
}

void action_unmaximize_vert(union ActionData *data)
{
    client_action_start(data);
    client_maximize(data->client.any.c, FALSE, 2);
    client_action_end(data, config_focus_under_mouse);
}

void action_toggle_maximize_vert(union ActionData *data)
{
    client_action_start(data);
    client_maximize(data->client.any.c,
                    !data->client.any.c->max_vert, 2);
    client_action_end(data, config_focus_under_mouse);
}

void action_toggle_fullscreen(union ActionData *data)
{
    client_action_start(data);
    client_fullscreen(data->client.any.c, !(data->client.any.c->fullscreen));
    client_action_end(data, config_focus_under_mouse);
}

void action_send_to_desktop(union ActionData *data)
{
    ObClient *c = data->sendto.any.c;

    if (!client_normal(c)) return;

    if (data->sendto.desk < screen_num_desktops ||
        data->sendto.desk == DESKTOP_ALL) {
        client_set_desktop(c, data->sendto.desk, data->sendto.follow, FALSE);
        if (data->sendto.follow && data->sendto.desk != screen_desktop)
            screen_set_desktop(data->sendto.desk, TRUE);
    }
}

void action_desktop(union ActionData *data)
{
    /* XXX add the interactive/dialog option back again once the dialog
       has been made to not use grabs */
    if (data->desktop.desk < screen_num_desktops ||
        data->desktop.desk == DESKTOP_ALL)
    {
        screen_set_desktop(data->desktop.desk, TRUE);
        if (data->inter.any.interactive)
            screen_desktop_popup(data->desktop.desk, TRUE);
    }
}

void action_desktop_dir(union ActionData *data)
{
    guint d;

    d = screen_cycle_desktop(data->desktopdir.dir,
                             data->desktopdir.wrap,
                             data->desktopdir.linear,
                             data->desktopdir.inter.any.interactive,
                             data->desktopdir.inter.final,
                             data->desktopdir.inter.cancel);
    /* only move the desktop when the action is complete. if we switch
       desktops during the interactive action, focus will move but with
       NotifyWhileGrabbed and applications don't like that. */
    if (!data->sendtodir.inter.any.interactive ||
        (data->sendtodir.inter.final && !data->sendtodir.inter.cancel))
    {
        if (d != screen_desktop)
            screen_set_desktop(d, TRUE);
    }
}

void action_send_to_desktop_dir(union ActionData *data)
{
    ObClient *c = data->sendtodir.inter.any.c;
    guint d;

    if (!client_normal(c)) return;

    d = screen_cycle_desktop(data->sendtodir.dir, data->sendtodir.wrap,
                             data->sendtodir.linear,
                             data->sendtodir.inter.any.interactive,
                             data->sendtodir.inter.final,
                             data->sendtodir.inter.cancel);
    /* only move the desktop when the action is complete. if we switch
       desktops during the interactive action, focus will move but with
       NotifyWhileGrabbed and applications don't like that. */
    if (!data->sendtodir.inter.any.interactive ||
        (data->sendtodir.inter.final && !data->sendtodir.inter.cancel))
    {
        client_set_desktop(c, d, data->sendtodir.follow, FALSE);
        if (data->sendtodir.follow && d != screen_desktop)
            screen_set_desktop(d, TRUE);
    }
}

void action_desktop_last(union ActionData *data)
{
    if (screen_last_desktop < screen_num_desktops)
        screen_set_desktop(screen_last_desktop, TRUE);
}

void action_toggle_decorations(union ActionData *data)
{
    ObClient *c = data->client.any.c;

    client_action_start(data);
    client_set_undecorated(c, !c->undecorated);
    client_action_end(data, FALSE);
}

static guint32 pick_corner(gint x, gint y, gint cx, gint cy, gint cw, gint ch,
                           gboolean shaded)
{
    /* let's make x and y client relative instead of screen relative */
    x = x - cx;
    y = ch - (y - cy); /* y is inverted, 0 is at the bottom of the window */

#define X x*ch/cw
#define A -4*X + 7*ch/3
#define B  4*X -15*ch/9
#define C -X/4 + 2*ch/3
#define D  X/4 + 5*ch/12
#define E  X/4 +   ch/3
#define F -X/4 + 7*ch/12
#define G  4*X - 4*ch/3
#define H -4*X + 8*ch/3
#define a (y > 5*ch/9)
#define b (x < 4*cw/9)
#define c (x > 5*cw/9)
#define d (y < 4*ch/9)

    /*
      Each of these defines (except X which is just there for fun), represents
      the equation of a line. The lines they represent are shown in the diagram
      below. Checking y against these lines, we are able to choose a region
      of the window as shown.

      +---------------------A-------|-------|-------B---------------------+
      |                     |A                     B|                     |
      |                     |A      |       |      B|                     |
      |                     | A                   B |                     |
      |                     | A     |       |     B |                     |
      |                     |  A                 B  |                     |
      |                     |  A    |       |    B  |                     |
      |        northwest    |   A     north     B   |   northeast         |
      |                     |   A   |       |   B   |                     |
      |                     |    A             B    |                     |
      C---------------------+----A--+-------+--B----+---------------------D
      |CCCCCCC              |     A           B     |              DDDDDDD|
      |       CCCCCCCC      |     A |       | B     |      DDDDDDDD       |
      |               CCCCCCC      A         B      DDDDDDD               |
      - - - - - - - - - - - +CCCCCCC+aaaaaaa+DDDDDDD+ - - - - - - - - - - - -
      |                     |       b       c       |                     | sh
      |             west    |       b  move c       |   east              | ad
      |                     |       b       c       |                     | ed
      - - - - - - - - - - - +EEEEEEE+ddddddd+FFFFFFF+- - - - - - - - - - -  -
      |               EEEEEEE      G         H      FFFFFFF               |
      |       EEEEEEEE      |     G |       | H     |      FFFFFFFF       |
      |EEEEEEE              |     G           H     |              FFFFFFF|
      E---------------------+----G--+-------+--H----+---------------------F
      |                     |    G             H    |                     |
      |                     |   G   |       |   H   |                     |
      |        southwest    |   G     south     H   |   southeast         |
      |                     |  G    |       |    H  |                     |
      |                     |  G                 H  |                     |
      |                     | G     |       |     H |                     |
      |                     | G                   H |                     |
      |                     |G      |       |      H|                     |
      |                     |G                     H|                     |
      +---------------------G-------|-------|-------H---------------------+
    */

    if (shaded) {
        /* for shaded windows, you can only resize west/east and move */
        if (b)
            return prop_atoms.net_wm_moveresize_size_left;
        if (c)
            return prop_atoms.net_wm_moveresize_size_right;
        return prop_atoms.net_wm_moveresize_move;
    }

    if (y < A && y >= C)
        return prop_atoms.net_wm_moveresize_size_topleft;
    else if (y >= A && y >= B && a)
        return prop_atoms.net_wm_moveresize_size_top;
    else if (y < B && y >= D)
        return prop_atoms.net_wm_moveresize_size_topright;
    else if (y < C && y >= E && b)
        return prop_atoms.net_wm_moveresize_size_left;
    else if (y < D && y >= F && c)
        return prop_atoms.net_wm_moveresize_size_right;
    else if (y < E && y >= G)
        return prop_atoms.net_wm_moveresize_size_bottomleft;
    else if (y < G && y < H && d)
        return prop_atoms.net_wm_moveresize_size_bottom;
    else if (y >= H && y < F)
        return prop_atoms.net_wm_moveresize_size_bottomright;
    else
        return prop_atoms.net_wm_moveresize_move;

#undef X
#undef A
#undef B
#undef C
#undef D
#undef E
#undef F
#undef G
#undef H
#undef a
#undef b
#undef c
#undef d
}

void action_move(union ActionData *data)
{
    ObClient *c = data->moveresize.any.c;
    guint32 corner;

    if (data->moveresize.keyboard)
        corner = prop_atoms.net_wm_moveresize_move_keyboard;
    else
        corner = prop_atoms.net_wm_moveresize_move;

    moveresize_start(c, data->any.x, data->any.y, data->any.button, corner);
}

void action_resize(union ActionData *data)
{
    ObClient *c = data->moveresize.any.c;
    guint32 corner;

    if (data->moveresize.keyboard)
        corner = prop_atoms.net_wm_moveresize_size_keyboard;
    else if (data->moveresize.corner)
        corner = data->moveresize.corner; /* it was specified in the binding */
    else
        corner = pick_corner(data->any.x, data->any.y,
                             c->frame->area.x, c->frame->area.y,
                             /* use the client size because the frame
                                can be differently sized (shaded
                                windows) and we want this based on the
                                clients size */
                             c->area.width + c->frame->size.left +
                             c->frame->size.right,
                             c->area.height + c->frame->size.top +
                             c->frame->size.bottom, c->shaded);

    moveresize_start(c, data->any.x, data->any.y, data->any.button, corner);
}

void action_reconfigure(union ActionData *data)
{
    ob_reconfigure();
}

void action_restart(union ActionData *data)
{
    ob_restart_other(data->execute.path);
}

void action_exit(union ActionData *data)
{
    ob_exit(0);
}

void action_showmenu(union ActionData *data)
{
    if (data->showmenu.name) {
        menu_show(data->showmenu.name, data->any.x, data->any.y,
                  data->any.button, data->showmenu.any.c);
    }
}

void action_cycle_windows(union ActionData *data)
{
    /* if using focus_delay, stop the timer now so that focus doesn't go moving
       on us */
    event_halt_focus_delay();

    focus_cycle(data->cycle.forward,
                data->cycle.all_desktops,
                data->cycle.dock_windows,
                data->cycle.desktop_windows,
                data->cycle.linear, data->any.interactive,
                data->cycle.dialog,
                data->cycle.inter.final, data->cycle.inter.cancel);
}

void action_directional_focus(union ActionData *data)
{
    /* if using focus_delay, stop the timer now so that focus doesn't go moving
       on us */
    event_halt_focus_delay();

    focus_directional_cycle(data->interdiraction.direction,
                            data->interdiraction.dock_windows,
                            data->interdiraction.desktop_windows,
                            data->any.interactive,
                            data->interdiraction.dialog,
                            data->interdiraction.inter.final,
                            data->interdiraction.inter.cancel);
}

void action_movetoedge(union ActionData *data)
{
    gint x, y;
    ObClient *c = data->diraction.any.c;

    x = c->frame->area.x;
    y = c->frame->area.y;
    
    switch(data->diraction.direction) {
    case OB_DIRECTION_NORTH:
        y = client_directional_edge_search(c, OB_DIRECTION_NORTH,
                                           data->diraction.hang)
            - (data->diraction.hang ? c->frame->area.height : 0);
        break;
    case OB_DIRECTION_WEST:
        x = client_directional_edge_search(c, OB_DIRECTION_WEST,
                                           data->diraction.hang)
            - (data->diraction.hang ? c->frame->area.width : 0);
        break;
    case OB_DIRECTION_SOUTH:
        y = client_directional_edge_search(c, OB_DIRECTION_SOUTH,
                                           data->diraction.hang)
            - (data->diraction.hang ? 0 : c->frame->area.height);
        break;
    case OB_DIRECTION_EAST:
        x = client_directional_edge_search(c, OB_DIRECTION_EAST,
                                           data->diraction.hang)
            - (data->diraction.hang ? 0 : c->frame->area.width);
        break;
    default:
        g_assert_not_reached();
    }
    frame_frame_gravity(c->frame, &x, &y, c->area.width, c->area.height);
    client_action_start(data);
    client_move(c, x, y);
    client_action_end(data, FALSE);
}

void action_growtoedge(union ActionData *data)
{
    gint x, y, width, height, dest;
    ObClient *c = data->diraction.any.c;
    Rect *a;

    a = screen_area(c->desktop, SCREEN_AREA_ALL_MONITORS, &c->frame->area);
    x = c->frame->area.x;
    y = c->frame->area.y;
    /* get the unshaded frame's dimensions..if it is shaded */
    width = c->area.width + c->frame->size.left + c->frame->size.right;
    height = c->area.height + c->frame->size.top + c->frame->size.bottom;

    switch(data->diraction.direction) {
    case OB_DIRECTION_NORTH:
        if (c->shaded) break; /* don't allow vertical resize if shaded */

        dest = client_directional_edge_search(c, OB_DIRECTION_NORTH, FALSE);
        if (a->y == y)
            height = height / 2;
        else {
            height = c->frame->area.y + height - dest;
            y = dest;
        }
        break;
    case OB_DIRECTION_WEST:
        dest = client_directional_edge_search(c, OB_DIRECTION_WEST, FALSE);
        if (a->x == x)
            width = width / 2;
        else {
            width = c->frame->area.x + width - dest;
            x = dest;
        }
        break;
    case OB_DIRECTION_SOUTH:
        if (c->shaded) break; /* don't allow vertical resize if shaded */

        dest = client_directional_edge_search(c, OB_DIRECTION_SOUTH, FALSE);
        if (a->y + a->height == y + c->frame->area.height) {
            height = c->frame->area.height / 2;
            y = a->y + a->height - height;
        } else
            height = dest - c->frame->area.y;
        y += (height - c->frame->area.height) % c->size_inc.height;
        height -= (height - c->frame->area.height) % c->size_inc.height;
        break;
    case OB_DIRECTION_EAST:
        dest = client_directional_edge_search(c, OB_DIRECTION_EAST, FALSE);
        if (a->x + a->width == x + c->frame->area.width) {
            width = c->frame->area.width / 2;
            x = a->x + a->width - width;
        } else
            width = dest - c->frame->area.x;
        x += (width - c->frame->area.width) % c->size_inc.width;
        width -= (width - c->frame->area.width) % c->size_inc.width;
        break;
    default:
        g_assert_not_reached();
    }
    width -= c->frame->size.left + c->frame->size.right;
    height -= c->frame->size.top + c->frame->size.bottom;
    frame_frame_gravity(c->frame, &x, &y, width, height);
    client_action_start(data);
    client_move_resize(c, x, y, width, height);
    client_action_end(data, FALSE);
    g_free(a);
}

void action_send_to_layer(union ActionData *data)
{
    client_set_layer(data->layer.any.c, data->layer.layer);
}

void action_toggle_layer(union ActionData *data)
{
    ObClient *c = data->layer.any.c;

    client_action_start(data);
    if (data->layer.layer < 0)
        client_set_layer(c, c->below ? 0 : -1);
    else if (data->layer.layer > 0)
        client_set_layer(c, c->above ? 0 : 1);
    client_action_end(data, config_focus_under_mouse);
}

void action_toggle_dockautohide(union ActionData *data)
{
    config_dock_hide = !config_dock_hide;
    dock_configure();
}

void action_toggle_show_desktop(union ActionData *data)
{
    screen_show_desktop(!screen_showing_desktop, NULL);
}

void action_show_desktop(union ActionData *data)
{
    screen_show_desktop(TRUE, NULL);
}

void action_unshow_desktop(union ActionData *data)
{
    screen_show_desktop(FALSE, NULL);
}

void action_break_chroot(union ActionData *data)
{
    /* break out of one chroot */
    keyboard_reset_chains(1);
}

void action_add_desktop(union ActionData *data)
{
    client_action_start(data);
    screen_set_num_desktops(screen_num_desktops+1);

    /* move all the clients over */
    if (data->addremovedesktop.current) {
        GList *it;

        for (it = client_list; it; it = g_list_next(it)) {
            ObClient *c = it->data;
            if (c->desktop != DESKTOP_ALL && c->desktop >= screen_desktop)
                client_set_desktop(c, c->desktop+1, FALSE, TRUE);
        }
    }

    client_action_end(data, config_focus_under_mouse);
}

void action_remove_desktop(union ActionData *data)
{
    if (screen_num_desktops < 2) return;

    client_action_start(data);

    if (screen_desktop == screen_num_desktops - 1)
        data->addremovedesktop.current = FALSE;

    /* move all the clients over */
    if (data->addremovedesktop.current) {
        GList *it, *stacking_copy;

        /* make a copy of the list cuz we're changing it */
        stacking_copy = g_list_copy(stacking_list);
        for (it = g_list_last(stacking_copy); it; it = g_list_previous(it)) {
            if (WINDOW_IS_CLIENT(it->data)) {
                ObClient *c = it->data;
                if (c->desktop != DESKTOP_ALL && c->desktop > screen_desktop)
                    client_set_desktop(c, c->desktop - 1, TRUE, TRUE);
                /* raise all the windows that are on the current desktop which
                   is being merged */
                else if (c->desktop == DESKTOP_ALL ||
                         c->desktop == screen_desktop)
                    stacking_raise(CLIENT_AS_WINDOW(c));
            }
        }
    }

    /* act like we're changing desktops */
    if (screen_desktop < screen_num_desktops - 1) {
        gint d = screen_desktop;
        screen_desktop = screen_last_desktop;
        screen_set_desktop(d, TRUE);
        ob_debug("fake desktop change\n");
    }

    screen_set_num_desktops(screen_num_desktops-1);

    client_action_end(data, config_focus_under_mouse);
}
