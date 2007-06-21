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

typedef struct _ObActionsDefinition   ObActionsDefinition;
typedef struct _ObActionsAct          ObActionsAct;
typedef struct _ObActionsData         ObActionsData;
typedef struct _ObActionsAnyData      ObActionsAnyData;
typedef struct _ObActionsGlobalData   ObActionsGlobalData;
typedef struct _ObActionsClientData   ObActionsClientData;
typedef struct _ObActionsSelectorData ObActionsSelectorData;

typedef enum {
    OB_ACTION_DONE,
    OB_ACTION_CANCELLED,
    OB_ACTION_INTERACTING,
    OB_NUM_ACTIONS_INTERACTIVE_STATES
} ObActionsInteractiveState;

typedef gpointer (*ObActionsDataSetupFunc)(ObParseInst *i,
                                           xmlDocPtr doc, xmlNodePtr node);
typedef void     (*ObActionsDataFreeFunc)(gpointer options);
typedef void     (*ObActionsRunFunc)(ObActionsData *data,
                                     gpointer options);

/*
  The theory goes:

  06:10 (@dana) hm i think there are 3 types of actions
  06:10 (@dana) global actions, window actions, and selector actions
  06:11 (@dana) eg show menu/exit, raise/focus, and cycling/directional/expose
*/

typedef enum {
    OB_ACTION_TYPE_GLOBAL,
    OB_ACTION_TYPE_CLIENT,
    OB_ACTION_TYPE_SELECTOR
} ObActionsType;

struct _ObActionsAnyData {
    ObUserAction uact;
    gint x;
    gint y;
    gint button;
    Time time;

    ObActionsInteractiveState interactive;
};

struct _ObActionsGlobalData {
    ObActionsAnyData any;
};

struct _ObActionsClientData {
    ObActionsAnyData any;

    struct _ObClient *c;
    ObFrameContext context;
};

struct _ObActionsSelectorData {
    ObActionsAnyData any;

    GSList *actions;
};

struct _ObActionsData {
    ObActionsType type;

    union {
        ObActionsAnyData      any;
        ObActionsGlobalData   global;
        ObActionsClientData   client;
        ObActionsSelectorData selector;
    };
};

void actions_startup(gboolean reconfigure);
void actions_shutdown(gboolean reconfigure);

gboolean actions_register(const gchar *name,
                          gboolean allow_interactive,
                          ObActionsDataSetupFunc setup,
                          ObActionsDataFreeFunc free,
                          ObActionsRunFunc run);

ObActionsAct* actions_parse(ObParseInst *i,
                            xmlDocPtr doc,
                            xmlNodePtr node);

void actions_act_ref(ObActionsAct *act);
void actions_act_unref(ObActionsAct *act);
