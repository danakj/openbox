/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   action.c for the Openbox window manager
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
#include "client.h"
#include "focus.h"
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
#include "config.h"
#include "mainloop.h"

#include <glib.h>

inline void client_action_start(union ActionData *data)
{
    if (config_focus_follow)
        if (data->any.context != OB_FRAME_CONTEXT_CLIENT && !data->any.button)
            grab_pointer(TRUE, OB_CURSOR_NONE);
}

inline void client_action_end(union ActionData *data)
{
    if (config_focus_follow)
        if (data->any.context != OB_FRAME_CONTEXT_CLIENT) {
            if (!data->any.button) {
                grab_pointer(FALSE, OB_CURSOR_NONE);
            } else {
                ObClient *c;

                /* usually this is sorta redundant, but with a press action
                   the enter event will come as a GrabNotify which is
                   ignored, so this will handle that case */
                if ((c = client_under_pointer()))
                    event_enter_client(c);
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
    else if (a->func == action_showmenu)
        a->data.showmenu.name = g_strdup(a->data.showmenu.name);

    return a;
}

void setup_action_directional_focus_north(ObAction **a, ObUserAction uact)
{
    (*a)->data.interdiraction.inter.any.interactive = TRUE;
    (*a)->data.interdiraction.direction = OB_DIRECTION_NORTH;
    (*a)->data.interdiraction.dialog = TRUE;
}

void setup_action_directional_focus_east(ObAction **a, ObUserAction uact)
{
    (*a)->data.interdiraction.inter.any.interactive = TRUE;
    (*a)->data.interdiraction.direction = OB_DIRECTION_EAST;
    (*a)->data.interdiraction.dialog = TRUE;
}

void setup_action_directional_focus_south(ObAction **a, ObUserAction uact)
{
    (*a)->data.interdiraction.inter.any.interactive = TRUE;
    (*a)->data.interdiraction.direction = OB_DIRECTION_SOUTH;
    (*a)->data.interdiraction.dialog = TRUE;
}

void setup_action_directional_focus_west(ObAction **a, ObUserAction uact)
{
    (*a)->data.interdiraction.inter.any.interactive = TRUE;
    (*a)->data.interdiraction.direction = OB_DIRECTION_WEST;
    (*a)->data.interdiraction.dialog = TRUE;
}

void setup_action_directional_focus_northeast(ObAction **a, ObUserAction uact)
{
    (*a)->data.interdiraction.inter.any.interactive = TRUE;
    (*a)->data.interdiraction.direction = OB_DIRECTION_NORTHEAST;
    (*a)->data.interdiraction.dialog = TRUE;
}

void setup_action_directional_focus_southeast(ObAction **a, ObUserAction uact)
{
    (*a)->data.interdiraction.inter.any.interactive = TRUE;
    (*a)->data.interdiraction.direction = OB_DIRECTION_SOUTHEAST;
    (*a)->data.interdiraction.dialog = TRUE;
}

void setup_action_directional_focus_southwest(ObAction **a, ObUserAction uact)
{
    (*a)->data.interdiraction.inter.any.interactive = TRUE;
    (*a)->data.interdiraction.direction = OB_DIRECTION_SOUTHWEST;
    (*a)->data.interdiraction.dialog = TRUE;
}

void setup_action_directional_focus_northwest(ObAction **a, ObUserAction uact)
{
    (*a)->data.interdiraction.inter.any.interactive = TRUE;
    (*a)->data.interdiraction.direction = OB_DIRECTION_NORTHWEST;
    (*a)->data.interdiraction.dialog = TRUE;
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
    (*a)->data.desktop.inter.any.interactive = TRUE;
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
}

void setup_action_cycle_windows_previous(ObAction **a, ObUserAction uact)
{
    (*a)->data.cycle.inter.any.interactive = TRUE;
    (*a)->data.cycle.linear = FALSE;
    (*a)->data.cycle.forward = FALSE;
    (*a)->data.cycle.dialog = TRUE;
}

void setup_action_movetoedge_north(ObAction **a, ObUserAction uact)
{
    (*a)->data.diraction.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.diraction.direction = OB_DIRECTION_NORTH;
}

void setup_action_movetoedge_south(ObAction **a, ObUserAction uact)
{
    (*a)->data.diraction.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.diraction.direction = OB_DIRECTION_SOUTH;
}

void setup_action_movetoedge_east(ObAction **a, ObUserAction uact)
{
    (*a)->data.diraction.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.diraction.direction = OB_DIRECTION_EAST;
}

void setup_action_movetoedge_west(ObAction **a, ObUserAction uact)
{
    (*a)->data.diraction.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.diraction.direction = OB_DIRECTION_WEST;
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
    (*a)->data.moveresize.move = TRUE;
    (*a)->data.moveresize.keyboard =
        (uact == OB_USER_ACTION_NONE ||
         uact == OB_USER_ACTION_KEYBOARD_KEY ||
         uact == OB_USER_ACTION_MENU_SELECTION);
}

void setup_action_resize(ObAction **a, ObUserAction uact)
{
    (*a)->data.moveresize.any.client_action = OB_CLIENT_ACTION_ALWAYS;
    (*a)->data.moveresize.move = FALSE;
    (*a)->data.moveresize.keyboard =
        (uact == OB_USER_ACTION_NONE ||
         uact == OB_USER_ACTION_KEYBOARD_KEY ||
         uact == OB_USER_ACTION_MENU_SELECTION);
}

void setup_action_showmenu(ObAction **a, ObUserAction uact)
{
    (*a)->data.showmenu.any.client_action = OB_CLIENT_ACTION_OPTIONAL;
    /* you cannot call ShowMenu from inside a menu, cuz the menu code makes
       assumptions that there is only one menu (and submenus) open at
       a time! */
    if (uact == OB_USER_ACTION_MENU_SELECTION) {
        action_unref(*a);
        a = NULL;
    }
}

void setup_client_action(ObAction **a, ObUserAction uact)
{
    (*a)->data.any.client_action = OB_CLIENT_ACTION_ALWAYS;
}

ActionString actionstrings[] =
{
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
        setup_client_action
    },
    {
        "focus",
        action_focus,
        setup_client_action
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
        action_moveresize,
        setup_action_move
    },
    {
        "resize",
        action_moveresize,
        setup_action_resize
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
        NULL,
        NULL,
        NULL
    }
};

ObAction *action_from_string(const gchar *name, ObUserAction uact)
{
    ObAction *a = NULL;
    gboolean exist = FALSE;
    int i;

    for (i = 0; actionstrings[i].name; i++)
        if (!g_ascii_strcasecmp(name, actionstrings[i].name)) {
            exist = TRUE;
            a = action_new(actionstrings[i].func);
            if (actionstrings[i].setup)
                actionstrings[i].setup(&a, uact);
            /* only key bindings can be interactive. thus saith the xor.
             because of how the mouse is grabbed, mouse events dont even get
             read during interactive events, so no dice! >:) */
            if (uact != OB_USER_ACTION_KEYBOARD_KEY)
                a->data.any.interactive = FALSE;
            break;
        }
    if (!exist)
        g_warning("Invalid action '%s' requested. No such action exists.",
                  name);
    if (!a)
        g_warning("Invalid use of action '%s'. Action will be ignored.", name);
    return a;
}

ObAction *action_parse(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                       ObUserAction uact)
{
    char *actname;
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
            } else if (act->func == action_showmenu) {
                if ((n = parse_find_node("menu", node->xmlChildrenNode)))
                    act->data.showmenu.name = parse_string(doc, n);
            } else if (act->func == action_move_relative_horz ||
                       act->func == action_move_relative_vert ||
                       act->func == action_resize_relative_horz ||
                       act->func == action_resize_relative_vert) {
                if ((n = parse_find_node("delta", node->xmlChildrenNode)))
                    act->data.relative.delta = parse_int(doc, n);
            } else if (act->func == action_desktop) {
                if ((n = parse_find_node("desktop", node->xmlChildrenNode)))
                    act->data.desktop.desk = parse_int(doc, n);
                if (act->data.desktop.desk > 0) act->data.desktop.desk--;
                if ((n = parse_find_node("dialog", node->xmlChildrenNode)))
                    act->data.desktop.inter.any.interactive =
                        parse_bool(doc, n);
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
            } else if (act->func == action_directional_focus) {
                if ((n = parse_find_node("dialog", node->xmlChildrenNode)))
                    act->data.cycle.dialog = parse_bool(doc, n);
            }
        }
        g_free(actname);
    }
    return act;
}

