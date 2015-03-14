/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   client_list_menu.c for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
   Copyright (c) 2003-2007   Dana Jansens

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

#include "openbox.h"
#include "menu.h"
#include "menuframe.h"
#include "screen.h"
#include "client.h"
#include "desktop_list_menu.h"
#include "focus.h"
#include "config.h"
#include "gettext.h"

#include <glib.h>

#define MENU_NAME "desktop-list-menu"

static ObMenu *desk_menu;

#define SEPARATOR -1
#define ADD_DESKTOP -2
#define REMOVE_DESKTOP -3

static void self_cleanup(ObMenu *menu, gpointer data)
{
    menu_clear_entries(menu);
}

static gboolean self_update(ObMenuFrame *frame, gpointer data)
{
    ObMenu *menu = frame->menu;
    guint desktop;

    menu_clear_entries(menu);

    menu_add_separator(menu, SEPARATOR, "Desktops");
    for (desktop = 0; desktop < screen_num_desktops; desktop++) {
        menu_add_normal(menu, desktop, screen_desktop_names[desktop],
                        NULL, FALSE);
    }

    return TRUE; /* always show the menu */
}

static void menu_execute(ObMenuEntry *self, ObMenuFrame *f,
                         ObClient *c, guint state, gpointer data)
{
    screen_set_desktop(self->id, TRUE, FALSE);
}

void desktop_list_menu_startup(gboolean reconfig)
{
    desk_menu = menu_new(MENU_NAME, _("Desktops"), TRUE, NULL);
    menu_set_update_func(desk_menu, self_update);
    menu_set_cleanup_func(desk_menu, self_cleanup);
    menu_set_execute_func(desk_menu, menu_execute);
}

void desktop_list_menu_shutdown(gboolean reconfig)
{
    ;
}
