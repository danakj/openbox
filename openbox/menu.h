#ifndef __menu_h
#define __menu_h

#include "action.h"
#include "render/render.h"

#include <glib.h>

extern GHashTable *menu_map;

struct Menu;

typedef void(*menu_controller_show)(struct Menu *self, int x, int y, Client *);
typedef void(*menu_controller_update)(struct Menu *self);

typedef struct Menu {
    char *label;
    char *name;
    
    GList *entries;

    gboolean shown;
    gboolean invalid;

    struct Menu *parent;

    /* place a menu on screen */
    menu_controller_show show;
    void (*hide)( /* some bummu */);

    /* render a menu */
    menu_controller_update update;
    void (*mouseover)( /* some bummu */);
    void (*selected)( /* some bummu */);


    /* render stuff */
    Client *client;
    Window frame;
    Window title;
    Appearance *a_title;
    int title_min_w, title_h;
    Window items;
    Appearance *a_items;
    int bullet_w;
    int item_h;
    int width;
} Menu;

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
    gboolean hilite;
    gboolean enabled;
    gboolean boolean_value;

    Menu *submenu;

    /* render stuff */
    Window item;
    Appearance *a_item;
    Appearance *a_disabled;
    Appearance *a_hilite;
    int y;
    int min_w;
} MenuEntry;

void menu_startup();
void menu_shutdown();

#define menu_new(l, n, p) \
  menu_new_full(l, n, p, NULL, NULL)

Menu *menu_new_full(char *label, char *name, Menu *parent, 
                    menu_controller_show show, menu_controller_update update);
void menu_free(char *name);

void menu_show(char *name, int x, int y, Client *client);
void menu_hide(Menu *self);

MenuEntry *menu_entry_new_full(char *label, Action *action,
                               MenuEntryRenderType render_type,
                               gpointer submenu);

#define menu_entry_new(label, action) \
  menu_entry_new_full(label, action, MenuEntryRenderType_None, NULL)

void menu_entry_free(MenuEntry *entry);

void menu_entry_set_submenu(MenuEntry *entry, Menu *submenu);

void menu_add_entry(Menu *menu, MenuEntry *entry);

MenuEntry *menu_find_entry(Menu *menu, Window win);

void menu_entry_render(MenuEntry *self);

void menu_entry_fire(MenuEntry *self);

void menu_render(Menu *self);

#endif