void action_run_list(GSList *acts, ObClient *c, ObFrameContext context,
                     guint state, guint button, gint x, gint y,
                     gboolean cancel, gboolean done)
{
    GSList *it;
    ObAction *a;
    gboolean inter = FALSE;

    if (!acts)
        return;

    if (x < 0 && y < 0)
        screen_pointer_pos(&x, &y);

    if (grab_on_keyboard())
        inter = TRUE;
    else
        for (it = acts; it; it = g_slist_next(it)) {
            a = it->data;
            if (a->data.any.interactive) {
                inter = TRUE;
                break;
            }
        }

    if (!inter) {
        /* sometimes when we execute another app as an action,
           it won't work right unless we XUngrabKeyboard first,
           even though we grabbed the key/button Asychronously.
           e.g. "gnome-panel-control --main-menu" */
        XUngrabKeyboard(ob_display, event_lasttime);
    }

    for (it = acts; it; it = g_slist_next(it)) {
        a = it->data;

        if (!(a->data.any.client_action == OB_CLIENT_ACTION_ALWAYS && !c)) {
            a->data.any.c = a->data.any.client_action ? c : NULL;
            a->data.any.context = context;
            a->data.any.x = x;
            a->data.any.y = y;

            a->data.any.button = button;

            if (a->data.any.interactive) {
                a->data.inter.cancel = cancel;
                a->data.inter.final = done;
                if (!(cancel || done))
                    keyboard_interactive_grab(state, a->data.any.c, a);
            }

            ob_main_loop_queue_action(ob_main_loop, a);
        }
    }
}

