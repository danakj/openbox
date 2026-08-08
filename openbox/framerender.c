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
#include "config.h"
#include "framerender.h"
#include "obrender/theme.h"

static void framerender_label(ObFrame *self, RrAppearance *a);
static void framerender_icon(ObFrame *self, RrAppearance *a);
static void framerender_max(ObFrame *self, RrAppearance *a);
static void framerender_iconify(ObFrame *self, RrAppearance *a);
static void framerender_desk(ObFrame *self, RrAppearance *a);
static void framerender_shade(ObFrame *self, RrAppearance *a);
static void framerender_close(ObFrame *self, RrAppearance *a);

void framerender_frame(ObFrame *self)
{
    if (frame_iconify_animating(self))
        return; /* delay redrawing until the animation is done */
    if (!self->need_render)
        return;
    if (!self->visible)
        return;
    self->need_render = FALSE;

    {
        if ((self->decorations & OB_FRAME_DECOR_TITLEBAR) && !self->max_horz && !self->max_vert && config_theme_roundcorners)
        {
            XGCValues xgcv;
            XWindowAttributes wd_att;
            XGetWindowAttributes (obt_display, self->window, &wd_att);
            Pixmap mask = XCreatePixmap (obt_display, self->window, wd_att.width, wd_att.height, 1);
            GC shape_gc = XCreateGC (obt_display, mask, 0, &xgcv);
            XSetForeground (obt_display, shape_gc, 1);
            XFillRectangle (obt_display, mask, shape_gc, 0, 0, wd_att.width, wd_att.height);
            XSetForeground (obt_display, shape_gc, 0);

            XFillRectangle (obt_display, mask, shape_gc, 0, 0, 2, 2);
            XFillRectangle (obt_display, mask, shape_gc, 2, 0, 2, 1);
            XFillRectangle (obt_display, mask, shape_gc, 0, 2, 1, 2);

            XFillRectangle (obt_display, mask, shape_gc, 0, wd_att.height - 2, 2, 2);
            XFillRectangle (obt_display, mask, shape_gc, 2, wd_att.height - 1, 2, 1);
            XFillRectangle (obt_display, mask, shape_gc, 0, wd_att.height - 4, 1, 2);

            XFillRectangle (obt_display, mask, shape_gc, wd_att.width - 2, 0, 2, 2);
            XFillRectangle (obt_display, mask, shape_gc, wd_att.width - 4, 0, 2, 1);
            XFillRectangle (obt_display, mask, shape_gc, wd_att.width - 1, 2, 1, 2);

            XFillRectangle (obt_display, mask, shape_gc, wd_att.width - 2, wd_att.height - 2, 2, 2);
            XFillRectangle (obt_display, mask, shape_gc, wd_att.width - 4, wd_att.height - 1, 2, 1);
            XFillRectangle (obt_display, mask, shape_gc, wd_att.width - 1, wd_att.height - 4, 1, 2);

            XShapeCombineMask (obt_display, self->window, ShapeBounding, 0, 0, mask, ShapeSet);
            XFreePixmap (obt_display, mask);
        }

        gulong px;

        px = (self->focused ?
              RrColorPixel(ob_rr_theme->cb_focused_color) :
              RrColorPixel(ob_rr_theme->cb_unfocused_color));

        XSetWindowBackground(obt_display, self->backback, px);
        XClearWindow(obt_display, self->backback);
        XSetWindowBackground(obt_display, self->innerleft, px);
        XClearWindow(obt_display, self->innerleft);
        XSetWindowBackground(obt_display, self->innertop, px);
        XClearWindow(obt_display, self->innertop);
        XSetWindowBackground(obt_display, self->innerright, px);
        XClearWindow(obt_display, self->innerright);
        XSetWindowBackground(obt_display, self->innerbottom, px);
        XClearWindow(obt_display, self->innerbottom);
        XSetWindowBackground(obt_display, self->innerbll, px);
        XClearWindow(obt_display, self->innerbll);
        XSetWindowBackground(obt_display, self->innerbrr, px);
        XClearWindow(obt_display, self->innerbrr);
        XSetWindowBackground(obt_display, self->innerblb, px);
        XClearWindow(obt_display, self->innerblb);
        XSetWindowBackground(obt_display, self->innerbrb, px);
        XClearWindow(obt_display, self->innerbrb);

        px = RrColorPixel(self->focused ?
            (self->client->undecorated ?
             ob_rr_theme->frame_undecorated_focused_border_color :
             ob_rr_theme->frame_focused_border_color) :
            (self->client->undecorated ?
             ob_rr_theme->frame_undecorated_unfocused_border_color :
             ob_rr_theme->frame_unfocused_border_color));

        XSetWindowBackground(obt_display, self->left, px);
        XClearWindow(obt_display, self->left);
        XSetWindowBackground(obt_display, self->right, px);
        XClearWindow(obt_display, self->right);

        XSetWindowBackground(obt_display, self->titleleft, px);
        XClearWindow(obt_display, self->titleleft);
        XSetWindowBackground(obt_display, self->titletop, px);
        XClearWindow(obt_display, self->titletop);
        XSetWindowBackground(obt_display, self->titletopleft, px);
        XClearWindow(obt_display, self->titletopleft);
        XSetWindowBackground(obt_display, self->titletopright, px);
        XClearWindow(obt_display, self->titletopright);
        XSetWindowBackground(obt_display, self->titleright, px);
        XClearWindow(obt_display, self->titleright);

        XSetWindowBackground(obt_display, self->handleleft, px);
        XClearWindow(obt_display, self->handleleft);
        XSetWindowBackground(obt_display, self->handletop, px);
        XClearWindow(obt_display, self->handletop);
        XSetWindowBackground(obt_display, self->handleright, px);
        XClearWindow(obt_display, self->handleright);
        XSetWindowBackground(obt_display, self->handlebottom, px);
        XClearWindow(obt_display, self->handlebottom);

        XSetWindowBackground(obt_display, self->lgripleft, px);
        XClearWindow(obt_display, self->lgripleft);
        XSetWindowBackground(obt_display, self->lgriptop, px);
        XClearWindow(obt_display, self->lgriptop);
        XSetWindowBackground(obt_display, self->lgripbottom, px);
        XClearWindow(obt_display, self->lgripbottom);

        XSetWindowBackground(obt_display, self->rgripright, px);
        XClearWindow(obt_display, self->rgripright);
        XSetWindowBackground(obt_display, self->rgriptop, px);
        XClearWindow(obt_display, self->rgriptop);
        XSetWindowBackground(obt_display, self->rgripbottom, px);
        XClearWindow(obt_display, self->rgripbottom);

        /* don't use the separator color for shaded windows */
        if (!self->client->shaded)
            px = (self->focused ?
                  RrColorPixel(ob_rr_theme->title_separator_focused_color) :
                  RrColorPixel(ob_rr_theme->title_separator_unfocused_color));

        XSetWindowBackground(obt_display, self->titlebottom, px);
        XClearWindow(obt_display, self->titlebottom);

        px = (self->focused ?
            RrColorPixel (ob_rr_theme->a_focused_title->surface.primary) :
            RrColorPixel (ob_rr_theme->a_unfocused_title->surface.primary));

        XSetWindowBackground(obt_display, self->outertop, px);
        XClearWindow(obt_display, self->outertop);
        XSetWindowBackground(obt_display, self->outerlefttop, px);
        XClearWindow(obt_display, self->outerlefttop);
        XSetWindowBackground(obt_display, self->outerrighttop, px);
        XClearWindow(obt_display, self->outerrighttop);
        XSetWindowBackground(obt_display, self->outertopleft, px);
        XClearWindow(obt_display, self->outertopleft);
        XSetWindowBackground(obt_display, self->outertopright, px);
        XClearWindow(obt_display, self->outertopright);

        px = (self->focused ?
              RrColorPixel(ob_rr_theme->cb_focused_color) :
              RrColorPixel(ob_rr_theme->cb_unfocused_color));

        XSetWindowBackground(obt_display, self->outerleft, px);
        XClearWindow(obt_display, self->outerleft);
        XSetWindowBackground(obt_display, self->outerright, px);
        XClearWindow(obt_display, self->outerright);
        XSetWindowBackground(obt_display, self->outerbottom, px);
        XClearWindow(obt_display, self->outerbottom);
        XSetWindowBackground(obt_display, self->outerleftbottom, px);
        XClearWindow(obt_display, self->outerleftbottom);
        XSetWindowBackground(obt_display, self->outerrightbottom, px);
        XClearWindow(obt_display, self->outerrightbottom);
        XSetWindowBackground(obt_display, self->outerbottomleft, px);
        XClearWindow(obt_display, self->outerbottomleft);
        XSetWindowBackground(obt_display, self->outerbottomright, px);
        XClearWindow(obt_display, self->outerbottomright);

        px = RrColorPixel (ob_rr_theme->frame_focused_border_color);

        XSetWindowBackground(obt_display, self->edgeleft, px);
        XClearWindow(obt_display, self->edgeleft);
        XSetWindowBackground(obt_display, self->edgeright, px);
        XClearWindow(obt_display, self->edgeright);
        XSetWindowBackground(obt_display, self->edgebottom, px);
        XClearWindow(obt_display, self->edgebottom);
        XSetWindowBackground(obt_display, self->edgebottomleft, px);
        XClearWindow(obt_display, self->edgebottomleft);
        XSetWindowBackground(obt_display, self->edgebottomright, px);
        XClearWindow(obt_display, self->edgebottomright);
        XSetWindowBackground(obt_display, self->edgeleftbottom, px);
        XClearWindow(obt_display, self->edgeleftbottom);
        XSetWindowBackground(obt_display, self->edgerightbottom, px);
        XClearWindow(obt_display, self->edgerightbottom);
        XSetWindowBackground(obt_display, self->ce_bl_b, px);
        XClearWindow(obt_display, self->ce_bl_b);
        XSetWindowBackground(obt_display, self->ce_bl_l, px);
        XClearWindow(obt_display, self->ce_bl_l);
        XSetWindowBackground(obt_display, self->ce_br_b, px);
        XClearWindow(obt_display, self->ce_br_b);
        XSetWindowBackground(obt_display, self->ce_br_r, px);
        XClearWindow(obt_display, self->ce_br_r);

        px = (self->focused ?
              RrColorPixel (ob_rr_theme->a_focused_title->surface.primary) :
              RrColorPixel(ob_rr_theme->frame_focused_border_color));

        XSetWindowBackground(obt_display, self->edgetop, px);
        XClearWindow(obt_display, self->edgetop);
        XSetWindowBackground(obt_display, self->edgelefttop, px);
        XClearWindow(obt_display, self->edgelefttop);
        XSetWindowBackground(obt_display, self->edgerighttop, px);
        XClearWindow(obt_display, self->edgerighttop);
        XSetWindowBackground(obt_display, self->edgetopleft, px);
        XClearWindow(obt_display, self->edgetopleft);
        XSetWindowBackground(obt_display, self->edgetopright, px);
        XClearWindow(obt_display, self->edgetopright);
        XSetWindowBackground(obt_display, self->ce_tl_t, px);
        XClearWindow(obt_display, self->ce_tl_t);
        XSetWindowBackground(obt_display, self->ce_tl_l, px);
        XClearWindow(obt_display, self->ce_tl_l);
        XSetWindowBackground(obt_display, self->ce_tr_t, px);
        XClearWindow(obt_display, self->ce_tr_t);
        XSetWindowBackground(obt_display, self->ce_tr_r, px);
        XClearWindow(obt_display, self->ce_tr_r);
    }

    if (self->decorations & OB_FRAME_DECOR_TITLEBAR) {
        RrAppearance *t, *l, *m, *n, *i, *d, *s, *c, *clear;
        if (self->focused) {
            t = ob_rr_theme->a_focused_title;
            l = ob_rr_theme->a_focused_label;
            m = (!(self->decorations & OB_FRAME_DECOR_MAXIMIZE) ?
                 ob_rr_theme->btn_max->a_focused_disabled :
                 (self->client->max_vert || self->client->max_horz ?
                  (self->max_press ?
                   ob_rr_theme->btn_max->a_focused_pressed_toggled :
                   (self->max_hover ?
                    ob_rr_theme->btn_max->a_focused_hover_toggled :
                    ob_rr_theme->btn_max->a_focused_unpressed_toggled)) :
                  (self->max_press ?
                   ob_rr_theme->btn_max->a_focused_pressed :
                   (self->max_hover ?
                    ob_rr_theme->btn_max->a_focused_hover :
                    ob_rr_theme->btn_max->a_focused_unpressed))));
            n = ob_rr_theme->a_icon;
            i = (!(self->decorations & OB_FRAME_DECOR_ICONIFY) ?
                 ob_rr_theme->btn_iconify->a_focused_disabled :
                 (self->iconify_press ?
                  ob_rr_theme->btn_iconify->a_focused_pressed :
                  (self->iconify_hover ?
                   ob_rr_theme->btn_iconify->a_focused_hover :
                   ob_rr_theme->btn_iconify->a_focused_unpressed)));
            d = (!(self->decorations & OB_FRAME_DECOR_ALLDESKTOPS) ?
                 ob_rr_theme->btn_desk->a_focused_disabled :
                 (self->client->desktop == DESKTOP_ALL ?
                  (self->desk_press ?
                   ob_rr_theme->btn_desk->a_focused_pressed_toggled :
                   (self->desk_hover ?
                    ob_rr_theme->btn_desk->a_focused_hover_toggled :
                    ob_rr_theme->btn_desk->a_focused_unpressed_toggled)) :
                  (self->desk_press ?
                   ob_rr_theme->btn_desk->a_focused_pressed :
                   (self->desk_hover ?
                    ob_rr_theme->btn_desk->a_focused_hover :
                    ob_rr_theme->btn_desk->a_focused_unpressed))));
            s = (!(self->decorations & OB_FRAME_DECOR_SHADE) ?
                 ob_rr_theme->btn_shade->a_focused_disabled :
                 (self->client->shaded ?
                  (self->shade_press ?
                   ob_rr_theme->btn_shade->a_focused_pressed_toggled :
                   (self->shade_hover ?
                    ob_rr_theme->btn_shade->a_focused_hover_toggled :
                    ob_rr_theme->btn_shade->a_focused_unpressed_toggled)) :
                  (self->shade_press ?
                   ob_rr_theme->btn_shade->a_focused_pressed :
                   (self->shade_hover ?
                    ob_rr_theme->btn_shade->a_focused_hover :
                    ob_rr_theme->btn_shade->a_focused_unpressed))));
            c = (!(self->decorations & OB_FRAME_DECOR_CLOSE) ?
                 ob_rr_theme->btn_close->a_focused_disabled :
                 (self->close_press ?
                  ob_rr_theme->btn_close->a_focused_pressed :
                  (self->close_hover ?
                   ob_rr_theme->btn_close->a_focused_hover :
                   ob_rr_theme->btn_close->a_focused_unpressed)));
        } else {
            t = ob_rr_theme->a_unfocused_title;
            l = ob_rr_theme->a_unfocused_label;
            m = (!(self->decorations & OB_FRAME_DECOR_MAXIMIZE) ?
                 ob_rr_theme->btn_max->a_unfocused_disabled :
                 (self->client->max_vert || self->client->max_horz ?
                  (self->max_press ?
                   ob_rr_theme->btn_max->a_unfocused_pressed_toggled :
                   (self->max_hover ?
                    ob_rr_theme->btn_max->a_unfocused_hover_toggled :
                    ob_rr_theme->btn_max->a_unfocused_unpressed_toggled)) :
                  (self->max_press ?
                   ob_rr_theme->btn_max->a_unfocused_pressed :
                   (self->max_hover ?
                    ob_rr_theme->btn_max->a_unfocused_hover :
                    ob_rr_theme->btn_max->a_unfocused_unpressed))));
            n = ob_rr_theme->a_icon;
            i = (!(self->decorations & OB_FRAME_DECOR_ICONIFY) ?
                 ob_rr_theme->btn_iconify->a_unfocused_disabled :
                 (self->iconify_press ?
                  ob_rr_theme->btn_iconify->a_unfocused_pressed :
                  (self->iconify_hover ?
                   ob_rr_theme->btn_iconify->a_unfocused_hover :
                   ob_rr_theme->btn_iconify->a_unfocused_unpressed)));
            d = (!(self->decorations & OB_FRAME_DECOR_ALLDESKTOPS) ?
                 ob_rr_theme->btn_desk->a_unfocused_disabled :
                 (self->client->desktop == DESKTOP_ALL ?
                  (self->desk_press ?
                   ob_rr_theme->btn_desk->a_unfocused_pressed_toggled :
                   (self->desk_hover ?
                    ob_rr_theme->btn_desk->a_unfocused_hover_toggled :
                    ob_rr_theme->btn_desk->a_unfocused_unpressed_toggled)) :
                  (self->desk_press ?
                   ob_rr_theme->btn_desk->a_unfocused_pressed :
                   (self->desk_hover ?
                    ob_rr_theme->btn_desk->a_unfocused_hover :
                    ob_rr_theme->btn_desk->a_unfocused_unpressed))));
            s = (!(self->decorations & OB_FRAME_DECOR_SHADE) ?
                 ob_rr_theme->btn_shade->a_unfocused_disabled :
                 (self->client->shaded ?
                  (self->shade_press ?
                   ob_rr_theme->btn_shade->a_unfocused_pressed_toggled :
                   (self->shade_hover ?
                    ob_rr_theme->btn_shade->a_unfocused_hover_toggled :
                    ob_rr_theme->btn_shade->a_unfocused_unpressed_toggled)) :
                  (self->shade_press ?
                   ob_rr_theme->btn_shade->a_unfocused_pressed :
                   (self->shade_hover ?
                    ob_rr_theme->btn_shade->a_unfocused_hover :
                    ob_rr_theme->btn_shade->a_unfocused_unpressed))));
            c = (!(self->decorations & OB_FRAME_DECOR_CLOSE) ?
                 ob_rr_theme->btn_close->a_unfocused_disabled :
                 (self->close_press ?
                  ob_rr_theme->btn_close->a_unfocused_pressed :
                  (self->close_hover ?
                   ob_rr_theme->btn_close->a_unfocused_hover :
                   ob_rr_theme->btn_close->a_unfocused_unpressed)));
        }
        clear = ob_rr_theme->a_clear;

        /* mirrors the vertical-centering math in frame.c's layout_title();
           these values must stay in sync with the actual XMoveWindow
           calls there, since this block only feeds parent-relative
           (RR_SURFACE_PARENTREL) appearances the coordinates they need
           to sample the correct region of the parent's background --
           it doesn't move any real window itself. A mismatch here
           doesn't crash anything, but produces exactly the kind of
           misplaced/blank rectangle artifact seen with parent-relative
           (transparent-background) button themes: the real button
           window sits at the new centered position, while the parent-
           relative fill is sampled from the old, stale offset. */
        {
        const gint label_y =
            (ob_rr_theme->title_height - ob_rr_theme->label_height) / 2;
        const gint button_y =
            (ob_rr_theme->title_height - ob_rr_theme->button_height) / 2;
        const gint icon_y = button_y; /* icon is sized to button_height */

        RrPaint(t, self->title, self->width, ob_rr_theme->title_height);

        clear->surface.parent = t;
        clear->surface.parenty = 0;

        clear->surface.parentx = ob_rr_theme->grip_width;

        /* topresize's height is kgrip now (see frame.c's
           frame_adjust_area), not paddingy + 1 -- keep this RrPaint
           size argument in sync with that window's real XResizeWindow
           size, or the clear/parent-relative fill will be built for
           the wrong dimensions */
        RrPaint(clear, self->topresize,
                self->width - ob_rr_theme->grip_width * 2,
                ob_rr_theme->kgrip);

        clear->surface.parentx = 0;

        if (ob_rr_theme->grip_width > 0)
            RrPaint(clear, self->tltresize,
                    ob_rr_theme->grip_width, ob_rr_theme->kgrip);
        if (ob_rr_theme->title_height > 0)
            RrPaint(clear, self->tllresize,
                    ob_rr_theme->paddingx + 1, ob_rr_theme->title_height);

        clear->surface.parentx = self->width - ob_rr_theme->grip_width;

        if (ob_rr_theme->grip_width > 0)
            RrPaint(clear, self->trtresize,
                    ob_rr_theme->grip_width, ob_rr_theme->kgrip);

        clear->surface.parentx = self->width - (ob_rr_theme->paddingx + 1);

        if (ob_rr_theme->title_height > 0)
            RrPaint(clear, self->trrresize,
                    ob_rr_theme->paddingx + 1, ob_rr_theme->title_height);

        /* set parents for any parent relative guys */
        l->surface.parent = t;
        l->surface.parentx = self->label_x;
        l->surface.parenty = label_y;

        m->surface.parent = t;
        m->surface.parentx = self->max_x;
        m->surface.parenty = button_y;

        n->surface.parent = t;
        n->surface.parentx = self->icon_x;
        n->surface.parenty = icon_y;

        i->surface.parent = t;
        i->surface.parentx = self->iconify_x;
        i->surface.parenty = button_y;

        d->surface.parent = t;
        d->surface.parentx = self->desk_x;
        d->surface.parenty = button_y;

        s->surface.parent = t;
        s->surface.parentx = self->shade_x;
        s->surface.parenty = button_y;

        c->surface.parent = t;
        c->surface.parentx = self->close_x;
        c->surface.parenty = button_y;
        }

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
             ob_rr_theme->a_focused_handle : ob_rr_theme->a_unfocused_handle);

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

    XFlush(obt_display);
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
    RrImage *icon;

    if (!self->icon_on) return;

    icon = client_icon(self->client);

    if (icon) {
        RrAppearanceClearTextures(a);
        a->texture[0].type = RR_TEXTURE_IMAGE;
        a->texture[0].data.image.alpha = 0xff;
        a->texture[0].data.image.image = icon;
    } else {
        RrAppearanceClearTextures(a);
        a->texture[0].type = RR_TEXTURE_NONE;
    }

    /* the icon is square, sized to match the button height */
    RrPaint(a, self->icon,
            ob_rr_theme->button_height, ob_rr_theme->button_height);
}

static void framerender_max(ObFrame *self, RrAppearance *a)
{
    if (!self->max_on) return;
    RrPaint(a, self->max,
            ob_rr_theme->button_width, ob_rr_theme->button_height);
}

static void framerender_iconify(ObFrame *self, RrAppearance *a)
{
    if (!self->iconify_on) return;
    RrPaint(a, self->iconify,
            ob_rr_theme->button_width, ob_rr_theme->button_height);
}

static void framerender_desk(ObFrame *self, RrAppearance *a)
{
    if (!self->desk_on) return;
    RrPaint(a, self->desk,
            ob_rr_theme->button_width, ob_rr_theme->button_height);
}

static void framerender_shade(ObFrame *self, RrAppearance *a)
{
    if (!self->shade_on) return;
    RrPaint(a, self->shade,
            ob_rr_theme->button_width, ob_rr_theme->button_height);
}

static void framerender_close(ObFrame *self, RrAppearance *a)
{
    if (!self->close_on) return;
    RrPaint(a, self->close,
            ob_rr_theme->button_width, ob_rr_theme->button_height);
}
