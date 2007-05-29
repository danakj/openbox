/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   menuframe.c for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
   Copyright (c) 2003-2007   Dana Jansens

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   See the COPYING file for a copy of the GNU General Public License.
*/

#include "menuframe.h"
#include "client.h"
#include "menu.h"
#include "screen.h"
#include "grab.h"
#include "openbox.h"
#include "mainloop.h"
#include "config.h"
#include "render/theme.h"

#define PADDING 2
#define SEPARATOR_HEIGHT 3
#define MAX_MENU_WIDTH 400

#define ITEM_HEIGHT (ob_rr_theme->menu_font_height + 2*PADDING)

#define FRAME_EVENTMASK (ButtonPressMask |ButtonMotionMask | EnterWindowMask |\
                         LeaveWindowMask)
#define ENTRY_EVENTMASK (EnterWindowMask | LeaveWindowMask | \
                         ButtonPressMask | ButtonReleaseMask)

GList *menu_frame_visible;

static ObMenuEntryFrame* menu_entry_frame_new(ObMenuEntry *entry,
                                              ObMenuFrame *frame);
static void menu_entry_frame_free(ObMenuEntryFrame *self);
static void menu_frame_update(ObMenuFrame *self);
static gboolean menu_entry_frame_submenu_timeout(gpointer data);
static void menu_frame_hide(ObMenuFrame *self);

static Window createWindow(Window parent, gulong mask,
                           XSetWindowAttributes *attrib)
{
    return XCreateWindow(ob_display, parent, 0, 0, 1, 1, 0,
                         RrDepth(ob_rr_inst), InputOutput,
                         RrVisual(ob_rr_inst), mask, attrib);
}

GHashTable *menu_frame_map;

void menu_frame_startup(gboolean reconfig)
{
    if (reconfig) return;

    menu_frame_map = g_hash_table_new(g_int_hash, g_int_equal);
}

void menu_frame_shutdown(gboolean reconfig)
{
    if (reconfig) return;

    g_hash_table_destroy(menu_frame_map);
}

ObMenuFrame* menu_frame_new(ObMenu *menu, guint show_from, ObClient *client)
{
    ObMenuFrame *self;
    XSetWindowAttributes attr;

    self = g_new0(ObMenuFrame, 1);
    self->type = Window_Menu;
    self->menu = menu;
    self->selected = NULL;
    self->client = client;
    self->direction_right = TRUE;
    self->show_from = show_from;

    attr.event_mask = FRAME_EVENTMASK;
    self->window = createWindow(RootWindow(ob_display, ob_screen),
                                CWEventMask, &attr);

    XSetWindowBorderWidth(ob_display, self->window, ob_rr_theme->mbwidth);
    XSetWindowBorder(ob_display, self->window,
                     RrColorPixel(ob_rr_theme->menu_border_color));

    self->a_title = RrAppearanceCopy(ob_rr_theme->a_menu_title);
    self->a_items = RrAppearanceCopy(ob_rr_theme->a_menu);

    stacking_add(MENU_AS_WINDOW(self));

    return self;
}

void menu_frame_free(ObMenuFrame *self)
{
    if (self) {
        while (self->entries) {
            menu_entry_frame_free(self->entries->data);
            self->entries = g_list_delete_link(self->entries, self->entries);
        }

        stacking_remove(MENU_AS_WINDOW(self));

        XDestroyWindow(ob_display, self->window);

        RrAppearanceFree(self->a_items);
        RrAppearanceFree(self->a_title);

        g_free(self);
    }
}

static ObMenuEntryFrame* menu_entry_frame_new(ObMenuEntry *entry,
                                              ObMenuFrame *frame)
{
    ObMenuEntryFrame *self;
    XSetWindowAttributes attr;

    self = g_new0(ObMenuEntryFrame, 1);
    self->entry = entry;
    self->frame = frame;

    menu_entry_ref(entry);

    attr.event_mask = ENTRY_EVENTMASK;
    self->window = createWindow(self->frame->window, CWEventMask, &attr);
    self->text = createWindow(self->window, 0, NULL);
    g_hash_table_insert(menu_frame_map, &self->window, self);
    g_hash_table_insert(menu_frame_map, &self->text, self);
    if (entry->type == OB_MENU_ENTRY_TYPE_NORMAL) {
        self->icon = createWindow(self->window, 0, NULL);
        g_hash_table_insert(menu_frame_map, &self->icon, self);
    }
    if (entry->type == OB_MENU_ENTRY_TYPE_SUBMENU) {
        self->bullet = createWindow(self->window, 0, NULL);
        g_hash_table_insert(menu_frame_map, &self->bullet, self);
    }

    XMapWindow(ob_display, self->window);
    XMapWindow(ob_display, self->text);

    self->a_normal = RrAppearanceCopy(ob_rr_theme->a_menu_normal);
    self->a_selected = RrAppearanceCopy(ob_rr_theme->a_menu_selected);
    self->a_disabled = RrAppearanceCopy(ob_rr_theme->a_menu_disabled);
    self->a_disabled_selected =
        RrAppearanceCopy(ob_rr_theme->a_menu_disabled_selected);

    if (entry->type == OB_MENU_ENTRY_TYPE_SEPARATOR) {
        self->a_separator = RrAppearanceCopy(ob_rr_theme->a_clear_tex);
        self->a_separator->texture[0].type = RR_TEXTURE_LINE_ART;
    } else {
        self->a_icon = RrAppearanceCopy(ob_rr_theme->a_clear_tex);
        self->a_icon->texture[0].type = RR_TEXTURE_RGBA;
        self->a_mask = RrAppearanceCopy(ob_rr_theme->a_clear_tex);
        self->a_mask->texture[0].type = RR_TEXTURE_MASK;
        self->a_bullet_normal =
            RrAppearanceCopy(ob_rr_theme->a_menu_bullet_normal);
        self->a_bullet_selected =
            RrAppearanceCopy(ob_rr_theme->a_menu_bullet_selected);
    }

    self->a_text_normal =
        RrAppearanceCopy(ob_rr_theme->a_menu_text_normal);
    self->a_text_selected =
        RrAppearanceCopy(ob_rr_theme->a_menu_text_selected);
    self->a_text_disabled =
        RrAppearanceCopy(ob_rr_theme->a_menu_text_disabled);
    self->a_text_disabled_selected =
        RrAppearanceCopy(ob_rr_theme->a_menu_text_disabled_selected);
    self->a_text_title =
        RrAppearanceCopy(ob_rr_theme->a_menu_text_title);

    return self;
}

