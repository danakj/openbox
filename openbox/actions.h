/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   actions.h for the Openbox window manager
   Copyright (c) 2007        Dana Jansens

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
#include "engine_interface.h"
#include "obt/parse.h"

#include <glib.h>
#include <X11/Xlib.h>

typedef struct _ObActionsDefinition   ObActionsDefinition;
typedef struct _ObActionsAct          ObActionsAct;
typedef struct _ObActionsData         ObActionsData;
typedef struct _ObActionsAnyData      ObActionsAnyData;
typedef struct _ObActionsGlobalData   ObActionsGlobalData;
typedef struct _ObActionsClientData   ObActionsClientData;
typedef struct _ObActionsSelectorData ObActionsSelectorData;

typedef gpointer (*ObActionsDataSetupFunc)(xmlNodePtr node);
typedef void     (*ObActionsDataFreeFunc)(gpointer options);
typedef gboolean (*ObActionsRunFunc)(ObActionsData *data,
                                     gpointer options);
typedef gboolean (*ObActionsInteractiveInputFunc)(guint initial_state,
                                                  XEvent *e,
                                                  gpointer options,
                                                  gboolean *used);
typedef void     (*ObActionsInteractiveCancelFunc)(gpointer options);

struct _ObActionsData {
    ObUserAction uact;
    guint state;
    gint x;
    gint y;
    gint button;

    struct _ObClient *client;
    ObFrameContext context;
};

void actions_startup(gboolean reconfigure);
void actions_shutdown(gboolean reconfigure);

/*! If the action is interactive, then i_input and i_cancel are not NULL.
  Otherwise, they should both be NULL. */
gboolean actions_register(const gchar *name,
                          ObActionsDataSetupFunc setup,
                          ObActionsDataFreeFunc free,
                          ObActionsRunFunc run,
                          ObActionsInteractiveInputFunc i_input,
                          ObActionsInteractiveCancelFunc i_cancel);

ObActionsAct* actions_parse(xmlNodePtr node);
ObActionsAct* actions_parse_string(const gchar *name);

gboolean actions_act_is_interactive(ObActionsAct *act);

void actions_act_ref(ObActionsAct *act);
void actions_act_unref(ObActionsAct *act);

/*! When this is true, an XAllowEvents with ReplayPointer will be called
  if an action is going to maybe try moving windows around on screen (or
  map/unmap windows)
*/
void actions_set_need_pointer_replay_before_move(gboolean replay);
/*! Returns if a ReplayPointer is still needed.  If it was called while running
  actions then this will be false */
gboolean actions_get_need_pointer_replay_before_move();

/*! Pass in a GSList of ObActionsAct's to be run. */
void actions_run_acts(GSList *acts,
                      ObUserAction uact,
                      guint state,
                      gint x,
                      gint y,
                      gint button,
                      ObFrameContext con,
                      struct _ObClient *client);

gboolean actions_interactive_act_running();
void actions_interactive_cancel_act();

gboolean actions_interactive_input_event(XEvent *e);

/*! Function for actions to call when they are moving a client around */
void actions_client_move(ObActionsData *data, gboolean start);
