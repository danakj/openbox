#include "menu.h"
#include "openbox.h"
#include "stacking.h"
#include "render/theme.h"

static GHashTable *menu_hash = NULL;
GHashTable *menu_map = NULL;

#define TITLE_EVENTMASK (ButtonPressMask | ButtonMotionMask)
#define ENTRY_EVENTMASK (EnterWindowMask | LeaveWindowMask | \
                         ButtonPressMask | ButtonReleaseMask)

void menu_destroy_hash_key(gpointer data)
{
    g_free(data);
}

void menu_destroy_hash_value(Menu *self)
{
    GList *it;

    for (it = self->entries; it; it = it->next)
        menu_entry_free(it->data);
    g_list_free(self->entries);

    g_free(self->label);
    g_free(self->name);

    g_hash_table_remove(menu_map, &self->title);
    g_hash_table_remove(menu_map, &self->frame);
    g_hash_table_remove(menu_map, &self->items);

    appearance_free(self->a_title);
    XDestroyWindow(ob_display, self->title);
    XDestroyWindow(ob_display, self->frame);
    XDestroyWindow(ob_display, self->items);

    g_free(self);
}

void menu_entry_free(MenuEntry *self)
{
    g_free(self->label);
    action_free(self->action);

    g_hash_table_remove(menu_map, &self->item);

    appearance_free(self->a_item);
    appearance_free(self->a_disabled);
    appearance_free(self->a_hilite);
    XDestroyWindow(ob_display, self->item);

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

    self = g_new0(Menu, 1);
    self->label = g_strdup(label);
    self->name = g_strdup(name);
    self->parent = parent;

    self->entries = NULL;
    self->shown = FALSE;
    self->invalid = FALSE;
    /* default controllers? */

    attrib.override_redirect = TRUE;
    self->frame = createWindow(ob_root, CWOverrideRedirect, &attrib);
    attrib.event_mask = TITLE_EVENTMASK;
    self->title = createWindow(self->frame, CWEventMask, &attrib);
    self->items = createWindow(self->frame, 0, &attrib);

    XSetWindowBorderWidth(ob_display, self->frame, theme_bwidth);
    XSetWindowBorderWidth(ob_display, self->title, theme_bwidth);
    XSetWindowBorder(ob_display, self->frame, theme_b_color->pixel);
    XSetWindowBorder(ob_display, self->title, theme_b_color->pixel);

    XMapWindow(ob_display, self->title);
    XMapWindow(ob_display, self->items);

    self->a_title = appearance_copy(theme_a_menu_title);
    self->a_items = appearance_copy(theme_a_menu);

    g_hash_table_insert(menu_map, &self->frame, self);
    g_hash_table_insert(menu_map, &self->title, self);
    g_hash_table_insert(menu_map, &self->items, self);
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
    MenuEntry *menu_entry = g_new0(MenuEntry, 1);

    menu_entry->label = g_strdup(label);
    menu_entry->render_type = render_type;
    menu_entry->action = action;

    menu_entry->hilite = FALSE;
    menu_entry->enabled = TRUE;

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
    XSetWindowAttributes attrib;

    g_assert(menu != NULL && entry != NULL && entry->item == None);

    menu->entries = g_list_append(menu->entries, entry);
    entry->parent = menu;

    attrib.event_mask = ENTRY_EVENTMASK;
    entry->item = createWindow(menu->items, CWEventMask, &attrib);
    XMapWindow(ob_display, entry->item);
    entry->a_item = appearance_copy(theme_a_menu_item);
    entry->a_disabled = appearance_copy(theme_a_menu_disabled);
    entry->a_hilite = appearance_copy(theme_a_menu_hilite);

    menu->invalid = TRUE;

    g_hash_table_insert(menu_map, &entry->item, menu);
}

void menu_show(char *name, int x, int y, Client *client)
{
    Menu *self;
    GList *it;
    int items_h;
    int nitems = 0; /* each item, only one is used */
    int item_y;

    self = g_hash_table_lookup(menu_hash, name);
    if (!self) {
        g_warning("Attempted to show menu '%s' but it does not exist.",
                  name);
        return;
    }

    self->width = 1;
    self->item_h = 0;

    /* set texture data and size them mofos out */
    self->a_title->texture[0].data.text.string = self->label;
    appearance_minsize(self->a_title, &self->title_min_w, &self->title_h);
    self->title_min_w += theme_bevel * 2;
    self->title_h += theme_bevel * 2;
    self->width = MAX(self->width, self->title_min_w);

    for (it = self->entries; it; it = it->next) {
        MenuEntry *e = it->data;
        int h;

        e->a_item->texture[0].data.text.string = e->label;
        appearance_minsize(e->a_item, &e->min_w, &self->item_h);
        self->width = MAX(self->width, e->min_w);

        e->a_disabled->texture[0].data.text.string = e->label;
        appearance_minsize(e->a_disabled, &e->min_w, &h);
        self->item_h = MAX(self->item_h, h);
        self->width = MAX(self->width, e->min_w);

        e->a_hilite->texture[0].data.text.string = e->label;
        appearance_minsize(e->a_hilite, &e->min_w, &h);
        self->item_h = MAX(self->item_h, h);
        self->width = MAX(self->width, e->min_w);

        e->min_w += theme_bevel * 2;
        ++nitems;
    }
    self->bullet_w = self->item_h + theme_bevel;
    self->width += 2 * self->bullet_w;
    self->item_h += theme_bevel * 2;
    items_h = self->item_h * nitems;

    RECT_SET(self->a_title->area, 0, 0, self->width, self->title_h);
    RECT_SET(self->a_title->texture[0].position, 0, 0, self->width,
             self->title_h);
    RECT_SET(self->a_items->area, 0, 0, self->width, items_h);

    XMoveResizeWindow(ob_display, self->frame, x, y, self->width,
                      self->title_h + items_h);
    XMoveResizeWindow(ob_display, self->title, -theme_bwidth, -theme_bwidth,
                      self->width, self->title_h);
    XMoveResizeWindow(ob_display, self->items, 0, self->title_h + theme_bwidth,
                      self->width, items_h);

    paint(self->title, self->a_title);
    paint(self->items, self->a_items);

    item_y = 0;
    for (it = self->entries; it; it = it->next) {
        ((MenuEntry*)it->data)->y = item_y;
        menu_entry_render(it->data);
        item_y += self->item_h;
    }

    stacking_raise_internal(self->frame);
    XMapWindow(ob_display, self->frame);
}

MenuEntry *menu_find_entry(Menu *menu, Window win)
{
    GList *it;

    for (it = menu->entries; it; it = it->next) {
        MenuEntry *entry = it->data;
        if (entry->item == win)
            return entry;
    }
    return NULL;
}

void menu_entry_render(MenuEntry *self)
{
    Menu *menu = self->parent;
    Appearance *a;

    a = !self->enabled ? self->a_disabled :
        (self->hilite ? self->a_hilite : self->a_item);

    RECT_SET(a->area, 0, 0, menu->width,
             menu->item_h);
    RECT_SET(a->texture[0].position, menu->bullet_w,
             0, menu->width - 2 * menu->bullet_w,
             menu->item_h);

    XMoveResizeWindow(ob_display, self->item, 0, self->y,
                      menu->width, menu->item_h);
    a->surface.data.planar.parent = menu->a_items;
    a->surface.data.planar.parentx = 0;
    a->surface.data.planar.parenty = self->y;

    paint(self->item, a);
}