static void menu_entry_frame_free(ObMenuEntryFrame *self)
{
    if (self) {
        menu_entry_unref(self->entry);

        XDestroyWindow(ob_display, self->text);
        XDestroyWindow(ob_display, self->window);
        g_hash_table_remove(menu_frame_map, &self->text);
        g_hash_table_remove(menu_frame_map, &self->window);
        if (self->entry->type == OB_MENU_ENTRY_TYPE_NORMAL) {
            XDestroyWindow(ob_display, self->icon);
            g_hash_table_remove(menu_frame_map, &self->icon);
        }
        if (self->entry->type == OB_MENU_ENTRY_TYPE_SUBMENU) {
            XDestroyWindow(ob_display, self->bullet);
            g_hash_table_remove(menu_frame_map, &self->bullet);
        }

        RrAppearanceFree(self->a_normal);
        RrAppearanceFree(self->a_selected);
        RrAppearanceFree(self->a_disabled);
        RrAppearanceFree(self->a_disabled_selected);

        RrAppearanceFree(self->a_separator);
        RrAppearanceFree(self->a_icon);
        RrAppearanceFree(self->a_mask);
        RrAppearanceFree(self->a_text_normal);
        RrAppearanceFree(self->a_text_selected);
        RrAppearanceFree(self->a_text_disabled);
        RrAppearanceFree(self->a_text_disabled_selected);
        RrAppearanceFree(self->a_text_title);
        RrAppearanceFree(self->a_bullet_normal);
        RrAppearanceFree(self->a_bullet_selected);

        g_free(self);
    }
}

void menu_frame_move(ObMenuFrame *self, gint x, gint y)
{
    RECT_SET_POINT(self->area, x, y);
    XMoveWindow(ob_display, self->window, self->area.x, self->area.y);
}

static void menu_frame_place_topmenu(ObMenuFrame *self, gint *x, gint *y)
{
    gint dx, dy;

    if (config_menu_middle) {
        gint myx;

        myx = *x;
        *y -= self->area.height / 2;

        /* try to the right of the cursor */
        menu_frame_move_on_screen(self, myx, *y, &dx, &dy);
        self->direction_right = TRUE;
        if (dx != 0) {
            /* try to the left of the cursor */
            myx = *x - self->area.width;
            menu_frame_move_on_screen(self, myx, *y, &dx, &dy);
            self->direction_right = FALSE;
        }
        if (dx != 0) {
            /* if didnt fit on either side so just use what it says */
            myx = *x;
            menu_frame_move_on_screen(self, myx, *y, &dx, &dy);
            self->direction_right = TRUE;
        }
        *x = myx + dx;
        *y += dy;
    } else {
        gint myx, myy;

        myx = *x;
        myy = *y;

        /* try to the bottom right of the cursor */
        menu_frame_move_on_screen(self, myx, myy, &dx, &dy);
        self->direction_right = TRUE;
        if (dx != 0 || dy != 0) {
            /* try to the bottom left of the cursor */
            myx = *x - self->area.width;
            myy = *y;
            menu_frame_move_on_screen(self, myx, myy, &dx, &dy);
            self->direction_right = FALSE;
        }
        if (dx != 0 || dy != 0) {
            /* try to the top right of the cursor */
            myx = *x;
            myy = *y - self->area.height;
            menu_frame_move_on_screen(self, myx, myy, &dx, &dy);
            self->direction_right = TRUE;
        }
        if (dx != 0 || dy != 0) {
            /* try to the top left of the cursor */
            myx = *x - self->area.width;
            myy = *y - self->area.height;
            menu_frame_move_on_screen(self, myx, myy, &dx, &dy);
            self->direction_right = FALSE;
        }
        if (dx != 0 || dy != 0) {
            /* if didnt fit on either side so just use what it says */
            myx = *x;
            myy = *y;
            menu_frame_move_on_screen(self, myx, myy, &dx, &dy);
            self->direction_right = TRUE;
        }
        *x = myx + dx;
        *y = myy + dy;
    }
}