void action_run_string(const gchar *name, struct _ObClient *c)
{
    ObAction *a;
    GSList *l;

    a = action_from_string(name, OB_USER_ACTION_NONE);
    g_assert(a);

    l = g_slist_append(NULL, a);

    action_run(l, c, 0);
}

void action_execute(union ActionData *data)
{
    GError *e = NULL;
    char *cmd;
    if (data->execute.path) {
        cmd = g_filename_from_utf8(data->execute.path, -1, NULL, NULL, NULL);
        if (cmd) {
            if (!g_spawn_command_line_async(cmd, &e)) {
                g_warning("failed to execute '%s': %s",
                          cmd, e->message);
            }
            g_free(cmd);
        } else {
            g_warning("failed to convert '%s' from utf8", data->execute.path);
        }
    }
}

void action_activate(union ActionData *data)
{
    client_activate(data->activate.any.c, data->activate.here);
}

void action_focus(union ActionData *data)
{
    client_focus(data->client.any.c);
}

void action_unfocus (union ActionData *data)
{
    client_unfocus(data->client.any.c);
}

void action_iconify(union ActionData *data)
{
    client_iconify(data->client.any.c, TRUE, TRUE);
}

void action_raiselower(union ActionData *data)
{
    ObClient *c = data->client.any.c;
    GList *it;
    gboolean raise = FALSE;

    for (it = stacking_list; it; it = g_list_next(it)) {
        ObClient *cit = it->data;

        if (cit == c) break;
        if (client_normal(cit) == client_normal(c) &&
            cit->layer == c->layer &&
            cit->frame->visible &&
            !client_search_transient(c, cit))
        {
            if (RECT_INTERSECTS_RECT(cit->frame->area, c->frame->area)) {
                raise = TRUE;
                break;
            }
        }
    }

    action_run_string((raise ? "Raise" : "Lower"), c);
}

