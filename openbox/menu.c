#include "menu.h"
#include "openbox.h"
#include "render/theme.h"

static GHashTable *menu_hash = NULL;
GHashTable *menu_map = NULL;

#define TITLE_EVENTMASK (ButtonMotionMask)
#define ENTRY_EVENTMASK (EnterWindowMask | LeaveWindowMask | \
                         ButtonPressMask | ButtonReleaseMask)

void menu_destroy_hash_key(gpointer data)
{
    g_free(data);
}

void menu_destroy_hash_value(Menu *self)
{
    GList *it;
    MenuRenderData *data = self->render_data;

    for (it = self->entries; it; it = it->next)
        menu_entry_free(it->data);
    g_list_free(self->entries);

    g_free(self->label);
    g_free(self->name);

    g_hash_table_remove(menu_map, &data->title);
    g_hash_table_remove(menu_map, &data->frame);
    g_hash_table_remove(menu_map, &data->items);

    appearance_free(data->a_title);
    XDestroyWindow(ob_display, data->title);
    XDestroyWindow(ob_display, data->frame);
    XDestroyWindow(ob_display, data->items);

    g_free(self);
}

void menu_entry_free(MenuEntry *self)
{
    MenuEntryRenderData *data = self->render_data;

    g_free(self->label);
    g_free(self->render_data);
    action_free(self->action);

    g_hash_table_remove(menu_map, &data->item);

    appearance_free(data->a_item);
    XDestroyWindow(ob_display, data->item);

    g_free(self);
}
    
void menu_startup()
{
    Menu *m;

    menu_hash = g_hash_table_new_full(g_str_hash, g_str_equal,
                                      menu_destroy_hash_key,
                                      (GDestroyNotify)menu_destroy_hash_value);
    menu_map = g_hash_table_new(g_int_hash, g_int_equal);

    m = menu_new("sex menu", "root", NULL);
    menu_add_entry(m, menu_entry_new("foo shit etc bleh",
                                     action_from_string("restart")));
    menu_add_entry(m, menu_entry_new("more shit",
                                     action_from_string("restart")));
    menu_add_entry(m, menu_entry_new("",
                                     action_from_string("restart")));
    menu_add_entry(m, menu_entry_new("and yet more",
                                     action_from_string("restart")));
}

void menu_shutdown()
{
    g_hash_table_destroy(menu_hash);
    g_hash_table_destroy(menu_map);
}

static Window createWindow(Window parent, unsigned long mask,
			   XSetWindowAttributes *attrib)
{
    return XCreateWindow(ob_display, parent, 0, 0, 1, 1, 0,
			 render_depth, InputOutput, render_visual,
			 mask, attrib);
                       
}

Menu *menu_new(char *label, char *name, Menu *parent)
{
    XSetWindowAttributes attrib;
    Menu *self;
    MenuRenderData *data;

    self = g_new0(Menu, 1);
    self->label = g_strdup(label);
    self->name = g_strdup(name);
    self->parent = parent;

    self->entries = NULL;
    self->shown = FALSE;
    self->invalid = FALSE;
    /* default controllers? */

    data = g_new(MenuRenderData, 1);

    attrib.override_redirect = TRUE;
    data->frame = createWindow(ob_root, CWOverrideRedirect, &attrib);
    attrib.event_mask = TITLE_EVENTMASK;
    data->title = createWindow(data->frame, CWEventMask, &attrib);
    data->items = createWindow(data->frame, 0, &attrib);

    XSetWindowBorderWidth(ob_display, data->frame, theme_bwidth);
    XSetWindowBorderWidth(ob_display, data->title, theme_bwidth);
    XSetWindowBorder(ob_display, data->frame, theme_b_color->pixel);
    XSetWindowBorder(ob_display, data->title, theme_b_color->pixel);

    XMapWindow(ob_display, data->title);
    XMapWindow(ob_display, data->items);

    data->a_title = appearance_copy(theme_a_menu_title);
    data->a_items = appearance_copy(theme_a_menu);

    self->render_data = data;

    g_hash_table_insert(menu_map, &data->frame, self);
    g_hash_table_insert(menu_map, &data->title, self);
    g_hash_table_insert(menu_map, &data->items, self);
    g_hash_table_insert(menu_hash, g_strdup(name), self);
    return self;
}

void menu_free(char *name)
{
    g_hash_table_remove(menu_hash, name);
}

