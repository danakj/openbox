/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   menu.h for the Openbox window manager
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

#ifndef __menu_h
#define __menu_h

#include "action.h"
#include "window.h"
#include "geom.h"
#include "render/render.h"
#include "parser/parse.h"

#include <glib.h>

struct _ObClient;
struct _ObMenuFrame;
struct _ObMenuEntryFrame;

typedef struct _ObMenu ObMenu;
typedef struct _ObMenuEntry ObMenuEntry;
typedef struct _ObNormalMenuEntry ObNormalMenuEntry;
typedef struct _ObSubmenuMenuEntry ObSubmenuMenuEntry;
typedef struct _ObSeparatorMenuEntry ObSeparatorMenuEntry;

typedef void (*ObMenuUpdateFunc)(struct _ObMenuFrame *frame, gpointer data);
typedef void (*ObMenuExecuteFunc)(struct _ObMenuEntry *entry,
                                  guint state, gpointer data);
typedef void (*ObMenuDestroyFunc)(struct _ObMenu *menu, gpointer data);

struct _ObMenu
{
    /* Name of the menu. Used in the showmenu action. */
    gchar *name;
    /* Displayed title */
    gchar *title;

    /* Command to execute to rebuild the menu */
    gchar *execute;

    /* ObMenuEntry list */
    GList *entries;

    /* plugin data */
    gpointer data;

    ObMenuUpdateFunc update_func;
    ObMenuExecuteFunc execute_func;
    ObMenuDestroyFunc destroy_func;

    /* Pipe-menu parent, we get destroyed when it is destroyed */
    ObMenu *pipe_creator;
};

typedef enum
{
    OB_MENU_ENTRY_TYPE_NORMAL,
    OB_MENU_ENTRY_TYPE_SUBMENU,
    OB_MENU_ENTRY_TYPE_SEPARATOR
} ObMenuEntryType;

struct _ObNormalMenuEntry {
    gchar *label;

    /* state */
    gboolean enabled;

    /* List of ObActions */
    GSList *actions;

    /* Icon shit */
    gint icon_width;
    gint icon_height;
    RrPixel32 *icon_data;

    /* Mask icon */
    RrPixmapMask *mask;
    RrColor *mask_normal_color;
    RrColor *mask_disabled_color;
    RrColor *mask_selected_color;
};

struct _ObSubmenuMenuEntry {
    gchar *name;
    ObMenu *submenu;
};

struct _ObSeparatorMenuEntry {
    gchar foo; /* placeholder */
};

struct _ObMenuEntry
{
    ObMenuEntryType type;
    ObMenu *menu;

    gint id;

    union u {
        ObNormalMenuEntry normal;
        ObSubmenuMenuEntry submenu;
        ObSeparatorMenuEntry separator;
    } data;
};

void menu_startup(gboolean reconfig);
void menu_shutdown(gboolean reconfig);

ObMenu* menu_new(gchar *name, gchar *title, gpointer data);
void menu_free(ObMenu *menu);

/* Repopulate a pipe-menu by running its command */
void menu_pipe_execute(ObMenu *self);

void menu_show(gchar *name, gint x, gint y, struct _ObClient *client);

void menu_set_update_func(ObMenu *menu, ObMenuUpdateFunc func);
void menu_set_execute_func(ObMenu *menu, ObMenuExecuteFunc func);
void menu_set_destroy_func(ObMenu *menu, ObMenuDestroyFunc func);

/* functions for building menus */
ObMenuEntry* menu_add_normal(ObMenu *menu, gint id, gchar *label,
                             GSList *actions);
ObMenuEntry* menu_add_submenu(ObMenu *menu, gint id, gchar *submenu);
ObMenuEntry* menu_add_separator(ObMenu *menu, gint id);

void menu_clear_entries(ObMenu *menu);
void menu_entry_remove(ObMenuEntry *self);

ObMenuEntry* menu_find_entry_id(ObMenu *self, gint id);

/* fills in the submenus, for use when a menu is being shown */
void menu_find_submenus(ObMenu *self);

#endif
