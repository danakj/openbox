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

#define FRAME_EVENTMASK (ButtonPressMask |ButtonMotionMask | EnterWindowMask |\
                         LeaveWindowMask)
#define ENTRY_EVENTMASK (EnterWindowMask | LeaveWindowMask | \
                         ButtonPressMask | ButtonReleaseMask)

GList *menu_frame_visible;

static ObMenuEntryFrame* menu_entry_frame_new(ObMenuEntry *entry,
                                              ObMenuFrame *frame);
static void menu_entry_frame_free(ObMenuEntryFrame *self);
static void menu_frame_render(ObMenuFrame *self);
static void menu_frame_update(ObMenuFrame *self);
static gboolean menu_entry_frame_submenu_timeout(gpointer data);

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

ObMenuFrame* menu_frame_new(ObMenu *menu, ObClient *client)
{
    ObMenuFrame *self;
    XSetWindowAttributes attr;

    self = g_new0(ObMenuFrame, 1);
    self->type = Window_Menu;
    self->menu = menu;
    self->selected = NULL;
    self->client = client;
    self->direction_right = TRUE;

    attr.event_mask = FRAME_EVENTMASK;
    self->window = createWindow(RootWindow(ob_display, ob_screen),
                                CWEventMask, &attr);

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
    self->a_disabled = RrAppearanceCopy(ob_rr_theme->a_menu_disabled);
    self->a_selected = RrAppearanceCopy(ob_rr_theme->a_menu_selected);

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
    self->a_text_disabled =
        RrAppearanceCopy(ob_rr_theme->a_menu_text_disabled);
    self->a_text_selected =
        RrAppearanceCopy(ob_rr_theme->a_menu_text_selected);
    self->a_text_title =
        RrAppearanceCopy(ob_rr_theme->a_menu_text_title);

    return self;
}

