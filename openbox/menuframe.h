/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   menuframe.h for the Openbox window manager
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

#ifndef ob__menuframe_h
#define ob__menuframe_h

#include "geom.h"
#include "window.h"
#include "render/render.h"

#include <glib.h>

struct _ObClient;
struct _ObMenu;
struct _ObMenuEntry;

typedef struct _ObMenuFrame ObMenuFrame;
typedef struct _ObMenuEntryFrame ObMenuEntryFrame;

extern GList *menu_frame_visible;

struct _ObMenuFrame
{
    /* stuff to be an ObWindow */
    Window_InternalType type;
    Window window;

    struct _ObMenu *menu;

    /* The client that the visual instance of the menu is associated with for
       its actions */
    struct _ObClient *client;

    ObMenuFrame *parent;
    ObMenuFrame *child;

    GList *entries;
    ObMenuEntryFrame *selected;

    /* If a titlebar is displayed for the menu or not (for top-level menus) */
    gboolean show_title;

    /* On-screen area (including borders!) */
    Rect area;
    Strut item_margin;
    gint inner_w; /* inside the borders */
    gint title_h; /* includes the bwidth below it */
    gint item_h;  /* height of all normal items */
    gint text_x;  /* offset at which the text appears in the items */
    gint text_w;  /* width of the text area in the items */

    gint monitor; /* monitor on which to show the menu in xinerama */

    Window title;
    Window items;

    RrAppearance *a_title;
    RrAppearance *a_items;
};

struct _ObMenuEntryFrame
{
    struct _ObMenuEntry *entry;
    ObMenuFrame *frame;

    Rect area;

    Window window;
    Window icon;
    Window text;
    Window bullet;

    RrAppearance *a_normal;
    RrAppearance *a_disabled;
    RrAppearance *a_selected;

    RrAppearance *a_icon;
    RrAppearance *a_mask;
    RrAppearance *a_bullet_normal;
    RrAppearance *a_bullet_selected;
    RrAppearance *a_separator;
    RrAppearance *a_text_normal;
    RrAppearance *a_text_disabled;
    RrAppearance *a_text_selected;
};

ObMenuFrame* menu_frame_new(struct _ObMenu *menu, struct _ObClient *client);
void menu_frame_free(ObMenuFrame *self);

void menu_frame_move(ObMenuFrame *self, gint x, gint y);
void menu_frame_move_on_screen(ObMenuFrame *self);

gboolean menu_frame_show(ObMenuFrame *self, ObMenuFrame *parent);
void menu_frame_hide(ObMenuFrame *self);

void menu_frame_hide_all();
void menu_frame_hide_all_client(struct _ObClient *client);

void menu_frame_select(ObMenuFrame *self, ObMenuEntryFrame *entry);
void menu_frame_select_previous(ObMenuFrame *self);
void menu_frame_select_next(ObMenuFrame *self);

ObMenuFrame* menu_frame_under(gint x, gint y);
ObMenuEntryFrame* menu_entry_frame_under(gint x, gint y);

void menu_entry_frame_show_submenu(ObMenuEntryFrame *self);

void menu_entry_frame_execute(ObMenuEntryFrame *self, guint state);

#endif
