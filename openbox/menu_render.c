/* Functions for default rendering of menus. Might become pluginnable */

#include "menu.h"
#include "openbox.h"
#include "render/theme.h"

void menu_render_full(Menu *self);

void menu_render(Menu *self) {
    if (self->invalid) {
	if (self->update) {
	    self->update(self);
	} else {
	    menu_render_full(self);
	}
    }
}
	    

void menu_render_full(Menu *self) {
    GList *it;
    int items_h = 0;
    int nitems = 0; /* each item, only one is used */
    int item_y;

    self->size.width = 1;
    self->item_h = 1;

    /* set texture data and size them mofos out */
    if (self->label) {
	self->a_title->texture[0].data.text.string = self->label;
	appearance_minsize(self->a_title, &self->title_min_w, &self->title_h);
	self->title_min_w += theme_bevel * 2;
	self->title_h += theme_bevel * 2;
	self->size.width = MAX(self->size.width, self->title_min_w);
    }

    for (it = self->entries; it; it = it->next) {
        MenuEntry *e = it->data;
        int h;

        e->a_item->texture[0].data.text.string = e->label;
        appearance_minsize(e->a_item, &e->min_w, &self->item_h);
        self->size.width = MAX(self->size.width, e->min_w);

        e->a_disabled->texture[0].data.text.string = e->label;
        appearance_minsize(e->a_disabled, &e->min_w, &h);
        self->item_h = MAX(self->item_h, h);
        self->size.width = MAX(self->size.width, e->min_w);
	
        e->a_hilite->texture[0].data.text.string = e->label;
        appearance_minsize(e->a_hilite, &e->min_w, &h);
        self->item_h = MAX(self->item_h, h);
        self->size.width = MAX(self->size.width, e->min_w);

        e->min_w += theme_bevel * 2;
        ++nitems;
    }
    self->bullet_w = self->item_h + theme_bevel;
    self->size.width += 2 * self->bullet_w + 2 * theme_bevel;
    self->item_h += theme_bevel * 2;
    items_h = self->item_h * MAX(nitems, 1);

    if (self->label) {
	RECT_SET(self->a_title->area, 0, 0, self->size.width, 
		 self->title_h);
	RECT_SET(self->a_title->texture[0].position, 0, 0, self->size.width,
		 self->title_h);
    }

    RECT_SET(self->a_items->area, 0, 0, self->size.width, items_h);

    XResizeWindow(ob_display, self->frame, self->size.width,
		  MAX(self->title_h + items_h, 1));
    if (self->label)
	XMoveResizeWindow(ob_display, self->title, -theme_bwidth,
			  -theme_bwidth, self->size.width, self->title_h);

    XMoveResizeWindow(ob_display, self->items, 0, 
		      self->title_h + theme_bwidth, self->size.width, 
		      items_h);

    if (self->label)
	paint(self->title, self->a_title);
    paint(self->items, self->a_items);

    item_y = 0;
    for (it = self->entries; it; it = it->next) {
        ((MenuEntry*)it->data)->y = item_y;
        menu_entry_render(it->data);
        item_y += self->item_h;
    }
    
    self->size.height = item_y;
    self->invalid = FALSE;
}

void menu_entry_render(MenuEntry *self)
{
    Menu *menu = self->parent;
    Appearance *a;
    
    switch (self->render_type) {
    case MenuEntryRenderType_Submenu:
	/* TODO: submenu mask */
    case MenuEntryRenderType_Boolean:
	/* TODO: boolean check */
	a = self->enabled ? (self->hilite ? self->a_hilite : self->a_item) 
	    : self->a_disabled;
	break;
    case MenuEntryRenderType_None:
	a = self->enabled ? (self->hilite ? self->a_hilite : self->a_item )
	    : self->a_disabled;
	break;
    case MenuEntryRenderType_Separator:
	a = self->a_item;
	break;

    default:
	g_message("unhandled render_type");
	a = !self->enabled ? self->a_disabled :
        (self->hilite && 
         (self->action || self->render_type == MenuEntryRenderType_Submenu) ? 
         self->a_hilite : self->a_item);
	break;
    }

    RECT_SET(a->area, 0, 0, menu->size.width,
             menu->item_h);
    RECT_SET(a->texture[0].position, menu->bullet_w,
             0, menu->size.width - 2 * menu->bullet_w,
             menu->item_h);

    XMoveResizeWindow(ob_display, self->item, 0, self->y,
                      menu->size.width, menu->item_h);
    a->surface.data.planar.parent = menu->a_items;
    a->surface.data.planar.parentx = 0;
    a->surface.data.planar.parenty = self->y;

    paint(self->item, a);
}
