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
#include "misc.h"
#include "client_menu.h"
#include "client_list_menu.h"
#include "parser/parse.h"

static GHashTable *menu_hash = NULL;

ObParseInst *menu_parse_inst;

typedef struct _ObMenuParseState ObMenuParseState;

struct _ObMenuParseState
{
    GSList *menus;
};

static void menu_destroy_hash_value(ObMenu *self);
static void parse_menu_item(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                            gpointer data);
static void parse_menu_separator(ObParseInst *i,
                                 xmlDocPtr doc, xmlNodePtr node,
                                 gpointer data);
static void parse_menu(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                       gpointer data);

static gboolean menu_open(gchar *file, xmlDocPtr *doc, xmlNodePtr *node)
{
    gboolean loaded = TRUE;
    gchar *p;

    p = g_build_filename(g_get_home_dir(), ".openbox", file, NULL);
    if (!parse_load(p, "openbox_menu", doc, node)) {
        g_free(p);
        p = g_build_filename(RCDIR, file, NULL);
        if (!parse_load(p, "openbox_menu", doc, node)) {
            g_free(p);
            p = g_strdup(file);
            if (!parse_load(p, "openbox_menu", doc, node)) {
                g_warning("Failed to load menu from '%s'", file);
                loaded = FALSE;
            }
        }
    }
    g_free(p);
    return loaded;
}

void menu_startup()
{
    ObMenuParseState parse_state;
    xmlDocPtr doc;
    xmlNodePtr node;
    gboolean loaded = FALSE;
    GSList *it;

    menu_hash = g_hash_table_new_full(g_str_hash, g_str_equal, NULL,
                                      (GDestroyNotify)menu_destroy_hash_value);

    client_list_menu_startup();
    client_menu_startup();

    menu_parse_inst = parse_startup();

    for (it = config_menu_files; it; it = g_slist_next(it)) {
        if (menu_open(it->data, &doc, &node))
            loaded = TRUE;

    }
    if (!loaded)
        loaded = menu_open("menu", &doc, &node);

    if (loaded) {
        parse_state.menus = NULL;

        parse_register(menu_parse_inst, "menu", parse_menu, &parse_state);
        parse_register(menu_parse_inst, "item", parse_menu_item, &parse_state);
        parse_register(menu_parse_inst, "separator",
                       parse_menu_separator, &parse_state);
        parse_tree(menu_parse_inst, doc, node->xmlChildrenNode);
        xmlFreeDoc(doc);
    }
}

void menu_shutdown()
{
    parse_shutdown(menu_parse_inst);
    menu_parse_inst = NULL;

    menu_frame_hide_all();
    g_hash_table_destroy(menu_hash);
    menu_hash = NULL;
}

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
                    acts = g_slist_append(acts, action_parse(i, doc, node));
            menu_add_normal(state->menus->data, -1, label, acts);
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
        menu_add_separator(state->menus->data, -1);
}

static void parse_menu(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                       gpointer data)
{
    ObMenuParseState *state = data;
    gchar *name = NULL, *title = NULL;
    ObMenu *menu;

    if (!parse_attr_string("id", node, &name))
        goto parse_menu_fail;

    if (!g_hash_table_lookup(menu_hash, name)) {
        if (!parse_attr_string("label", node, &title))
            goto parse_menu_fail;

        if ((menu = menu_new(name, title, NULL))) {
            state->menus = g_slist_prepend(state->menus, menu);
            parse_tree(i, doc, node->xmlChildrenNode);
            state->menus = g_slist_delete_link(state->menus, state->menus);
        }
    }

    if (state->menus)
        menu_add_submenu(state->menus->data, -1, name);

parse_menu_fail:
    g_free(name);
    g_free(title);
}


static void menu_destroy_hash_value(ObMenu *self)
{
    /* XXX make sure its not visible */

    if (self->destroy_func)
        self->destroy_func(self, self->data);

    menu_clear_entries(self);
    g_free(self->name);
    g_free(self->title);
}

ObMenu* menu_new(gchar *name, gchar *title, gpointer data)
{
    ObMenu *self;

    /*if (g_hash_table_lookup(menu_hash, name)) return FALSE;*/

    self = g_new0(ObMenu, 1);
    self->name = g_strdup(name);
    self->title = g_strdup(title);
    self->data = data;

    g_hash_table_replace(menu_hash, self->name, self);

    return self;
}

void menu_free(ObMenu *menu)
{
    g_hash_table_remove(menu_hash, menu->name);
}

void menu_show(gchar *name, gint x, gint y, ObClient *client)
{
    ObMenu *self;
    ObMenuFrame *frame;

    if (!(self = menu_from_name(name))) return;

    frame = menu_frame_new(self, client);
    menu_frame_move(frame, x, y);
    menu_frame_show(frame, NULL);
    if (frame->entries)
        menu_frame_select_next(frame);
}

static ObMenuEntry* menu_entry_new(ObMenu *menu, ObMenuEntryType type, gint id)
{
    ObMenuEntry *self;

    g_assert(menu);

    self = g_new0(ObMenuEntry, 1);
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

void menu_entry_free(ObMenuEntry *self)
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
    /* XXX assert that the menu isn't visible */

    while (self->entries) {
	menu_entry_free(self->entries->data);
        self->entries = g_list_delete_link(self->entries, self->entries);
    }
}

void menu_entry_remove(ObMenuEntry *self)
{
    self->menu->entries = g_list_remove(self->menu->entries, self);
    menu_entry_free(self);
}

ObMenuEntry* menu_add_normal(ObMenu *self, gint id, gchar *label,
                             GSList *actions)
{
    ObMenuEntry *e;

    e = menu_entry_new(self, OB_MENU_ENTRY_TYPE_NORMAL, id);
    e->data.normal.label = g_strdup(label);
    e->data.normal.actions = actions;

    self->entries = g_list_append(self->entries, e);
    return e;
}

ObMenuEntry* menu_add_submenu(ObMenu *self, gint id, gchar *submenu)
{
    ObMenuEntry *e;

    e = menu_entry_new(self, OB_MENU_ENTRY_TYPE_SUBMENU, id);
    e->data.submenu.name = g_strdup(submenu);

    self->entries = g_list_append(self->entries, e);
    return e;
}

ObMenuEntry* menu_add_separator(ObMenu *self, gint id)
{
    ObMenuEntry *e;

    e = menu_entry_new(self, OB_MENU_ENTRY_TYPE_SEPARATOR, id);

    self->entries = g_list_append(self->entries, e);
    return e;
}

void menu_set_update_func(ObMenu *self, ObMenuUpdateFunc func)
{
    self->update_func = func;
}

void menu_set_execute_func(ObMenu *self, ObMenuExecuteFunc func)
{
    self->execute_func = func;
}

void menu_set_destroy_func(ObMenu *self, ObMenuDestroyFunc func)
{
    self->destroy_func = func;
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
