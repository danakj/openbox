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
typedef void (*ObMenuExecuteFunc)(struct _ObMenuEntryFrame *frame,
                                  gpointer data);
typedef void (*ObMenuDestroyFunc)(struct _ObMenu *menu, gpointer data);

extern GList *menu_visible;

struct _ObMenu
{
    /* Name of the menu. Used in the showmenu action. */
    gchar *name;
    /* Displayed title */
    gchar *title;

    /* ObMenuEntry list */
    GList *entries;

    /* plugin data */
    gpointer data;

    ObMenuUpdateFunc update_func;
    ObMenuExecuteFunc execute_func;
    ObMenuDestroyFunc destroy_func;
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
};

struct _ObSubmenuMenuEntry {
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

void menu_startup();
void menu_shutdown();

void menu_parse();

gboolean menu_new(gchar *name, gchar *title, gpointer data);
void menu_free(gchar *name);

gboolean menu_open_plugin(ObParseInst *i, gchar *name, gchar *plugin);

void menu_set_update_func(gchar *name, ObMenuUpdateFunc func);
void menu_set_execute_func(gchar *name, ObMenuExecuteFunc func);
void menu_set_destroy_func(gchar *name, ObMenuDestroyFunc func);

void menu_show(gchar *name, gint x, gint y, struct _ObClient *client);

/* functions for building menus */
void menu_clear_entries(gchar *name);
void menu_add_normal(gchar *name, gint id, gchar *label, GSList *actions);
void menu_add_submenu(gchar *name, gint id, gchar *submenu);
void menu_add_separator(gchar *name, gint id);

ObMenuEntry* menu_find_entry_id(ObMenu *self, gint id);

#endif