void action_raise(union ActionData *data)
{
    client_action_start(data);
    stacking_raise(CLIENT_AS_WINDOW(data->client.any.c));
    client_action_end(data);
}

void action_unshaderaise(union ActionData *data)
{
    if (data->client.any.c->shaded)
        action_run_string("Unshade", data->client.any.c);
    else
        action_run_string("Raise", data->client.any.c);
}

void action_shadelower(union ActionData *data)
{
    if (data->client.any.c->shaded)
        action_run_string("Lower", data->client.any.c);
    else
        action_run_string("Shade", data->client.any.c);
}

void action_lower(union ActionData *data)
{
    client_action_start(data);
    stacking_lower(CLIENT_AS_WINDOW(data->client.any.c));
    client_action_end(data);
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
    client_action_end(data);
}

void action_unshade(union ActionData *data)
{
    client_action_start(data);
    client_shade(data->client.any.c, FALSE);
    client_action_end(data);
}

void action_toggle_shade(union ActionData *data)
{
    client_action_start(data);
    client_shade(data->client.any.c, !data->client.any.c->shaded);
    client_action_end(data);
}

void action_toggle_omnipresent(union ActionData *data)
{ 
    client_set_desktop(data->client.any.c,
                       data->client.any.c->desktop == DESKTOP_ALL ?
                       screen_desktop : DESKTOP_ALL, FALSE);
}

void action_move_relative_horz(union ActionData *data)
{
    ObClient *c = data->relative.any.c;
    client_action_start(data);
    client_move(c, c->area.x + data->relative.delta, c->area.y);
    client_action_end(data);
}

void action_move_relative_vert(union ActionData *data)
{
    ObClient *c = data->relative.any.c;
    client_action_start(data);
    client_move(c, c->area.x, c->area.y + data->relative.delta);
    client_action_end(data);
}

void action_resize_relative_horz(union ActionData *data)
{
    ObClient *c = data->relative.any.c;
    client_action_start(data);
    client_resize(c,
                  c->area.width + data->relative.delta * c->size_inc.width,
                  c->area.height);
    client_action_end(data);
}

void action_resize_relative_vert(union ActionData *data)
{
    ObClient *c = data->relative.any.c;
    if (!c->shaded) {
        client_action_start(data);
        client_resize(c, c->area.width, c->area.height +
                      data->relative.delta * c->size_inc.height);
        client_action_end(data);
    }
}

void action_maximize_full(union ActionData *data)
{
    client_action_start(data);
    client_maximize(data->client.any.c, TRUE, 0, TRUE);
    client_action_end(data);
}

void action_unmaximize_full(union ActionData *data)
{
    client_action_start(data);
    client_maximize(data->client.any.c, FALSE, 0, TRUE);
    client_action_end(data);
}

void action_toggle_maximize_full(union ActionData *data)
{
    client_action_start(data);
    client_maximize(data->client.any.c,
                    !(data->client.any.c->max_horz ||
                      data->client.any.c->max_vert),
                    0, TRUE);
    client_action_end(data);
}

void action_maximize_horz(union ActionData *data)
{
    client_action_start(data);
    client_maximize(data->client.any.c, TRUE, 1, TRUE);
    client_action_end(data);
}

void action_unmaximize_horz(union ActionData *data)
{
    client_action_start(data);
    client_maximize(data->client.any.c, FALSE, 1, TRUE);
    client_action_end(data);
}

