#include "debug.h"
#include "menu.h"
#include "openbox.h"
#include "stacking.h"
#include "client.h"
#include "grab.h"
#include "screen.h"
#include "geom.h"
#include "plugin.h"
#include "misc.h"

GHashTable *menu_hash = NULL;
GList *menu_visible = NULL;

#define FRAME_EVENTMASK (ButtonPressMask |ButtonMotionMask | EnterWindowMask |\
			 LeaveWindowMask)
#define TITLE_EVENTMASK (ButtonPressMask | ButtonMotionMask)
#define ENTRY_EVENTMASK (EnterWindowMask | LeaveWindowMask | \
                         ButtonPressMask | ButtonReleaseMask)

static void parse_menu(xmlDocPtr doc, xmlNodePtr node, void *data)
{
    parse_menu_full(doc, node, data, TRUE);
}


void parse_menu_full(xmlDocPtr doc, xmlNodePtr node, void *data,
                       gboolean newmenu)
{
    Action *act;
    xmlNodePtr nact;

    gchar *id = NULL, *title = NULL, *label = NULL, *plugin;
    ObMenu *menu = NULL, *parent;

    if (newmenu == TRUE) {
        if (!parse_attr_string("id", node, &id))
            goto parse_menu_fail;
        if (!parse_attr_string("label", node, &title))
            goto parse_menu_fail;
        ob_debug("menu label %s\n", title);

        if (parse_attr_string("plugin", node, &plugin)) {
            PluginMenuCreateData data;
            data.doc = doc;
            data.node = node;
            data.parent = menu;
            parent = plugin_create(plugin, &data);
            g_free(plugin);
        } else
            menu = menu_new(title, id, data ? *((ObMenu**)data) : NULL);
            
        if (data)
            *((ObMenu**)data) = menu;
    } else {
        menu = (ObMenu *)data;
    }

    node = node->xmlChildrenNode;
    
    while (node) {
        if (!xmlStrcasecmp(node->name, (const xmlChar*) "menu")) {
            if (parse_attr_string("plugin", node, &plugin)) {
                PluginMenuCreateData data;
                data.doc = doc;
                data.node = node;
                data.parent = menu;
                parent = plugin_create(plugin, &data);
                g_free(plugin);
            } else {
                parent = menu;
                parse_menu(doc, node, &parent);
                menu_add_entry(menu, menu_entry_new_submenu(parent->label,
                                                            parent));
            }

        }
        else if (!xmlStrcasecmp(node->name, (const xmlChar*) "item")) {
            if (parse_attr_string("label", node, &label)) {
                if ((nact = parse_find_node("action", node->xmlChildrenNode)))
                    act = action_parse(doc, nact);
                else
                    act = NULL;
                if (act)
                    menu_add_entry(menu, menu_entry_new(label, act));
                else
                    menu_add_entry(menu, menu_entry_new_separator(label));
                g_free(label);
            }
        }
        node = node->next;
    }

parse_menu_fail:
    g_free(id);
    g_free(title);
}

void menu_control_show(ObMenu *self, int x, int y, ObClient *client);

void menu_destroy_hash_key(ObMenu *menu)
{
    g_free(menu);
}

void menu_destroy_hash_value(ObMenu *self)
{
    GList *it;

    for (it = self->entries; it; it = it->next)
        menu_entry_free(it->data);
    g_list_free(self->entries);

    g_free(self->label);
    g_free(self->name);

    g_hash_table_remove(window_map, &self->title);
    g_hash_table_remove(window_map, &self->frame);
    g_hash_table_remove(window_map, &self->items);

    stacking_remove(self);

    RrAppearanceFree(self->a_title);
    RrAppearanceFree(self->a_items);
    XDestroyWindow(ob_display, self->title);
    XDestroyWindow(ob_display, self->frame);
    XDestroyWindow(ob_display, self->items);

    g_free(self);
}

void menu_entry_free(ObMenuEntry *self)
{
    g_free(self->label);
    action_free(self->action);

    g_hash_table_remove(window_map, &self->item);

    RrAppearanceFree(self->a_item);
    RrAppearanceFree(self->a_disabled);
    RrAppearanceFree(self->a_hilite);
    XDestroyWindow(ob_display, self->item);

    g_free(self);
}
    
