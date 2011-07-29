/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   filters/all.c for the Openbox window manager
   Copyright (c) 2011             Dana Jansens

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

#include "openbox/action_filter.h"
#include "openbox/event.h"

static gboolean reduce(struct _ObClient *c, gpointer data)
{
    return c != event_current_target(); /* remove anything not the target */
}
static gboolean expand(struct _ObClient *c, gpointer data)
{
    return c == event_current_target(); /* add only the target */
}

void filter_target_startup(void)
{
    action_filter_register("target", NULL, NULL, reduce, expand);
}
