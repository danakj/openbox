/* Functions for default rendering of menus. Might become pluginnable */

#include "menu.h"
#include "openbox.h"
#include "render2/theme.h"

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
        RrSurfaceCopy(self->s_title, ob_theme->menu_title);
        RrTextureSetText(self->s_title, 0, ob_theme->title_font,
                         ob_theme->title_justify, &ob_theme->title_color_f,
                         self->label);
        RrSurfaceMinSize(self->s_title, &self->title_min_w, &self->title_h);
	self->title_min_w += (ob_theme->bevel + ob_theme->bwidth) * 2;
	self->title_h += (ob_theme->bevel + ob_theme->bwidth) * 2;
	self->size.width = MAX(self->size.width, self->title_min_w);
    }

    for (it = self->entries; it; it = it->next) {
        MenuEntry *e = it->data;
        int h;

        RrTextureSetText(e->s_text, 0, ob_theme->title_font,
                         ob_theme->title_justify, &ob_theme->title_color_f,
                         e->label);

        RrSurfaceCopy(e->s_item, ob_theme->menu_item);
        RrSurfaceMinSize(e->s_item, &e->min_w, &self->item_h);
        self->size.width = MAX(self->size.width, e->min_w);

        RrSurfaceCopy(e->s_item, ob_theme->menu_disabled);
        RrSurfaceMinSize(e->s_item, &e->min_w, &h);
        self->item_h = MAX(self->item_h, h);
        self->size.width = MAX(self->size.width, e->min_w);
	
        RrSurfaceCopy(e->s_item, ob_theme->menu_hilite);
        RrSurfaceMinSize(e->s_item, &e->min_w, &h);
        self->item_h = MAX(self->item_h, h);
        self->size.width = MAX(self->size.width, e->min_w);
	
        e->min_w += ob_theme->bevel * 2;
        ++nitems;
    }
    self->bullet_w = self->item_h + ob_theme->bevel;
    self->size.width += 2 * self->bullet_w + 2 * ob_theme->bevel;
    self->item_h += ob_theme->bevel * 2;
    items_h = self->item_h * MAX(nitems, 1);

    RrSurfaceSetArea(self->s_frame, self->location.x, self->location.y,
                     self->size.width, MAX(self->title_h + items_h, 1));

    if (self->label) {
        RrSurfaceSetArea(self->s_title, 0, 0, self->size.width, self->title_h);
        RrSurfaceShow(self->s_title);
    } else
        RrSurfaceHide(self->s_title);

    RrSurfaceSetArea(self->s_items, 0, self->title_h + ob_theme->bwidth,
                     self->size.width, items_h);

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
    struct RrSurface *s;
    
    switch (self->render_type) {
    case MenuEntryRenderType_Submenu:
	/* TODO: submenu mask */
    case MenuEntryRenderType_Boolean:
	/* TODO: boolean check */
	s = self->enabled ?
            (self->hilite ? ob_theme->menu_hilite : ob_theme->menu_item) :
	    ob_theme->menu_disabled;
	break;
    case MenuEntryRenderType_None:
	s = self->enabled ?
            (self->hilite ? ob_theme->menu_hilite : ob_theme->menu_item) :
	    ob_theme->menu_disabled;
	break;
    case MenuEntryRenderType_Separator:
	s = ob_theme->menu_item;
	break;

    default:
	g_message("unhandled render_type");
	s = !self->enabled ? ob_theme->menu_disabled :
            (self->hilite && 
             (self->action ||
              self->render_type == MenuEntryRenderType_Submenu) ? 
             ob_theme->menu_hilite : ob_theme->menu_item);
	break;
    }

    RrSurfaceCopy(self->s_item, s);

    RrSurfaceSetArea(self->s_item, 0, self->y, menu->size.width, menu->item_h);
    RrSurfaceSetArea(self->s_text, menu->bullet_w, 0,
                     menu->size.width - 2 * menu->bullet_w, menu->item_h);
}
