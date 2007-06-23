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

void setup_client_action(ObAction **a, ObUserAction uact)
{
    (*a)->data.any.client_action = OB_CLIENT_ACTION_ALWAYS;
}

ActionString actionstrings[] =
{
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

void action_growtoedge(union ActionData *data)
{
}
