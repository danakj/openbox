/* Functions for default rendering of menus. Might become pluginnable */

#include "menu.h"
#include "openbox.h"
#include "render/theme.h"

void menu_render(Menu *self) {
    GList *it;
    int items_h;
    int nitems = 0; /* each item, only one is used */
    int item_y;

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

    XResizeWindow(ob_display, self->frame, self->width, 
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

    self->invalid = FALSE;
}
