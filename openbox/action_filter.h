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

struct _ObActionValue;
struct _ObClient;

typedef struct _ObActionFilter ObActionFilter;
typedef struct _ObActionFilterFuncs ObActionFilterFuncs;

typedef gpointer (*ObActionFilterSetupFunc)(gboolean invert,
                                            struct _ObActionValue *v);
typedef void (*ObActionFilterDestroyFunc)(gpointer data);
/*! Runs the filter and modifies the client set as appropriate.  This function
  is given a set of clients and may simply remove clients that do not
  match its criteria. */
typedef void (*ObActionFilterReduceFunc)(GHashTable *client_set);
/*! Runs the filter and creates a new client set as appropriate.  This function
  is given a set of clients and must add all unlisted clients possible that
  match its criteria. */
typedef void (*ObActionFilterExpandFunc)(GHashTable *client_set);

void action_filter_startup(gboolean reconfig);
void action_filter_shutdown(gboolean reconfig);

gboolean action_filter_register(const gchar *name,
                                ObActionFilterSetupFunc setup,
                                ObActionFilterDestroyFunc destroy,
                                ObActionFilterReduceFunc reduce,
                                ObActionFilterExpandFunc expand);

ObActionFilter* action_filter_new(const gchar *key, struct _ObActionValue *v);
void action_filter_ref(ObActionFilter *f);
void action_filter_unref(ObActionFilter *f);

void action_filter_expand(ObActionFilter *f, GHashTable *client_set);
void action_filter_reduce(ObActionFilter *f, GHashTable *client_set);