static void menu_entry_frame_free(ObMenuEntryFrame *self)
{
    if (self) {
        XDestroyWindow(ob_display, self->text);
        XDestroyWindow(ob_display, self->window);
        g_hash_table_insert(menu_frame_map, &self->text, self);
        g_hash_table_insert(menu_frame_map, &self->window, self);
        if (self->entry->type == OB_MENU_ENTRY_TYPE_NORMAL) {
            XDestroyWindow(ob_display, self->icon);
            g_hash_table_insert(menu_frame_map, &self->icon, self);
        }
        if (self->entry->type == OB_MENU_ENTRY_TYPE_SUBMENU) {
            XDestroyWindow(ob_display, self->bullet);
            g_hash_table_insert(menu_frame_map, &self->bullet, self);
        }

        RrAppearanceFree(self->a_normal);
        RrAppearanceFree(self->a_disabled);
        RrAppearanceFree(self->a_selected);

        RrAppearanceFree(self->a_separator);
        RrAppearanceFree(self->a_icon);
        RrAppearanceFree(self->a_mask);
        RrAppearanceFree(self->a_text_normal);
        RrAppearanceFree(self->a_text_disabled);
        RrAppearanceFree(self->a_text_selected);
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

void menu_frame_place_topmenu(ObMenuFrame *self, gint x, gint y)
{
    if (self->client && x < 0 && y < 0) {
        x = self->client->frame->area.x + self->client->frame->size.left;
        y = self->client->frame->area.y + self->client->frame->size.top;
    } else {
        if (config_menu_middle)
            y -= self->area.height / 2;
    }
    menu_frame_move(self, x, y);
}

void menu_frame_place_submenu(ObMenuFrame *self)
{
    gint x, y;
    gint overlap;
    gint bwidth;

    overlap = ob_rr_theme->menu_overlap;
    bwidth = ob_rr_theme->mbwidth;

    if (self->direction_right)
        x = self->parent->area.x + self->parent->area.width - overlap - bwidth;
    else
        x = self->parent->area.x - self->area.width + overlap + bwidth;

    y = self->parent->area.y + self->parent_entry->area.y;
    if (config_menu_middle)
        y -= (self->area.height - (bwidth * 2) - self->item_h) / 2;
    else
        y += overlap;

    menu_frame_move(self, x, y);
}

void menu_frame_move_on_screen(ObMenuFrame *self, gint *dx, gint *dy)
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
        *dx = MAX(*dx, a->x - self->area.x);
        *dy = MAX(*dy, a->y - self->area.y);
    }
    *dx = MIN(*dx, (a->x + a->width) - (self->area.x + self->area.width));
    *dy = MIN(*dy, (a->y + a->height) - (self->area.y + self->area.height));
    /* if in the top half then check this stuff last, will keep the top
       edge of the menu visible */
    if (pos <= half) {
        *dx = MAX(*dx, a->x - self->area.x);
        *dy = MAX(*dy, a->y - self->area.y);
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
        item_a = ((self->entry->type == OB_MENU_ENTRY_TYPE_NORMAL &&
                   !self->entry->data.normal.enabled) ?
                  self->a_disabled :
                  (self == self->frame->selected ?
                   self->a_selected :
                   self->a_normal));
        th = self->frame->item_h;
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
        text_a = (!self->entry->data.normal.enabled ?
                  self->a_text_disabled :
                  (self == self->frame->selected ?
                   self->a_text_selected :
                   self->a_text_normal));
        text_a->texture[0].data.text.string = self->entry->data.normal.label;
        break;
    case OB_MENU_ENTRY_TYPE_SUBMENU:
        text_a = (self == self->frame->selected ?
                  self->a_text_selected :
                  self->a_text_normal);
        sub = self->entry->data.submenu.submenu;
        text_a->texture[0].data.text.string = sub ? sub->title : "";
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
                          self->frame->item_h - 2*PADDING);
        text_a->surface.parent = item_a;
        text_a->surface.parentx = self->frame->text_x;
        text_a->surface.parenty = PADDING;
        RrPaint(text_a, self->text, self->frame->text_w,
                self->frame->item_h - 2*PADDING);
        break;
    case OB_MENU_ENTRY_TYPE_SUBMENU:
        XMoveResizeWindow(ob_display, self->text,
                          self->frame->text_x, PADDING,
                          self->frame->text_w - self->frame->item_h,
                          self->frame->item_h - 2*PADDING);
        text_a->surface.parent = item_a;
        text_a->surface.parentx = self->frame->text_x;
        text_a->surface.parenty = PADDING;
        RrPaint(text_a, self->text, self->frame->text_w - self->frame->item_h,
                self->frame->item_h - 2*PADDING);
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
                          self->frame->item_h - frame->item_margin.top
                          - frame->item_margin.bottom,
                          self->frame->item_h - frame->item_margin.top
                          - frame->item_margin.bottom);
        self->a_icon->texture[0].data.rgba.width =
            self->entry->data.normal.icon_width;
        self->a_icon->texture[0].data.rgba.height =
            self->entry->data.normal.icon_height;
        self->a_icon->texture[0].data.rgba.data =
            self->entry->data.normal.icon_data;
        self->a_icon->surface.parent = item_a;
        self->a_icon->surface.parentx = PADDING;
        self->a_icon->surface.parenty = frame->item_margin.top;
        RrPaint(self->a_icon, self->icon,
                self->frame->item_h - frame->item_margin.top
                - frame->item_margin.bottom,
                self->frame->item_h - frame->item_margin.top
                - frame->item_margin.bottom);
        XMapWindow(ob_display, self->icon);
    } else if (self->entry->type == OB_MENU_ENTRY_TYPE_NORMAL &&
               self->entry->data.normal.mask)
    {
        RrColor *c;

        XMoveResizeWindow(ob_display, self->icon,
                          PADDING, frame->item_margin.top,
                          self->frame->item_h - frame->item_margin.top
                          - frame->item_margin.bottom,
                          self->frame->item_h - frame->item_margin.top
                          - frame->item_margin.bottom);
        self->a_mask->texture[0].data.mask.mask =
            self->entry->data.normal.mask;

        c = ((self->entry->type == OB_MENU_ENTRY_TYPE_NORMAL &&
              !self->entry->data.normal.enabled) ?
             self->entry->data.normal.mask_disabled_color :
             (self == self->frame->selected ?
              self->entry->data.normal.mask_selected_color :
              self->entry->data.normal.mask_normal_color));
        self->a_mask->texture[0].data.mask.color = c;

        self->a_mask->surface.parent = item_a;
        self->a_mask->surface.parentx = PADDING;
        self->a_mask->surface.parenty = frame->item_margin.top;
        RrPaint(self->a_mask, self->icon,
                self->frame->item_h - frame->item_margin.top
                - frame->item_margin.bottom,
                self->frame->item_h - frame->item_margin.top
                - frame->item_margin.bottom);
        XMapWindow(ob_display, self->icon);
    } else
        XUnmapWindow(ob_display, self->icon);

    if (self->entry->type == OB_MENU_ENTRY_TYPE_SUBMENU) {
        RrAppearance *bullet_a;
        XMoveResizeWindow(ob_display, self->bullet,
                          self->frame->text_x + self->frame->text_w
                          - self->frame->item_h + PADDING, PADDING,
                          self->frame->item_h - 2*PADDING,
                          self->frame->item_h - 2*PADDING);
        bullet_a = (self == self->frame->selected ?
                    self->a_bullet_selected :
                    self->a_bullet_normal);
        bullet_a->surface.parent = item_a;
        bullet_a->surface.parentx =
            self->frame->text_x + self->frame->text_w - self->frame->item_h
            + PADDING;
        bullet_a->surface.parenty = PADDING;
        RrPaint(bullet_a, self->bullet,
                self->frame->item_h - 2*PADDING,
                self->frame->item_h - 2*PADDING);
        XMapWindow(ob_display, self->bullet);
    } else
        XUnmapWindow(ob_display, self->bullet);

    XFlush(ob_display);
}

