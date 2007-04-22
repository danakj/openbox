/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   client_menu.c for the Openbox window manager
   Copyright (c) 2003        Ben Jansens

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

#include "debug.h"
#include "menu.h"
#include "menuframe.h"
#include "screen.h"
#include "client.h"
#include "openbox.h"
#include "frame.h"
#include "gettext.h"

#include <glib.h>

#define CLIENT_MENU_NAME  "client-menu"
#define SEND_TO_MENU_NAME "client-send-to-menu"
#define LAYER_MENU_NAME   "client-layer-menu"

enum {
    LAYER_TOP,
    LAYER_NORMAL,
    LAYER_BOTTOM
};

enum {
    CLIENT_SEND_TO,
    CLIENT_LAYER,
    CLIENT_ICONIFY,
    CLIENT_MAXIMIZE,
    CLIENT_RAISE,
    CLIENT_LOWER,
    CLIENT_SHADE,
    CLIENT_DECORATE,
    CLIENT_MOVE,
    CLIENT_RESIZE,
    CLIENT_CLOSE
};

static void client_update(ObMenuFrame *frame, gpointer data)
{
    ObMenu *menu = frame->menu;
    ObMenuEntry *e;
    GList *it;

    for (it = menu->entries; it; it = g_list_next(it)) {
        e = it->data;
        if (e->type == OB_MENU_ENTRY_TYPE_NORMAL)
            e->data.normal.enabled = !!frame->client;
    }

    if (!frame->client)
        return;

    e = menu_find_entry_id(menu, CLIENT_ICONIFY);
    e->data.normal.enabled = frame->client->functions & OB_CLIENT_FUNC_ICONIFY;

    e = menu_find_entry_id(menu, CLIENT_MAXIMIZE);
    g_free(e->data.normal.label);
    e->data.normal.label =
        g_strdup(frame->client->max_vert || frame->client->max_horz ?
                 _("Restore") : _("Maximize"));
    e->data.normal.enabled =frame->client->functions & OB_CLIENT_FUNC_MAXIMIZE;

    e = menu_find_entry_id(menu, CLIENT_SHADE);
    g_free(e->data.normal.label);
    e->data.normal.label = g_strdup(frame->client->shaded ?
                                    _("Roll down") : _("Roll up"));
    e->data.normal.enabled = frame->client->functions & OB_CLIENT_FUNC_SHADE;

    e = menu_find_entry_id(menu, CLIENT_MOVE);
    e->data.normal.enabled = frame->client->functions & OB_CLIENT_FUNC_MOVE;

    e = menu_find_entry_id(menu, CLIENT_RESIZE);
    e->data.normal.enabled = frame->client->functions & OB_CLIENT_FUNC_RESIZE;

    e = menu_find_entry_id(menu, CLIENT_CLOSE);
    e->data.normal.enabled = frame->client->functions & OB_CLIENT_FUNC_CLOSE;

    e = menu_find_entry_id(menu, CLIENT_DECORATE);
    e->data.normal.enabled = client_normal(frame->client);
}

static void layer_update(ObMenuFrame *frame, gpointer data)
{
    ObMenu *menu = frame->menu;
    ObMenuEntry *e;
    GList *it;

    for (it = menu->entries; it; it = g_list_next(it)) {
        e = it->data;
        if (e->type == OB_MENU_ENTRY_TYPE_NORMAL)
            e->data.normal.enabled = !!frame->client;
    }

    if (!frame->client)
        return;

    e = menu_find_entry_id(menu, LAYER_TOP);
    e->data.normal.enabled = !frame->client->above;

    e = menu_find_entry_id(menu, LAYER_NORMAL);
    e->data.normal.enabled = (frame->client->above || frame->client->below);

    e = menu_find_entry_id(menu, LAYER_BOTTOM);
    e->data.normal.enabled = !frame->client->below;
}

static void send_to_update(ObMenuFrame *frame, gpointer data)
{
    ObMenu *menu = frame->menu;
    guint i;
    GSList *acts;
    ObAction *act;
    ObMenuEntry *e;;

    menu_clear_entries(menu);

    if (!frame->client)
        return;

    for (i = 0; i <= screen_num_desktops; ++i) {
        const gchar *name;
        guint desk;

        if (i >= screen_num_desktops) {
            menu_add_separator(menu, -1, NULL);

            desk = DESKTOP_ALL;
            name = _("All desktops");
        } else {
            desk = i;
            name = screen_desktop_names[i];
        }

        act = action_from_string("SendToDesktop",
                                 OB_USER_ACTION_MENU_SELECTION);
        act->data.sendto.desk = desk;
        act->data.sendto.follow = FALSE;
        acts = g_slist_prepend(NULL, act);
        e = menu_add_normal(menu, desk, name, acts);

        if (frame->client->desktop == desk)
            e->data.normal.enabled = FALSE;
    }
}

