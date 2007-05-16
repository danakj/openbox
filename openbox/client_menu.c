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
#include "moveresize.h"
#include "prop.h"
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

static gboolean client_menu_update(ObMenuFrame *frame, gpointer data)
{
    ObMenu *menu = frame->menu;
    GList *it;

    if (frame->client == NULL || !client_normal(frame->client))
        return FALSE; /* don't show the menu */

    for (it = menu->entries; it; it = g_list_next(it)) {
        ObMenuEntry *e = it->data;
        gboolean *en = &e->data.normal.enabled; /* save some typing */
        ObClient *c = frame->client;

        if (e->type == OB_MENU_ENTRY_TYPE_NORMAL) {
            switch (e->id) {
            case CLIENT_ICONIFY:
                *en = c->functions & OB_CLIENT_FUNC_ICONIFY;
                break;
            case CLIENT_RESTORE:
                *en = c->max_horz || c->max_vert;
                break;
            case CLIENT_MAXIMIZE:
                *en = ((c->functions & OB_CLIENT_FUNC_MAXIMIZE) &&
                       (!c->max_horz || !c->max_vert));
                break;
            case CLIENT_SHADE:
                *en = c->functions & OB_CLIENT_FUNC_SHADE;
                break;
            case CLIENT_MOVE:
                *en = c->functions & OB_CLIENT_FUNC_MOVE;
                break;
            case CLIENT_RESIZE:
                *en = c->functions & OB_CLIENT_FUNC_RESIZE;
                break;
            case CLIENT_CLOSE:
                *en = c->functions & OB_CLIENT_FUNC_CLOSE;
                break;
            case CLIENT_DECORATE:
                *en = client_normal(c);
                break;
            default:
                *en = TRUE;
            }
        }
    }
    return TRUE; /* show the menu */
}

static void client_menu_execute(ObMenuEntry *e, ObMenuFrame *f,
                                ObClient *c, guint state, gpointer data,
                                Time time)
{
    gint x, y;

    g_assert(c);

    switch (e->id) {
    case CLIENT_ICONIFY:
        /* the client won't be on screen anymore so hide the menu */
        menu_frame_hide_all();
        f = NULL; /* and don't update */

        client_iconify(c, TRUE, FALSE, FALSE);
        break;
    case CLIENT_RESTORE:
        client_maximize(c, FALSE, 0);
        break;
    case CLIENT_MAXIMIZE:
        client_maximize(c, TRUE, 0);
        break;
    case CLIENT_SHADE:
        client_shade(c, !c->shaded);
        break;
    case CLIENT_DECORATE:
        client_set_undecorated(c, !c->undecorated);
        break;
    case CLIENT_MOVE:
        /* this needs to grab the keyboard so hide the menu */
        menu_frame_hide_all();
        f = NULL; /* and don't update */

        screen_pointer_pos(&x, &y);
        moveresize_start(c, x, y, 0,
                         prop_atoms.net_wm_moveresize_move_keyboard);
        break;
    case CLIENT_RESIZE:
        /* this needs to grab the keyboard so hide the menu */
        menu_frame_hide_all();
        f = NULL; /* and don't update */

        screen_pointer_pos(&x, &y);
        moveresize_start(c, x, y, 0,
                         prop_atoms.net_wm_moveresize_size_keyboard);
        break;
    case CLIENT_CLOSE:
        client_close(c);
        break;
    default:
        g_assert_not_reached();
    }

    /* update the menu cuz stuff can have changed */
    if (f) {
        client_menu_update(f, NULL);
        menu_frame_render(f);
    }
}

static gboolean layer_menu_update(ObMenuFrame *frame, gpointer data)
{
    ObMenu *menu = frame->menu;
    GList *it;

    if (frame->client == NULL || !client_normal(frame->client))
        return FALSE; /* don't show the menu */

    for (it = menu->entries; it; it = g_list_next(it)) {
        ObMenuEntry *e = it->data;
        gboolean *en = &e->data.normal.enabled; /* save some typing */
        ObClient *c = frame->client;

        if (e->type == OB_MENU_ENTRY_TYPE_NORMAL) {
            switch (e->id) {
            case LAYER_TOP:
                *en = !c->above && (c->functions & OB_CLIENT_FUNC_ABOVE);
                break;
            case LAYER_NORMAL:
                *en = c->above || c->below;
                break;
            case LAYER_BOTTOM:
                *en = !c->below && (c->functions & OB_CLIENT_FUNC_BELOW);
                break;
            default:
                *en = TRUE;
            }
        }
    }
    return TRUE; /* show the menu */
}

static void layer_menu_execute(ObMenuEntry *e, ObMenuFrame *f,
                               ObClient *c, guint state, gpointer data,
                               Time time)
{
    g_assert(c);

    switch (e->id) {
    case LAYER_TOP:
        client_set_layer(c, 1);
        break;
    case LAYER_NORMAL:
        client_set_layer(c, 0);
        break;
    case LAYER_BOTTOM:
        client_set_layer(c, -1);
        break;
    default:
        g_assert_not_reached();
    }

    /* update the menu cuz stuff can have changed */
    if (f) {
        layer_menu_update(f, NULL);
        menu_frame_render(f);
    }
}

