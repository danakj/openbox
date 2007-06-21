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

typedef enum {
    OB_ACTION_DONE,
    OB_ACTION_CANCELLED,
    OB_ACTION_INTERACTING,
    OB_NUM_ACTIONS_INTERACTIVE_STATES
} ObActionsInteractiveState;

typedef gpointer (*ObActionsDataSetupFunc)();
typedef void     (*ObActionsDataParseFunc)(gpointer action_data,
                                           ObParseInst *i,
                                           xmlDocPtr doc, xmlNodePtr node);
typedef void     (*ObActionsDataFreeFunc)(gpointer action_data);
typedef void     (*ObActionsRunFunc)(ObActionsAnyData *data);

struct _ObActionsDefinition {
    gchar *name;
    gboolean interactive;

    ObActionsDataSetupFunc setup;
    ObActionsDataParseFunc parse;
    ObActionsDataFreeFunc free;
    ObActionsRunFunc run;
};

struct _ObActionsAnyData {
    ObUserAction uact;
    gint x;
    gint y;
    gint button;
    Time time;

    ObActionsInteractiveState interactive;

    gpointer action_data;
};

struct _ObActionsGlobalData {
    ObActionsData any;
};

struct _ObActionsClientData {
    ObActionsData any;

    struct _ObClient *c;
    ObFrameContext context;
};

struct _ObActionsSelectorData {
    ObActionsData any;

    GSList *actions;
};

void actions_startup(gboolean reconfigure);
void actions_shutdown(gboolean reconfigure);

gboolean actions_register(const gchar *name,
                          gboolean interactive,
                          ObActionsDataSetupFunc setup,
                          ObActionsDataParseFunc parse,
                          ObActionsDataFreeFunc free,
                          ObActionsRunFunc run);
