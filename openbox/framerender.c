#include "frame.h"
#include "openbox.h"
#include "screen.h"
#include "framerender.h"
#include "render/theme.h"

static void framerender_label(ObFrame *self, RrAppearance *a);
static void framerender_icon(ObFrame *self, RrAppearance *a);
static void framerender_max(ObFrame *self, RrAppearance *a);
static void framerender_iconify(ObFrame *self, RrAppearance *a);
static void framerender_desk(ObFrame *self, RrAppearance *a);
static void framerender_shade(ObFrame *self, RrAppearance *a);
static void framerender_close(ObFrame *self, RrAppearance *a);

void framerender_frame(ObFrame *self)
{
    if (self->focused)
        XSetWindowBorder(ob_display, self->plate,
                         ob_rr_theme->cb_focused_color->pixel);
    else
        XSetWindowBorder(ob_display, self->plate,
                         ob_rr_theme->cb_unfocused_color->pixel);

    if (self->client->decorations & Decor_Titlebar) {
        RrAppearance *t, *l, *m, *n, *i, *d, *s, *c;

        t = (self->focused ?
             self->a_focused_title : self->a_unfocused_title);
        l = (self->focused ?
             self->a_focused_label : self->a_unfocused_label);
        m = (self->focused ?
             (self->client->max_vert || self->client->max_horz ?
              ob_rr_theme->a_focused_pressed_set_max :
              (self->max_press ?
               ob_rr_theme->a_focused_pressed_max :
               ob_rr_theme->a_focused_unpressed_max)) :
             (self->client->max_vert || self->client->max_horz ?
              ob_rr_theme->a_unfocused_pressed_set_max :
              (self->max_press ?
               ob_rr_theme->a_unfocused_pressed_max :
               ob_rr_theme->a_unfocused_unpressed_max)));
        n = self->a_icon;
        i = (self->focused ?
             (self->iconify_press ?
              ob_rr_theme->a_focused_pressed_iconify :
              ob_rr_theme->a_focused_unpressed_iconify) :
             (self->iconify_press ?
              ob_rr_theme->a_unfocused_pressed_iconify :
              ob_rr_theme->a_unfocused_unpressed_iconify));
        d = (self->focused ?
             (self->client->desktop == DESKTOP_ALL ?
              ob_rr_theme->a_focused_pressed_set_desk :
              (self->desk_press ?
               ob_rr_theme->a_focused_pressed_desk :
               ob_rr_theme->a_focused_unpressed_desk)) :
             (self->client->desktop == DESKTOP_ALL ?
              ob_rr_theme->a_unfocused_pressed_set_desk :
              (self->desk_press ?
               ob_rr_theme->a_unfocused_pressed_desk :
               ob_rr_theme->a_unfocused_unpressed_desk)));
        s = (self->focused ?
             (self->client->shaded ?
              ob_rr_theme->a_focused_pressed_set_shade :
              (self->shade_press ?
               ob_rr_theme->a_focused_pressed_shade :
               ob_rr_theme->a_focused_unpressed_shade)) :
             (self->client->shaded ?
              ob_rr_theme->a_unfocused_pressed_set_shade :
              (self->shade_press ?
               ob_rr_theme->a_unfocused_pressed_shade :
               ob_rr_theme->a_unfocused_unpressed_shade)));
        c = (self->focused ?
             (self->close_press ?
              ob_rr_theme->a_focused_pressed_close :
              ob_rr_theme->a_focused_unpressed_close) :
             (self->close_press ?
              ob_rr_theme->a_unfocused_pressed_close :
              ob_rr_theme->a_unfocused_unpressed_close));

        RrPaint(t, self->title, self->width, ob_rr_theme->title_height);

        /* set parents for any parent relative guys */
        l->surface.parent = t;
        l->surface.parentx = self->label_x;
        l->surface.parenty = ob_rr_theme->bevel;

        m->surface.parent = t;
        m->surface.parentx = self->max_x;
        m->surface.parenty = ob_rr_theme->bevel + 1;

        n->surface.parent = t;
        n->surface.parentx = self->icon_x;
        n->surface.parenty = ob_rr_theme->bevel;

        i->surface.parent = t;
        i->surface.parentx = self->iconify_x;
        i->surface.parenty = ob_rr_theme->bevel + 1;

        d->surface.parent = t;
        d->surface.parentx = self->desk_x;
        d->surface.parenty = ob_rr_theme->bevel + 1;

        s->surface.parent = t;
        s->surface.parentx = self->shade_x;
        s->surface.parenty = ob_rr_theme->bevel + 1;

        c->surface.parent = t;
        c->surface.parentx = self->close_x;
        c->surface.parenty = ob_rr_theme->bevel + 1;

        framerender_label(self, l);
        framerender_max(self, m);
        framerender_icon(self, n);
        framerender_iconify(self, i);
        framerender_desk(self, d);
        framerender_shade(self, s);
        framerender_close(self, c);
    }

    if (self->client->decorations & Decor_Handle) {
        RrAppearance *h, *g;

        h = (self->focused ?
             self->a_focused_handle : self->a_unfocused_handle);

        RrPaint(h, self->handle, self->width, ob_rr_theme->handle_height);

        g = (self->focused ?
             ob_rr_theme->a_focused_grip : ob_rr_theme->a_unfocused_grip);

        if (g->surface.grad == RR_SURFACE_PARENTREL)
            g->surface.parent = h;

        g->surface.parentx = 0;
        g->surface.parenty = 0;

        RrPaint(g, self->lgrip,
                ob_rr_theme->grip_width, ob_rr_theme->handle_height);

        g->surface.parentx = self->width - ob_rr_theme->grip_width;
        g->surface.parenty = 0;

        RrPaint(g, self->rgrip,
                ob_rr_theme->grip_width, ob_rr_theme->handle_height);
    }
}

