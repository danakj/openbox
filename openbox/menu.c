#include "debug.h"
#include "menu.h"
#include "openbox.h"
#include "stacking.h"
#include "client.h"
#include "grab.h"
#include "config.h"
#include "screen.h"
#include "geom.h"
#include "plugin.h"
#include "misc.h"
#include "parser/parse.h"

GHashTable *menu_hash = NULL;
GList *menu_visible = NULL;

#define FRAME_EVENTMASK (ButtonPressMask |ButtonMotionMask | EnterWindowMask |\
			 LeaveWindowMask)
#define TITLE_EVENTMASK (ButtonPressMask | ButtonMotionMask)
#define ENTRY_EVENTMASK (EnterWindowMask | LeaveWindowMask | \
                         ButtonPressMask | ButtonReleaseMask)

static void parse_menu(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                       gpointer data)
{
    g_message("%s", __FUNCTION__);
    parse_menu_full(i, doc, node, data, TRUE);
}


void parse_menu_full(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                     gpointer data, gboolean newmenu)
{
    ObAction *act;
    xmlNodePtr nact;

    gchar *id = NULL, *title = NULL, *label = NULL, *plugin;
    ObMenu *menu = NULL, *parent = NULL;

    if (newmenu == TRUE) {
        if (!parse_attr_string("id", node, &id))
            goto parse_menu_fail;
        if (!parse_attr_string("label", node, &title))
            goto parse_menu_fail;
        ob_debug("menu label %s\n", title);

        if (parse_attr_string("plugin", node, &plugin)) {
            PluginMenuCreateData data;
            data.parse_inst = i;
            data.doc = doc;
            data.node = node;
            data.parent = menu;

            if (plugin_open_reopen(plugin, i))
                menu = plugin_create(plugin, &data);
            g_free(plugin);
        } else
            menu = menu_new(title, id, data ? *((ObMenu**)data) : NULL);
            
        if (data && menu)
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
                if (plugin_open_reopen(plugin, i))
                    parent = plugin_create(plugin, &data);
                g_free(plugin);
            } else {
                parent = menu;
                parse_menu(i, doc, node, &parent);
            }

            if (parent)
                menu_add_entry(menu, menu_entry_new_submenu(parent->label,
                                                            parent));

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

    if (self->destroy) self->destroy(self);

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
    RrAppearanceFree(self->a_submenu);
    XDestroyWindow(ob_display, self->item);
    XDestroyWindow(ob_display, self->submenu_pic);
    g_free(self);
}
 
void menu_startup(ObParseInst *i)
{
    menu_hash = g_hash_table_new_full(g_str_hash, g_str_equal,
                                      (GDestroyNotify)menu_destroy_hash_key,
                                      (GDestroyNotify)menu_destroy_hash_value);
}

void menu_shutdown()
{
    g_hash_table_destroy(menu_hash);
}

void menu_parse()
{
    ObParseInst *i;
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
        parse_register(i, "menu", parse_menu, NULL);
        parse_tree(i, doc, node->xmlChildrenNode);
    }

    parse_shutdown(i);
}

static Window createWindow(Window parent, unsigned long mask,
			   XSetWindowAttributes *attrib)
{
    return XCreateWindow(ob_display, parent, 0, 0, 1, 1, 0,
			 RrDepth(ob_rr_inst), InputOutput,
                         RrVisual(ob_rr_inst), mask, attrib);
                       
}

