#include <glib.h>
#include "menu.h"

Menu *menu_new(char *label, Menu *parent)
{
    Menu *new_menu = g_new(Menu, 1);
    new_menu->label = g_strdup(lable);
    new_menu->parent = parent;

    new_menu->entries = NULL;
    new_menu->tail = NULL;
    new_menu->shown = FALSE;
    new_menu->invalid = FALSE;
    /* default controllers? */

    return new_menu;
}

MenuEntry *menu_entry_new_full(char *label, Action *action,
                          MenuEntryRenderType render_type,
                          gpointer render_data, gpointer submenu)
{
    MenuEntry *menu_entry = g_new(MenuEntry, 1);

    menu_entry->label = g_strdup(label);
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