static gboolean send_to_menu_update(ObMenuFrame *frame, gpointer data)
{
    ObMenu *menu = frame->menu;
    guint i;
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

        e = menu_add_normal(menu, desk, name, NULL, FALSE);
        e->id = desk;
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

static void send_to_menu_execute(ObMenuEntry *e, ObMenuFrame *f,
                                 ObClient *c, guint state, gpointer data,
                                 Time time)
{
    g_assert(c);

    client_set_desktop(c, e->id, FALSE);
    /* the client won't even be on the screen anymore, so hide the menu */
    if (f)
        menu_frame_hide_all();
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
    ObMenu *menu;
    ObMenuEntry *e;

    menu = menu_new(LAYER_MENU_NAME, _("&Layer"), TRUE, NULL);
    menu_show_all_shortcuts(menu, TRUE);
    menu_set_update_func(menu, layer_menu_update);
    menu_set_execute_func(menu, layer_menu_execute);

    menu_add_normal(menu, LAYER_TOP, _("Always on &top"), NULL, TRUE);
    menu_add_normal(menu, LAYER_NORMAL, _("&Normal"), NULL, TRUE);
    menu_add_normal(menu, LAYER_BOTTOM, _("Always on &bottom"),NULL, TRUE);


    menu = menu_new(SEND_TO_MENU_NAME, _("&Send to desktop"), TRUE, NULL);
    menu_set_update_func(menu, send_to_menu_update);
    menu_set_execute_func(menu, send_to_menu_execute);

    menu = menu_new(CLIENT_MENU_NAME, _("Client menu"), TRUE, NULL);
    menu_show_all_shortcuts(menu, TRUE);
    menu_set_update_func(menu, client_menu_update);
    menu_set_place_func(menu, client_menu_place);
    menu_set_execute_func(menu, client_menu_execute);

    e = menu_add_normal(menu, CLIENT_RESTORE, _("R&estore"), NULL, TRUE);
    e->data.normal.mask = ob_rr_theme->max_toggled_mask; 
    e->data.normal.mask_normal_color = ob_rr_theme->menu_color;
    e->data.normal.mask_selected_color = ob_rr_theme->menu_selected_color;
    e->data.normal.mask_disabled_color = ob_rr_theme->menu_disabled_color;
    e->data.normal.mask_disabled_selected_color =
        ob_rr_theme->menu_disabled_selected_color;

    menu_add_normal(menu, CLIENT_MOVE, _("&Move"), NULL, TRUE);

    menu_add_normal(menu, CLIENT_RESIZE, _("Resi&ze"), NULL, TRUE);

    e = menu_add_normal(menu, CLIENT_ICONIFY, _("Ico&nify"), NULL, TRUE);
    e->data.normal.mask = ob_rr_theme->iconify_mask;
    e->data.normal.mask_normal_color = ob_rr_theme->menu_color;
    e->data.normal.mask_selected_color = ob_rr_theme->menu_selected_color;
    e->data.normal.mask_disabled_color = ob_rr_theme->menu_disabled_color;
    e->data.normal.mask_disabled_selected_color =
        ob_rr_theme->menu_disabled_selected_color;

    e = menu_add_normal(menu, CLIENT_MAXIMIZE, _("Ma&ximize"), NULL, TRUE);
    e->data.normal.mask = ob_rr_theme->max_mask; 
    e->data.normal.mask_normal_color = ob_rr_theme->menu_color;
    e->data.normal.mask_selected_color = ob_rr_theme->menu_selected_color;
    e->data.normal.mask_disabled_color = ob_rr_theme->menu_disabled_color;
    e->data.normal.mask_disabled_selected_color =
        ob_rr_theme->menu_disabled_selected_color;

    menu_add_normal(menu, CLIENT_SHADE, _("&Roll up/down"), NULL, TRUE);

    menu_add_normal(menu, CLIENT_DECORATE, _("Un/&Decorate"), NULL, TRUE);

    menu_add_separator(menu, -1, NULL);

    menu_add_submenu(menu, CLIENT_SEND_TO, SEND_TO_MENU_NAME);

    menu_add_submenu(menu, CLIENT_LAYER, LAYER_MENU_NAME);

    menu_add_separator(menu, -1, NULL);

    e = menu_add_normal(menu, CLIENT_CLOSE, _("&Close"), NULL, TRUE);
    e->data.normal.mask = ob_rr_theme->close_mask;
    e->data.normal.mask_normal_color = ob_rr_theme->menu_color;
    e->data.normal.mask_selected_color = ob_rr_theme->menu_selected_color;
    e->data.normal.mask_disabled_color = ob_rr_theme->menu_disabled_color;
    e->data.normal.mask_disabled_selected_color =
        ob_rr_theme->menu_disabled_selected_color;
}
