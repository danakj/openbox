/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   action.h for the Openbox window manager
   Copyright (c) 2007-2011   Dana Jansens

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
#include "obt/xml.h"
#include "obt/keyboard.h"

#include <glib.h>
#include <X11/Xlib.h>

struct _ObActionList;
struct _ObActionListRun;
struct _ObClientSet;

typedef struct _ObActionDefinition   ObActionDefinition;
typedef struct _ObAction             ObAction;
typedef struct _ObActionData         ObActionData;

typedef void     (*ObActionDataFreeFunc)(gpointer options);
typedef gboolean (*ObActionRunFunc)(const struct _ObClientSet *set,
                                    const struct _ObActionListRun *data,
                                    gpointer options);
typedef gpointer (*ObActionDataSetupFunc)(GHashTable *config);
typedef void     (*ObActionShutdownFunc)(void);

/* functions for interactive actions */
/*! Returns TRUE if the action is going to be interactive, or FALSE to change
  your mind and make it not. */
typedef gboolean (*ObActionIPreFunc)(guint initial_state, gpointer options);
typedef void     (*ObActionIPostFunc)(gpointer options);
/*! Returns TRUE to continue the interactive action, and FALSE to end it. */
typedef gboolean (*ObActionIInputFunc)(guint initial_state,
                                       XEvent *e,
                                       ObtIC *ic,
                                       gpointer options,
                                       gboolean *used);
typedef void     (*ObActionICancelFunc)(gpointer options);
typedef gpointer (*ObActionIDataSetupFunc)(GHashTable *config,
                                           ObActionIPreFunc *pre,
                                           ObActionIInputFunc *input,
                                           ObActionICancelFunc *cancel,
                                           ObActionIPostFunc *post);

/*! The default filter an action would like if no filter is provided by the
  user */
typedef enum {
    OB_ACTION_DEFAULT_FILTER_EMPTY,
    OB_ACTION_DEFAULT_FILTER_SINGLE,
    OB_ACTION_DEFAULT_FILTER_ALL,
    OB_NUM_ACTION_DEFAULT_FILTERS
} ObActionDefaultFilter;

void action_startup(gboolean reconfigure);
void action_shutdown(gboolean reconfigure);

/*! Use this if the actions created from this name may be interactive */
gboolean action_register_i(const gchar *name,
                           ObActionDefaultFilter def_filter,
                           ObActionIDataSetupFunc setup,
                           ObActionDataFreeFunc free,
                           ObActionRunFunc run);

gboolean action_register(const gchar *name,
                         ObActionDefaultFilter def_filter,
                         ObActionDataSetupFunc setup,
                         ObActionDataFreeFunc free,
                         ObActionRunFunc run);

gboolean action_set_shutdown(const gchar *name,
                             ObActionShutdownFunc shutdown);

gboolean action_is_interactive(ObAction *act);

/*! Create a new ObAction structure.
  @name The name of the action.
  @keys The names of the options passed to the action.
  @values The values of the options passed to the action, paired with the
    keys.  These are ObActionListValue objects.
*/
ObAction* action_new(const gchar *name, GHashTable *config);

void action_ref(ObAction *act);
void action_unref(ObAction *act);

ObActionDefaultFilter action_default_filter(ObAction *act);

/*! Runs an action.
  @return TRUE if an interactive action was started, FALSE otherwise.
*/
gboolean action_run(ObAction *act, const struct _ObActionListRun *data,
                    struct _ObClientSet *set);

gboolean action_interactive_act_running(void);
void action_interactive_cancel_act(void);

gboolean action_interactive_input_event(XEvent *e);

/*! Function for actions to call when they are moving a client around */
void action_client_move(const struct _ObActionListRun *data,
                        gboolean start);