static void menu_frame_place_submenu(ObMenuFrame *self, gint *x, gint *y)
{
    gint overlap;
    gint bwidth;

    overlap = ob_rr_theme->menu_overlap;
    bwidth = ob_rr_theme->mbwidth;

    if (self->direction_right)
        *x = self->parent->area.x + self->parent->area.width -
            overlap - bwidth;
    else
        *x = self->parent->area.x - self->area.width + overlap + bwidth;

    *y = self->parent->area.y + self->parent_entry->area.y;
    if (config_menu_middle)
        *y -= (self->area.height - (bwidth * 2) - ITEM_HEIGHT) / 2;
    else
        *y += overlap;
}

void menu_frame_move_on_screen(ObMenuFrame *self, gint x, gint y,
                               gint *dx, gint *dy)
{
    Rect *a = NULL;
    gint pos, half;

    *dx = *dy = 0;

    a = screen_physical_area_monitor(self->monitor);

    half = g_list_length(self->entries) / 2;
    pos = g_list_index(self->entries, self->selected);

    /* if in the bottom half then check this stuff first, will keep the bottom
       edge of the menu visible */
    if (pos > half) {
        *dx = MAX(*dx, a->x - x);
        *dy = MAX(*dy, a->y - y);
    }
    *dx = MIN(*dx, (a->x + a->width) - (x + self->area.width));
    *dy = MIN(*dy, (a->y + a->height) - (y + self->area.height));
    /* if in the top half then check this stuff last, will keep the top
       edge of the menu visible */
    if (pos <= half) {
        *dx = MAX(*dx, a->x - x);
        *dy = MAX(*dy, a->y - y);
    }
}

