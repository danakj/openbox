#include <glib.h>
#include "menu.h"
#include <assert.h>

GHashTable *menu_hash = NULL;

void menu_destroy_hash_key(const gpointer data)
{
    g_free(data);
}

void menu_free_entries(const Menu *menu)
{
    GList *it;

    for (it = menu->entries; it; it = it->next)
        menu_entry_free((MenuEntry *)it->data);

    g_list_free(menu->entries);
}

void menu_destroy_hash_value(const gpointer data)
{
    const Menu *del_menu = (Menu *)data;

    g_free(del_menu->label);
    g_free(del_menu->name);

    menu_free_entries(del_menu);
}

void menu_entry_free(const MenuEntry *entry)
{
    g_free(entry->label);
    g_free(entry->render_data);
}
    
void menu_startup()
{
    menu_hash = g_hash_table_new_full(g_str_hash, g_str_equal,
                                      menu_destroy_hash_key,
                                      menu_destroy_hash_value);
}

void menu_shutdown()
{
    g_hash_table_destroy(menu_hash);
}

Menu *menu_new(const char *label, const char *name, Menu *parent)
{
    Menu *new_menu = g_new0(Menu, 1);
    new_menu->label = g_strdup(label);
    new_menu->name = g_strdup(name);
    new_menu->parent = parent;

    new_menu->entries = NULL;
    new_menu->shown = FALSE;
    new_menu->invalid = FALSE;
    /* default controllers? */

    g_hash_table_insert(menu_hash, g_strdup(name), new_menu);
    return new_menu;
}

void menu_free(const char *name)
{
    g_hash_table_remove(menu_hash, name);
}

MenuEntry *menu_entry_new_full(const char *label, Action *action,
                          const MenuEntryRenderType render_type,
                          gpointer render_data, gpointer submenu)
{
    MenuEntry *menu_entry = g_new(MenuEntry, 1);

    menu_entry->label = g_strdup(label);
    menu_entry->render_type = render_type;
    menu_entry->action.func = action->func;
    menu_entry->action.data = action->data; //watch out. copying Client * ptr

    menu_entry->render_data = render_data; //watch out.
    menu_entry->submenu = submenu;

    return menu_entry;
}

void menu_entry_set_submenu(MenuEntry *entry, Menu *submenu)
{
    assert(entry != NULL);
    
    entry->submenu = submenu;

    if(entry->parent != NULL)
        entry->parent->invalid = TRUE;
}

void menu_add_entry(Menu *menu, MenuEntry *entry)
{
    assert(menu != NULL && entry != NULL);

    menu->entries = g_list_append(menu->entries, entry);
    entry->parent = menu;

    menu->invalid = TRUE;
}
