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
#include "frame.h"
#include "parser/parse.h"
#include <glib.h>
#include <X11/Xlib.h>

typedef struct _ObActionsDefinition   ObActionsDefinition;
typedef struct _ObActionsAct          ObActionsAct;
typedef struct _ObActionsData         ObActionsData;
typedef struct _ObActionsAnyData      ObActionsAnyData;
typedef struct _ObActionsGlobalData   ObActionsGlobalData;
typedef struct _ObActionsClientData   ObActionsClientData;
typedef struct _ObActionsSelectorData ObActionsSelectorData;

typedef gpointer (*ObActionsDataSetupFunc)(ObParseInst *i,
                                           xmlDocPtr doc, xmlNodePtr node);
typedef void     (*ObActionsDataFreeFunc)(gpointer options);
typedef gboolean (*ObActionsRunFunc)(ObActionsData *data,
                                     gpointer options);
typedef gboolean (*ObActionsInteractiveInputFunc)(guint initial_state,
                                                  XEvent *e,
                                                  gpointer options);
typedef void     (*ObActionsInteractiveCancelFunc)(gpointer options);

typedef enum {
    OB_ACTION_TYPE_GLOBAL,
    OB_ACTION_TYPE_CLIENT
} ObActionsType;

/* These structures are all castable as eachother */

struct _ObActionsAnyData {
    ObUserAction uact;
    Time time;
    guint state;
    gint x;
    gint y;
};

struct _ObActionsGlobalData {
    ObActionsType type;
    ObActionsAnyData any;
};

struct _ObActionsClientData {
    ObActionsType type;
    ObActionsAnyData any;

    struct _ObClient *c;
    ObFrameContext context;
};

struct _ObActionsData {
    ObActionsType type;

    union {
        ObActionsAnyData      any;
        ObActionsGlobalData   global;
        ObActionsClientData   client;
    };
};

void actions_startup(gboolean reconfigure);
void actions_shutdown(gboolean reconfigure);

/*! If the action is interactive, then i_input and i_cancel are not NULL.
  Otherwise, they should both be NULL. */
gboolean actions_register(const gchar *name,
                          ObActionsType type,
                          ObActionsDataSetupFunc setup,
                          ObActionsDataFreeFunc free,
                          ObActionsRunFunc run,
                          ObActionsInteractiveInputFunc i_input,
                          ObActionsInteractiveCancelFunc i_cancel);

ObActionsAct* actions_parse(ObParseInst *i,
                            xmlDocPtr doc,
                            xmlNodePtr node);
ObActionsAct* actions_parse_string(const gchar *name);

gboolean actions_act_is_interactive(ObActionsAct *act);

void actions_act_ref(ObActionsAct *act);
void actions_act_unref(ObActionsAct *act);

/*! Pass in a GSList of ObActionsAct's to be run.
  @return TRUE if an action is in interactive state, FALSE is none are
*/
void actions_run_acts(GSList *acts,
                      ObUserAction uact,
                      Time time,
                      guint state,
                      gint x,
                      gint y,
                      ObFrameContext con,
                      struct _ObClient *client);
