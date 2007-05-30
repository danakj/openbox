/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   menu.c for the Openbox window manager
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

#include "debug.h"
#include "menu.h"
#include "openbox.h"
#include "stacking.h"
#include "client.h"
#include "config.h"
#include "screen.h"
#include "menuframe.h"
#include "keyboard.h"
#include "geom.h"
#include "misc.h"
#include "client_menu.h"
#include "client_list_menu.h"
#include "client_list_combined_menu.h"
#include "gettext.h"
#include "parser/parse.h"

typedef struct _ObMenuParseState ObMenuParseState;

struct _ObMenuParseState
{
    ObMenu *parent;
    ObMenu *pipe_creator;
};

static GHashTable *menu_hash = NULL;
static ObParseInst *menu_parse_inst;
static ObMenuParseState menu_parse_state;

static void menu_destroy_hash_value(ObMenu *self);
static void parse_menu_item(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                            gpointer data);
static void parse_menu_separator(ObParseInst *i,
                                 xmlDocPtr doc, xmlNodePtr node,
                                 gpointer data);
static void parse_menu(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                       gpointer data);
static gunichar parse_shortcut(const gchar *label, gboolean allow_shortcut,
                               gchar **strippedlabel, guint *position);


static void client_dest(ObClient *client, gpointer data)
{
    /* menus can be associated with a client, so close any that are since
       we are disappearing now */
    menu_frame_hide_all_client(client);
}

void menu_startup(gboolean reconfig)
{
    xmlDocPtr doc;
    xmlNodePtr node;
    gboolean loaded = FALSE;
    GSList *it;

    menu_hash = g_hash_table_new_full(g_str_hash, g_str_equal, NULL,
                                      (GDestroyNotify)menu_destroy_hash_value);

    client_list_menu_startup(reconfig);
    client_list_combined_menu_startup(reconfig);
    client_menu_startup();

    menu_parse_inst = parse_startup();

    menu_parse_state.parent = NULL;
    menu_parse_state.pipe_creator = NULL;
    parse_register(menu_parse_inst, "menu", parse_menu, &menu_parse_state);
    parse_register(menu_parse_inst, "item", parse_menu_item,
                   &menu_parse_state);
    parse_register(menu_parse_inst, "separator",
                   parse_menu_separator, &menu_parse_state);

    for (it = config_menu_files; it; it = g_slist_next(it)) {
        if (parse_load_menu(it->data, &doc, &node)) {
            loaded = TRUE;
            parse_tree(menu_parse_inst, doc, node->children);
            xmlFreeDoc(doc);
        } else
            g_message(_("Unable to find a valid menu file '%s'"),
                      (const gchar*)it->data);
    }
    if (!loaded) {
        if (parse_load_menu("menu.xml", &doc, &node)) {
            parse_tree(menu_parse_inst, doc, node->children);
            xmlFreeDoc(doc);
        } else
            g_message(_("Unable to find a valid menu file '%s'"),
                      "menu.xml");
    }
    
    g_assert(menu_parse_state.parent == NULL);

    if (!reconfig)
        client_add_destroy_notify(client_dest, NULL);
}

void menu_shutdown(gboolean reconfig)
{
    if (!reconfig)
        client_remove_destroy_notify(client_dest);

    parse_shutdown(menu_parse_inst);
    menu_parse_inst = NULL;

    client_list_menu_shutdown(reconfig);
    client_list_combined_menu_shutdown(reconfig);

    menu_frame_hide_all();
    g_hash_table_destroy(menu_hash);
    menu_hash = NULL;
}

static gboolean menu_pipe_submenu(gpointer key, gpointer val, gpointer data)
{
    ObMenu *menu = val;
    return menu->pipe_creator == data;
}

void menu_pipe_execute(ObMenu *self)
{
    xmlDocPtr doc;
    xmlNodePtr node;
    gchar *output;
    GError *err = NULL;

    if (!self->execute)
        return;

    if (!g_spawn_command_line_sync(self->execute, &output, NULL, NULL, &err)) {
        g_message(_("Failed to execute command for pipe-menu '%s': %s"),
                  self->execute, err->message);
        g_error_free(err);
        return;
    }

    if (parse_load_mem(output, strlen(output),
                       "openbox_pipe_menu", &doc, &node))
    {
        g_hash_table_foreach_remove(menu_hash, menu_pipe_submenu, self);
        menu_clear_entries(self);

        menu_parse_state.pipe_creator = self;
        menu_parse_state.parent = self;
        parse_tree(menu_parse_inst, doc, node->children);
        xmlFreeDoc(doc);
    } else {
        g_message(_("Invalid output from pipe-menu '%s'"), self->execute);
    }

    g_free(output);
}