void menu_startup()
{
/*
    ObMenu *m;
    ObMenu *s;
    ObMenu *t;
    Action *a;
*/

    menu_hash = g_hash_table_new_full(g_str_hash, g_str_equal,
                                      (GDestroyNotify)menu_destroy_hash_key,
                                      (GDestroyNotify)menu_destroy_hash_value);

    parse_register("menu", parse_menu, NULL);

/*
    m = menu_new("sex menu", "root", NULL);
 
    a = action_from_string("execute");
    a->data.execute.path = g_strdup("xterm");
    menu_add_entry(m, menu_entry_new("xterm", a));
    a = action_from_string("restart");
    menu_add_entry(m, menu_entry_new("restart", a));
    menu_add_entry(m, menu_entry_new_separator("--"));
    a = action_from_string("exit");
    menu_add_entry(m, menu_entry_new("exit", a));
*/

    /*
    s = menu_new("subsex menu", "submenu", m);
    a = action_from_string("execute");
    a->data.execute.path = g_strdup("xclock");
    menu_add_entry(s, menu_entry_new("xclock", a));

    menu_add_entry(m, menu_entry_new_submenu("subz", s));

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
    */
}

void menu_shutdown()
{
    g_hash_table_destroy(menu_hash);
}

static Window createWindow(Window parent, unsigned long mask,
			   XSetWindowAttributes *attrib)
{
    return XCreateWindow(ob_display, parent, 0, 0, 1, 1, 0,
			 RrDepth(ob_rr_inst), InputOutput,
                         RrVisual(ob_rr_inst), mask, attrib);
                       
}

ObMenu *menu_new_full(char *label, char *name, ObMenu *parent, 
                    menu_controller_show show, menu_controller_update update)
{
    XSetWindowAttributes attrib;
    ObMenu *self;

    self = g_new0(ObMenu, 1);
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
    self->frame = createWindow(RootWindow(ob_display, ob_screen),
                               CWOverrideRedirect|CWEventMask, &attrib);
    attrib.event_mask = TITLE_EVENTMASK;
    self->title = createWindow(self->frame, CWEventMask, &attrib);
    self->items = createWindow(self->frame, 0, &attrib);

    self->a_title = self->a_items = NULL;

    XMapWindow(ob_display, self->title);
    XMapWindow(ob_display, self->items);

    g_hash_table_insert(window_map, &self->frame, self);
    g_hash_table_insert(window_map, &self->title, self);
    g_hash_table_insert(window_map, &self->items, self);
    g_hash_table_insert(menu_hash, g_strdup(name), self);

    stacking_add(MENU_AS_WINDOW(self));
    stacking_raise(MENU_AS_WINDOW(self));

    return self;
}

void menu_free(char *name)
{
    g_hash_table_remove(menu_hash, name);
}

ObMenuEntry *menu_entry_new_full(char *label, Action *action,
                               ObMenuEntryRenderType render_type,
                               gpointer submenu)
{
    ObMenuEntry *menu_entry = g_new0(ObMenuEntry, 1);

    menu_entry->label = g_strdup(label);
    menu_entry->render_type = render_type;
    menu_entry->action = action;

    menu_entry->hilite = FALSE;
    menu_entry->enabled = TRUE;

    menu_entry->submenu = submenu;

    return menu_entry;
}

void menu_entry_set_submenu(ObMenuEntry *entry, ObMenu *submenu)
{
    g_assert(entry != NULL);
    
    entry->submenu = submenu;

    if(entry->parent != NULL)
        entry->parent->invalid = TRUE;
}

void menu_add_entry(ObMenu *menu, ObMenuEntry *entry)
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

    entry->a_item = entry->a_disabled = entry->a_hilite = NULL;

    menu->invalid = TRUE;

    g_hash_table_insert(window_map, &entry->item, menu);
}

void menu_show(char *name, int x, int y, ObClient *client)
{
    ObMenu *self;
  
    self = g_hash_table_lookup(menu_hash, name);
    if (!self) {
        g_warning("Attempted to show menu '%s' but it does not exist.",
                  name);
        return;
    }

    menu_show_full(self, x, y, client);
}  