void action_toggle_maximize_horz(union ActionData *data)
{
    client_action_start(data);
    client_maximize(data->client.any.c,
                    !data->client.any.c->max_horz, 1, TRUE);
    client_action_end(data);
}

void action_maximize_vert(union ActionData *data)
{
    client_action_start(data);
    client_maximize(data->client.any.c, TRUE, 2, TRUE);
    client_action_end(data);
}

void action_unmaximize_vert(union ActionData *data)
{
    client_action_start(data);
    client_maximize(data->client.any.c, FALSE, 2, TRUE);
    client_action_end(data);
}

void action_toggle_maximize_vert(union ActionData *data)
{
    client_action_start(data);
    client_maximize(data->client.any.c,
                    !data->client.any.c->max_vert, 2, TRUE);
    client_action_end(data);
}

void action_send_to_desktop(union ActionData *data)
{
    ObClient *c = data->sendto.any.c;

    if (!client_normal(c)) return;

    if (data->sendto.desk < screen_num_desktops ||
        data->sendto.desk == DESKTOP_ALL) {
        client_set_desktop(c, data->sendto.desk, data->sendto.follow);
        if (data->sendto.follow)
            screen_set_desktop(data->sendto.desk);
    }
}

void action_desktop(union ActionData *data)
{
    static guint first = (unsigned) -1;

    if (data->inter.any.interactive && first == (unsigned) -1)
        first = screen_desktop;

    if (!data->inter.any.interactive ||
        (!data->inter.cancel && !data->inter.final))
    {
        if (data->desktop.desk < screen_num_desktops ||
            data->desktop.desk == DESKTOP_ALL)
        {
            screen_set_desktop(data->desktop.desk);
            if (data->inter.any.interactive)
                screen_desktop_popup(data->desktop.desk, TRUE);
        }
    } else if (data->inter.cancel) {
        screen_set_desktop(first);
    }

    if (data->inter.any.interactive && data->inter.final) {
        screen_desktop_popup(0, FALSE);
        first = (unsigned) -1;
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
    if (!data->sendtodir.inter.any.interactive ||
        !data->sendtodir.inter.final)
    {
        screen_set_desktop(d);
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
    if (!data->sendtodir.inter.any.interactive ||
        !data->sendtodir.inter.final)
    {
        client_set_desktop(c, d, data->sendtodir.follow);
        if (data->sendtodir.follow)
            screen_set_desktop(d);
    }
}

void action_desktop_last(union ActionData *data)
{
    screen_set_desktop(screen_last_desktop);
}

void action_toggle_decorations(union ActionData *data)
{
    ObClient *c = data->client.any.c;

    client_action_start(data);
    client_set_undecorated(c, !c->undecorated);
    client_action_end(data);
}

static guint32 pick_corner(int x, int y, int cx, int cy, int cw, int ch)
{
    if (x - cx > cw / 2) {
        if (y - cy > ch / 2)
            return prop_atoms.net_wm_moveresize_size_bottomright;
        else
            return prop_atoms.net_wm_moveresize_size_topright;
    } else {
        if (y - cy > ch / 2)
            return prop_atoms.net_wm_moveresize_size_bottomleft;
        else
            return prop_atoms.net_wm_moveresize_size_topleft;
    }
}

void action_moveresize(union ActionData *data)
{
    ObClient *c = data->moveresize.any.c;
    guint32 corner;

    if (!client_normal(c)) return;

    if (data->moveresize.keyboard) {
        corner = (data->moveresize.move ?
                  prop_atoms.net_wm_moveresize_move_keyboard :
                  prop_atoms.net_wm_moveresize_size_keyboard);
    } else {
        corner = (data->moveresize.move ?
                  prop_atoms.net_wm_moveresize_move :
                  pick_corner(data->any.x, data->any.y,
                              c->frame->area.x, c->frame->area.y,
                              /* use the client size because the frame
                                 can be differently sized (shaded
                                 windows) and we want this based on the
                                 clients size */
                              c->area.width + c->frame->size.left +
                              c->frame->size.right,
                              c->area.height + c->frame->size.top +
                              c->frame->size.bottom));
    }

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
                  data->showmenu.any.c);
    }
}