static ObMenu* menu_from_name(gchar *name)
{
    ObMenu *self = NULL;

    g_assert(name != NULL);

    if (!(self = g_hash_table_lookup(menu_hash, name)))
        g_message(_("Attempted to access menu '%s' but it does not exist"),
                  name);
    return self;
}  

#define VALID_SHORTCUT(c) (((c) >= '0' && (c) <= '9') || \
                           ((c) >= 'A' && (c) <= 'Z') || \
                           ((c) >= 'a' && (c) <= 'z'))

static gunichar parse_shortcut(const gchar *label, gboolean allow_shortcut,
                               gchar **strippedlabel, guint *position)
{
    gunichar shortcut = 0;
    
    *position = 0;

    g_assert(strippedlabel != NULL);

    if (label == NULL) {
        *strippedlabel = NULL;
    } else {
        gchar *i;

        *strippedlabel = g_strdup(label);

        /* if allow_shortcut is false, then you can't use the &, instead you
           have to just use the first valid character
        */

        i = strchr(*strippedlabel, '&');
        if (allow_shortcut && i != NULL) {
            /* there is an ampersand in the string */

            /* you have to use a printable ascii character for shortcuts
               don't allow space either, so you can have like "a & b"
            */
            if (VALID_SHORTCUT(*(i+1))) {
                shortcut = g_unichar_tolower(g_utf8_get_char(i+1));
                *position = i - *strippedlabel;

                /* remove the & from the string */
                for (; *i != '\0'; ++i)
                    *i = *(i+1);
            }
        } else {
            /* there is no ampersand, so find the first valid character to use
               instead */

            for (i = *strippedlabel; *i != '\0'; ++i)
                if (VALID_SHORTCUT(*i)) {
                    *position = i - *strippedlabel;
                    shortcut = g_unichar_tolower(g_utf8_get_char(i));
                    break;
                }
        }
    }
    return shortcut;
}

static void parse_menu_item(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                            gpointer data)
{
    ObMenuParseState *state = data;
    gchar *label;
    
    if (state->parent) {
        if (parse_attr_string("label", node, &label)) {
            GSList *acts = NULL;

            for (node = node->children; node; node = node->next)
                if (!xmlStrcasecmp(node->name, (const xmlChar*) "action")) {
                    ObAction *a = action_parse
                        (i, doc, node, OB_USER_ACTION_MENU_SELECTION);
                    if (a)
                        acts = g_slist_append(acts, a);
                }
            menu_add_normal(state->parent, -1, label, acts, FALSE);
            g_free(label);
        }
    }
}

static void parse_menu_separator(ObParseInst *i,
                                 xmlDocPtr doc, xmlNodePtr node,
                                 gpointer data)
{
    ObMenuParseState *state = data;

    if (state->parent) {
        gchar *label;

        if (!parse_attr_string("label", node, &label))
            label = NULL;

        menu_add_separator(state->parent, -1, label);
        g_free(label);
    }
}

static void parse_menu(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                       gpointer data)
{
    ObMenuParseState *state = data;
    gchar *name = NULL, *title = NULL, *script = NULL;
    ObMenu *menu;

    if (!parse_attr_string("id", node, &name))
        goto parse_menu_fail;

    if (!g_hash_table_lookup(menu_hash, name)) {
        if (!parse_attr_string("label", node, &title))
            goto parse_menu_fail;

        if ((menu = menu_new(name, title, FALSE, NULL))) {
            menu->pipe_creator = state->pipe_creator;
            if (parse_attr_string("execute", node, &script)) {
                menu->execute = parse_expand_tilde(script);
            } else {
                ObMenu *old;

                old = state->parent;
                state->parent = menu;
                parse_tree(i, doc, node->children);
                state->parent = old;
            }
        }
    }

    if (state->parent)
        menu_add_submenu(state->parent, -1, name);

parse_menu_fail:
    g_free(name);
    g_free(title);
    g_free(script);
}

