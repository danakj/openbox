#ifndef __menu_h
#define __menu_h

#include "action.h"
#include "window.h"
#include "render/render.h"
#include "geom.h"

#include <glib.h>

struct _ObClient;

typedef struct _ObMenu      ObMenu;
typedef struct _ObMenuEntry ObMenuEntry;

typedef void(*menu_controller_show)(ObMenu *self, int x, int y,
                                    struct _ObClient *);
typedef void(*menu_controller_update)(ObMenu *self);
typedef void(*menu_controller_mouseover)(ObMenuEntry *self, gboolean enter);

extern GHashTable *menu_hash;
extern GSList *menu_visible;

struct _ObMenu
{
    ObWindow obwin;

    gchar *label;
    gchar *name;
    
    GList *entries;

    gboolean shown;
    gboolean invalid;

    ObMenu *parent;
    
    ObMenu *open_submenu;

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
    gint title_min_w, title_h;
    Window items;
    RrAppearance *a_items;
    gint bullet_w;
    gint item_h;
    Point location;
    Size size;
    guint xin_area; /* index of the xinerama head/area */

    /* plugin stuff */
    char *plugin;
    void *plugin_data;
};

typedef enum
{
    OB_MENU_ENTRY_RENDER_TYPE_NONE,
    OB_MENU_ENTRY_RENDER_TYPE_SUBMENU,
    OB_MENU_ENTRY_RENDER_TYPE_BOOLEAN,
    OB_MENU_ENTRY_RENDER_TYPE_SEPARATOR,
    OB_MENU_ENTRY_RENDER_TYPE_OTHER /* XXX what is this? */
} ObMenuEntryRenderType;

struct _ObMenuEntry
{
    char *label;
    ObMenu *parent;

    Action *action;    
    
    ObMenuEntryRenderType render_type;
    gboolean hilite;
    gboolean enabled;
    gboolean boolean_value;

    ObMenu *submenu;

    /* render stuff */
    Window item;
    RrAppearance *a_item;
    RrAppearance *a_disabled;
    RrAppearance *a_hilite;
    gint y;
    gint min_w;
} MenuEntry;

typedef struct PluginMenuCreateData{
    xmlDocPtr doc;
    xmlNodePtr node;
    ObMenu *parent;
} PluginMenuCreateData;


void menu_startup();
void menu_shutdown();

#define menu_new(l, n, p) \
  menu_new_full(l, n, p, NULL, NULL)

ObMenu *menu_new_full(char *label, char *name, ObMenu *parent, 
                    menu_controller_show show, menu_controller_update update);
void menu_free(char *name);

void menu_show(char *name, int x, int y, struct _ObClient *client);
void menu_show_full(ObMenu *menu, int x, int y, struct _ObClient *client);

void menu_hide(ObMenu *self);

void menu_clear(ObMenu *self);

ObMenuEntry *menu_entry_new_full(char *label, Action *action,
                               ObMenuEntryRenderType render_type,
                               gpointer submenu);

#define menu_entry_new(label, action) \
menu_entry_new_full(label, action, OB_MENU_ENTRY_RENDER_TYPE_NONE, NULL)

#define menu_entry_new_separator(label) \
menu_entry_new_full(label, NULL, OB_MENU_ENTRY_RENDER_TYPE_SEPARATOR, NULL)

#define menu_entry_new_submenu(label, submenu) \
menu_entry_new_full(label, NULL, OB_MENU_ENTRY_RENDER_TYPE_SUBMENU, submenu)

#define menu_entry_new_boolean(label, action) \
menu_entry_new_full(label, action, OB_MENU_ENTRY_RENDER_TYPE_BOOLEAN, NULL)

void menu_entry_free(ObMenuEntry *entry);

void menu_entry_set_submenu(ObMenuEntry *entry, ObMenu *submenu);

void menu_add_entry(ObMenu *menu, ObMenuEntry *entry);

ObMenuEntry *menu_find_entry(ObMenu *menu, Window win);
ObMenuEntry *menu_find_entry_by_pos(ObMenu *menu, int x, int y);

void menu_entry_render(ObMenuEntry *self);

void menu_entry_fire(ObMenuEntry *self);

void menu_render(ObMenu *self);
void menu_render_full(ObMenu *self);

//so plugins can call it?
void parse_menu_full(xmlDocPtr doc, xmlNodePtr node, void *data, gboolean new);
void menu_control_mouseover(ObMenuEntry *entry, gboolean enter);
#endif
