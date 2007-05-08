/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   client_menu.c for the Openbox window manager
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
    CLIENT_RESTORE,
    CLIENT_MAXIMIZE,
    CLIENT_SHADE,
    CLIENT_DECORATE,
    CLIENT_MOVE,
    CLIENT_RESIZE,
    CLIENT_CLOSE
};

static gboolean client_update(ObMenuFrame *frame, gpointer data)
{
    ObMenu *menu = frame->menu;
    ObMenuEntry *e;
    GList *it;

    if (frame->client == NULL || !client_normal(frame->client))
        return FALSE; /* don't show the menu */

    for (it = menu->entries; it; it = g_list_next(it)) {
        e = it->data;
        if (e->type == OB_MENU_ENTRY_TYPE_NORMAL)
            e->data.normal.enabled = TRUE;
    }

    e = menu_find_entry_id(menu, CLIENT_ICONIFY);
    e->data.normal.enabled = frame->client->functions & OB_CLIENT_FUNC_ICONIFY;

    e = menu_find_entry_id(menu, CLIENT_RESTORE);
    e->data.normal.enabled =frame->client->max_horz || frame->client->max_vert;

    e = menu_find_entry_id(menu, CLIENT_MAXIMIZE);
    e->data.normal.enabled =
        (frame->client->functions & OB_CLIENT_FUNC_MAXIMIZE) &&
        (!frame->client->max_horz || !frame->client->max_vert);

    e = menu_find_entry_id(menu, CLIENT_SHADE);
    e->data.normal.enabled = frame->client->functions & OB_CLIENT_FUNC_SHADE;

    e = menu_find_entry_id(menu, CLIENT_MOVE);
    e->data.normal.enabled = frame->client->functions & OB_CLIENT_FUNC_MOVE;

    e = menu_find_entry_id(menu, CLIENT_RESIZE);
    e->data.normal.enabled = frame->client->functions & OB_CLIENT_FUNC_RESIZE;

    e = menu_find_entry_id(menu, CLIENT_CLOSE);
    e->data.normal.enabled = frame->client->functions & OB_CLIENT_FUNC_CLOSE;

    e = menu_find_entry_id(menu, CLIENT_DECORATE);
    e->data.normal.enabled = client_normal(frame->client);
    return TRUE; /* show the menu */
}

static gboolean layer_update(ObMenuFrame *frame, gpointer data)
{
    ObMenu *menu = frame->menu;
    ObMenuEntry *e;
    GList *it;

    if (frame->client == NULL || !client_normal(frame->client))
        return FALSE; /* don't show the menu */

    for (it = menu->entries; it; it = g_list_next(it)) {
        e = it->data;
        if (e->type == OB_MENU_ENTRY_TYPE_NORMAL)
            e->data.normal.enabled = TRUE;
    }

    e = menu_find_entry_id(menu, LAYER_TOP);
    e->data.normal.enabled = !frame->client->above;

    e = menu_find_entry_id(menu, LAYER_NORMAL);
    e->data.normal.enabled = (frame->client->above || frame->client->below);

    e = menu_find_entry_id(menu, LAYER_BOTTOM);
    e->data.normal.enabled = !frame->client->below;
    return TRUE; /* show the menu */
}

static gboolean send_to_update(ObMenuFrame *frame, gpointer data)
{
    ObMenu *menu = frame->menu;
    guint i;
    GSList *acts;
    ObAction *act;
    ObMenuEntry *e;

    menu_clear_entries(menu);

    if (frame->client == NULL || !client_normal(frame->client))
        return FALSE; /* don't show the menu */

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
        e = menu_add_normal(menu, desk, name, acts, FALSE);
        if (desk == DESKTOP_ALL) {
            e->data.normal.mask = ob_rr_theme->desk_mask;
            e->data.normal.mask_normal_color = ob_rr_theme->menu_color;
            e->data.normal.mask_selected_color =
                ob_rr_theme->menu_selected_color;
            e->data.normal.mask_disabled_color =
                ob_rr_theme->menu_disabled_color;
            e->data.normal.mask_disabled_selected_color =
                ob_rr_theme->menu_disabled_selected_color;
        }

        if (frame->client->desktop == desk)
            e->data.normal.enabled = FALSE;
    }
    return TRUE; /* show the menu */
}

