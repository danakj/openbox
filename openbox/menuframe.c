#include "menuframe.h"
#include "client.h"
#include "menu.h"
#include "screen.h"
#include "grab.h"
#include "openbox.h"
#include "render/theme.h"

#define PADDING 2
#define SEPARATOR_HEIGHT 3
#define MAX_MENU_WIDTH 400

#define FRAME_EVENTMASK (ButtonPressMask |ButtonMotionMask | EnterWindowMask |\
			 LeaveWindowMask)
#define TITLE_EVENTMASK (ButtonPressMask | ButtonMotionMask)
#define ENTRY_EVENTMASK (EnterWindowMask | LeaveWindowMask | \
                         ButtonPressMask | ButtonReleaseMask)

GList *menu_frame_visible;

static ObMenuEntryFrame* menu_entry_frame_new(ObMenuEntry *entry,
                                              ObMenuFrame *frame);
static void menu_entry_frame_free(ObMenuEntryFrame *self);
static void menu_frame_render(ObMenuFrame *self);
static void menu_frame_update(ObMenuFrame *self);

static Window createWindow(Window parent, unsigned long mask,
			   XSetWindowAttributes *attrib)
{
    return XCreateWindow(ob_display, parent, 0, 0, 1, 1, 0,
			 RrDepth(ob_rr_inst), InputOutput,
                         RrVisual(ob_rr_inst), mask, attrib);
}