static void menu_entry_frame_render(ObMenuEntryFrame *self)
{
    RrAppearance *item_a, *text_a;
    gint th; /* temp */
    ObMenu *sub;
    ObMenuFrame *frame = self->frame;

    switch (self->entry->type) {
    case OB_MENU_ENTRY_TYPE_NORMAL:
    case OB_MENU_ENTRY_TYPE_SUBMENU:
        item_a = (self->entry->type == OB_MENU_ENTRY_TYPE_NORMAL &&
                  !self->entry->data.normal.enabled ?
                  /* disabled */
                  (self == self->frame->selected ?
                   self->a_disabled_selected : self->a_disabled) :
                  /* enabled */
                  (self == self->frame->selected ?
                   self->a_selected : self->a_normal));
        th = ITEM_HEIGHT;
        break;
    case OB_MENU_ENTRY_TYPE_SEPARATOR:
        if (self->entry->data.separator.label) {
            item_a = self->frame->a_title;
            th = ob_rr_theme->menu_title_height;
        } else {
            item_a = self->a_normal;
            th = SEPARATOR_HEIGHT + 2*PADDING;
        }
        break;
    default:
        g_assert_not_reached();
    }
    RECT_SET_SIZE(self->area, self->frame->inner_w, th);
    XResizeWindow(ob_display, self->window,
                  self->area.width, self->area.height);
    item_a->surface.parent = self->frame->a_items;
    item_a->surface.parentx = self->area.x;
    item_a->surface.parenty = self->area.y;
    RrPaint(item_a, self->window, self->area.width, self->area.height);

    switch (self->entry->type) {
    case OB_MENU_ENTRY_TYPE_NORMAL:
        text_a = (self->entry->type == OB_MENU_ENTRY_TYPE_NORMAL &&
                  !self->entry->data.normal.enabled ?
                  /* disabled */
                  (self == self->frame->selected ?
                   self->a_text_disabled_selected : self->a_text_disabled) :
                  /* enabled */
                  (self == self->frame->selected ?
                   self->a_text_selected : self->a_text_normal));
        text_a->texture[0].data.text.string = self->entry->data.normal.label;
        if (self->entry->data.normal.shortcut &&
            (self->frame->menu->show_all_shortcuts ||
             self->entry->data.normal.shortcut_position > 0))
        {
            text_a->texture[0].data.text.shortcut = TRUE;
            text_a->texture[0].data.text.shortcut_pos =
                self->entry->data.normal.shortcut_position;
        } else
            text_a->texture[0].data.text.shortcut = FALSE;
        break;
    case OB_MENU_ENTRY_TYPE_SUBMENU:
        text_a = (self == self->frame->selected ?
                  self->a_text_selected :
                  self->a_text_normal);
        sub = self->entry->data.submenu.submenu;
        text_a->texture[0].data.text.string = sub ? sub->title : "";
        if (sub->shortcut && (self->frame->menu->show_all_shortcuts ||
                              sub->shortcut_position > 0))
        {
            text_a->texture[0].data.text.shortcut = TRUE;
            text_a->texture[0].data.text.shortcut_pos = sub->shortcut_position;
        } else
            text_a->texture[0].data.text.shortcut = FALSE;
        break;
    case OB_MENU_ENTRY_TYPE_SEPARATOR:
        if (self->entry->data.separator.label != NULL)
            text_a = self->a_text_title;
        else
            text_a = self->a_text_normal;
        break;
    }

    switch (self->entry->type) {
    case OB_MENU_ENTRY_TYPE_NORMAL:
        XMoveResizeWindow(ob_display, self->text,
                          self->frame->text_x, PADDING,
                          self->frame->text_w,
                          ITEM_HEIGHT - 2*PADDING);
        text_a->surface.parent = item_a;
        text_a->surface.parentx = self->frame->text_x;
        text_a->surface.parenty = PADDING;
        RrPaint(text_a, self->text, self->frame->text_w,
                ITEM_HEIGHT - 2*PADDING);
        break;
    case OB_MENU_ENTRY_TYPE_SUBMENU:
        XMoveResizeWindow(ob_display, self->text,
                          self->frame->text_x, PADDING,
                          self->frame->text_w - ITEM_HEIGHT,
                          ITEM_HEIGHT - 2*PADDING);
        text_a->surface.parent = item_a;
        text_a->surface.parentx = self->frame->text_x;
        text_a->surface.parenty = PADDING;
        RrPaint(text_a, self->text, self->frame->text_w - ITEM_HEIGHT,
                ITEM_HEIGHT - 2*PADDING);
        break;
    case OB_MENU_ENTRY_TYPE_SEPARATOR:
        if (self->entry->data.separator.label != NULL) {
            /* labeled separator */
            XMoveResizeWindow(ob_display, self->text,
                              ob_rr_theme->paddingx, ob_rr_theme->paddingy,
                              self->area.width - 2*ob_rr_theme->paddingx,
                              ob_rr_theme->menu_title_height -
                              2*ob_rr_theme->paddingy);
            text_a->surface.parent = item_a;
            text_a->surface.parentx = ob_rr_theme->paddingx;
            text_a->surface.parenty = ob_rr_theme->paddingy;
            RrPaint(text_a, self->text,
                    self->area.width - 2*ob_rr_theme->paddingx,
                    ob_rr_theme->menu_title_height -
                    2*ob_rr_theme->paddingy);
        } else {
            /* unlabeled separaator */
            XMoveResizeWindow(ob_display, self->text, PADDING, PADDING,
                              self->area.width - 2*PADDING, SEPARATOR_HEIGHT);
            self->a_separator->surface.parent = item_a;
            self->a_separator->surface.parentx = PADDING;
            self->a_separator->surface.parenty = PADDING;
            self->a_separator->texture[0].data.lineart.color =
                text_a->texture[0].data.text.color;
            self->a_separator->texture[0].data.lineart.x1 = 2*PADDING;
            self->a_separator->texture[0].data.lineart.y1 = SEPARATOR_HEIGHT/2;
            self->a_separator->texture[0].data.lineart.x2 =
                self->area.width - 4*PADDING;
            self->a_separator->texture[0].data.lineart.y2 = SEPARATOR_HEIGHT/2;
            RrPaint(self->a_separator, self->text,
                    self->area.width - 2*PADDING, SEPARATOR_HEIGHT);
        }
        break;
    }

    if (self->entry->type == OB_MENU_ENTRY_TYPE_NORMAL &&
        self->entry->data.normal.icon_data)
    {
        XMoveResizeWindow(ob_display, self->icon,
                          PADDING, frame->item_margin.top,
                          ITEM_HEIGHT - frame->item_margin.top
                          - frame->item_margin.bottom,
                          ITEM_HEIGHT - frame->item_margin.top
                          - frame->item_margin.bottom);
        self->a_icon->texture[0].data.rgba.width =
            self->entry->data.normal.icon_width;
        self->a_icon->texture[0].data.rgba.height =
            self->entry->data.normal.icon_height;
        self->a_icon->texture[0].data.rgba.width =
            self->entry->data.normal.icon_alpha;
        self->a_icon->texture[0].data.rgba.data =
            self->entry->data.normal.icon_data;
        self->a_icon->surface.parent = item_a;
        self->a_icon->surface.parentx = PADDING;
        self->a_icon->surface.parenty = frame->item_margin.top;
        RrPaint(self->a_icon, self->icon,
                ITEM_HEIGHT - frame->item_margin.top
                - frame->item_margin.bottom,
                ITEM_HEIGHT - frame->item_margin.top
                - frame->item_margin.bottom);
        XMapWindow(ob_display, self->icon);
    } else if (self->entry->type == OB_MENU_ENTRY_TYPE_NORMAL &&
               self->entry->data.normal.mask)
    {
        RrColor *c;

        XMoveResizeWindow(ob_display, self->icon,
                          PADDING, frame->item_margin.top,
                          ITEM_HEIGHT - frame->item_margin.top
                          - frame->item_margin.bottom,
                          ITEM_HEIGHT - frame->item_margin.top
                          - frame->item_margin.bottom);
        self->a_mask->texture[0].data.mask.mask =
            self->entry->data.normal.mask;

        c = (self->entry->type == OB_MENU_ENTRY_TYPE_NORMAL &&
             !self->entry->data.normal.enabled ?
             /* disabled */
             (self == self->frame->selected ?
              self->entry->data.normal.mask_disabled_selected_color :
              self->entry->data.normal.mask_disabled_color) :
             /* enabled */
             (self == self->frame->selected ?
              self->entry->data.normal.mask_selected_color :
              self->entry->data.normal.mask_normal_color));
        self->a_mask->texture[0].data.mask.color = c;

        self->a_mask->surface.parent = item_a;
        self->a_mask->surface.parentx = PADDING;
        self->a_mask->surface.parenty = frame->item_margin.top;
        RrPaint(self->a_mask, self->icon,
                ITEM_HEIGHT - frame->item_margin.top
                - frame->item_margin.bottom,
                ITEM_HEIGHT - frame->item_margin.top
                - frame->item_margin.bottom);
        XMapWindow(ob_display, self->icon);
    } else
        XUnmapWindow(ob_display, self->icon);

    if (self->entry->type == OB_MENU_ENTRY_TYPE_SUBMENU) {
        RrAppearance *bullet_a;
        XMoveResizeWindow(ob_display, self->bullet,
                          self->frame->text_x + self->frame->text_w -
                          ITEM_HEIGHT + PADDING, PADDING,
                          ITEM_HEIGHT - 2*PADDING,
                          ITEM_HEIGHT - 2*PADDING);
        bullet_a = (self == self->frame->selected ?
                    self->a_bullet_selected :
                    self->a_bullet_normal);
        bullet_a->surface.parent = item_a;
        bullet_a->surface.parentx =
            self->frame->text_x + self->frame->text_w - ITEM_HEIGHT + PADDING;
        bullet_a->surface.parenty = PADDING;
        RrPaint(bullet_a, self->bullet,
                ITEM_HEIGHT - 2*PADDING,
                ITEM_HEIGHT - 2*PADDING);
        XMapWindow(ob_display, self->bullet);
    } else
        XUnmapWindow(ob_display, self->bullet);

    XFlush(ob_display);
}

