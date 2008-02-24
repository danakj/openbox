/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

 frame_default_render.c for the Openbox window manager
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
#include "render.h"

#include "plugin.h"

#include "openbox/screen.h"

static void framerender_label(ObDefaultFrame *self, RrAppearance *a);
static void framerender_icon(ObDefaultFrame *self, RrAppearance *a);
static void framerender_max(ObDefaultFrame *self, RrAppearance *a);
static void framerender_iconify(ObDefaultFrame *self, RrAppearance *a);
static void framerender_desk(ObDefaultFrame *self, RrAppearance *a);
static void framerender_shade(ObDefaultFrame *self, RrAppearance *a);
static void framerender_close(ObDefaultFrame *self, RrAppearance *a);

void frame_update_skin(gpointer _self)
{
    ObDefaultFrame * self = (ObDefaultFrame *) _self;
    
    if (!self->visible)
        return;

    {
        gulong px;

        px = (self->focused ? RrColorPixel(theme_config.cb_focused_color)
                : RrColorPixel(theme_config.cb_unfocused_color));

        XSetWindowBackground(obp_display, self->backback, px);
        XClearWindow(obp_display, self->backback);
        XSetWindowBackground(obp_display, self->innerleft, px);
        XClearWindow(obp_display, self->innerleft);
        XSetWindowBackground(obp_display, self->innertop, px);
        XClearWindow(obp_display, self->innertop);
        XSetWindowBackground(obp_display, self->innerright, px);
        XClearWindow(obp_display, self->innerright);
        XSetWindowBackground(obp_display, self->innerbottom, px);
        XClearWindow(obp_display, self->innerbottom);
        XSetWindowBackground(obp_display, self->innerbll, px);
        XClearWindow(obp_display, self->innerbll);
        XSetWindowBackground(obp_display, self->innerbrr, px);
        XClearWindow(obp_display, self->innerbrr);
        XSetWindowBackground(obp_display, self->innerblb, px);
        XClearWindow(obp_display, self->innerblb);
        XSetWindowBackground(obp_display, self->innerbrb, px);
        XClearWindow(obp_display, self->innerbrb);

        px
                = (self->focused ? RrColorPixel(theme_config.frame_focused_border_color)
                        : RrColorPixel(theme_config.frame_unfocused_border_color));

        XSetWindowBackground(obp_display, self->left, px);
        XClearWindow(obp_display, self->left);
        XSetWindowBackground(obp_display, self->right, px);
        XClearWindow(obp_display, self->right);

        XSetWindowBackground(obp_display, self->titleleft, px);
        XClearWindow(obp_display, self->titleleft);
        XSetWindowBackground(obp_display, self->titletop, px);
        XClearWindow(obp_display, self->titletop);
        XSetWindowBackground(obp_display, self->titletopleft, px);
        XClearWindow(obp_display, self->titletopleft);
        XSetWindowBackground(obp_display, self->titletopright, px);
        XClearWindow(obp_display, self->titletopright);
        XSetWindowBackground(obp_display, self->titleright, px);
        XClearWindow(obp_display, self->titleright);

        XSetWindowBackground(obp_display, self->handleleft, px);
        XClearWindow(obp_display, self->handleleft);
        XSetWindowBackground(obp_display, self->handletop, px);
        XClearWindow(obp_display, self->handletop);
        XSetWindowBackground(obp_display, self->handleright, px);
        XClearWindow(obp_display, self->handleright);
        XSetWindowBackground(obp_display, self->handlebottom, px);
        XClearWindow(obp_display, self->handlebottom);

        XSetWindowBackground(obp_display, self->lgripleft, px);
        XClearWindow(obp_display, self->lgripleft);
        XSetWindowBackground(obp_display, self->lgriptop, px);
        XClearWindow(obp_display, self->lgriptop);
        XSetWindowBackground(obp_display, self->lgripbottom, px);
        XClearWindow(obp_display, self->lgripbottom);

        XSetWindowBackground(obp_display, self->rgripright, px);
        XClearWindow(obp_display, self->rgripright);
        XSetWindowBackground(obp_display, self->rgriptop, px);
        XClearWindow(obp_display, self->rgriptop);
        XSetWindowBackground(obp_display, self->rgripbottom, px);
        XClearWindow(obp_display, self->rgripbottom);

        /* don't use the separator color for shaded windows */
        if (!self->shaded)
            px
                    = (self->focused ? RrColorPixel(theme_config.title_separator_focused_color)
                            : RrColorPixel(theme_config.title_separator_unfocused_color));

        XSetWindowBackground(obp_display, self->titlebottom, px);
        XClearWindow(obp_display, self->titlebottom);
    }

    if (self->decorations & OB_FRAME_DECOR_TITLEBAR) {
        RrAppearance *t, *l, *m, *n, *i, *d, *s, *c, *clear;
        if (self->focused) {

            t = self->a_focused_title;
            l = self->a_focused_label;

            m
                    = (!(self->decorations & OB_FRAME_DECOR_MAXIMIZE) ? theme_config.a_disabled_focused_max
                            : (self->max_vert || self->max_horz ? (self->press_flag
                                    == OB_BUTTON_MAX ? theme_config.a_toggled_focused_pressed_max
                                    : (self->hover_flag == OB_BUTTON_MAX ? theme_config.a_toggled_hover_focused_max
                                            : theme_config.a_toggled_focused_unpressed_max))
                                    : (self->press_flag == OB_BUTTON_MAX ? theme_config.a_focused_pressed_max
                                            : (self->hover_flag
                                                    == OB_BUTTON_MAX ? theme_config.a_hover_focused_max
                                                    : theme_config.a_focused_unpressed_max))));
            n = self->a_icon;
            i
                    = (!(self->decorations & OB_FRAME_DECOR_ICONIFY) ? theme_config.a_disabled_focused_iconify
                            : (self->press_flag == OB_BUTTON_ICONIFY ? theme_config.a_focused_pressed_iconify
                                    : (self->hover_flag == OB_BUTTON_ICONIFY ? theme_config.a_hover_focused_iconify
                                            : theme_config.a_focused_unpressed_iconify)));
            d
                    = (!(self->decorations & OB_FRAME_DECOR_ALLDESKTOPS) ? theme_config.a_disabled_focused_desk
                            : (self->client->desktop == DESKTOP_ALL ? (self->press_flag
                                    == OB_BUTTON_DESK ? theme_config.a_toggled_focused_pressed_desk
                                    : (self->hover_flag == OB_BUTTON_DESK ? theme_config.a_toggled_hover_focused_desk
                                            : theme_config.a_toggled_focused_unpressed_desk))
                                    : (self->press_flag == OB_BUTTON_DESK ? theme_config.a_focused_pressed_desk
                                            : (self->hover_flag
                                                    == OB_BUTTON_DESK ? theme_config.a_hover_focused_desk
                                                    : theme_config.a_focused_unpressed_desk))));
            s
                    = (!(self->decorations & OB_FRAME_DECOR_SHADE) ? theme_config.a_disabled_focused_shade
                            : (self->shaded ? (self->press_flag
                                    == OB_BUTTON_SHADE ? theme_config.a_toggled_focused_pressed_shade
                                    : (self->hover_flag == OB_BUTTON_SHADE ? theme_config.a_toggled_hover_focused_shade
                                            : theme_config.a_toggled_focused_unpressed_shade))
                                    : (self->press_flag == OB_BUTTON_SHADE ? theme_config.a_focused_pressed_shade
                                            : (self->hover_flag
                                                    == OB_BUTTON_SHADE ? theme_config.a_hover_focused_shade
                                                    : theme_config.a_focused_unpressed_shade))));
            c
                    = (!(self->decorations & OB_FRAME_DECOR_CLOSE) ? theme_config.a_disabled_focused_close
                            : (self->press_flag == OB_BUTTON_CLOSE ? theme_config.a_focused_pressed_close
                                    : (self->hover_flag == OB_BUTTON_CLOSE ? theme_config.a_hover_focused_close
                                            : theme_config.a_focused_unpressed_close)));
        }
        else {
            t = self->a_unfocused_title;
            l = self->a_unfocused_label;
            m
                    = (!(self->decorations & OB_FRAME_DECOR_MAXIMIZE) ? theme_config.a_disabled_unfocused_max
                            : (self->max_vert || self->max_horz ? (self->press_flag
                                    == OB_BUTTON_MAX ? theme_config.a_toggled_unfocused_pressed_max
                                    : (self->hover_flag == OB_BUTTON_MAX ? theme_config.a_toggled_hover_unfocused_max
                                            : theme_config.a_toggled_unfocused_unpressed_max))
                                    : (self->press_flag == OB_BUTTON_MAX ? theme_config.a_unfocused_pressed_max
                                            : (self->hover_flag
                                                    == OB_BUTTON_MAX ? theme_config.a_hover_unfocused_max
                                                    : theme_config.a_unfocused_unpressed_max))));
            n = self->a_icon;
            i
                    = (!(self->decorations & OB_FRAME_DECOR_ICONIFY) ? theme_config.a_disabled_unfocused_iconify
                            : (self->press_flag == OB_BUTTON_ICONIFY ? theme_config.a_unfocused_pressed_iconify
                                    : (self->hover_flag == OB_BUTTON_ICONIFY ? theme_config.a_hover_unfocused_iconify
                                            : theme_config.a_unfocused_unpressed_iconify)));
            d
                    = (!(self->decorations & OB_FRAME_DECOR_ALLDESKTOPS) ? theme_config.a_disabled_unfocused_desk
                            : (self->client->desktop == DESKTOP_ALL ? (self->press_flag
                                    == OB_BUTTON_DESK ? theme_config.a_toggled_unfocused_pressed_desk
                                    : (self->hover_flag == OB_BUTTON_DESK ? theme_config.a_toggled_hover_unfocused_desk
                                            : theme_config.a_toggled_unfocused_unpressed_desk))
                                    : (self->hover_flag == OB_BUTTON_DESK ? theme_config.a_unfocused_pressed_desk
                                            : (self->hover_flag
                                                    == OB_BUTTON_DESK ? theme_config.a_hover_unfocused_desk
                                                    : theme_config.a_unfocused_unpressed_desk))));
            s
                    = (!(self->decorations & OB_FRAME_DECOR_SHADE) ? theme_config.a_disabled_unfocused_shade
                            : (self->shaded ? (self->press_flag
                                    == OB_BUTTON_SHADE ? theme_config.a_toggled_unfocused_pressed_shade
                                    : (self->hover_flag == OB_BUTTON_SHADE ? theme_config.a_toggled_hover_unfocused_shade
                                            : theme_config.a_toggled_unfocused_unpressed_shade))
                                    : (self->press_flag == OB_BUTTON_SHADE ? theme_config.a_unfocused_pressed_shade
                                            : (self->hover_flag
                                                    == OB_BUTTON_SHADE ? theme_config.a_hover_unfocused_shade
                                                    : theme_config.a_unfocused_unpressed_shade))));
            c
                    = (!(self->decorations & OB_FRAME_DECOR_CLOSE) ? theme_config.a_disabled_unfocused_close
                            : (self->press_flag == OB_BUTTON_CLOSE ? theme_config.a_unfocused_pressed_close
                                    : (self->hover_flag == OB_BUTTON_CLOSE ? theme_config.a_hover_unfocused_close
                                            : theme_config.a_unfocused_unpressed_close)));
        }
        clear = theme_config.a_clear;

        RrPaint(t, self->title, self->width, theme_config.title_height);

        clear->surface.parent = t;
        clear->surface.parenty = 0;

        clear->surface.parentx = theme_config.grip_width;

        RrPaint(clear, self->topresize, self->width - theme_config.grip_width
                * 2, theme_config.paddingy + 1);

        clear->surface.parentx = 0;

        if (theme_config.grip_width > 0)
            RrPaint(clear, self->tltresize, theme_config.grip_width,
                    theme_config.paddingy + 1);
        if (theme_config.title_height > 0)
            RrPaint(clear, self->tllresize, theme_config.paddingx + 1,
                    theme_config.title_height);

        clear->surface.parentx = self->width - theme_config.grip_width;

        if (theme_config.grip_width > 0)
            RrPaint(clear, self->trtresize, theme_config.grip_width,
                    theme_config.paddingy + 1);

        clear->surface.parentx = self->width - (theme_config.paddingx + 1);

        if (theme_config.title_height > 0)
            RrPaint(clear, self->trrresize, theme_config.paddingx + 1,
                    theme_config.title_height);

        /* set parents for any parent relative guys */
        l->surface.parent = t;
        l->surface.parentx = self->label_x;
        l->surface.parenty = theme_config.paddingy;

        m->surface.parent = t;
        m->surface.parentx = self->max_x;
        m->surface.parenty = theme_config.paddingy + 1;

        n->surface.parent = t;
        n->surface.parentx = self->icon_x;
        n->surface.parenty = theme_config.paddingy;

        i->surface.parent = t;
        i->surface.parentx = self->iconify_x;
        i->surface.parenty = theme_config.paddingy + 1;

        d->surface.parent = t;
        d->surface.parentx = self->desk_x;
        d->surface.parenty = theme_config.paddingy + 1;

        s->surface.parent = t;
        s->surface.parentx = self->shade_x;
        s->surface.parenty = theme_config.paddingy + 1;

        c->surface.parent = t;
        c->surface.parentx = self->close_x;
        c->surface.parenty = theme_config.paddingy + 1;

        framerender_label(self, l);
        framerender_max(self, m);
        framerender_icon(self, n);
        framerender_iconify(self, i);
        framerender_desk(self, d);
        framerender_shade(self, s);
        framerender_close(self, c);
    }

    if (self->decorations & OB_FRAME_DECOR_HANDLE && theme_config.handle_height
            > 0) {
        RrAppearance *h, *g;

        h = (self->focused ? self->a_focused_handle : self->a_unfocused_handle);

        RrPaint(h, self->handle, self->width, theme_config.handle_height);

        if (self->decorations & OB_FRAME_DECOR_GRIPS) {
            g = (self->focused ? theme_config.a_focused_grip
                    : theme_config.a_unfocused_grip);

            if (g->surface.grad == RR_SURFACE_PARENTREL)
                g->surface.parent = h;

            g->surface.parentx = 0;
            g->surface.parenty = 0;

            RrPaint(g, self->lgrip, theme_config.grip_width,
                    theme_config.handle_height);

            g->surface.parentx = self->width - theme_config.grip_width;
            g->surface.parenty = 0;

            RrPaint(g, self->rgrip, theme_config.grip_width,
                    theme_config.handle_height);
        }
    }

}

