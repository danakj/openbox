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
#include "focus.h"
#include "config.h"
#include "gettext.h"

#include <glib.h>

#define MENU_NAME "client-list-menu"

static GSList *desktop_menus;

typedef struct
{
    guint desktop;
} DesktopData;

static gboolean desk_menu_update(ObMenuFrame *frame, gpointer data)
{
    ObMenu *menu = frame->menu;
    DesktopData *d = data;
    GList *it;
    gboolean empty = TRUE;
    gboolean onlyiconic = TRUE;

    menu_clear_entries(menu);

    for (it = focus_order; it; it = g_list_next(it)) {
        ObClient *c = it->data;
        if (client_normal(c) && (!c->skip_taskbar || c->iconic) &&
            (c->desktop == d->desktop || c->desktop == DESKTOP_ALL))
        {
            ObMenuEntry *e;
            const ObClientIcon *icon;

            empty = FALSE;

            if (c->iconic) {
                gchar *title = g_strdup_printf("(%s)", c->icon_title);
                e = menu_add_normal(menu, -1, title, NULL, FALSE);
                g_free(title);
            } else {
                onlyiconic = FALSE;
                e = menu_add_normal(menu, -1, c->title, NULL, FALSE);
            }

            if (config_menu_client_list_icons
                && (icon = client_icon(c, 32, 32))) {
                e->data.normal.icon_width = icon->width;
                e->data.normal.icon_height = icon->height;
                e->data.normal.icon_data = icon->data;
                e->data.normal.icon_alpha = c->iconic ? OB_ICONIC_ALPHA : 0xff;
            }

            e->data.normal.data = c;
        }
    }

    if (empty || onlyiconic) {
        ObMenuEntry *e;

        /* no entries or only iconified windows, so add a
         * way to go to this desktop without uniconifying a window */
        if (!empty)
            menu_add_separator(menu, -1, NULL);

        e = menu_add_normal(menu, d->desktop, _("Go there..."), NULL, TRUE);
        if (d->desktop == screen_desktop)
            e->data.normal.enabled = FALSE;
    }
    return TRUE; /* always show */
}

static void desk_menu_execute(ObMenuEntry *self, ObMenuFrame *f,
                              ObClient *c, guint state, gpointer data)
{
    if (self->id == -1) {
        if (self->data.normal.data) /* it's set to NULL if its destroyed */
            client_activate(self->data.normal.data, FALSE, TRUE);
    }
    else
        screen_set_desktop(self->id, TRUE);
}

static void desk_menu_destroy(ObMenu *menu, gpointer data)
{
    DesktopData *d = data;

    g_free(d);

    desktop_menus = g_slist_remove(desktop_menus, menu);
}

static gboolean self_update(ObMenuFrame *frame, gpointer data)
{
    ObMenu *menu = frame->menu;
    guint i;

    menu_clear_entries(menu);

    while (desktop_menus) {
        menu_free(desktop_menus->data);
        desktop_menus = g_slist_delete_link(desktop_menus, desktop_menus);
    }
    
    for (i = 0; i < screen_num_desktops; ++i) {
        ObMenu *submenu;
        gchar *name = g_strdup_printf("%s-%u", MENU_NAME, i);
        DesktopData *data = g_new(DesktopData, 1);

        data->desktop = i;
        submenu = menu_new(name, screen_desktop_names[i], FALSE, data);
        menu_set_update_func(submenu, desk_menu_update);
        menu_set_execute_func(submenu, desk_menu_execute);
        menu_set_destroy_func(submenu, desk_menu_destroy);

        menu_add_submenu(menu, i, name);

        g_free(name);

        desktop_menus = g_slist_append(desktop_menus, submenu);
    }

    return TRUE; /* always show */
}

static void client_dest(ObClient *client, gpointer data)
{
    /* This concise function removes all references to a closed
     * client in the client_list_menu, so we don't have to check
     * in client.c */
    GSList *it;
    for (it = desktop_menus; it; it = g_slist_next(it)) {
        ObMenu *mit = it->data;
        GList *eit;
        for (eit = mit->entries; eit; eit = g_list_next(eit)) {
            ObMenuEntry *meit = eit->data;
            if (meit->type == OB_MENU_ENTRY_TYPE_NORMAL &&
                meit->data.normal.data == client)
            {
                    meit->data.normal.data = NULL;
            }
        }
    }
}

void client_list_menu_startup(gboolean reconfig)
{
    ObMenu *menu;

    if (!reconfig)
        client_add_destroy_notify(client_dest, NULL);

    menu = menu_new(MENU_NAME, _("Desktops"), TRUE, NULL);
    menu_set_update_func(menu, self_update);
}

void client_list_menu_shutdown(gboolean reconfig)
{
    if (!reconfig)
        client_remove_destroy_notify(client_dest);
}