/*! this code is taken from the menu_frame_render. if that changes, this won't
  work.. */
static gint menu_entry_frame_get_height(ObMenuEntryFrame *self,
                                        gboolean first_entry,
                                        gboolean last_entry)
{
    ObMenuEntryType t;
    gint h = 0;

    h += 2*PADDING;

    if (self)
        t = self->entry->type;
    else
        /* this is the More... entry, it's NORMAL type */
        t = OB_MENU_ENTRY_TYPE_NORMAL;

    switch (t) {
    case OB_MENU_ENTRY_TYPE_NORMAL:
    case OB_MENU_ENTRY_TYPE_SUBMENU:
        h += ob_rr_theme->menu_font_height;
        break;
    case OB_MENU_ENTRY_TYPE_SEPARATOR:
        if (self->entry->data.separator.label != NULL) {
            h += ob_rr_theme->menu_title_height +
                (ob_rr_theme->mbwidth - PADDING) * 2;
 
            /* if the first entry is a labeled separator, then make its border
               overlap with the menu's outside border */
            if (first_entry)
                h -= ob_rr_theme->mbwidth;
            /* if the last entry is a labeled separator, then make its border
               overlap with the menu's outside border */
            if (last_entry)
                h -= ob_rr_theme->mbwidth;
        } else {
            h += SEPARATOR_HEIGHT;
        }
        break;
    }

    return h;
}

