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

#include "per_app_settings.h"
#include "screen.h"

GSList *per_app_settings;

ObAppSetting *get_client_settings(ObClient *client)
{
    GSList *a = per_app_settings;

    while (a) {
        ObAppSetting *app = (ObAppSetting *) a->data;
        
        if (!strcmp(app->name, client->name)) {
            ob_debug("Window matching: %s\n", app->name);

            return (ObAppSetting *) a->data;
        }

        a = a->next;
    }
    return NULL;
}

void place_window_from_settings(ObAppSetting *setting, ObClient *client, gint *x, gint *y)
{
    gint px, py, i;
    Rect *screen;

    /* Find which head the pointer is on, partly taken from place.c */
    if (setting->head == -1) {
        screen_pointer_pos(&px, &py);

        for (i = 0; i < screen_num_monitors; i++) {
            screen = screen_area_monitor(client->desktop, i);
            if (RECT_CONTAINS(*screen, px, py))
                break;
        }

        if (i == screen_num_monitors)
            screen = screen_area_monitor(client->desktop, 0);
    }
    else
        screen = screen_area_monitor(client->desktop, setting->head);

    if (setting->position.x == -1 && setting->center_x)
        *x = screen->x + screen->width / 2 - client->area.width / 2;
    else if (setting->position.x != -1)
        *x = screen->x + setting->position.x;

    if (setting->position.y == -1 && setting->center_y)
        *y = screen->y + screen->height / 2 - client->area.height / 2;
    else if (setting->position.y != -1)
        *y = screen->y + setting->position.y;

}