ObMenu* menu_new(const gchar *name, const gchar *title,
                 gboolean allow_shortcut_selection, gpointer data)
{
    ObMenu *self;

    self = g_new0(ObMenu, 1);
    self->name = g_strdup(name);
    self->data = data;

    self->shortcut = parse_shortcut(title, allow_shortcut_selection,
                                    &self->title, &self->shortcut_position);

    g_hash_table_replace(menu_hash, self->name, self);

    /* Each menu has a single more_menu.  When the menu spills past what
       can fit on the screen, a new menu frame entry is created from this
       more_menu, and a new menu frame for the submenu is created for this
       menu, also pointing to the more_menu.

       This can be done multiple times using the same more_menu.

       more_menu->more_menu will always be NULL, since there is only 1 for
       each menu. */
    self->more_menu = g_new0(ObMenu, 1);
    self->more_menu->name = _("More...");
    self->more_menu->title = _("More...");
    self->more_menu->data = data;
    self->more_menu->shortcut = g_unichar_tolower(g_utf8_get_char("M"));

    self->more_menu->show_func = self->show_func;
    self->more_menu->hide_func = self->hide_func;
    self->more_menu->update_func = self->update_func;
    self->more_menu->execute_func = self->execute_func;
    self->more_menu->destroy_func = self->destroy_func;
    self->more_menu->place_func = self->place_func;

    return self;
}

static void menu_destroy_hash_value(ObMenu *self)
{
    /* make sure its not visible */
    {
        GList *it;
        ObMenuFrame *f;

        for (it = menu_frame_visible; it; it = g_list_next(it)) {
            f = it->data;
            if (f->menu == self)
                menu_frame_hide_all();
        }
    }

    if (self->destroy_func)
        self->destroy_func(self, self->data);

    menu_clear_entries(self);
    g_free(self->name);
    g_free(self->title);
    g_free(self->execute);
    g_free(self->more_menu);

    g_free(self);
}

void menu_free(ObMenu *menu)
{
    if (menu)
        g_hash_table_remove(menu_hash, menu->name);
}

void menu_show(gchar *name, gint x, gint y, gint button, ObClient *client)
{
    ObMenu *self;
    ObMenuFrame *frame;

    if (!(self = menu_from_name(name))
        || keyboard_interactively_grabbed()) return;

    /* if the requested menu is already the top visible menu, then don't
       bother */
    if (menu_frame_visible) {
        frame = menu_frame_visible->data;
        if (frame->menu == self)
            return;
    }

    menu_frame_hide_all();

    frame = menu_frame_new(self, 0, client);
    if (!menu_frame_show_topmenu(frame, x, y, button))
        menu_frame_free(frame);
    else if (!button) {
        /* select the first entry if it's not a submenu and we opened
         * the menu with the keyboard, and skip all headers */
        GList *it = frame->entries;
        while (it) {
            ObMenuEntryFrame *e = it->data;
            if (e->entry->type == OB_MENU_ENTRY_TYPE_NORMAL) {
                menu_frame_select(frame, e, FALSE);
                break;
            } else if (e->entry->type == OB_MENU_ENTRY_TYPE_SEPARATOR)
                it = g_list_next(it);
            else
                break;
        }
    }
}

static ObMenuEntry* menu_entry_new(ObMenu *menu, ObMenuEntryType type, gint id)
{
    ObMenuEntry *self;

    g_assert(menu);

    self = g_new0(ObMenuEntry, 1);
    self->ref = 1;
    self->type = type;
    self->menu = menu;
    self->id = id;

    switch (type) {
    case OB_MENU_ENTRY_TYPE_NORMAL:
        self->data.normal.enabled = TRUE;
        break;
    case OB_MENU_ENTRY_TYPE_SUBMENU:
    case OB_MENU_ENTRY_TYPE_SEPARATOR:
        break;
    }

    return self;
}

void menu_entry_ref(ObMenuEntry *self)
{
    ++self->ref;
}

void menu_entry_unref(ObMenuEntry *self)
{
    if (self && --self->ref == 0) {
        switch (self->type) {
        case OB_MENU_ENTRY_TYPE_NORMAL:
            g_free(self->data.normal.label);
            while (self->data.normal.actions) {
                action_unref(self->data.normal.actions->data);
                self->data.normal.actions =
                    g_slist_delete_link(self->data.normal.actions,
                                        self->data.normal.actions);
            }
            break;
        case OB_MENU_ENTRY_TYPE_SUBMENU:
            g_free(self->data.submenu.name);
            break;
        case OB_MENU_ENTRY_TYPE_SEPARATOR:
            break;
        }

        g_free(self);
    }
}

void menu_clear_entries(ObMenu *self)
{
#ifdef DEBUG
    /* assert that the menu isn't visible */
    {
        GList *it;
        ObMenuFrame *f;

        for (it = menu_frame_visible; it; it = g_list_next(it)) {
            f = it->data;
            g_assert(f->menu != self);
        }
    }
#endif

    while (self->entries) {
        menu_entry_unref(self->entries->data);
        self->entries = g_list_delete_link(self->entries, self->entries);
    }
    self->more_menu->entries = self->entries; /* keep it in sync */
}

