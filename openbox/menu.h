#ifndef __menu_h
#define __menu_h

#include "action.h"
#include "render/render.h"

#include <glib.h>

typedef struct Menu {
    char *label;
    char *name;
    
    GList *entries;
    /* GList *tail; */

    /* ? */
    gboolean shown;
    gboolean invalid;
    gpointer render_data; /* where the engine can store anything it likes */

    struct Menu *parent;

    /* waste o' pointers */
    void (*show)( /* some bummu */);
    void (*hide)( /* some bummu */);
    void (*update)( /* some bummu */);
    void (*mouseover)( /* some bummu */);
    void (*selected)( /* some bummu */);
} Menu;

typedef struct MenuRenderData {
    Window frame;
    Window title;
    Appearance *a_title;
    int title_min_w, title_h;
    Window items;
    Appearance *a_items;
    int item_h;
} MenuRenderData;

typedef enum MenuEntryRenderType {
    MenuEntryRenderType_None = 0,
    MenuEntryRenderType_Submenu = 1 << 0,
    MenuEntryRenderType_Boolean = 1 << 1,
    MenuEntryRenderType_Separator = 1 << 2,
    
    MenuEntryRenderType_Other = 1 << 7
} MenuEntryRenderType;

typedef struct {
    char *label;
    Menu *parent;

    Action *action;    
    
    MenuEntryRenderType render_type;
    gboolean enabled;
    gboolean boolean_value;
    gpointer render_data; /* where the engine can store anything it likes */

    Menu *submenu;
} MenuEntry;

typedef struct MenuEntryRenderData {
    Window item;
    Appearance *a_item;
    int min_w;
} MenuEntryRenderData;

void menu_startup();
void menu_shutdown();

Menu *menu_new(char *label, char *name, Menu *parent);
void menu_free(char *name);

void menu_show(char *name, int x, int y, Client *client);

MenuEntry *menu_entry_new_full(char *label, Action *action,
                               MenuEntryRenderType render_type,
                               gpointer submenu);

#define menu_entry_new(label, action) \
  menu_entry_new_full(label, action, MenuEntryRenderType_None, NULL)

void menu_entry_free(MenuEntry *entry);

void menu_entry_set_submenu(MenuEntry *entry, Menu *submenu);

void menu_add_entry(Menu *menu, MenuEntry *entry);
#endif