void menu_frame_render(ObMenuFrame *self)
{
    gint w = 0, h = 0;
    gint tw, th; /* temps */
    GList *it;
    gboolean has_icon = FALSE;
    ObMenu *sub;
    ObMenuEntryFrame *e;

    /* find text dimensions */

    STRUT_SET(self->item_margin, 0, 0, 0, 0);

    if (self->entries) {
        ObMenuEntryFrame *e = self->entries->data;
        gint l, t, r, b;

        e->a_text_normal->texture[0].data.text.string = "";
        tw = RrMinWidth(e->a_text_normal);
        tw += 2*PADDING;

        th = ITEM_HEIGHT;

        RrMargins(e->a_normal, &l, &t, &r, &b);
        STRUT_SET(self->item_margin,
                  MAX(self->item_margin.left, l),
                  MAX(self->item_margin.top, t),
                  MAX(self->item_margin.right, r),
                  MAX(self->item_margin.bottom, b));
        RrMargins(e->a_selected, &l, &t, &r, &b);
        STRUT_SET(self->item_margin,
                  MAX(self->item_margin.left, l),
                  MAX(self->item_margin.top, t),
                  MAX(self->item_margin.right, r),
                  MAX(self->item_margin.bottom, b));
        RrMargins(e->a_disabled, &l, &t, &r, &b);
        STRUT_SET(self->item_margin,
                  MAX(self->item_margin.left, l),
                  MAX(self->item_margin.top, t),
                  MAX(self->item_margin.right, r),
                  MAX(self->item_margin.bottom, b));
        RrMargins(e->a_disabled_selected, &l, &t, &r, &b);
        STRUT_SET(self->item_margin,
                  MAX(self->item_margin.left, l),
                  MAX(self->item_margin.top, t),
                  MAX(self->item_margin.right, r),
                  MAX(self->item_margin.bottom, b));
    }

    /* render the entries */

    for (it = self->entries; it; it = g_list_next(it)) {
        RrAppearance *text_a;
        e = it->data;

        /* if the first entry is a labeled separator, then make its border
           overlap with the menu's outside border */
        if (it == self->entries &&
            e->entry->type == OB_MENU_ENTRY_TYPE_SEPARATOR &&
            e->entry->data.separator.label)
        {
            h -= ob_rr_theme->mbwidth;
        }

        if (e->entry->type == OB_MENU_ENTRY_TYPE_SEPARATOR &&
            e->entry->data.separator.label)
        {
            e->border = ob_rr_theme->mbwidth;
        }

        RECT_SET_POINT(e->area, 0, h+e->border);
        XMoveWindow(ob_display, e->window, e->area.x-e->border, e->area.y-e->border);
        XSetWindowBorderWidth(ob_display, e->window, e->border);
        XSetWindowBorder(ob_display, e->window,
                         RrColorPixel(ob_rr_theme->menu_border_color));


        text_a = (e->entry->type == OB_MENU_ENTRY_TYPE_NORMAL &&
                  !e->entry->data.normal.enabled ?
                  /* disabled */
                  (e == self->selected ?
                   e->a_text_disabled_selected : e->a_text_disabled) :
                  /* enabled */
                  (e == self->selected ?
                   e->a_text_selected : e->a_text_normal));
        switch (e->entry->type) {
        case OB_MENU_ENTRY_TYPE_NORMAL:
            text_a->texture[0].data.text.string = e->entry->data.normal.label;
            tw = RrMinWidth(text_a);
            tw = MIN(tw, MAX_MENU_WIDTH);
            th = ob_rr_theme->menu_font_height;

            if (e->entry->data.normal.icon_data ||
                e->entry->data.normal.mask)
                has_icon = TRUE;
            break;
        case OB_MENU_ENTRY_TYPE_SUBMENU:
            sub = e->entry->data.submenu.submenu;
            text_a->texture[0].data.text.string = sub ? sub->title : "";
            tw = RrMinWidth(text_a);
            tw = MIN(tw, MAX_MENU_WIDTH);
            th = ob_rr_theme->menu_font_height;

            if (e->entry->data.normal.icon_data ||
                e->entry->data.normal.mask)
                has_icon = TRUE;

            tw += ITEM_HEIGHT - PADDING;
            break;
        case OB_MENU_ENTRY_TYPE_SEPARATOR:
            if (e->entry->data.separator.label != NULL) {
                e->a_text_title->texture[0].data.text.string =
                    e->entry->data.separator.label;
                tw = RrMinWidth(e->a_text_title);
                tw = MIN(tw, MAX_MENU_WIDTH);
                th = ob_rr_theme->menu_title_height +
                    (ob_rr_theme->mbwidth - PADDING) *2;
            } else {
                tw = 0;
                th = SEPARATOR_HEIGHT;
            }
            break;
        }
        tw += 2*PADDING;
        th += 2*PADDING;
        w = MAX(w, tw);
        h += th;
    }

    /* if the last entry is a labeled separator, then make its border
       overlap with the menu's outside border */
    it = g_list_last(self->entries);
    e = it ? it->data : NULL;
    if (e && e->entry->type == OB_MENU_ENTRY_TYPE_SEPARATOR &&
        e->entry->data.separator.label)
    {
        h -= ob_rr_theme->mbwidth;
    }

    self->text_x = PADDING;
    self->text_w = w;

    if (self->entries) {
        if (has_icon) {
            w += ITEM_HEIGHT + PADDING;
            self->text_x += ITEM_HEIGHT + PADDING;
        }
    }

    if (!w) w = 10;
    if (!h) h = 3;

    XResizeWindow(ob_display, self->window, w, h);

    self->inner_w = w;

    RrPaint(self->a_items, self->window, w, h);

    for (it = self->entries; it; it = g_list_next(it))
        menu_entry_frame_render(it->data);

    w += ob_rr_theme->mbwidth * 2;
    h += ob_rr_theme->mbwidth * 2;

    RECT_SET_SIZE(self->area, w, h);

    XFlush(ob_display);
}

static void menu_frame_update(ObMenuFrame *self)
{
    GList *mit, *fit;
    Rect *a;
    gint h;

    menu_pipe_execute(self->menu);
    menu_find_submenus(self->menu);

    self->selected = NULL;

    /* start at show_from */
    mit = g_list_nth(self->menu->entries, self->show_from);

    /* go through the menu's and frame's entries and connect the frame entries
       to the menu entries */
    for (fit = self->entries; mit && fit;
         mit = g_list_next(mit), fit = g_list_next(fit))
    {
        ObMenuEntryFrame *f = fit->data;
        f->entry = mit->data;
    }

    /* if there are more menu entries than in the frame, add them */
    while (mit) {
        ObMenuEntryFrame *e = menu_entry_frame_new(mit->data, self);
        self->entries = g_list_append(self->entries, e);
        mit = g_list_next(mit);
    }

    /* if there are more frame entries than menu entries then get rid of
       them */
    while (fit) {
        GList *n = g_list_next(fit);
        menu_entry_frame_free(fit->data);
        self->entries = g_list_delete_link(self->entries, fit);
        fit = n;
    }

    /* * make the menu fit on the screen */

    /* calculate the height of the menu */
    h = 0;
    for (fit = self->entries; fit; fit = g_list_next(fit))
        h += menu_entry_frame_get_height(fit->data,
                                         fit == self->entries,
                                         g_list_next(fit) == NULL);
    /* add the border at the top and bottom */
    h += ob_rr_theme->mbwidth * 2;

    a = screen_physical_area_monitor(self->monitor);

    if (h > a->height) {
        GList *flast, *tmp;
        gboolean last_entry = TRUE;

        /* take the height of our More... entry into account */
        h += menu_entry_frame_get_height(NULL, FALSE, TRUE);

        /* start at the end of the entries */
        flast = g_list_last(self->entries);

        /* pull out all the entries from the frame that don't
           fit on the screen, leaving at least 1 though */
        while (h > a->height && g_list_previous(flast) != NULL) {
            /* update the height, without this entry */
            h -= menu_entry_frame_get_height(flast->data, FALSE, last_entry);

            /* destroy the entry we're not displaying */
            tmp = flast;
            flast = g_list_previous(flast);
            menu_entry_frame_free(tmp->data);
            self->entries = g_list_delete_link(self->entries, tmp);

            /* only the first one that we see is the last entry in the menu */
            last_entry = FALSE;
        };

        {
            ObMenuEntry *more_entry;
            ObMenuEntryFrame *more_frame;
            /* make the More... menu entry frame which will display in this
               frame.
               if self->menu->more_menu is NULL that means that this is already
               More... menu, so just use ourself.
            */
            more_entry = menu_get_more((self->menu->more_menu ?
                                        self->menu->more_menu :
                                        self->menu),
                                       /* continue where we left off */
                                       self->show_from +
                                       g_list_length(self->entries));
            more_frame = menu_entry_frame_new(more_entry, self);
            /* make it get deleted when the menu frame goes away */
            menu_entry_unref(more_entry);
                                       
            /* add our More... entry to the frame */
            self->entries = g_list_append(self->entries, more_frame);
        }
    }

    menu_frame_render(self);
}