static void menu_frame_render(ObMenuFrame *self)
{
    gint w = 0, h = 0;
    gint tw, th; /* temps */
    GList *it;
    gboolean has_icon = FALSE;
    ObMenu *sub;
    ObMenuEntryFrame *e;

    XSetWindowBorderWidth(ob_display, self->window, ob_rr_theme->mbwidth);
    XSetWindowBorder(ob_display, self->window,
                     RrColorPixel(ob_rr_theme->menu_b_color));

    /* find text dimensions */

    STRUT_SET(self->item_margin, 0, 0, 0, 0);

    if (self->entries) {
        ObMenuEntryFrame *e = self->entries->data;
        gint l, t, r, b;

        e->a_text_normal->texture[0].data.text.string = "";
        RrMinsize(e->a_text_normal, &tw, &th);
        tw += 2*PADDING;
        th += 2*PADDING;
        self->item_h = th;

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
    } else
        self->item_h = 0;

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
                         RrColorPixel(ob_rr_theme->menu_b_color));

        text_a = ((e->entry->type == OB_MENU_ENTRY_TYPE_NORMAL &&
                   !e->entry->data.normal.enabled) ?
                  e->a_text_disabled :
                  (e == self->selected ?
                   e->a_text_selected :
                   e->a_text_normal));
        switch (e->entry->type) {
        case OB_MENU_ENTRY_TYPE_NORMAL:
            text_a->texture[0].data.text.string = e->entry->data.normal.label;
            RrMinsize(text_a, &tw, &th);
            tw = MIN(tw, MAX_MENU_WIDTH);

            if (e->entry->data.normal.icon_data ||
                e->entry->data.normal.mask)
                has_icon = TRUE;
            break;
        case OB_MENU_ENTRY_TYPE_SUBMENU:
            sub = e->entry->data.submenu.submenu;
            text_a->texture[0].data.text.string = sub ? sub->title : "";
            RrMinsize(text_a, &tw, &th);
            tw = MIN(tw, MAX_MENU_WIDTH);

            if (e->entry->data.normal.icon_data ||
                e->entry->data.normal.mask)
                has_icon = TRUE;

            tw += self->item_h - PADDING;
            break;
        case OB_MENU_ENTRY_TYPE_SEPARATOR:
            if (e->entry->data.separator.label != NULL) {
                e->a_text_title->texture[0].data.text.string =
                    e->entry->data.separator.label;
                RrMinsize(e->a_text_title, &tw, &th);
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
            w += self->item_h + PADDING;
            self->text_x += self->item_h + PADDING;
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

    menu_pipe_execute(self->menu);
    menu_find_submenus(self->menu);

    self->selected = NULL;

    for (mit = self->menu->entries, fit = self->entries; mit && fit;
         mit = g_list_next(mit), fit = g_list_next(fit))
    {
        ObMenuEntryFrame *f = fit->data;
        f->entry = mit->data;
    }

    while (mit) {
        ObMenuEntryFrame *e = menu_entry_frame_new(mit->data, self);
        self->entries = g_list_append(self->entries, e);
        mit = g_list_next(mit);
    }
    
    while (fit) {
        GList *n = g_list_next(fit);
        menu_entry_frame_free(fit->data);
        self->entries = g_list_delete_link(self->entries, fit);
        fit = n;
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

    if (menu_frame_visible == NULL) {
        /* no menus shown yet */
        if (!grab_pointer(TRUE, OB_CURSOR_POINTER))
            return FALSE;
        if (!grab_keyboard(TRUE)) {
            grab_pointer(FALSE, OB_CURSOR_POINTER);
            return FALSE;
        }
    }

    /* determine if the underlying menu is already visible */
    for (it = menu_frame_visible; it; it = g_list_next(it)) {
        ObMenuFrame *f = it->data;
        if (f->menu == self->menu)
            break;
    }
    if (!it) {
        if (self->menu->update_func)
            self->menu->update_func(self, self->menu->data);
    }

    menu_frame_update(self);

    menu_frame_visible = g_list_prepend(menu_frame_visible, self);

    return TRUE;
}

gboolean menu_frame_show_topmenu(ObMenuFrame *self, gint x, gint y)
{
    gint dx, dy;
    guint i;

    if (menu_frame_is_visible(self))
        return TRUE;
    if (!menu_frame_show(self))
        return FALSE;

    menu_frame_place_topmenu(self, x, y);

    /* find the monitor the menu is on */
    for (i = 0; i < screen_num_monitors; ++i) {
        Rect *a = screen_physical_area_monitor(i);
        if (RECT_CONTAINS(*a, x, y)) {
            self->monitor = i;
            break;
        }
    }

    menu_frame_move_on_screen(self, &dx, &dy);
    menu_frame_move(self, self->area.x + dx, self->area.y + dy);

    XMapWindow(ob_display, self->window);

    return TRUE;
}

gboolean menu_frame_show_submenu(ObMenuFrame *self, ObMenuFrame *parent,
                                 ObMenuEntryFrame *parent_entry)
{
    ObMenuEntryFrame *e;
    gint dx, dy;

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

    menu_frame_place_submenu(self);
    menu_frame_move_on_screen(self, &dx, &dy);

    if (dx == 0) {
        menu_frame_move(self, self->area.x, self->area.y + dy);
    } else {
        gboolean dir;

        /* flip the direction in which we're placing submenus */
        if (dx > 0)
            dir = TRUE;
        else
            dir = FALSE;

        /* if it changed, then replace the menu on the opposite side,
           and try keep it on the screen too */
        if (dir != self->direction_right) {
            self->direction_right = dir;
            menu_frame_place_submenu(self);
            menu_frame_move_on_screen(self, &dx, &dy);
            menu_frame_move(self, self->area.x + dx, self->area.y + dy);
        }
    }

    XMapWindow(ob_display, self->window);

    if (screen_pointer_pos(&dx, &dy) && (e = menu_entry_frame_under(dx, dy)) &&
        e->frame == self)
        ++e->ignore_enters;

    return TRUE;
}

void menu_frame_hide(ObMenuFrame *self)
{
    GList *it = g_list_find(menu_frame_visible, self);

    if (!it)
        return;

    if (self->child)
        menu_frame_hide(self->child);

    if (self->parent)
        self->parent->child = NULL;
    self->parent = NULL;
    self->parent_entry = NULL;

    menu_frame_visible = g_list_delete_link(menu_frame_visible, it);

    if (menu_frame_visible == NULL) {
        /* last menu shown */
        grab_pointer(FALSE, OB_CURSOR_NONE);
        grab_keyboard(FALSE);
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

void menu_frame_select(ObMenuFrame *self, ObMenuEntryFrame *entry)
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
            if (config_submenu_show_delay) {
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

        /* release grabs before executing the shit */
        if (!(state & ControlMask))
            menu_frame_hide_all();

        if (func)
            func(entry, state, data, time);
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
                if (e->entry->type == OB_MENU_ENTRY_TYPE_NORMAL &&
                    e->entry->data.normal.enabled)
                    break;
            }
        }
    }
    menu_frame_select(self, it ? it->data : NULL);
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
                if (e->entry->type == OB_MENU_ENTRY_TYPE_NORMAL &&
                    e->entry->data.normal.enabled)
                    break;
            }
        }
    }
    menu_frame_select(self, it ? it->data : NULL);
}
