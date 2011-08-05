/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   action_filter.h for the Openbox window manager
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

#include <glib.h>

struct _ObActionListRun;
struct _ObConfigValue;
struct _ObClient;
struct _ObClientSet;

typedef struct _ObActionFilter ObActionFilter;
typedef struct _ObActionFilterFuncs ObActionFilterFuncs;
typedef enum _ObActionFilterDefault ObActionFilterDefault;

typedef gpointer (*ObActionFilterSetupFunc)(gboolean invert,
                                            struct _ObConfigValue *v);
typedef void (*ObActionFilterDestroyFunc)(gpointer data);
typedef struct _ObClientSet* (*ObActionFilterFunc)(
    gboolean invert, const struct _ObActionListRun *run, gpointer data);

void action_filter_startup(gboolean reconfig);
void action_filter_shutdown(gboolean reconfig);

/*! Registers a filter test in the system.
  @name The name of the key for the filter. [foo] or [foo=bar] would register
    "foo" as its name.
  @setup A setup function which takes the parameter given to the filter.
    This would receive the bar in [foo=bar].  This returns a pointer to data
    used by the filter.
  @destroy Destroys the data returned from @setup.
  @filter A function that returns an ObClientSet* of clients that this filter
    includes.
  @return TRUE if the registration was successful.
*/
gboolean action_filter_register(const gchar *name,
                                ObActionFilterSetupFunc setup,
                                ObActionFilterDestroyFunc destroy,
                                ObActionFilterFunc set);

ObActionFilter* action_filter_new(const gchar *key, struct _ObConfigValue *v);
void action_filter_ref(ObActionFilter *f);
void action_filter_unref(ObActionFilter *f);

/*! Returns a set of clients for a filter.
  @f The filter.
  @run Data for the user event which caused this filter to be run.
*/
struct _ObClientSet* action_filter_set(ObActionFilter *f,
                                       const struct _ObActionListRun *run);