ObMenuFrame* menu_frame_new(ObMenu *menu, ObClient *client)
{
    ObMenuFrame *self;
    XSetWindowAttributes attr;

    self = g_new0(ObMenuFrame, 1);
    self->type = Window_Menu;
    self->menu = menu;
    self->selected = NULL;
    self->show_title = TRUE;
    self->client = client;

    attr.event_mask = FRAME_EVENTMASK;
    self->window = createWindow(RootWindow(ob_display, ob_screen),
                                   CWEventMask, &attr);
    attr.event_mask = TITLE_EVENTMASK;
    self->title = createWindow(self->window, CWEventMask, &attr);
    self->items = createWindow(self->window, 0, NULL);

    XMapWindow(ob_display, self->items);

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

        XDestroyWindow(ob_display, self->items);
        XDestroyWindow(ob_display, self->title);
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
    self->window = createWindow(self->frame->items, CWEventMask, &attr);
    self->text = createWindow(self->window, 0, NULL);
    if (entry->type != OB_MENU_ENTRY_TYPE_SEPARATOR) {
        self->icon = createWindow(self->window, 0, NULL);
        self->bullet = createWindow(self->window, 0, NULL);
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

    return self;
}

static void menu_entry_frame_free(ObMenuEntryFrame *self)
{
    if (self) {
        XDestroyWindow(ob_display, self->text);
        XDestroyWindow(ob_display, self->window);
        if (self->entry->type != OB_MENU_ENTRY_TYPE_SEPARATOR) {
            XDestroyWindow(ob_display, self->icon);
            XDestroyWindow(ob_display, self->bullet);
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

void menu_frame_move_on_screen(ObMenuFrame *self)
{
    Rect *a;
    guint i;
    gint dx = 0, dy = 0;

    for (i = 0; i < screen_num_monitors; ++i) {
        a = screen_physical_area_monitor(i);
        if (RECT_INTERSECTS_RECT(*a, self->area))
            break;
    }
    if (!a) a = screen_physical_area_monitor(0);

    dx = MIN(0, (a->x + a->width) - (self->area.x + self->area.width));
    dy = MIN(0, (a->y + a->height) - (self->area.y + self->area.height));
    if (!dx) dx = MAX(0, a->x - self->area.x);
    if (!dy) dy = MAX(0, a->y - self->area.y);

    if (dx || dy) {
        ObMenuFrame *f;

        for (f = self; f; f = f->parent)
            menu_frame_move(f, f->area.x + dx, f->area.y + dy);
        for (f = self->child; f; f = f->child)
            menu_frame_move(f, f->area.x + dx, f->area.y + dy);
        XWarpPointer(ob_display, None, None, 0, 0, 0, 0, dx, dy);
    }
}

static void menu_entry_frame_render(ObMenuEntryFrame *self)
{
    RrAppearance *item_a, *text_a;
    gint th; /* temp */
    ObMenu *sub;
    ObMenuFrame *frame = self->frame;

    item_a = ((self->entry->type == OB_MENU_ENTRY_TYPE_NORMAL &&
               !self->entry->data.normal.enabled) ?
              self->a_disabled :
              (self == self->frame->selected ?
               self->a_selected :
               self->a_normal));
    switch (self->entry->type) {
    case OB_MENU_ENTRY_TYPE_NORMAL:
    case OB_MENU_ENTRY_TYPE_SUBMENU:
        th = self->frame->item_h;
        break;
    case OB_MENU_ENTRY_TYPE_SEPARATOR:
        th = SEPARATOR_HEIGHT + 2*PADDING;
        break;
    }
    RECT_SET_SIZE(self->area, self->frame->inner_w, th);
    XResizeWindow(ob_display, self->window,
                  self->area.width, self->area.height);
    item_a->surface.parent = self->frame->a_items;
    item_a->surface.parentx = self->area.x;
    item_a->surface.parenty = self->area.y;
    RrPaint(item_a, self->window, self->area.width, self->area.height);

    text_a = ((self->entry->type == OB_MENU_ENTRY_TYPE_NORMAL &&
               !self->entry->data.normal.enabled) ?
              self->a_text_disabled :
              (self == self->frame->selected ?
               self->a_text_selected :
               self->a_text_normal));
    switch (self->entry->type) {
    case OB_MENU_ENTRY_TYPE_NORMAL:
        text_a->texture[0].data.text.string = self->entry->data.normal.label;
        break;
    case OB_MENU_ENTRY_TYPE_SUBMENU:
        sub = self->entry->data.submenu.submenu;
        text_a->texture[0].data.text.string = sub ? sub->title : "";
        break;
    case OB_MENU_ENTRY_TYPE_SEPARATOR:
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
        XMoveResizeWindow(ob_display, self->text, PADDING, PADDING,
                          self->area.width - 2*PADDING, SEPARATOR_HEIGHT);
        self->a_separator->surface.parent = item_a;
        self->a_separator->surface.parentx = PADDING;
        self->a_separator->surface.parenty = PADDING;
        self->a_separator->texture[0].data.lineart.color =
            text_a->texture[0].data.text.color;
        self->a_separator->texture[0].data.lineart.x1 = 2*PADDING;
        self->a_separator->texture[0].data.lineart.y1 = SEPARATOR_HEIGHT / 2;
        self->a_separator->texture[0].data.lineart.x2 =
            self->area.width - 4*PADDING;
        self->a_separator->texture[0].data.lineart.y2 = SEPARATOR_HEIGHT / 2;
        RrPaint(self->a_separator, self->text,
                self->area.width - 2*PADDING, SEPARATOR_HEIGHT);
        break;
    }

    if (self->entry->type != OB_MENU_ENTRY_TYPE_SEPARATOR &&
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
    } else if (self->entry->type != OB_MENU_ENTRY_TYPE_SEPARATOR &&
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
}

static void menu_frame_render(ObMenuFrame *self)
{
    gint w = 0, h = 0;
    gint allitems_h = 0;
    gint tw, th; /* temps */
    GList *it;
    gboolean has_icon = FALSE;
    ObMenu *sub;

    XSetWindowBorderWidth(ob_display, self->window, ob_rr_theme->bwidth);
    XSetWindowBorder(ob_display, self->window,
                     RrColorPixel(ob_rr_theme->b_color));

    if (!self->parent && self->show_title) {
        XMoveWindow(ob_display, self->title, 
                    -ob_rr_theme->bwidth, h - ob_rr_theme->bwidth);

        self->a_title->texture[0].data.text.string = self->menu->title;
        RrMinsize(self->a_title, &tw, &th);
        tw = MIN(tw, MAX_MENU_WIDTH);
        tw += 2*PADDING;
        th += 2*PADDING;
        w = MAX(w, tw);
        h += (self->title_h = th + ob_rr_theme->bwidth);

        XSetWindowBorderWidth(ob_display, self->title, ob_rr_theme->bwidth);
        XSetWindowBorder(ob_display, self->title,
                         RrColorPixel(ob_rr_theme->b_color));
    }

    XMoveWindow(ob_display, self->items, 0, h);

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

    for (it = self->entries; it; it = g_list_next(it)) {
        RrAppearance *text_a;
        ObMenuEntryFrame *e = it->data;

        RECT_SET_POINT(e->area, 0, allitems_h);
        XMoveWindow(ob_display, e->window, 0, e->area.y);

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
            tw = 0;
            th = SEPARATOR_HEIGHT;
            break;
        }
        tw += 2*PADDING;
        th += 2*PADDING;
        w = MAX(w, tw);
        h += th;
        allitems_h += th;
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
    if (!allitems_h) {
        allitems_h = 3;
        h += 3;
    }

    XResizeWindow(ob_display, self->window, w, h);
    XResizeWindow(ob_display, self->items, w, allitems_h);

    self->inner_w = w;

    if (!self->parent && self->show_title) {
        XResizeWindow(ob_display, self->title,
                      w, self->title_h - ob_rr_theme->bwidth);
        RrPaint(self->a_title, self->title,
                w, self->title_h - ob_rr_theme->bwidth);
        XMapWindow(ob_display, self->title);
    } else
        XUnmapWindow(ob_display, self->title);

    RrPaint(self->a_items, self->items, w, allitems_h);

    for (it = self->entries; it; it = g_list_next(it))
        menu_entry_frame_render(it->data);

    w += ob_rr_theme->bwidth * 2;
    h += ob_rr_theme->bwidth * 2;

    RECT_SET_SIZE(self->area, w, h);
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

void menu_frame_show(ObMenuFrame *self, ObMenuFrame *parent)
{
    GList *it;

    if (g_list_find(menu_frame_visible, self))
        return;

    if (parent) {
        if (parent->child)
            menu_frame_hide(parent->child);
        parent->child = self;
    }
    self->parent = parent;

    if (menu_frame_visible == NULL) {
        /* no menus shown yet */
        grab_pointer(TRUE, OB_CURSOR_NONE);
        grab_keyboard(TRUE);
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

    menu_frame_visible = g_list_prepend(menu_frame_visible, self);
    menu_frame_update(self);

    menu_frame_move_on_screen(self);

    XMapWindow(ob_display, self->window);
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
    GList *it = g_list_last(menu_frame_visible);
    if (it) 
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
        x -= ob_rr_theme->bwidth + frame->area.x;
        y -= frame->title_h + ob_rr_theme->bwidth + frame->area.y;

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

void menu_frame_select(ObMenuFrame *self, ObMenuEntryFrame *entry)
{
    ObMenuEntryFrame *old = self->selected;
    ObMenuFrame *oldchild = self->child;

    if (entry && entry->entry->type == OB_MENU_ENTRY_TYPE_SEPARATOR)
        entry = old;

    if (old == entry) return;

    self->selected = entry;

    if (old)
        menu_entry_frame_render(old);
    if (oldchild)
        menu_frame_hide(oldchild);

    if (self->selected) {
        menu_entry_frame_render(self->selected);

        if (self->selected->entry->type == OB_MENU_ENTRY_TYPE_SUBMENU)
            menu_entry_frame_show_submenu(self->selected);
    }
}

void menu_entry_frame_show_submenu(ObMenuEntryFrame *self)
{
    ObMenuFrame *f;

    if (!self->entry->data.submenu.submenu) return;

    f = menu_frame_new(self->entry->data.submenu.submenu,
                       self->frame->client);
    menu_frame_move(f,
                    self->frame->area.x + self->frame->area.width
                    - ob_rr_theme->menu_overlap - ob_rr_theme->bwidth,
                    self->frame->area.y + self->frame->title_h +
                    self->area.y + ob_rr_theme->menu_overlap);
    menu_frame_show(f, self->frame);
}

void menu_entry_frame_execute(ObMenuEntryFrame *self, guint state)
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
            func(entry, state, data);
        else {
            GSList *it;

            for (it = acts; it; it = g_slist_next(it))
                action_run(it->data, client, state);
        }
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
