#include "menu.h"
#include "openbox.h"
#include "stacking.h"
#include "grab.h"
#include "render/theme.h"
#include "screen.h"
#include "geom.h"
#include "plugin.h"

static GHashTable *menu_hash = NULL;
GHashTable *menu_map = NULL;

#define FRAME_EVENTMASK (ButtonPressMask |ButtonMotionMask | EnterWindowMask | \
			 LeaveWindowMask)
#define TITLE_EVENTMASK (ButtonPressMask | ButtonMotionMask)
#define ENTRY_EVENTMASK (EnterWindowMask | LeaveWindowMask | \
                         ButtonPressMask | ButtonReleaseMask)

void menu_control_show(Menu *self, int x, int y, Client *client);

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
    Menu *s;
    Menu *t;
    Action *a;

    menu_hash = g_hash_table_new_full(g_str_hash, g_str_equal,
                                      menu_destroy_hash_key,
                                      (GDestroyNotify)menu_destroy_hash_value);
    menu_map = g_hash_table_new(g_int_hash, g_int_equal);

    m = menu_new(NULL, "root", NULL);
 
    a = action_from_string("execute");
    a->data.execute.path = g_strdup("xterm");
    menu_add_entry(m, menu_entry_new("xterm", a));
    a = action_from_string("restart");
    menu_add_entry(m, menu_entry_new("restart", a));
    menu_add_entry(m, menu_entry_new_separator("--"));
    a = action_from_string("exit");
    menu_add_entry(m, menu_entry_new("exit", a));
    s = menu_new("subsex menu", "submenu", m);
    a = action_from_string("execute");
    a->data.execute.path = g_strdup("xclock");
    menu_add_entry(s, menu_entry_new("xclock", a));

    menu_add_entry(m, menu_entry_new_submenu("subz", s));

    /*
    t = (Menu *)plugin_create("timed_menu");
    a = action_from_string("execute");
    a->data.execute.path = g_strdup("xeyes");
    menu_add_entry(t, menu_entry_new("xeyes", a));
    menu_add_entry(m, menu_entry_new_submenu("timed", t));
    */
    
    s = menu_new("empty", "chub", m);
    menu_add_entry(m, menu_entry_new_submenu("empty", s));

    s = menu_new("", "s-club", m);
    menu_add_entry(m, menu_entry_new_submenu("empty", s));

    s = menu_new(NULL, "h-club", m);
    menu_add_entry(m, menu_entry_new_submenu("empty", s));

    s = menu_new(NULL, "g-club", m);

    a = action_from_string("execute");
    a->data.execute.path = g_strdup("xterm");
    menu_add_entry(s, menu_entry_new("xterm", a));
    a = action_from_string("restart");
    menu_add_entry(s, menu_entry_new("restart", a));
    menu_add_entry(s, menu_entry_new_separator("--"));
    a = action_from_string("exit");
    menu_add_entry(s, menu_entry_new("exit", a));

    menu_add_entry(m, menu_entry_new_submenu("long", s));

    m = menu_new(NULL, "client", NULL);
    a = action_from_string("iconify");
    menu_add_entry(m, menu_entry_new("iconify", a));
    a = action_from_string("toggleshade");
    menu_add_entry(m, menu_entry_new("(un)shade", a));
    a = action_from_string("togglemaximizefull");
    menu_add_entry(m, menu_entry_new("(un)maximize", a));
    a = action_from_string("close");
    menu_add_entry(m, menu_entry_new("close", a));

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

