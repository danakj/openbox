#include "obengine.h"
#include "../../kernel/openbox.h"
#include "../../kernel/screen.h"

static void obrender_label(ObFrame *self, Appearance *a);
static void obrender_max(ObFrame *self, Appearance *a);
static void obrender_icon(ObFrame *self, Appearance *a);
static void obrender_iconify(ObFrame *self, Appearance *a);
static void obrender_desk(ObFrame *self, Appearance *a);
static void obrender_close(ObFrame *self, Appearance *a);

void obrender_frame(ObFrame *self)
{
    if (client_focused(self->frame.client)) {
        XSetWindowBorder(ob_display, self->frame.plate,
                         ob_s_cb_focused_color->pixel);
    } else {
        XSetWindowBorder(ob_display, self->frame.plate,
                         ob_s_cb_unfocused_color->pixel);
    }

    if (self->frame.client->decorations & Decor_Titlebar) {
        Appearance *t, *l, *m, *n, *i, *d, *c;

        t = (client_focused(self->frame.client) ?
             self->a_focused_title : self->a_unfocused_title);
        l = (client_focused(self->frame.client) ?
             self->a_focused_label : self->a_unfocused_label);
        m = (client_focused(self->frame.client) ?
             ((self->max_press ||
              self->frame.client->max_vert || self->frame.client->max_horz) ?
              ob_a_focused_pressed_max : ob_a_focused_unpressed_max) :
             ((self->max_press ||
              self->frame.client->max_vert || self->frame.client->max_horz) ?
              ob_a_unfocused_pressed_max : ob_a_unfocused_unpressed_max));
        n = self->a_icon;
        i = (client_focused(self->frame.client) ?
             (self->iconify_press ?
              ob_a_focused_pressed_iconify : ob_a_focused_unpressed_iconify) :
             (self->iconify_press ?
              ob_a_unfocused_pressed_iconify :
              ob_a_unfocused_unpressed_iconify));
        d = (client_focused(self->frame.client) ?
             (self->desk_press || self->frame.client->desktop == DESKTOP_ALL ?
              ob_a_focused_pressed_desk : ob_a_focused_unpressed_desk) :
             (self->desk_press || self->frame.client->desktop == DESKTOP_ALL ?
              ob_a_unfocused_pressed_desk : ob_a_unfocused_unpressed_desk));
        c = (client_focused(self->frame.client) ?
             (self->close_press ?
              ob_a_focused_pressed_close : ob_a_focused_unpressed_close) :
             (self->close_press ?
              ob_a_unfocused_pressed_close : ob_a_unfocused_unpressed_close));

        paint(self->title, t);

        /* set parents for any parent relative guys */
        l->surface.data.planar.parent = t;
        l->surface.data.planar.parentx = self->label_x;
        l->surface.data.planar.parenty = ob_s_bevel;

        m->surface.data.planar.parent = t;
        m->surface.data.planar.parentx = self->max_x;
        m->surface.data.planar.parenty = ob_s_bevel + 1;

        n->surface.data.planar.parent = t;
        n->surface.data.planar.parentx = self->icon_x;
        n->surface.data.planar.parenty = ob_s_bevel + 1;

        i->surface.data.planar.parent = t;
        i->surface.data.planar.parentx = self->iconify_x;
        i->surface.data.planar.parenty = ob_s_bevel + 1;

        d->surface.data.planar.parent = t;
        d->surface.data.planar.parentx = self->desk_x;
        d->surface.data.planar.parenty = ob_s_bevel + 1;

        c->surface.data.planar.parent = t;
        c->surface.data.planar.parentx = self->close_x;
        c->surface.data.planar.parenty = ob_s_bevel + 1;

        obrender_label(self, l);
        obrender_max(self, m);
        obrender_icon(self, n);
        obrender_iconify(self, i);
        obrender_desk(self, d);
        obrender_close(self, c);
    }

    if (self->frame.client->decorations & Decor_Handle) {
        Appearance *h, *g;

        h = (client_focused(self->frame.client) ?
             self->a_focused_handle : self->a_unfocused_handle);
        g = (client_focused(self->frame.client) ?
             ob_a_focused_grip : ob_a_unfocused_grip);

        if (g->surface.data.planar.grad == Background_ParentRelative) {
            g->surface.data.planar.parent = h;
            paint(self->handle, h);
        } else
            paint(self->handle, h);

        g->surface.data.planar.parentx = 0;
        g->surface.data.planar.parenty = 0;

        paint(self->lgrip, g);

        g->surface.data.planar.parentx = self->width - GRIP_WIDTH;
        g->surface.data.planar.parenty = 0;

        paint(self->rgrip, g);
    }
}

static void obrender_label(ObFrame *self, Appearance *a)
{
    if (self->label_x < 0) return;


    /* set the texture's text! */
    a->texture[0].data.text.string = self->frame.client->title;
    RECT_SET(a->texture[0].position, 0, 0, self->label_width, LABEL_HEIGHT);

    paint(self->label, a);
}

static void obrender_icon(ObFrame *self, Appearance *a)
{
    if (self->icon_x < 0) return;

    if (self->frame.client->nicons) {
        Icon *icon = client_icon(self->frame.client, BUTTON_SIZE, BUTTON_SIZE);
        a->texture[0].type = RGBA;
        a->texture[0].data.rgba.width = icon->width;
        a->texture[0].data.rgba.height = icon->height;
        a->texture[0].data.rgba.data = icon->data;
        RECT_SET(self->a_icon->texture[0].position, 0, 0,
                 BUTTON_SIZE,BUTTON_SIZE);
    } else
        a->texture[0].type = NoTexture;

    paint(self->icon, a);
}

static void obrender_max(ObFrame *self, Appearance *a)
{
    if (self->max_x < 0) return;

    RECT_SET(a->texture[0].position, 0, 0, BUTTON_SIZE,BUTTON_SIZE);
    paint(self->max, a);
}

static void obrender_iconify(ObFrame *self, Appearance *a)
{
    if (self->iconify_x < 0) return;

    RECT_SET(a->texture[0].position, 0, 0, BUTTON_SIZE,BUTTON_SIZE);
    paint(self->iconify, a);
}

static void obrender_desk(ObFrame *self, Appearance *a)
{
    if (self->desk_x < 0) return;

    RECT_SET(a->texture[0].position, 0, 0, BUTTON_SIZE,BUTTON_SIZE);
    paint(self->desk, a);
}

static void obrender_close(ObFrame *self, Appearance *a)
{
    if (self->close_x < 0) return;

    RECT_SET(a->texture[0].position, 0, 0, BUTTON_SIZE,BUTTON_SIZE);
    paint(self->close, a);
}