static void desktop_change_callback(ObClient *c, gpointer data)
{
    ObMenuFrame *frame = data;
    if (c == frame->client) {
        /* the client won't even be on the screen anymore, so hide the menu */
        menu_frame_hide_all();
    }
}

static void show_callback(ObMenuFrame *frame, gpointer data)
{
    client_add_desktop_notify(desktop_change_callback, frame);
}

static void hide_callback(ObMenuFrame *frame, gpointer data)
{
    client_remove_desktop_notify(desktop_change_callback);
}

static void client_menu_place(ObMenuFrame *frame, gint *x, gint *y,
                              gint button, gpointer data)
{
    gint dx, dy;

    if (button == 0 && frame->client) {
        *x = frame->client->frame->area.x;

        /* try below the titlebar */
        *y = frame->client->frame->area.y + frame->client->frame->size.top -
            frame->client->frame->bwidth;
        menu_frame_move_on_screen(frame, *x, *y, &dx, &dy);
        if (dy != 0) {
            /* try above the titlebar */
            *y = frame->client->frame->area.y + frame->client->frame->bwidth -
                frame->area.height;
            menu_frame_move_on_screen(frame, *x, *y, &dx, &dy);
        }
        if (dy != 0) {
            /* didnt fit either way, use move on screen's values */
            *y = frame->client->frame->area.y + frame->client->frame->size.top;
            menu_frame_move_on_screen(frame, *x, *y, &dx, &dy);
        }

        *x += dx;
        *y += dy;
    } else {
        gint myx, myy;

        myx = *x;
        myy = *y;

        /* try to the bottom right of the cursor */
        menu_frame_move_on_screen(frame, myx, myy, &dx, &dy);
        if (dx != 0 || dy != 0) {
            /* try to the bottom left of the cursor */
            myx = *x - frame->area.width;
            myy = *y;
            menu_frame_move_on_screen(frame, myx, myy, &dx, &dy);
        }
        if (dx != 0 || dy != 0) {
            /* try to the top right of the cursor */
            myx = *x;
            myy = *y - frame->area.height;
            menu_frame_move_on_screen(frame, myx, myy, &dx, &dy);
        }
        if (dx != 0 || dy != 0) {
            /* try to the top left of the cursor */
            myx = *x - frame->area.width;
            myy = *y - frame->area.height;
            menu_frame_move_on_screen(frame, myx, myy, &dx, &dy);
        }
        if (dx != 0 || dy != 0) {
            /* if didnt fit on either side so just use what it says */
            myx = *x;
            myy = *y;
            menu_frame_move_on_screen(frame, myx, myy, &dx, &dy);
        }
        *x = myx + dx;
        *y = myy + dy;
    }
}