Menu *menu_new_full(char *label, char *name, Menu *parent, 
                    menu_controller_show show, menu_controller_update update)
{
    XSetWindowAttributes attrib;
    Menu *self;

    self = g_new0(Menu, 1);
    self->obwin.type = Window_Menu;
    self->label = g_strdup(label);
    self->name = g_strdup(name);
    self->parent = parent;
    self->open_submenu = NULL;

    self->entries = NULL;
    self->shown = FALSE;
    self->invalid = TRUE;

    /* default controllers */
    self->show = show;
    self->hide = NULL;
    self->update = update;
    self->mouseover = NULL;
    self->selected = NULL;

    self->plugin = NULL;
    self->plugin_data = NULL;

    attrib.override_redirect = TRUE;
    attrib.event_mask = FRAME_EVENTMASK;
    self->frame = createWindow(ob_root, CWOverrideRedirect|CWEventMask, &attrib);
    attrib.event_mask = TITLE_EVENTMASK;
    self->title = createWindow(self->frame, CWEventMask, &attrib);
    self->items = createWindow(self->frame, 0, &attrib);

    XSetWindowBorderWidth(ob_display, self->frame, theme_bwidth);
    XSetWindowBackground(ob_display, self->frame, theme_b_color->pixel);
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

    g_assert(menu != NULL);
    g_assert(entry != NULL);
    g_assert(entry->item == None);

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
  
    self = g_hash_table_lookup(menu_hash, name);
    if (!self) {
        g_warning("Attempted to show menu '%s' but it does not exist.",
                  name);
        return;
    }
    
    menu_show_full(self, x, y, client);
}  

void menu_show_full(Menu *self, int x, int y, Client *client)
{
    g_assert(self != NULL);
       
    menu_render(self);
    
    self->client = client;

    if (self->show) {
	self->show(self, x, y, client);
    } else {
      menu_control_show(self, x, y, client);
    }
}


void menu_hide(Menu *self) {
    if (self->shown) {
        XUnmapWindow(ob_display, self->frame);
        self->shown = FALSE;
	if (self->open_submenu)
	    menu_hide(self->open_submenu);
	if (self->parent && self->parent->open_submenu == self)
	    self->parent->open_submenu = NULL;

    }
}

void menu_clear(Menu *self) {
    GList *it;
  
    for (it = self->entries; it; it = it->next) {
	MenuEntry *entry = it->data;
	menu_entry_free(entry);
    }
    self->entries = NULL;
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

void menu_entry_fire(MenuEntry *self)
{
    Menu *m;

    if (self->action) {
        self->action->data.any.c = self->parent->client;
        self->action->func(&self->action->data);

        /* hide the whole thing */
        m = self->parent;
        while (m->parent) m = m->parent;
        menu_hide(m);
    }
}

/* 
   Default menu controller action for showing.
*/

void menu_control_show(Menu *self, int x, int y, Client *client) {
    g_assert(!self->invalid);
    
    XMoveWindow(ob_display, self->frame, 
		MIN(x, screen_physical_size.width - self->size.width), 
		MIN(y, screen_physical_size.height - self->size.height));
    POINT_SET(self->location, 
	      MIN(x, screen_physical_size.width - self->size.width), 
	      MIN(y, screen_physical_size.height - self->size.height));

    if (!self->shown) {
        /* XXX gotta add to the stacking list first!
           stacking_raise(MENU_AS_WINDOW(self));
        */
	XMapWindow(ob_display, self->frame);
	self->shown = TRUE;
    } else if (self->shown && self->open_submenu) {
	menu_hide(self->open_submenu);
    }
}

void menu_control_mouseover(MenuEntry *self, gboolean enter) {
    int x;
    self->hilite = enter;
  
    if (enter) {
	if (self->parent->open_submenu && self->submenu 
	    != self->parent->open_submenu)
	    menu_hide(self->parent->open_submenu);
	
	if (self->submenu) {
	    self->parent->open_submenu = self->submenu;

	    /* shouldn't be invalid since it must be displayed */
	    g_assert(!self->parent->invalid);
	    /* TODO: I don't understand why these bevels should be here.
	       Something must be wrong in the width calculation */
	    x = self->parent->location.x + self->parent->size.width + 
		theme_bevel;

	    /* need to get the width. is this bad?*/
	    menu_render(self->submenu);

	    if (self->submenu->size.width + x > screen_physical_size.width)
		x = self->parent->location.x - self->submenu->size.width - 
		    theme_bevel;
	    
	    menu_show_full(self->submenu, x,
			   self->parent->location.y + self->y, NULL);
	} 
    }
}
