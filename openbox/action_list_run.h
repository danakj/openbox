/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   action_list_run.h for the Openbox window manager
   Copyright (c) 2011        Dana Jansens

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

#include "misc.h"
#include "frame.h"

#include <glib.h>

typedef struct _ObActionListRun ObActionListRun;

struct _ObActionList;

/*! This structure holds data about the user event which triggers running an
  action list and the filters/actions within it.
  All action list executions must come from some user event, and this describes
  it.
*/
struct _ObActionListRun {
    ObUserAction user_act;
    guint mod_state;

    gint pointer_x;
    gint pointer_y;
    gint pointer_button;
    ObFrameContext pointer_context;
    struct _ObClient *pointer_over;

    struct _ObClient *target;
};

/*! Run an action list.
  @acts The list of actions to run.
  @state The current state of the keyboard modifiers.
  @x The x coordinate of the pointer.
  @y The y coordinate of the pointer.
  @button The button used to initiate the action list (if a pointer
    initiated it).
  @con The frame context on which the pointer button was used to initiate the
    action list.
  @client The client on which the pointer button was used to initiate the
    action list.
  @return TRUE if an interactive action was started, or FALSE otherwise.
*/
gboolean action_list_run(struct _ObActionList *acts,
                         ObUserAction uact,
                         guint state,
                         gint x,
                         gint y,
                         gint button,
                         ObFrameContext con,
                         struct _ObClient *client);