void client_menu_startup()
{
    GSList *acts;
    ObMenu *menu;
    ObMenuEntry *e;

    menu = menu_new(LAYER_MENU_NAME, _("&Layer"), TRUE, NULL);
    menu_show_all_shortcuts(menu, TRUE);
    menu_set_update_func(menu, layer_update);

    acts = g_slist_prepend(NULL, action_from_string
                           ("SendToTopLayer", OB_USER_ACTION_MENU_SELECTION));
    menu_add_normal(menu, LAYER_TOP, _("Always on &top"), acts, TRUE);

    acts = g_slist_prepend(NULL, action_from_string
                           ("SendToNormalLayer",
                            OB_USER_ACTION_MENU_SELECTION));
    menu_add_normal(menu, LAYER_NORMAL, _("&Normal"), acts, TRUE);

    acts = g_slist_prepend(NULL, action_from_string
                           ("SendToBottomLayer",
                            OB_USER_ACTION_MENU_SELECTION));
    menu_add_normal(menu, LAYER_BOTTOM, _("Always on &bottom"),acts, TRUE);


    menu = menu_new(SEND_TO_MENU_NAME, _("&Send to desktop"), TRUE, NULL);
    menu_set_update_func(menu, send_to_update);
    menu_set_show_func(menu, show_callback);
    menu_set_hide_func(menu, hide_callback);


    menu = menu_new(CLIENT_MENU_NAME, _("Client menu"), TRUE, NULL);
    menu_show_all_shortcuts(menu, TRUE);
    menu_set_update_func(menu, client_update);
    menu_set_place_func(menu, client_menu_place);

    acts = g_slist_prepend(NULL, action_from_string
                           ("ToggleMaximizeFull",
                            OB_USER_ACTION_MENU_SELECTION));
    e = menu_add_normal(menu, CLIENT_RESTORE, _("R&estore"), acts, TRUE);
    e->data.normal.mask = ob_rr_theme->max_toggled_mask; 
    e->data.normal.mask_normal_color = ob_rr_theme->menu_color;
    e->data.normal.mask_selected_color = ob_rr_theme->menu_selected_color;
    e->data.normal.mask_disabled_color = ob_rr_theme->menu_disabled_color;
    e->data.normal.mask_disabled_selected_color =
        ob_rr_theme->menu_disabled_selected_color;

    acts = g_slist_prepend(NULL, action_from_string
                           ("Move", OB_USER_ACTION_MENU_SELECTION));
    menu_add_normal(menu, CLIENT_MOVE, _("&Move"), acts, TRUE);

    acts = g_slist_prepend(NULL, action_from_string
                           ("Resize", OB_USER_ACTION_MENU_SELECTION));
    menu_add_normal(menu, CLIENT_RESIZE, _("Resi&ze"), acts, TRUE);

    acts = g_slist_prepend(NULL, action_from_string
                           ("Iconify", OB_USER_ACTION_MENU_SELECTION));
    e = menu_add_normal(menu, CLIENT_ICONIFY, _("Ico&nify"), acts, TRUE);
    e->data.normal.mask = ob_rr_theme->iconify_mask;
    e->data.normal.mask_normal_color = ob_rr_theme->menu_color;
    e->data.normal.mask_selected_color = ob_rr_theme->menu_selected_color;
    e->data.normal.mask_disabled_color = ob_rr_theme->menu_disabled_color;
    e->data.normal.mask_disabled_selected_color =
        ob_rr_theme->menu_disabled_selected_color;

    acts = g_slist_prepend(NULL, action_from_string
                           ("ToggleMaximizeFull",
                            OB_USER_ACTION_MENU_SELECTION));
    e = menu_add_normal(menu, CLIENT_MAXIMIZE, _("Ma&ximize"), acts, TRUE);
    e->data.normal.mask = ob_rr_theme->max_mask; 
    e->data.normal.mask_normal_color = ob_rr_theme->menu_color;
    e->data.normal.mask_selected_color = ob_rr_theme->menu_selected_color;
    e->data.normal.mask_disabled_color = ob_rr_theme->menu_disabled_color;
    e->data.normal.mask_disabled_selected_color =
        ob_rr_theme->menu_disabled_selected_color;

    acts = g_slist_prepend(NULL, action_from_string
                           ("ToggleShade", OB_USER_ACTION_MENU_SELECTION));
    e = menu_add_normal(menu, CLIENT_SHADE, _("&Roll up/down"), acts, TRUE);
    e->data.normal.mask = ob_rr_theme->shade_mask;
    e->data.normal.mask_normal_color = ob_rr_theme->menu_color;
    e->data.normal.mask_selected_color = ob_rr_theme->menu_selected_color;
    e->data.normal.mask_disabled_color = ob_rr_theme->menu_disabled_color;
    e->data.normal.mask_disabled_selected_color =
        ob_rr_theme->menu_disabled_selected_color;

    acts = g_slist_prepend(NULL, action_from_string
                           ("ToggleDecorations",
                            OB_USER_ACTION_MENU_SELECTION));
    menu_add_normal(menu, CLIENT_DECORATE, _("Un/&Decorate"), acts, TRUE);

    menu_add_separator(menu, -1, NULL);

    menu_add_submenu(menu, CLIENT_SEND_TO, SEND_TO_MENU_NAME);

    menu_add_submenu(menu, CLIENT_LAYER, LAYER_MENU_NAME);

    menu_add_separator(menu, -1, NULL);

    acts = g_slist_prepend(NULL, action_from_string
                           ("Close", OB_USER_ACTION_MENU_SELECTION));
    e = menu_add_normal(menu, CLIENT_CLOSE, _("&Close"), acts, TRUE);
    e->data.normal.mask = ob_rr_theme->close_mask;
    e->data.normal.mask_normal_color = ob_rr_theme->menu_color;
    e->data.normal.mask_selected_color = ob_rr_theme->menu_selected_color;
    e->data.normal.mask_disabled_color = ob_rr_theme->menu_disabled_color;
    e->data.normal.mask_disabled_selected_color =
        ob_rr_theme->menu_disabled_selected_color;
}