static void framerender_label(ObDefaultFrame *self, RrAppearance *a)
{
    if (!self->label_on)
        return;
    /* set the texture's text! */
    a->texture[0].data.text.string = self->stitle;
    RrPaint(a, self->label, self->label_width, theme_config.label_height);
}

static void framerender_icon(ObDefaultFrame *self, RrAppearance *a)
{
    const ObClientIcon *icon;

    if (!self->icon_on)
        return;

    icon = client_icon(self->client, theme_config.button_size + 2,
            theme_config.button_size + 2);
    if (icon) {
        a->texture[0].type = RR_TEXTURE_RGBA;
        a->texture[0].data.rgba.width = icon->width;
        a->texture[0].data.rgba.height = icon->height;
        a->texture[0].data.rgba.alpha = 0xff;
        a->texture[0].data.rgba.data = icon->data;
    }
    else
        a->texture[0].type = RR_TEXTURE_NONE;

    RrPaint(a, self->icon, theme_config.button_size + 2,
            theme_config.button_size + 2);
}

static void framerender_max(ObDefaultFrame *self, RrAppearance *a)
{
    if (!self->max_on)
        return;
    RrPaint(a, self->max, theme_config.button_size, theme_config.button_size);
}

static void framerender_iconify(ObDefaultFrame *self, RrAppearance *a)
{
    if (!self->iconify_on)
        return;
    RrPaint(a, self->iconify, theme_config.button_size,
            theme_config.button_size);
}

static void framerender_desk(ObDefaultFrame *self, RrAppearance *a)
{
    if (!self->desk_on)
        return;
    RrPaint(a, self->desk, theme_config.button_size, theme_config.button_size);
}

static void framerender_shade(ObDefaultFrame *self, RrAppearance *a)
{
    if (!self->shade_on)
        return;
    RrPaint(a, self->shade, theme_config.button_size, theme_config.button_size);
}

static void framerender_close(ObDefaultFrame *self, RrAppearance *a)
{
    if (!self->close_on)
        return;
    RrPaint(a, self->close, theme_config.button_size, theme_config.button_size);
}
