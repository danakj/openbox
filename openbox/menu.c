#include "debug.h"
#include "menu.h"
#include "openbox.h"
#include "stacking.h"
#include "client.h"
#include "grab.h"
#include "config.h"
#include "screen.h"
#include "menuframe.h"
#include "geom.h"
#include "plugin.h"
#include "misc.h"
#include "parser/parse.h"

GHashTable *menu_hash = NULL;

typedef struct _ObMenuParseState ObMenuParseState;

struct _ObMenuParseState
{
    GSList *menus;
};

static void menu_clear_entries_internal(ObMenu *self);

static ObMenu* menu_from_name(gchar *name)
{
    ObMenu *self = NULL;

    g_assert(name != NULL);

    if (!(self = g_hash_table_lookup(menu_hash, name)))
        g_warning("Attempted to access menu '%s' but it does not exist.",
                  name);
    return self;
}  

static void parse_menu_item(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                            gpointer data)
{
    ObMenuParseState *state = data;
    gchar *label;
    
    if (state->menus) {
        if (parse_attr_string("label", node, &label)) {
            GSList *acts = NULL;
            
            for (node = node->xmlChildrenNode; node; node = node->next)
                if (!xmlStrcasecmp(node->name, (const xmlChar*) "action"))
                    acts = g_slist_append(acts, action_parse(doc, node));
            menu_add_normal(state->menus->data, label, acts);
            g_free(label);
        }
    }
}

static void parse_menu_separator(ObParseInst *i,
                                 xmlDocPtr doc, xmlNodePtr node,
                                 gpointer data)
{
    ObMenuParseState *state = data;
    
    if (state->menus)
        menu_add_separator(state->menus->data);
}

static void parse_menu(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                       gpointer data)
{
    ObMenuParseState *state = data;
    gchar *name = NULL, *title = NULL;

    if (!parse_attr_string("id", node, &name))
        goto parse_menu_fail;

    if (!g_hash_table_lookup(menu_hash, name)) {
        if (!parse_attr_string("label", node, &title))
            goto parse_menu_fail;

        if (menu_new(name, title, NULL)) {
            state->menus = g_slist_prepend(state->menus, name);
            parse_tree(i, doc, node->xmlChildrenNode);
            state->menus = g_slist_delete_link(state->menus, state->menus);
        }
    }

    if (state->menus)
        menu_add_submenu(state->menus->data, name);

parse_menu_fail:
    g_free(name);
    g_free(title);
}


void menu_destroy_hash_value(ObMenu *self)
{
    menu_clear_entries_internal(self);
    g_free(self->name);
    g_free(self->title);
}

void menu_startup(ObParseInst *i)
{
    menu_hash = g_hash_table_new_full(g_str_hash, g_str_equal, NULL,
                                      (GDestroyNotify)menu_destroy_hash_value);
}

void menu_shutdown()
{
    menu_frame_hide_all();
    g_hash_table_destroy(menu_hash);
}

void menu_parse()
{
    ObParseInst *i;
    ObMenuParseState parse_state;
    xmlDocPtr doc;
    xmlNodePtr node;
    gchar *p;
    gboolean loaded = FALSE;

    i = parse_startup();

    if (config_menu_path)
        if (!(loaded =
              parse_load(config_menu_path, "openbox_menu", &doc, &node)))
            g_warning("Failed to load menu from '%s'", config_menu_path);
    if (!loaded) {
        p = g_build_filename(g_get_home_dir(), ".openbox", "menu", NULL);
        if (!(loaded =
              parse_load(p, "openbox_menu", &doc, &node)))
            g_warning("Failed to load menu from '%s'", p);
        g_free(p);
    }
    if (!loaded) {
        p = g_build_filename(RCDIR, "menu", NULL);
        if (!(loaded =
              parse_load(p, "openbox_menu", &doc, &node)))
            g_warning("Failed to load menu from '%s'", p);
        g_free(p);
    }

    if (loaded) {
        parse_state.menus = NULL;

        parse_register(i, "menu", parse_menu, &parse_state);
        parse_register(i, "item", parse_menu_item, &parse_state);
        parse_register(i, "separator", parse_menu_separator, &parse_state);
        parse_tree(i, doc, node->xmlChildrenNode);
    }

    parse_shutdown(i);
}

gboolean menu_new(gchar *name, gchar *title, gpointer data)
{
    ObMenu *self;

    if (g_hash_table_lookup(menu_hash, name)) return FALSE;

    self = g_new0(ObMenu, 1);
    self->name = g_strdup(name);
    self->title = g_strdup(title);
    self->data = data;

    g_hash_table_insert(menu_hash, self->name, self);

    return TRUE;
}

void menu_show(gchar *name, gint x, gint y, ObClient *client)
{
    ObMenu *self;
    ObMenuFrame *frame;

    if (!(self = menu_from_name(name))) return;

    /* XXX update entries */

    frame = menu_frame_new(self, client);
    menu_frame_move(frame, x, y);
    menu_frame_show(frame, NULL);
}

static ObMenuEntry* menu_entry_new(ObMenu *menu, ObMenuEntryType type)
{
    ObMenuEntry *self;

    g_assert(menu);

    self = g_new0(ObMenuEntry, 1);
    self->type = type;
    self->menu = menu;
    self->enabled = TRUE;
    return self;
}

static void menu_entry_free(ObMenuEntry *self)
{
    if (self) {
        switch (self->type) {
        case OB_MENU_ENTRY_TYPE_NORMAL:
            g_free(self->data.normal.label);
            while (self->data.normal.actions) {
                action_free(self->data.normal.actions->data);
                self->data.normal.actions =
                    g_slist_delete_link(self->data.normal.actions,
                                        self->data.normal.actions);
            }
            break;
        case OB_MENU_ENTRY_TYPE_SUBMENU:
        case OB_MENU_ENTRY_TYPE_SEPARATOR:
            break;
        }

        g_free(self);
    }
}

void menu_clear_entries(gchar *name)
{
    ObMenu *self;

    if (!(self = menu_from_name(name))) return;

    menu_clear_entries_internal(self);
}

static void menu_clear_entries_internal(ObMenu *self)
{
    /* XXX assert that the menu isn't visible */

    while (self->entries) {
	menu_entry_free(self->entries->data);
        self->entries = g_list_delete_link(self->entries, self->entries);
    }
}

void menu_add_normal(gchar *name, gchar *label, GSList *actions)
{
    ObMenu *self;
    ObMenuEntry *e;

    if (!(self = menu_from_name(name))) return;

    e = menu_entry_new(self, OB_MENU_ENTRY_TYPE_NORMAL);
    e->data.normal.label = g_strdup(label);
    e->data.normal.actions = actions;

    self->entries = g_list_append(self->entries, e);
}

void menu_add_submenu(gchar *name, gchar *submenu)
{
    ObMenu *self, *sub;
    ObMenuEntry *e;

    if (!(self = menu_from_name(name))) return;
    if (!(sub = menu_from_name(submenu))) return;

    e = menu_entry_new(self, OB_MENU_ENTRY_TYPE_SUBMENU);
    e->data.submenu.submenu = sub;

    self->entries = g_list_append(self->entries, e);
}

void menu_add_separator(gchar *name)
{
    ObMenu *self;
    ObMenuEntry *e;

    if (!(self = menu_from_name(name))) return;

    e = menu_entry_new(self, OB_MENU_ENTRY_TYPE_SEPARATOR);

    self->entries = g_list_append(self->entries, e);
}