ObMenu *menu_new_full(char *label, char *name, ObMenu *parent, 
                      menu_controller_show show, menu_controller_update update,
                      menu_controller_selected selected,
                      menu_controller_hide hide,
                      menu_controller_mouseover mouseover,
                      menu_controller_destroy destroy)
{
    XSetWindowAttributes attrib;
    ObMenu *self;

    self = g_new0(ObMenu, 1);
    self->obwin.type = Window_Menu;
    self->label = g_strdup(label);
    self->name = g_strdup(name);
    self->parent = parent;
    self->open_submenu = NULL;
    self->over = NULL;

    self->entries = NULL;
    self->shown = FALSE;
    self->invalid = TRUE;

    /* default controllers */
    self->destroy = destroy;
    self->show = (show != NULL ? show : menu_show_full);
    self->hide = (hide != NULL ? hide : menu_hide);
    self->update = (update != NULL ? update : menu_render);
    self->mouseover = (mouseover != NULL ? mouseover :
                       menu_control_mouseover);
    self->selected = (selected != NULL ? selected : menu_entry_fire);

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

ObMenuEntry *menu_entry_new_full(char *label, ObAction *action,
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
    entry->submenu_pic = createWindow(menu->items, CWEventMask, &attrib);
    XMapWindow(ob_display, entry->item);
    XMapWindow(ob_display, entry->submenu_pic);

    entry->a_item = entry->a_disabled = entry->a_hilite = entry->a_submenu
        = NULL;

    menu->invalid = TRUE;

    g_hash_table_insert(window_map, &entry->item, menu);
    g_hash_table_insert(window_map, &entry->submenu_pic, menu);
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
       
    self->update(self);
    
    self->client = client;

    if (!self->shown) {
        if (!(self->parent && self->parent->shown)) {
            grab_pointer(TRUE, None);
            grab_keyboard(TRUE);
        }
        menu_visible = g_list_append(menu_visible, self);
    }

    menu_control_show(self, x, y, client);
}

void menu_hide(ObMenu *self) {
    if (self->shown) {
        XUnmapWindow(ob_display, self->frame);
        self->shown = FALSE;
	if (self->open_submenu)
	    self->open_submenu->hide(self->open_submenu);
	if (self->parent && self->parent->open_submenu == self) {
	    self->parent->open_submenu = NULL;
        }

        if (!(self->parent && self->parent->shown)) {
            grab_keyboard(FALSE);
            grab_pointer(FALSE, None);
        }
        menu_visible = g_list_remove(menu_visible, self);
        if (self->over) {
            ((ObMenuEntry *)self->over->data)->hilite = FALSE;
            menu_entry_render(self->over->data);
            self->over = NULL;
        }
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

void menu_entry_fire(ObMenuEntry *self, unsigned int button, unsigned int x,
                     unsigned int y)
{
    ObMenu *m;

    /* ignore wheel scrolling */
    if (button == 4 || button == 5) return;

    if (self->action) {
        self->action->data.any.c = self->parent->client;
        self->action->func(&self->action->data);

        /* hide the whole thing */
        m = self->parent;
        while (m->parent) m = m->parent;
        m->hide(m);
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
	self->open_submenu->hide(self->open_submenu);
    }
}

void menu_control_mouseover(ObMenuEntry *self, gboolean enter)
{
    int x;
    Rect *a;
    ObMenuEntry *e;

    g_assert(self != NULL);
    
    if (enter) {
        /* TODO: we prolly don't need open_submenu */
	if (self->parent->open_submenu && self->submenu 
	    != self->parent->open_submenu)
        {
            e = (ObMenuEntry *) self->parent->over->data;
            e->hilite = FALSE;
            menu_entry_render(e);
	    self->parent->open_submenu->hide(self->parent->open_submenu);
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
	    self->submenu->update(self->submenu);

            a = screen_physical_area_monitor(self->parent->xin_area);

	    if (self->submenu->size.width + x >= a->x + a->width) {
                int newparentx = a->x + a->width
                    - self->submenu->size.width
                    - self->parent->size.width
                    - ob_rr_theme->bwidth
                    - ob_rr_theme->menu_overlap;
                
                x = a->x + a->width - self->submenu->size.width
                    - ob_rr_theme->menu_overlap;
                XWarpPointer(ob_display, None, None, 0, 0, 0, 0,
                             newparentx - self->parent->location.x, 0);

                menu_show_full(self->parent, newparentx,
                               self->parent->location.y, self->parent->client);
            }
	    
	    menu_show_full(self->submenu, x,
			   self->parent->location.y + self->y,
                           self->parent->client);
	}
        self->hilite = TRUE;
        self->parent->over = g_list_find(self->parent->entries, self);
        
    } else
        self->hilite = FALSE;
    
    menu_entry_render(self);
}

void menu_control_keyboard_nav(unsigned int key)
{
    static ObMenu *current_menu = NULL;
    ObMenuEntry *e = NULL;

    ObKey obkey = OB_NUM_KEYS;

    /* hrmm. could be fixed */
    if (key == ob_keycode(OB_KEY_DOWN))
        obkey = OB_KEY_DOWN;
    else if (key == ob_keycode(OB_KEY_UP))
        obkey = OB_KEY_UP;
    else if (key == ob_keycode(OB_KEY_RIGHT)) /* fuck */
        obkey = OB_KEY_RIGHT;
    else if (key == ob_keycode(OB_KEY_LEFT)) /* users */
        obkey = OB_KEY_LEFT;
    else if (key == ob_keycode(OB_KEY_RETURN))
        obkey = OB_KEY_RETURN;

    
    if (current_menu == NULL)
        current_menu = menu_visible->data;
    
    switch (obkey) {
    case OB_KEY_DOWN: {
        if (current_menu->over) {
            current_menu->mouseover(current_menu->over->data, FALSE);
            current_menu->over = (current_menu->over->next != NULL ?
                          current_menu->over->next :
                          current_menu->entries);
        }
        else
            current_menu->over = current_menu->entries;

        if (current_menu->over)
            current_menu->mouseover(current_menu->over->data, TRUE);
        
        break;
    }
    case OB_KEY_UP: {
        if (current_menu->over) {
            current_menu->mouseover(current_menu->over->data, FALSE);
            current_menu->over = (current_menu->over->prev != NULL ?
                          current_menu->over->prev :
                g_list_last(current_menu->entries));
        } else
            current_menu->over = g_list_last(current_menu->entries);

        if (current_menu->over)
            current_menu->mouseover(current_menu->over->data, TRUE);
        
        break;
    }
    case OB_KEY_RIGHT: {
        if (current_menu->over == NULL)
            return;
        e = (ObMenuEntry *)current_menu->over->data;
        if (e->submenu) {
            current_menu->mouseover(e, TRUE);
            current_menu = e->submenu;
            current_menu->over = current_menu->entries;
            if (current_menu->over)
                current_menu->mouseover(current_menu->over->data, TRUE);
        }
        break;
    }

    case OB_KEY_RETURN: {
        if (current_menu->over == NULL)
            return;
        e = (ObMenuEntry *)current_menu->over->data;

        current_menu->mouseover(e, FALSE);
        current_menu->over = NULL;
        /* zero is enter */
        menu_entry_fire(e, 0, 0, 0);
    }
        
    case OB_KEY_LEFT: {
        if (current_menu->over != NULL) {
            current_menu->mouseover(current_menu->over->data, FALSE);
            current_menu->over = NULL;
        }
        
        current_menu->hide(current_menu);

        if (current_menu->parent)
            current_menu = current_menu->parent;
        
        break;
    }
    default:
        ((ObMenu *)menu_visible->data)->hide(menu_visible->data);
        current_menu = NULL;
    }
    return;
}

void menu_noop()
{
    /* This noop brought to you by OLS 2003 Email Garden. */
}