static gboolean menu_frame_is_visible(ObMenuFrame *self)
{
    return !!(g_list_find(menu_frame_visible, self));
}

static gboolean menu_frame_show(ObMenuFrame *self)
{
    GList *it;

    /* determine if the underlying menu is already visible */
    for (it = menu_frame_visible; it; it = g_list_next(it)) {
        ObMenuFrame *f = it->data;
        if (f->menu == self->menu)
            break;
    }
    if (!it) {
        if (self->menu->update_func)
            if (!self->menu->update_func(self, self->menu->data))
                return FALSE;
    }

    if (menu_frame_visible == NULL) {
        /* no menus shown yet */

        /* grab the pointer in such a way as to pass through "owner events"
           so that we can get enter/leave notifies in the menu. */
        if (!grab_pointer(TRUE, FALSE, OB_CURSOR_POINTER))
            return FALSE;
        if (!grab_keyboard()) {
            ungrab_pointer();
            return FALSE;
        }
    }

    menu_frame_update(self);

    menu_frame_visible = g_list_prepend(menu_frame_visible, self);

    if (self->menu->show_func)
        self->menu->show_func(self, self->menu->data);

    return TRUE;
}

gboolean menu_frame_show_topmenu(ObMenuFrame *self, gint x, gint y,
                                 gint button)
{
    gint px, py;
    guint i;

    if (menu_frame_is_visible(self))
        return TRUE;
    if (!menu_frame_show(self))
        return FALSE;

    /* find the monitor the menu is on */
    for (i = 0; i < screen_num_monitors; ++i) {
        Rect *a = screen_physical_area_monitor(i);
        if (RECT_CONTAINS(*a, x, y)) {
            self->monitor = i;
            break;
        }
    }

    if (self->menu->place_func)
        self->menu->place_func(self, &x, &y, button, self->menu->data);
    else
        menu_frame_place_topmenu(self, &x, &y);

    menu_frame_move(self, x, y);

    XMapWindow(ob_display, self->window);

    if (screen_pointer_pos(&px, &py)) {
        ObMenuEntryFrame *e = menu_entry_frame_under(px, py);
        if (e && e->frame == self)
            e->ignore_enters++;
    }

    return TRUE;
}

gboolean menu_frame_show_submenu(ObMenuFrame *self, ObMenuFrame *parent,
                                 ObMenuEntryFrame *parent_entry)
{
    gint x, y, dx, dy;
    gint px, py;

    if (menu_frame_is_visible(self))
        return TRUE;

    self->monitor = parent->monitor;
    self->parent = parent;
    self->parent_entry = parent_entry;

    /* set up parent's child to be us */
    if (parent->child)
        menu_frame_hide(parent->child);
    parent->child = self;

    if (!menu_frame_show(self))
        return FALSE;

    menu_frame_place_submenu(self, &x, &y);
    menu_frame_move_on_screen(self, x, y, &dx, &dy);

    if (dx != 0) {
        /*try the other side */
        self->direction_right = !self->direction_right;
        menu_frame_place_submenu(self, &x, &y);
        menu_frame_move_on_screen(self, x, y, &dx, &dy);
    }
    menu_frame_move(self, x + dx, y + dy);

    XMapWindow(ob_display, self->window);

    if (screen_pointer_pos(&px, &py)) {
        ObMenuEntryFrame *e = menu_entry_frame_under(px, py);
        if (e && e->frame == self)
            e->ignore_enters++;
    }

    return TRUE;
}

static void menu_frame_hide(ObMenuFrame *self)
{
    GList *it = g_list_find(menu_frame_visible, self);

    if (!it)
        return;

    if (self->menu->hide_func)
        self->menu->hide_func(self, self->menu->data);

    if (self->child)
        menu_frame_hide(self->child);

    if (self->parent)
        self->parent->child = NULL;
    self->parent = NULL;
    self->parent_entry = NULL;

    menu_frame_visible = g_list_delete_link(menu_frame_visible, it);

    if (menu_frame_visible == NULL) {
        /* last menu shown */
        ungrab_pointer();
        ungrab_keyboard();
    }

    XUnmapWindow(ob_display, self->window);

    menu_frame_free(self);
}

void menu_frame_hide_all()
{
    GList *it;

    if (config_submenu_show_delay) {
        /* remove any submenu open requests */
        ob_main_loop_timeout_remove(ob_main_loop,
                                    menu_entry_frame_submenu_timeout);
    }
    if ((it = g_list_last(menu_frame_visible)))
        menu_frame_hide(it->data);
}

