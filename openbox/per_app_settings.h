/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   client.h for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson

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

#ifndef __per_app_settings_h
#define __per_app_settings_h

#include "client.h"

typedef struct _ObAppSetting ObAppSetting;

struct _ObAppSetting
{
    gchar *name;
    gboolean decor;
    gboolean shade;
    gboolean focus;

    Point position;
    gboolean center_x;
    gboolean center_y;

    guint desktop;
    guint head;

    guint layer;
};

extern GSList *per_app_settings;

ObAppSetting *get_client_settings(ObClient *client);
void place_window_from_settings(ObAppSetting *setting, ObClient *client, gint *x, gint *y);

#endif