void action_cycle_windows(union ActionData *data)
{
    focus_cycle(data->cycle.forward, data->cycle.linear,
                data->cycle.dialog,
                data->cycle.inter.final, data->cycle.inter.cancel);
}

void action_directional_focus(union ActionData *data)
{
    focus_directional_cycle(data->interdiraction.direction,
                            data->interdiraction.dialog,
                            data->interdiraction.inter.final,
                            data->interdiraction.inter.cancel);
}

void action_movetoedge(union ActionData *data)
{
    int x, y;
    ObClient *c = data->diraction.any.c;

    x = c->frame->area.x;
    y = c->frame->area.y;
    
    switch(data->diraction.direction) {
    case OB_DIRECTION_NORTH:
        y = client_directional_edge_search(c, OB_DIRECTION_NORTH);
        break;
    case OB_DIRECTION_WEST:
        x = client_directional_edge_search(c, OB_DIRECTION_WEST);
        break;
    case OB_DIRECTION_SOUTH:
        y = client_directional_edge_search(c, OB_DIRECTION_SOUTH) -
            c->frame->area.height;
        break;
    case OB_DIRECTION_EAST:
        x = client_directional_edge_search(c, OB_DIRECTION_EAST) -
            c->frame->area.width;
        break;
    default:
        g_assert_not_reached();
    }
    frame_frame_gravity(c->frame, &x, &y);
    client_action_start(data);
    client_move(c, x, y);
    client_action_end(data);
}

void action_growtoedge(union ActionData *data)
{
    int x, y, width, height, dest;
    ObClient *c = data->diraction.any.c;
    Rect *a;

    a = screen_area(c->desktop);
    x = c->frame->area.x;
    y = c->frame->area.y;
    width = c->frame->area.width;
    height = c->frame->area.height;

    switch(data->diraction.direction) {
    case OB_DIRECTION_NORTH:
        dest = client_directional_edge_search(c, OB_DIRECTION_NORTH);
        if (a->y == y)
            height = c->frame->area.height / 2;
        else {
            height = c->frame->area.y + c->frame->area.height - dest;
            y = dest;
        }
        break;
    case OB_DIRECTION_WEST:
        dest = client_directional_edge_search(c, OB_DIRECTION_WEST);
        if (a->x == x)
            width = c->frame->area.width / 2;
        else {
            width = c->frame->area.x + c->frame->area.width - dest;
            x = dest;
        }
        break;
    case OB_DIRECTION_SOUTH:
        dest = client_directional_edge_search(c, OB_DIRECTION_SOUTH);
        if (a->y + a->height == y + c->frame->area.height) {
            height = c->frame->area.height / 2;
            y = a->y + a->height - height;
        } else
            height = dest - c->frame->area.y;
        y += (height - c->frame->area.height) % c->size_inc.height;
        height -= (height - c->frame->area.height) % c->size_inc.height;
        break;
    case OB_DIRECTION_EAST:
        dest = client_directional_edge_search(c, OB_DIRECTION_EAST);
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
    frame_frame_gravity(c->frame, &x, &y);
    width -= c->frame->size.left + c->frame->size.right;
    height -= c->frame->size.top + c->frame->size.bottom;
    client_action_start(data);
    client_move_resize(c, x, y, width, height);
    client_action_end(data);
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
    client_action_end(data);
}

void action_toggle_show_desktop(union ActionData *data)
{
    screen_show_desktop(!screen_showing_desktop);
}

void action_show_desktop(union ActionData *data)
{
    screen_show_desktop(TRUE);
}

void action_unshow_desktop(union ActionData *data)
{
    screen_show_desktop(FALSE);
}
