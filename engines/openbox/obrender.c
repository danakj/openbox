#include "obengine.h"
#include "../../kernel/openbox.h"
#include "../../kernel/screen.h"

static void render_label(ObFrame *self, Appearance *a);
static void render_max(ObFrame *self, Appearance *a);
static void render_icon(ObFrame *self, Appearance *a);
static void render_iconify(ObFrame *self, Appearance *a);
static void render_desk(ObFrame *self, Appearance *a);
static void render_close(ObFrame *self, Appearance *a);

void render_frame(ObFrame *self)
{
    if (client_focused(self->frame.client)) {
        XSetWindowBorder(ob_display, self->frame.plate,
                         s_cb_focused_color->pixel);
    } else {
        XSetWindowBorder(ob_display, self->frame.plate,
                         s_cb_unfocused_color->pixel);
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
              a_focused_pressed_max : a_focused_unpressed_max) :
             ((self->max_press ||
              self->frame.client->max_vert || self->frame.client->max_horz) ?
              a_unfocused_pressed_max : a_unfocused_unpressed_max));
        n = self->a_icon;
        i = (client_focused(self->frame.client) ?
             (self->iconify_press ?
              a_focused_pressed_iconify : a_focused_unpressed_iconify) :
             (self->iconify_press ?
              a_unfocused_pressed_iconify : a_unfocused_unpressed_iconify));
        d = (client_focused(self->frame.client) ?
             (self->desk_press || self->frame.client->desktop == DESKTOP_ALL ?
              a_focused_pressed_desk : a_focused_unpressed_desk) :
             (self->desk_press || self->frame.client->desktop == DESKTOP_ALL ?
              a_unfocused_pressed_desk : a_unfocused_unpressed_desk));
        c = (client_focused(self->frame.client) ?
             (self->close_press ?
              a_focused_pressed_close : a_focused_unpressed_close) :
             (self->close_press ?
              a_unfocused_pressed_close : a_unfocused_unpressed_close));

        paint(self->title, t);

        /* set parents for any parent relative guys */
        l->surface.data.planar.parent = t;
        l->surface.data.planar.parentx = self->label_x;
        l->surface.data.planar.parenty = s_bevel;

        m->surface.data.planar.parent = t;
        m->surface.data.planar.parentx = self->max_x;
        m->surface.data.planar.parenty = s_bevel + 1;

        n->surface.data.planar.parent = t;
        n->surface.data.planar.parentx = self->icon_x;
        n->surface.data.planar.parenty = s_bevel + 1;

        i->surface.data.planar.parent = t;
        i->surface.data.planar.parentx = self->iconify_x;
        i->surface.data.planar.parenty = s_bevel + 1;

        d->surface.data.planar.parent = t;
        d->surface.data.planar.parentx = self->desk_x;
        d->surface.data.planar.parenty = s_bevel + 1;

        c->surface.data.planar.parent = t;
        c->surface.data.planar.parentx = self->close_x;
        c->surface.data.planar.parenty = s_bevel + 1;

        render_label(self, l);
        render_max(self, m);
        render_icon(self, n);
        render_iconify(self, i);
        render_desk(self, d);
        render_close(self, c);
    }

    if (self->frame.client->decorations & Decor_Handle) {
        Appearance *h, *g;

        h = (client_focused(self->frame.client) ?
             self->a_focused_handle : self->a_unfocused_handle);
        g = (client_focused(self->frame.client) ?
             a_focused_grip : a_unfocused_grip);

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

static void render_label(ObFrame *self, Appearance *a)
{
    if (self->label_x < 0) return;


    /* set the texture's text! */
    a->texture[0].data.text.string = self->frame.client->title;
    RECT_SET(a->texture[0].position, 0, 0, self->label_width, LABEL_HEIGHT);

    paint(self->label, a);
}

static void render_icon(ObFrame *self, Appearance *a)
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

static void render_max(ObFrame *self, Appearance *a)
{
    if (self->max_x < 0) return;

    RECT_SET(a->texture[0].position, 0, 0, BUTTON_SIZE,BUTTON_SIZE);
    paint(self->max, a);
}

static void render_iconify(ObFrame *self, Appearance *a)
{
    if (self->iconify_x < 0) return;

    RECT_SET(a->texture[0].position, 0, 0, BUTTON_SIZE,BUTTON_SIZE);
    paint(self->iconify, a);
}

static void render_desk(ObFrame *self, Appearance *a)
{
    if (self->desk_x < 0) return;

    RECT_SET(a->texture[0].position, 0, 0, BUTTON_SIZE,BUTTON_SIZE);
    paint(self->desk, a);
}

static void render_close(ObFrame *self, Appearance *a)
{
    if (self->close_x < 0) return;

    RECT_SET(a->texture[0].position, 0, 0, BUTTON_SIZE,BUTTON_SIZE);
    paint(self->close, a);
}

GQuark get_context(Client *client, Window win)
{
    ObFrame *self;

    if (win == ob_root) return g_quark_try_string("root");
    if (client == NULL) return g_quark_try_string("none");
    if (win == client->window) return g_quark_try_string("client");

    self = (ObFrame*) client->frame;
    if (win == self->frame.window) return g_quark_try_string("frame");
    if (win == self->frame.plate)  return g_quark_try_string("client");
    if (win == self->title)  return g_quark_try_string("titlebar");
    if (win == self->label)  return g_quark_try_string("titlebar");
    if (win == self->handle) return g_quark_try_string("handle");
    if (win == self->lgrip)  return g_quark_try_string("blcorner");
    if (win == self->rgrip)  return g_quark_try_string("brcorner");
    if (win == self->max)  return g_quark_try_string("maximize");
    if (win == self->iconify)  return g_quark_try_string("iconify");
    if (win == self->close)  return g_quark_try_string("close");
    if (win == self->icon)  return g_quark_try_string("icon");
    if (win == self->desk)  return g_quark_try_string("alldesktops");

    return g_quark_try_string("none");
}