MenuEntry *menu_entry_new_full(char *label, Action *action,
                               MenuEntryRenderType render_type,
                               gpointer submenu)
{
    MenuEntry *menu_entry = g_new(MenuEntry, 1);

    menu_entry->label = g_strdup(label);
    menu_entry->render_type = render_type;
    menu_entry->action = action;

    menu_entry->render_data = NULL;
    menu_entry->submenu = submenu;

    return menu_entry;
}

void menu_entry_set_submenu(MenuEntry *entry, Menu *submenu)
{
    g_assert(entry != NULL);
    
    entry->submenu = submenu;

    if(entry->parent != NULL)
        entry->parent->invalid = TRUE;
}

void menu_add_entry(Menu *menu, MenuEntry *entry)
{
    MenuEntryRenderData *data;
    XSetWindowAttributes attrib;

    g_assert(menu != NULL && entry != NULL && entry->render_data == NULL);

    menu->entries = g_list_append(menu->entries, entry);
    entry->parent = menu;

    data = g_new(MenuEntryRenderData, 1);
    data->item = createWindow(((MenuRenderData*)menu->render_data)->items,
                              0, &attrib);
    XMapWindow(ob_display, data->item);
    data->a_item = appearance_copy(theme_a_menu_item);

    entry->render_data = data;

    menu->invalid = TRUE;

    g_hash_table_insert(menu_map, &data->item, menu);
}

void menu_show(char *name, int x, int y, Client *client)
{
    Menu *self;
    MenuRenderData *data;
    GList *it;
    int w = 1;
    int items_h;
    int item_h = 0, nitems = 0; /* each item, only one is used */
    int item_y;
    int bullet_w;

    self = g_hash_table_lookup(menu_hash, name);
    if (!self) {
        g_warning("Attempted to show menu '%s' but it does not exist.",
                  name);
        return;
    }

    data = self->render_data;

    /* set texture data and size them mofos out */
    data->a_title->texture[0].data.text.string = self->label;
    appearance_minsize(data->a_title, &data->title_min_w, &data->title_h);
    data->title_min_w += theme_bevel * 2;
    data->title_h += theme_bevel * 2;
    w = MAX(w, data->title_min_w);

    for (it = self->entries; it; it = it->next) {
        MenuEntryRenderData *r = ((MenuEntry*)it->data)->render_data;

        r->a_item->texture[0].data.text.string = ((MenuEntry*)it->data)->label;
        appearance_minsize(r->a_item, &r->min_w, &item_h);
        r->min_w += theme_bevel * 2;
        item_h += theme_bevel * 2;
        w = MAX(w, r->min_w);
        ++nitems;
    }
    bullet_w = item_h + theme_bevel;
    w += 2 * bullet_w;
    items_h = item_h * nitems;

    /* size appearances */
    RECT_SET(data->a_title->area, 0, 0, w, data->title_h);
    RECT_SET(data->a_title->texture[0].position, 0, 0, w, data->title_h);
    RECT_SET(data->a_items->area, 0, 0, w, items_h);
    for (it = self->entries; it; it = it->next) {
        MenuEntryRenderData *r = ((MenuEntry*)it->data)->render_data;
        RECT_SET(r->a_item->area, 0, 0, w, item_h);
        RECT_SET(r->a_item->texture[0].position, bullet_w, 0,
                 w - 2 * bullet_w, item_h);
    }

    /* size windows and paint the suckers */
    XMoveResizeWindow(ob_display, data->frame, x, y, w,
                      data->title_h + items_h);
    XMoveResizeWindow(ob_display, data->title, -theme_bwidth, -theme_bwidth,
                      w, data->title_h);
    paint(data->title, data->a_title);
    XMoveResizeWindow(ob_display, data->items, 0, data->title_h + theme_bwidth,
                      w, items_h);
    paint(data->items, data->a_items);
    for (item_y = 0, it = self->entries; it; item_y += item_h, it = it->next) {
        MenuEntryRenderData *r = ((MenuEntry*)it->data)->render_data;
        XMoveResizeWindow(ob_display, r->item, 0, item_y, w, item_h);
        r->a_item->surface.data.planar.parent = data->a_items;
        r->a_item->surface.data.planar.parentx = 0;
        r->a_item->surface.data.planar.parenty = item_y;
        paint(r->item, r->a_item);
    }


    XMapWindow(ob_display, data->frame);
}