void menu_entry_remove(ObMenuEntry *self)
{
    self->menu->entries = g_list_remove(self->menu->entries, self);
    menu_entry_unref(self);
}

ObMenuEntry* menu_add_normal(ObMenu *self, gint id, const gchar *label,
                             GSList *actions, gboolean allow_shortcut)
{
    ObMenuEntry *e;

    e = menu_entry_new(self, OB_MENU_ENTRY_TYPE_NORMAL, id);
    e->data.normal.actions = actions;

    menu_entry_set_label(e, label, allow_shortcut);

    self->entries = g_list_append(self->entries, e);
    self->more_menu->entries = self->entries; /* keep it in sync */
    return e;
}

ObMenuEntry* menu_get_more(ObMenu *self, guint show_from)
{
    ObMenuEntry *e;
    e = menu_entry_new(self, OB_MENU_ENTRY_TYPE_SUBMENU, -1);
    /* points to itself */
    e->data.submenu.name = g_strdup(self->name);
    e->data.submenu.submenu = self;
    e->data.submenu.show_from = show_from;
    return e;
}

ObMenuEntry* menu_add_submenu(ObMenu *self, gint id, const gchar *submenu)
{
    ObMenuEntry *e;

    e = menu_entry_new(self, OB_MENU_ENTRY_TYPE_SUBMENU, id);
    e->data.submenu.name = g_strdup(submenu);

    self->entries = g_list_append(self->entries, e);
    self->more_menu->entries = self->entries; /* keep it in sync */
    return e;
}

ObMenuEntry* menu_add_separator(ObMenu *self, gint id, const gchar *label)
{
    ObMenuEntry *e;

    e = menu_entry_new(self, OB_MENU_ENTRY_TYPE_SEPARATOR, id);

    menu_entry_set_label(e, label, FALSE);

    self->entries = g_list_append(self->entries, e);
    self->more_menu->entries = self->entries; /* keep it in sync */
    return e;
}

void menu_set_show_func(ObMenu *self, ObMenuShowFunc func)
{
    self->show_func = func;
    self->more_menu->show_func = func; /* keep it in sync */
}

void menu_set_hide_func(ObMenu *self, ObMenuHideFunc func)
{
    self->hide_func = func;
    self->more_menu->hide_func = func; /* keep it in sync */
}

void menu_set_update_func(ObMenu *self, ObMenuUpdateFunc func)
{
    self->update_func = func;
    self->more_menu->update_func = func; /* keep it in sync */
}

void menu_set_execute_func(ObMenu *self, ObMenuExecuteFunc func)
{
    self->execute_func = func;
    self->more_menu->execute_func = func; /* keep it in sync */
}

void menu_set_destroy_func(ObMenu *self, ObMenuDestroyFunc func)
{
    self->destroy_func = func;
    self->more_menu->destroy_func = func; /* keep it in sync */
}

void menu_set_place_func(ObMenu *self, ObMenuPlaceFunc func)
{
    self->place_func = func;
    self->more_menu->place_func = func; /* keep it in sync */
}

ObMenuEntry* menu_find_entry_id(ObMenu *self, gint id)
{
    ObMenuEntry *ret = NULL;
    GList *it;

    for (it = self->entries; it; it = g_list_next(it)) {
        ObMenuEntry *e = it->data;

        if (e->id == id) {
            ret = e;
            break;
        }
    }
    return ret;
}

void menu_find_submenus(ObMenu *self)
{
    GList *it;

    for (it = self->entries; it; it = g_list_next(it)) {
        ObMenuEntry *e = it->data;

        if (e->type == OB_MENU_ENTRY_TYPE_SUBMENU)
            e->data.submenu.submenu = menu_from_name(e->data.submenu.name);
    }
}

void menu_entry_set_label(ObMenuEntry *self, const gchar *label,
                          gboolean allow_shortcut)
{
    switch (self->type) {
    case OB_MENU_ENTRY_TYPE_SEPARATOR:
        g_free(self->data.separator.label);
        self->data.separator.label = g_strdup(label);
        break;
    case OB_MENU_ENTRY_TYPE_NORMAL:
        g_free(self->data.normal.label);
        self->data.normal.shortcut =
            parse_shortcut(label, allow_shortcut, &self->data.normal.label,
                           &self->data.normal.shortcut_position);
        break;
    default:
        g_assert_not_reached();
    }
}

void menu_show_all_shortcuts(ObMenu *self, gboolean show)
{
    self->show_all_shortcuts = show;
}
