/* Functions for default rendering of menus. Might become pluginnable */

#include "menu.h"
#include "openbox.h"
#include "screen.h"
#include "render2/render.h"

/* XXX temp */
static int theme_bevel = 1;
static int theme_bwidth = 0;

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
    self->size.height = 1;
    self->item_h = 1;

    /* set texture data and size them mofos out */
    if (self->label) {
        RrTextureSetText(self->s_title, 0, NULL, RR_CENTER,
                         self->label);
        RrSurfaceMinSize(self->s_title, &self->title_min_w, &self->title_h);
	self->title_min_w += theme_bevel * 2;
	self->title_h += theme_bevel * 2;
	self->size.width = MAX(self->size.width, self->title_min_w);
        self->size.height = MAX(self->size.height,
                                self->title_h + theme_bwidth);
    }

    for (it = self->entries; it; it = it->next) {
        MenuEntry *e = it->data;
        int h;

        RrTextureSetText(e->s_item, 0, NULL, RR_LEFT, e->label);
        RrSurfaceMinSize(e->s_item, &e->min_w, &self->item_h);
        self->size.width = MAX(self->size.width, e->min_w);

        RrTextureSetText(e->s_disabled, 0, NULL, RR_LEFT, e->label);
        RrSurfaceMinSize(e->s_disabled, &e->min_w, &h);
        self->item_h = MAX(self->item_h, h);
        self->size.width = MAX(self->size.width, e->min_w);
	
        RrTextureSetText(e->s_hilite, 0, NULL, RR_LEFT, e->label);
        RrSurfaceMinSize(e->s_hilite, &e->min_w, &h);
        self->item_h = MAX(self->item_h, h);
        self->size.width = MAX(self->size.width, e->min_w);

        e->min_w += theme_bevel * 2;
        ++nitems;
    }
    self->bullet_w = self->item_h + theme_bevel;
    self->size.width += 2*self->bullet_w + 2*theme_bevel + 2*theme_bwidth;
    self->item_h += theme_bevel * 2;
    items_h = self->item_h * MAX(nitems, 1);
    self->size.height += items_h + theme_bwidth * 2;

    if (self->label) {
        RrSurfaceSetArea(self->s_title, theme_bwidth, theme_bwidth,
                         self->size.width - theme_bwidth * 2,
                         self->title_h);
    }

    RrSurfaceSetArea(self->s_items, theme_bwidth,
                     theme_bwidth * 2 + self->title_h,
                     self->size.width - theme_bwidth * 2, items_h);

    
    RrSurfaceSetArea(self->s_frame, 
                     MIN(self->location.x,
                         screen_physical_size.width - self->size.width), 
                     MIN(self->location.y,
                         screen_physical_size.height - self->size.height),
                     self->size.width,
                     self->size.height);

    item_y = 0;
    for (it = self->entries; it; it = it->next) {
        MenuEntry *e = it->data;
        RrSurfaceSetArea(e->s_item, self->bullet_w, item_y,
                         self->size.width - 2*theme_bwidth - 2*self->bullet_w,
                         self->item_h);
        RrSurfaceSetArea(e->s_disabled, self->bullet_w, item_y,
                         self->size.width - 2*theme_bwidth - 2*self->bullet_w,
                         self->item_h);
        RrSurfaceSetArea(e->s_hilite, self->bullet_w, item_y,
                         self->size.width - 2*theme_bwidth - 2*self->bullet_w,
                         self->item_h);
        menu_entry_render(it->data);
        item_y += self->item_h;
    }
    
    self->invalid = FALSE;
}

void menu_entry_render(MenuEntry *self)
{
    switch (self->render_type) {
    case MenuEntryRenderType_Submenu:
	/* TODO: submenu mask */
    case MenuEntryRenderType_Boolean:
    case MenuEntryRenderType_None:
	/* TODO: boolean check */
        if (self->enabled) {
            if (self->hilite) {
                RrSurfaceHide(self->s_disabled);
                RrSurfaceHide(self->s_item);
                RrSurfaceShow(self->s_hilite);
            } else {
                RrSurfaceHide(self->s_disabled);
                RrSurfaceShow(self->s_item);
                RrSurfaceHide(self->s_hilite);
            }
        } else {
            RrSurfaceShow(self->s_disabled);
            RrSurfaceHide(self->s_item);
            RrSurfaceHide(self->s_hilite);
        }
	break;
    case MenuEntryRenderType_Separator:
        RrSurfaceHide(self->s_disabled);
        RrSurfaceShow(self->s_item);
        RrSurfaceHide(self->s_hilite);
	break;

    default:
	g_message("unhandled render_type");
        if (self->enabled) {
            if (self->hilite) {
                RrSurfaceHide(self->s_disabled);
                RrSurfaceHide(self->s_item);
                RrSurfaceShow(self->s_hilite);
            } else {
                RrSurfaceHide(self->s_disabled);
                RrSurfaceShow(self->s_item);
                RrSurfaceHide(self->s_hilite);
            }
        } else {
            RrSurfaceShow(self->s_disabled);
            RrSurfaceHide(self->s_item);
            RrSurfaceHide(self->s_hilite);
        }
/* used to be this... looks repetative from above
	a = !self->enabled ? self->a_disabled :
        (self->hilite && 
         (self->action || self->render_type == MenuEntryRenderType_Submenu) ? 
         self->a_hilite : self->a_item);
*/
	break;
    }
}
