/* Functions for default rendering of menus. Might become pluginnable */

#include "debug.h"
#include "menu.h"
#include "openbox.h"
#include "render/theme.h"

void menu_render(ObMenu *self) {
    GList *it;
    int items_h = 0;
    int nitems = 0; /* each item, only one is used */
    int item_y;

    self->size.width = 1;
    self->item_h = 1;

    if (self->a_title == NULL) {
        XSetWindowBorderWidth(ob_display, self->frame, ob_rr_theme->bwidth);
        XSetWindowBackground(ob_display, self->frame,
                             RrColorPixel(ob_rr_theme->b_color));
        XSetWindowBorderWidth(ob_display, self->title, ob_rr_theme->bwidth);
        XSetWindowBorder(ob_display, self->frame,
                         RrColorPixel(ob_rr_theme->b_color));
        XSetWindowBorder(ob_display, self->title,
                         RrColorPixel(ob_rr_theme->b_color));

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
        ObMenuEntry *e = it->data;
        int h;

        if (e->a_item == NULL) {
            e->a_item = RrAppearanceCopy(ob_rr_theme->a_menu_item);
            e->a_disabled = RrAppearanceCopy(ob_rr_theme->a_menu_disabled);
            e->a_hilite = RrAppearanceCopy(ob_rr_theme->a_menu_hilite);
            e->a_submenu = RrAppearanceCopy(ob_rr_theme->a_menu_bullet);
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

    self->size.height = MAX(self->title_h + items_h + ob_rr_theme->bwidth, 1);
    XResizeWindow(ob_display, self->frame, self->size.width,self->size.height);
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
        ((ObMenuEntry*)it->data)->y = item_y;
        menu_entry_render(it->data);
        item_y += self->item_h;
    }
    
    self->invalid = FALSE;
}

void menu_entry_render(ObMenuEntry *self)
{
    ObMenu *menu = self->parent;
    RrAppearance *a, *s = NULL;
    
    switch (self->render_type) {
    case OB_MENU_ENTRY_RENDER_TYPE_SUBMENU:
        a = self->enabled ? (self->hilite ? self->a_hilite : self->a_item)
            : self->a_disabled;
        s = self->a_submenu;
        break;
    case OB_MENU_ENTRY_RENDER_TYPE_BOOLEAN:
	/* TODO: boolean check */
	a = self->enabled ? (self->hilite ? self->a_hilite : self->a_item) 
	    : self->a_disabled;
	break;
    case OB_MENU_ENTRY_RENDER_TYPE_NONE:
	a = self->enabled ? (self->hilite ? self->a_hilite : self->a_item )
	    : self->a_disabled;
	break;
    case OB_MENU_ENTRY_RENDER_TYPE_SEPARATOR:
	a = self->a_item;
	break;

    default:
	g_assert_not_reached(); /* unhandled rendering type */
	break;
    }

    XMoveResizeWindow(ob_display, self->item, 0, self->y,
                      menu->size.width, menu->item_h);
    XMoveResizeWindow(ob_display, self->submenu_pic, menu->size.width - ob_rr_theme->bevel - 1, self->y,
                      8, 8);
    a->surface.parent = menu->a_items;
    a->surface.parentx = 0;
    a->surface.parenty = self->y;

    if (s) {
        s->surface.parent = a;
        s->surface.parentx = menu->size.width - 8;
        s->surface.parenty = 0;
    }

    RrPaint(a, self->item, menu->size.width, menu->item_h);

    if (s) 
        RrPaint(s, self->submenu_pic, 8, 8);
}