void menu_frame_hide_all_client(ObClient *client)
{
    GList *it = g_list_last(menu_frame_visible);
    if (it) {
        ObMenuFrame *f = it->data;
        if (f->client == client)
            menu_frame_hide(f);
    }
}


ObMenuFrame* menu_frame_under(gint x, gint y)
{
    ObMenuFrame *ret = NULL;
    GList *it;

    for (it = menu_frame_visible; it; it = g_list_next(it)) {
        ObMenuFrame *f = it->data;

        if (RECT_CONTAINS(f->area, x, y)) {
            ret = f;
            break;
        }
    }
    return ret;
}

ObMenuEntryFrame* menu_entry_frame_under(gint x, gint y)
{
    ObMenuFrame *frame;
    ObMenuEntryFrame *ret = NULL;
    GList *it;

    if ((frame = menu_frame_under(x, y))) {
        x -= ob_rr_theme->mbwidth + frame->area.x;
        y -= ob_rr_theme->mbwidth + frame->area.y;

        for (it = frame->entries; it; it = g_list_next(it)) {
            ObMenuEntryFrame *e = it->data;

            if (RECT_CONTAINS(e->area, x, y)) {
                ret = e;
                break;
            }
        }
    }
    return ret;
}

static gboolean menu_entry_frame_submenu_timeout(gpointer data)
{
    menu_entry_frame_show_submenu((ObMenuEntryFrame*)data);
    return FALSE;
}

void menu_frame_select(ObMenuFrame *self, ObMenuEntryFrame *entry,
                       gboolean immediate)
{
    ObMenuEntryFrame *old = self->selected;
    ObMenuFrame *oldchild = self->child;

    if (entry && entry->entry->type == OB_MENU_ENTRY_TYPE_SEPARATOR)
        entry = old;

    if (old == entry) return;
   
    if (config_submenu_show_delay) { 
        /* remove any submenu open requests */
        ob_main_loop_timeout_remove(ob_main_loop,
                                    menu_entry_frame_submenu_timeout);
    }

    self->selected = entry;

    if (old)
        menu_entry_frame_render(old);
    if (oldchild)
        menu_frame_hide(oldchild);

    if (self->selected) {
        menu_entry_frame_render(self->selected);

        if (self->selected->entry->type == OB_MENU_ENTRY_TYPE_SUBMENU) {
            if (config_submenu_show_delay && !immediate) {
                /* initiate a new submenu open request */
                ob_main_loop_timeout_add(ob_main_loop,
                                         config_submenu_show_delay * 1000,
                                         menu_entry_frame_submenu_timeout,
                                         self->selected, g_direct_equal,
                                         NULL);
            } else {
                menu_entry_frame_show_submenu(self->selected);
            }
        }
    }
}

void menu_entry_frame_show_submenu(ObMenuEntryFrame *self)
{
    ObMenuFrame *f;

    if (!self->entry->data.submenu.submenu) return;

    f = menu_frame_new(self->entry->data.submenu.submenu,
                       self->entry->data.submenu.show_from,
                       self->frame->client);
    /* pass our direction on to our child */
    f->direction_right = self->frame->direction_right;

    menu_frame_show_submenu(f, self->frame, self);
}

void menu_entry_frame_execute(ObMenuEntryFrame *self, guint state, Time time)
{
    if (self->entry->type == OB_MENU_ENTRY_TYPE_NORMAL &&
        self->entry->data.normal.enabled)
    {
        /* grab all this shizzle, cuz when the menu gets hidden, 'self'
           gets freed */
        ObMenuEntry *entry = self->entry;
        ObMenuExecuteFunc func = self->frame->menu->execute_func;
        gpointer data = self->frame->menu->data;
        GSList *acts = self->entry->data.normal.actions;
        ObClient *client = self->frame->client;
        ObMenuFrame *frame = self->frame;

        /* release grabs before executing the shit */
        if (!(state & ControlMask)) {
            menu_frame_hide_all();
            frame = NULL;
        }

        if (func)
            func(entry, frame, client, state, data, time);
        else
            action_run(acts, client, state, time);
    }
}

void menu_frame_select_previous(ObMenuFrame *self)
{
    GList *it = NULL, *start;

    if (self->entries) {
        start = it = g_list_find(self->entries, self->selected);
        while (TRUE) {
            ObMenuEntryFrame *e;

            it = it ? g_list_previous(it) : g_list_last(self->entries);
            if (it == start)
                break;

            if (it) {
                e = it->data;
                if (e->entry->type == OB_MENU_ENTRY_TYPE_SUBMENU)
                    break;
                if (e->entry->type == OB_MENU_ENTRY_TYPE_NORMAL)
                    break;
            }
        }
    }
    menu_frame_select(self, it ? it->data : NULL, TRUE);
}

void menu_frame_select_next(ObMenuFrame *self)
{
    GList *it = NULL, *start;

    if (self->entries) {
        start = it = g_list_find(self->entries, self->selected);
        while (TRUE) {
            ObMenuEntryFrame *e;

            it = it ? g_list_next(it) : self->entries;
            if (it == start)
                break;

            if (it) {
                e = it->data;
                if (e->entry->type == OB_MENU_ENTRY_TYPE_SUBMENU)
                    break;
                if (e->entry->type == OB_MENU_ENTRY_TYPE_NORMAL)
                    break;
            }
        }
    }
    menu_frame_select(self, it ? it->data : NULL, TRUE);
}