void menu_show_full(ObMenu *self, int x, int y, ObClient *client)
{
    g_assert(self != NULL);
       
    menu_render(self);
    
    self->client = client;

    if (!self->shown) {
        if (!(self->parent && self->parent->shown)) {
            grab_pointer(TRUE, None);
            grab_keyboard(TRUE);
        }
        menu_visible = g_list_append(menu_visible, self);
    }

    if (self->show) {
	self->show(self, x, y, client);
    } else {
      menu_control_show(self, x, y, client);
    }
}

void menu_hide(ObMenu *self) {
    if (self->shown) {
        XUnmapWindow(ob_display, self->frame);
        self->shown = FALSE;
	if (self->open_submenu)
	    menu_hide(self->open_submenu);
	if (self->parent && self->parent->open_submenu == self) {
            ObMenuEntry *e;

	    self->parent->open_submenu = NULL;

            e = menu_find_entry_by_submenu(self->parent, self);
            if (self->parent->mouseover)
                self->parent->mouseover(e, FALSE);
            else
                menu_control_mouseover(e, FALSE);
            menu_entry_render(e);
        }

        if (!(self->parent && self->parent->shown)) {
            grab_keyboard(FALSE);
            grab_pointer(FALSE, None);
        }
        menu_visible = g_list_remove(menu_visible, self);
    }
}

void menu_clear(ObMenu *self) {
    GList *it;
  
    for (it = self->entries; it; it = it->next) {
	ObMenuEntry *entry = it->data;
	menu_entry_free(entry);
    }
    self->entries = NULL;
    self->invalid = TRUE;
}


ObMenuEntry *menu_find_entry(ObMenu *menu, Window win)
{
    GList *it;

    for (it = menu->entries; it; it = it->next) {
        ObMenuEntry *entry = it->data;
        if (entry->item == win)
            return entry;
    }
    return NULL;
}

ObMenuEntry *menu_find_entry_by_submenu(ObMenu *menu, ObMenu *submenu)
{
    GList *it;

    for (it = menu->entries; it; it = it->next) {
        ObMenuEntry *entry = it->data;
        if (entry->submenu == submenu)
            return entry;
    }
    return NULL;
}

ObMenuEntry *menu_find_entry_by_pos(ObMenu *menu, int x, int y)
{
    if (x < 0 || x >= menu->size.width || y < 0 || y >= menu->size.height)
        return NULL;

    y -= menu->title_h + ob_rr_theme->bwidth;
    if (y < 0) return NULL;
    
    ob_debug("%d %p\n", y/menu->item_h,
             g_list_nth_data(menu->entries, y / menu->item_h));
    return g_list_nth_data(menu->entries, y / menu->item_h);
}