void client_menu_startup()
{
    GSList *acts;
    ObMenu *menu;
    ObMenuEntry *e;

    menu = menu_new(LAYER_MENU_NAME, _("Layer"), NULL);
    menu_set_update_func(menu, layer_update);

    acts = g_slist_prepend(NULL, action_from_string
                           ("SendToTopLayer", OB_USER_ACTION_MENU_SELECTION));
    menu_add_normal(menu, LAYER_TOP, _("Always on top"), acts);

    acts = g_slist_prepend(NULL, action_from_string
                           ("SendToNormalLayer",
                            OB_USER_ACTION_MENU_SELECTION));
    menu_add_normal(menu, LAYER_NORMAL, _("Normal"), acts);

    acts = g_slist_prepend(NULL, action_from_string
                           ("SendToBottomLayer",
                            OB_USER_ACTION_MENU_SELECTION));
    menu_add_normal(menu, LAYER_BOTTOM, _("Always on bottom"),acts);


    menu = menu_new(SEND_TO_MENU_NAME, _("Send to desktop"), NULL);
    menu_set_update_func(menu, send_to_update);


    menu = menu_new(CLIENT_MENU_NAME, _("Client menu"), NULL);
    menu_set_update_func(menu, client_update);

    menu_add_submenu(menu, CLIENT_SEND_TO, SEND_TO_MENU_NAME);

    menu_add_submenu(menu, CLIENT_LAYER, LAYER_MENU_NAME);

    acts = g_slist_prepend(NULL, action_from_string
                           ("Iconify", OB_USER_ACTION_MENU_SELECTION));
    e = menu_add_normal(menu, CLIENT_ICONIFY, _("Iconify"), acts);
    e->data.normal.mask = ob_rr_theme->iconify_mask;
    e->data.normal.mask_normal_color = ob_rr_theme->menu_color;
    e->data.normal.mask_disabled_color = ob_rr_theme->menu_disabled_color;
    e->data.normal.mask_selected_color = ob_rr_theme->menu_selected_color;

    acts = g_slist_prepend(NULL, action_from_string
                           ("ToggleMaximizeFull",
                            OB_USER_ACTION_MENU_SELECTION));
    e = menu_add_normal(menu, CLIENT_MAXIMIZE, "MAXIMIZE", acts);
    e->data.normal.mask = ob_rr_theme->max_mask; 
    e->data.normal.mask_normal_color = ob_rr_theme->menu_color;
    e->data.normal.mask_disabled_color = ob_rr_theme->menu_disabled_color;
    e->data.normal.mask_selected_color = ob_rr_theme->menu_selected_color;

    acts = g_slist_prepend(NULL, action_from_string
                           ("Raise", OB_USER_ACTION_MENU_SELECTION));
    menu_add_normal(menu, CLIENT_RAISE, _("Raise to top"), acts);

    acts = g_slist_prepend(NULL, action_from_string
                           ("Lower", OB_USER_ACTION_MENU_SELECTION));
    menu_add_normal(menu, CLIENT_LOWER, _("Lower to bottom"),acts);

    acts = g_slist_prepend(NULL, action_from_string
                           ("ToggleShade", OB_USER_ACTION_MENU_SELECTION));
    e = menu_add_normal(menu, CLIENT_SHADE, "SHADE", acts);
    e->data.normal.mask = ob_rr_theme->shade_mask;
    e->data.normal.mask_normal_color = ob_rr_theme->menu_color;
    e->data.normal.mask_disabled_color = ob_rr_theme->menu_disabled_color;
    e->data.normal.mask_selected_color = ob_rr_theme->menu_selected_color;

    acts = g_slist_prepend(NULL, action_from_string
                           ("ToggleDecorations",
                            OB_USER_ACTION_MENU_SELECTION));
    menu_add_normal(menu, CLIENT_DECORATE, _("Decorate"), acts);

    menu_add_separator(menu, -1, NULL);

    acts = g_slist_prepend(NULL, action_from_string
                           ("Move", OB_USER_ACTION_MENU_SELECTION));
    menu_add_normal(menu, CLIENT_MOVE, _("Move"), acts);

    acts = g_slist_prepend(NULL, action_from_string
                           ("Resize", OB_USER_ACTION_MENU_SELECTION));
    menu_add_normal(menu, CLIENT_RESIZE, _("Resize"), acts);

    menu_add_separator(menu, -1, NULL);

    acts = g_slist_prepend(NULL, action_from_string
                           ("Close", OB_USER_ACTION_MENU_SELECTION));
    e = menu_add_normal(menu, CLIENT_CLOSE, _("Close"), acts);
    e->data.normal.mask = ob_rr_theme->close_mask;
    e->data.normal.mask_normal_color = ob_rr_theme->menu_color;
    e->data.normal.mask_disabled_color = ob_rr_theme->menu_disabled_color;
    e->data.normal.mask_selected_color = ob_rr_theme->menu_selected_color;
}