static void framerender_label(ObFrame *self, RrAppearance *a)
{
    if (self->label_x < 0) return;
    /* set the texture's text! */
    a->texture[0].data.text.string = self->client->title;
    RrPaint(a, self->label, self->label_width, ob_rr_theme->label_height);
}

static void framerender_icon(ObFrame *self, RrAppearance *a)
{
    if (self->icon_x < 0) return;

    if (self->client->nicons) {
        ObClientIcon *icon = client_icon(self->client,
                                         ob_rr_theme->button_size + 2,
                                         ob_rr_theme->button_size + 2);
        a->texture[0].type = RR_TEXTURE_RGBA;
        a->texture[0].data.rgba.width = icon->width;
        a->texture[0].data.rgba.height = icon->height;
        a->texture[0].data.rgba.data = icon->data;
    } else
        a->texture[0].type = RR_TEXTURE_NONE;

    RrPaint(a, self->icon,
            ob_rr_theme->button_size + 2, ob_rr_theme->button_size + 2);
}

static void framerender_max(ObFrame *self, RrAppearance *a)
{
    if (self->max_x < 0) return;
    RrPaint(a, self->max, ob_rr_theme->button_size, ob_rr_theme->button_size);
}

static void framerender_iconify(ObFrame *self, RrAppearance *a)
{
    if (self->iconify_x < 0) return;
    RrPaint(a, self->iconify,
            ob_rr_theme->button_size, ob_rr_theme->button_size);
}

static void framerender_desk(ObFrame *self, RrAppearance *a)
{
    if (self->desk_x < 0) return;
    RrPaint(a, self->desk, ob_rr_theme->button_size, ob_rr_theme->button_size);
}

static void framerender_shade(ObFrame *self, RrAppearance *a)
{
    if (self->shade_x < 0) return;
    RrPaint(a, self->shade,
            ob_rr_theme->button_size, ob_rr_theme->button_size);
}

static void framerender_close(ObFrame *self, RrAppearance *a)
{
    if (self->close_x < 0) return;
    RrPaint(a, self->close,
            ob_rr_theme->button_size, ob_rr_theme->button_size);
}