void menu_entry_fire(ObMenuEntry *self)
{
    ObMenu *m;

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

void menu_control_show(ObMenu *self, int x, int y, ObClient *client)
{
    guint i;
    Rect *a = NULL;

    g_assert(!self->invalid);
    
    for (i = 0; i < screen_num_monitors; ++i) {
        a = screen_physical_area_monitor(i);
        if (RECT_CONTAINS(*a, x, y))
            break;
    }
    g_assert(a != NULL);
    self->xin_area = i;

    POINT_SET(self->location,
	      MIN(x, a->x + a->width - 1 - self->size.width), 
	      MIN(y, a->y + a->height - 1 - self->size.height));
    XMoveWindow(ob_display, self->frame, self->location.x, self->location.y);

    if (!self->shown) {
	XMapWindow(ob_display, self->frame);
        stacking_raise(MENU_AS_WINDOW(self));
	self->shown = TRUE;
    } else if (self->shown && self->open_submenu) {
	menu_hide(self->open_submenu);
    }
}

void menu_control_mouseover(ObMenuEntry *self, gboolean enter)
{
    int x;
    Rect *a;
    ObMenuEntry *e;

    if (enter) {
	if (self->parent->open_submenu && self->submenu 
	    != self->parent->open_submenu)
        {
            e = menu_find_entry_by_submenu(self->parent,
                                           self->parent->open_submenu);
            e->hilite = FALSE;
            menu_entry_render(e);
	    menu_hide(self->parent->open_submenu);
        }
	
	if (self->submenu && self->parent->open_submenu != self->submenu) {
	    self->parent->open_submenu = self->submenu;

	    /* shouldn't be invalid since it must be displayed */
	    g_assert(!self->parent->invalid);
	    /* TODO: I don't understand why these bevels should be here.
	       Something must be wrong in the width calculation */
	    x = self->parent->location.x + self->parent->size.width + 
		ob_rr_theme->bwidth - ob_rr_theme->menu_overlap;

	    /* need to get the width. is this bad?*/
	    menu_render(self->submenu);

            a = screen_physical_area_monitor(self->parent->xin_area);

	    if (self->submenu->size.width + x >= a->x + a->width)
		x = self->parent->location.x - self->submenu->size.width - 
		    ob_rr_theme->bwidth + ob_rr_theme->menu_overlap;
	    
	    menu_show_full(self->submenu, x,
			   self->parent->location.y + self->y,
                           self->parent->client);
	} 
    }

    if (enter || !self->submenu ||
        menu_find_entry_by_submenu(self->parent,
                                   self->parent->open_submenu) != self)
        self->hilite = enter;
}

ObMenuEntry *menu_control_keyboard_nav(ObMenuEntry *over, ObKey key)
{
    GList *it = NULL;
        
    switch (key) {
    case OB_KEY_DOWN: {
        if (over != NULL) {
            if (over->parent->mouseover)
                over->parent->mouseover(over, FALSE);
            else
                menu_control_mouseover(over, FALSE);
            menu_entry_render(over);
                
            it = over->parent->entries;
            while (it != NULL && it->data != over)
                it = it->next;
        }
            
        if (it && it->next)
            over = (ObMenuEntry *)it->next->data;
        else if (over == NULL) {
            if (menu_visible && ((ObMenu *)menu_visible->data)->entries)
                over = (ObMenuEntry *)
                    (((ObMenu *)menu_visible->data)->entries)->data;
            else
                over = NULL;
        } else {
            over = (over->parent->entries != NULL ?
                    over->parent->entries->data : NULL);
        }

        if (over) {
            if (over->parent->mouseover)
                over->parent->mouseover(over, TRUE);
            else
                menu_control_mouseover(over, TRUE);
            menu_entry_render(over);
        }
        
        break;
    }
    case OB_KEY_UP: {
        if (over != NULL) {
            if (over->parent->mouseover)
                over->parent->mouseover(over, FALSE);
            else
                menu_control_mouseover(over, FALSE);
            menu_entry_render(over);
                
            it = g_list_last(over->parent->entries);
            while (it != NULL && it->data != over)
                it = it->prev;
        } 
            
        if (it && it->prev)
            over = (ObMenuEntry *)it->prev->data;
        else if (over == NULL) {
            it = g_list_last(menu_visible);
            if (it != NULL) {
                it = g_list_last(((ObMenu *)it->data)->entries);
                over = (ObMenuEntry *)(it != NULL ? it->data : NULL);
            }
        } else
            over = (over->parent->entries != NULL ?
                    g_list_last(over->parent->entries)->data :
                    NULL);

        if (over->parent->mouseover)
            over->parent->mouseover(over, TRUE);
        else
            menu_control_mouseover(over, TRUE);
        menu_entry_render(over);
        break;
    }
    case OB_KEY_RETURN: {
        if (over == NULL)
            return over;

        if (over->submenu) {
            if (over->parent->mouseover)
                over->parent->mouseover(over, FALSE);
            else
                menu_control_mouseover(over, FALSE);
            menu_entry_render(over);

            if (over->submenu->entries)
                over = over->submenu->entries->data;

            if (over->parent->mouseover)
                over->parent->mouseover(over, TRUE);
            else
                menu_control_mouseover(over, TRUE);
            menu_entry_render(over);
        }
        else {
            if (over->parent->mouseover)
                over->parent->mouseover(over, FALSE);
            else
                menu_control_mouseover(over, FALSE);
            menu_entry_render(over);

            menu_entry_fire(over);
        }
        break;
    }
    case OB_KEY_ESCAPE: {
        if (over != NULL) {
            if (over->parent->mouseover)
                over->parent->mouseover(over, FALSE);
            else
                menu_control_mouseover(over, FALSE);
            menu_entry_render(over);

            menu_hide(over->parent);
        } else {
            it  = g_list_last(menu_visible);
            if (it) {
                menu_hide((ObMenu *)it->data);
            }
        }
        
        over = NULL;
        break;
    }
    default:
        g_error("Unknown key");
    }

    return over;
}
