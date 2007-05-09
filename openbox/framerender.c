/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   framerender.c for the Openbox window manager
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

#include "frame.h"
#include "openbox.h"
#include "screen.h"
#include "client.h"
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
    {
        gulong px;

        px = (self->focused ?
              RrColorPixel(ob_rr_theme->cb_focused_color) :
              RrColorPixel(ob_rr_theme->cb_unfocused_color));
        XSetWindowBackground(ob_display, self->inner, px);
        XClearWindow(ob_display, self->inner);
    }

    if (self->decorations & OB_FRAME_DECOR_TITLEBAR) {
        RrAppearance *t, *l, *m, *n, *i, *d, *s, *c;
        if (self->focused) {

          t = self->a_focused_title;
          l = self->a_focused_label;
          m = (!(self->decorations & OB_FRAME_DECOR_MAXIMIZE) ?
               ob_rr_theme->a_disabled_focused_max :
               (self->client->max_vert || self->client->max_horz ?
                ob_rr_theme->a_toggled_focused_max :
                (self->max_press ?
                 ob_rr_theme->a_focused_pressed_max :
                 (self->max_hover ?
                  ob_rr_theme->a_hover_focused_max : 
                  ob_rr_theme->a_focused_unpressed_max))));
          n = self->a_icon;
          i = (!(self->decorations & OB_FRAME_DECOR_ICONIFY) ?
               ob_rr_theme->a_disabled_focused_iconify :
               (self->iconify_press ?
                ob_rr_theme->a_focused_pressed_iconify :
                (self->iconify_hover ?
                 ob_rr_theme->a_hover_focused_iconify : 
                 ob_rr_theme->a_focused_unpressed_iconify)));
          d = (!(self->decorations & OB_FRAME_DECOR_ALLDESKTOPS) ?
               ob_rr_theme->a_disabled_focused_desk :
               (self->client->desktop == DESKTOP_ALL ?
                ob_rr_theme->a_toggled_focused_desk :
                (self->desk_press ?
                 ob_rr_theme->a_focused_pressed_desk :
                 (self->desk_hover ?
                  ob_rr_theme->a_hover_focused_desk : 
                  ob_rr_theme->a_focused_unpressed_desk))));
          s = (!(self->decorations & OB_FRAME_DECOR_SHADE) ?
               ob_rr_theme->a_disabled_focused_shade :
               (self->client->shaded ?
                ob_rr_theme->a_toggled_focused_shade :
                (self->shade_press ?
                 ob_rr_theme->a_focused_pressed_shade :
                 (self->shade_hover ?
                  ob_rr_theme->a_hover_focused_shade : 
                  ob_rr_theme->a_focused_unpressed_shade))));
          c = (!(self->decorations & OB_FRAME_DECOR_CLOSE) ?
               ob_rr_theme->a_disabled_focused_close :
               (self->close_press ?
                ob_rr_theme->a_focused_pressed_close :
                (self->close_hover ?
                 ob_rr_theme->a_hover_focused_close : 
                 ob_rr_theme->a_focused_unpressed_close)));
        } else {

            t = self->a_unfocused_title;
            l = self->a_unfocused_label;
            m = (!(self->decorations & OB_FRAME_DECOR_MAXIMIZE) ?
                 ob_rr_theme->a_disabled_unfocused_max :
                 (self->client->max_vert || self->client->max_horz ?
                  ob_rr_theme->a_toggled_unfocused_max :
                  (self->max_press ?
                   ob_rr_theme->a_unfocused_pressed_max :
                   (self->max_hover ?
                    ob_rr_theme->a_hover_unfocused_max : 
                    ob_rr_theme->a_unfocused_unpressed_max))));
            n = self->a_icon;
            i = (!(self->decorations & OB_FRAME_DECOR_ICONIFY) ?
                 ob_rr_theme->a_disabled_unfocused_iconify :
                 (self->iconify_press ?
                  ob_rr_theme->a_unfocused_pressed_iconify :
                  (self->iconify_hover ?
                   ob_rr_theme->a_hover_unfocused_iconify : 
                   ob_rr_theme->a_unfocused_unpressed_iconify)));
            d = (!(self->decorations & OB_FRAME_DECOR_ALLDESKTOPS) ?
                 ob_rr_theme->a_disabled_unfocused_desk :
                 (self->client->desktop == DESKTOP_ALL ?
                  ob_rr_theme->a_toggled_unfocused_desk :
                  (self->desk_press ?
                   ob_rr_theme->a_unfocused_pressed_desk :
                   (self->desk_hover ?
                    ob_rr_theme->a_hover_unfocused_desk : 
                    ob_rr_theme->a_unfocused_unpressed_desk))));
          s = (!(self->decorations & OB_FRAME_DECOR_SHADE) ?
              ob_rr_theme->a_disabled_unfocused_shade :
               (self->client->shaded ?
                ob_rr_theme->a_toggled_unfocused_shade :
                (self->shade_press ?
                 ob_rr_theme->a_unfocused_pressed_shade :
                 (self->shade_hover ?
                  ob_rr_theme->a_hover_unfocused_shade : 
                  ob_rr_theme->a_unfocused_unpressed_shade))));
          c = (!(self->decorations & OB_FRAME_DECOR_CLOSE) ?
               ob_rr_theme->a_disabled_unfocused_close :
               (self->close_press ?
                ob_rr_theme->a_unfocused_pressed_close :
                (self->close_hover ?
                 ob_rr_theme->a_hover_unfocused_close : 
                 ob_rr_theme->a_unfocused_unpressed_close)));
        }

        RrPaint(t, self->title, self->width, ob_rr_theme->title_height);

        ob_rr_theme->a_clear->surface.parent = t;
        ob_rr_theme->a_clear->surface.parentx = 0;
        ob_rr_theme->a_clear->surface.parenty = 0;

        if (ob_rr_theme->grip_width > 0)
            RrPaint(ob_rr_theme->a_clear, self->tltresize,
                    ob_rr_theme->grip_width, ob_rr_theme->paddingy + 1);
        if (ob_rr_theme->title_height > 0)
            RrPaint(ob_rr_theme->a_clear, self->tllresize,
                    ob_rr_theme->paddingx + 1, ob_rr_theme->title_height);

        ob_rr_theme->a_clear->surface.parentx =
            self->width - ob_rr_theme->grip_width;

        if (ob_rr_theme->grip_width > 0)
            RrPaint(ob_rr_theme->a_clear, self->trtresize,
                    ob_rr_theme->grip_width, ob_rr_theme->paddingy + 1);

        ob_rr_theme->a_clear->surface.parentx =
            self->width - (ob_rr_theme->paddingx + 1);

        if (ob_rr_theme->title_height > 0)
            RrPaint(ob_rr_theme->a_clear, self->trrresize,
                    ob_rr_theme->paddingx + 1, ob_rr_theme->title_height);

        /* set parents for any parent relative guys */
        l->surface.parent = t;
        l->surface.parentx = self->label_x;
        l->surface.parenty = ob_rr_theme->paddingy;

        m->surface.parent = t;
        m->surface.parentx = self->max_x;
        m->surface.parenty = ob_rr_theme->paddingy + 1;

        n->surface.parent = t;
        n->surface.parentx = self->icon_x;
        n->surface.parenty = ob_rr_theme->paddingy;

        i->surface.parent = t;
        i->surface.parentx = self->iconify_x;
        i->surface.parenty = ob_rr_theme->paddingy + 1;

        d->surface.parent = t;
        d->surface.parentx = self->desk_x;
        d->surface.parenty = ob_rr_theme->paddingy + 1;

        s->surface.parent = t;
        s->surface.parentx = self->shade_x;
        s->surface.parenty = ob_rr_theme->paddingy + 1;

        c->surface.parent = t;
        c->surface.parentx = self->close_x;
        c->surface.parenty = ob_rr_theme->paddingy + 1;

        framerender_label(self, l);
        framerender_max(self, m);
        framerender_icon(self, n);
        framerender_iconify(self, i);
        framerender_desk(self, d);
        framerender_shade(self, s);
        framerender_close(self, c);
    }

    if (self->decorations & OB_FRAME_DECOR_HANDLE &&
        ob_rr_theme->handle_height > 0)
    {
        RrAppearance *h, *g;

        h = (self->focused ?
             self->a_focused_handle : self->a_unfocused_handle);

        RrPaint(h, self->handle, self->width, ob_rr_theme->handle_height);

        if (self->decorations & OB_FRAME_DECOR_GRIPS) {
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

    XFlush(ob_display);
}

static void framerender_label(ObFrame *self, RrAppearance *a)
{
    if (!self->label_on) return;
    /* set the texture's text! */
    a->texture[0].data.text.string = self->client->title;
    RrPaint(a, self->label, self->label_width, ob_rr_theme->label_height);
}

static void framerender_icon(ObFrame *self, RrAppearance *a)
{
    const ObClientIcon *icon;

    if (!self->icon_on) return;

    icon = client_icon(self->client,
                       ob_rr_theme->button_size + 2,
                       ob_rr_theme->button_size + 2);
    if (icon) {
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
    if (!self->max_on) return;
    RrPaint(a, self->max, ob_rr_theme->button_size, ob_rr_theme->button_size);
}

static void framerender_iconify(ObFrame *self, RrAppearance *a)
{
    if (!self->iconify_on) return;
    RrPaint(a, self->iconify,
            ob_rr_theme->button_size, ob_rr_theme->button_size);
}

static void framerender_desk(ObFrame *self, RrAppearance *a)
{
    if (!self->desk_on) return;
    RrPaint(a, self->desk, ob_rr_theme->button_size, ob_rr_theme->button_size);
}

static void framerender_shade(ObFrame *self, RrAppearance *a)
{
    if (!self->shade_on) return;
    RrPaint(a, self->shade,
            ob_rr_theme->button_size, ob_rr_theme->button_size);
}

static void framerender_close(ObFrame *self, RrAppearance *a)
{
    if (!self->close_on) return;
    RrPaint(a, self->close,
            ob_rr_theme->button_size, ob_rr_theme->button_size);
}
