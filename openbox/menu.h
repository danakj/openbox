#ifndef __menu_h
#define __menu_h

#include "action.h"
#include "window.h"
#include "render/render.h"
#include "geom.h"

#include <glib.h>

struct _ObClient;

struct Menu;
struct MenuEntry;

typedef void(*menu_controller_show)(struct Menu *self,
                                    int x, int y, struct _ObClient *);
typedef void(*menu_controller_update)(struct Menu *self);
typedef void(*menu_controller_mouseover)(struct MenuEntry *self,
                                         gboolean enter);

extern GHashTable *menu_hash;
extern GSList *menu_visible;

typedef struct Menu {
    ObWindow obwin;

    char *label;
    char *name;
    
    GList *entries;

    gboolean shown;
    gboolean invalid;

    struct Menu *parent;
    
    struct Menu *open_submenu;

    /* place a menu on screen */
    menu_controller_show show;
    void (*hide)( /* some bummu */);

    /* render a menu */
    menu_controller_update update;
    menu_controller_mouseover mouseover;
    void (*selected)( /* some bummu */);


    /* render stuff */
    struct _ObClient *client;
    Window frame;
    Window title;
    RrAppearance *a_title;
    int title_min_w, title_h;
    Window items;
    RrAppearance *a_items;
    int bullet_w;
    int item_h;
    Point location;
    Size size;
    guint xin_area; /* index of the xinerama head/area */

    /* plugin stuff */
    char *plugin;
    void *plugin_data;
} Menu;

typedef enum MenuEntryRenderType {
    MenuEntryRenderType_None = 0,
    MenuEntryRenderType_Submenu = 1 << 0,
    MenuEntryRenderType_Boolean = 1 << 1,
    MenuEntryRenderType_Separator = 1 << 2,
    
    MenuEntryRenderType_Other = 1 << 7
} MenuEntryRenderType;

typedef struct MenuEntry {
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
    RrAppearance *a_item;
    RrAppearance *a_disabled;
    RrAppearance *a_hilite;
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

void menu_show(char *name, int x, int y, struct _ObClient *client);
void menu_show_full(Menu *menu, int x, int y, struct _ObClient *client);

void menu_hide(Menu *self);

void menu_clear(Menu *self);

MenuEntry *menu_entry_new_full(char *label, Action *action,
                               MenuEntryRenderType render_type,
                               gpointer submenu);

#define menu_entry_new(label, action) \
menu_entry_new_full(label, action, MenuEntryRenderType_None, NULL)

#define menu_entry_new_separator(label) \
menu_entry_new_full(label, NULL, MenuEntryRenderType_Separator, NULL)

#define menu_entry_new_submenu(label, submenu) \
menu_entry_new_full(label, NULL, MenuEntryRenderType_Submenu, submenu)

#define menu_entry_new_boolean(label, action) \
menu_entry_new_full(label, action, MenuEntryRenderType_Boolean, NULL)

void menu_entry_free(MenuEntry *entry);

void menu_entry_set_submenu(MenuEntry *entry, Menu *submenu);

void menu_add_entry(Menu *menu, MenuEntry *entry);

MenuEntry *menu_find_entry(Menu *menu, Window win);
MenuEntry *menu_find_entry_by_pos(Menu *menu, int x, int y);

void menu_entry_render(MenuEntry *self);

void menu_entry_fire(MenuEntry *self);

void menu_render(Menu *self);
void menu_render_full(Menu *self);

void menu_control_mouseover(MenuEntry *entry, gboolean enter);
#endif
