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

    if (self->a_title == NULL) {
        XSetWindowBorderWidth(ob_display, self->frame, ob_rr_theme->bwidth);
        XSetWindowBackground(ob_display, self->frame,
                             ob_rr_theme->b_color->pixel);
        XSetWindowBorderWidth(ob_display, self->title, ob_rr_theme->bwidth);
        XSetWindowBorder(ob_display, self->frame, ob_rr_theme->b_color->pixel);
        XSetWindowBorder(ob_display, self->title, ob_rr_theme->b_color->pixel);

        self->a_title = RrAppearanceCopy(ob_rr_theme->a_menu_title);
        self->a_items = RrAppearanceCopy(ob_rr_theme->a_menu);
    }
    
    /* set texture data and size them mofos out */
    if (self->label) {
	self->a_title->texture[0].data.text.string = self->label;
	RrMinsize(self->a_title, &self->title_min_w, &self->title_h);
	self->title_min_w += ob_rr_theme->bevel * 2;
	self->title_h += ob_rr_theme->bevel * 2;
	self->size.width = MAX(self->size.width, self->title_min_w);
    }

    for (it = self->entries; it; it = it->next) {
        MenuEntry *e = it->data;
        int h;

        if (e->a_item == NULL) {
            e->a_item = RrAppearanceCopy(ob_rr_theme->a_menu_item);
            e->a_disabled = RrAppearanceCopy(ob_rr_theme->a_menu_disabled);
            e->a_hilite = RrAppearanceCopy(ob_rr_theme->a_menu_hilite);
        }

        e->a_item->texture[0].data.text.string = e->label;
        RrMinsize(e->a_item, &e->min_w, &self->item_h);
        self->size.width = MAX(self->size.width, e->min_w);

        e->a_disabled->texture[0].data.text.string = e->label;
        RrMinsize(e->a_disabled, &e->min_w, &h);
        self->item_h = MAX(self->item_h, h);
        self->size.width = MAX(self->size.width, e->min_w);
	
        e->a_hilite->texture[0].data.text.string = e->label;
        RrMinsize(e->a_hilite, &e->min_w, &h);
        self->item_h = MAX(self->item_h, h);
        self->size.width = MAX(self->size.width, e->min_w);

        e->min_w += ob_rr_theme->bevel * 2;
        ++nitems;
    }
    self->bullet_w = self->item_h + ob_rr_theme->bevel;
    self->size.width += 2 * self->bullet_w + 2 * ob_rr_theme->bevel;
    self->item_h += ob_rr_theme->bevel * 2;
    items_h = self->item_h * MAX(nitems, 1);

    XResizeWindow(ob_display, self->frame, self->size.width,
		  MAX(self->title_h + items_h, 1));
    if (self->label)
	XMoveResizeWindow(ob_display, self->title, -ob_rr_theme->bwidth,
			  -ob_rr_theme->bwidth,
                          self->size.width, self->title_h);

    XMoveResizeWindow(ob_display, self->items, 0, 
		      self->title_h + ob_rr_theme->bwidth, self->size.width, 
		      items_h);

    if (self->label)
	RrPaint(self->a_title, self->title, self->size.width, self->title_h);
    RrPaint(self->a_items, self->items, self->size.width, items_h);

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
    RrAppearance *a;
    
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

    XMoveResizeWindow(ob_display, self->item, 0, self->y,
                      menu->size.width, menu->item_h);

    a->surface.parent = menu->a_items;
    a->surface.parentx = 0;
    a->surface.parenty = self->y;

    RrPaint(a, self->item, menu->size.width, menu->item_h);
}
