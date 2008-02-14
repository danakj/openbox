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
#include "frame_concept2_render.h"
#include "frame_concept2_plugin.h"

#include "openbox/engine_interface.h"
#include "openbox/openbox.h"
#include "openbox/screen.h"
#include "openbox/client.h"

#include "render/theme.h"

static void framerender_label(ObConceptFrame *self, RrAppearance *a);
static void framerender_icon(ObConceptFrame *self, RrAppearance *a);
static void framerender_max(ObConceptFrame *self, RrAppearance *a);
static void framerender_iconify(ObConceptFrame *self, RrAppearance *a);
static void framerender_desk(ObConceptFrame *self, RrAppearance *a);
static void framerender_shade(ObConceptFrame *self, RrAppearance *a);
static void framerender_close(ObConceptFrame *self, RrAppearance *a);

void framerender_frame(gpointer _self)
{
    ObConceptFrame * self = (ObConceptFrame *) _self;
    if (plugin.frame_iconify_animating(self))
        return; /* delay redrawing until the animation is done */
    if (!self->need_render)
        return;
    if (!self->visible)
        return;
    self->need_render = FALSE;

    gulong border_px, corner_px;

    if (self->focused) {
        border_px = RrColorPixel(theme_config.focus_border_color);
        corner_px = RrColorPixel(theme_config.focus_corner_color);
    }
    else {
        border_px = RrColorPixel(theme_config.unfocus_border_color);
        corner_px = RrColorPixel(theme_config.unfocus_corner_color);
    }

    XSetWindowBackground(plugin.ob_display, self->left, border_px);
    XClearWindow(plugin.ob_display, self->left);
    XSetWindowBackground(plugin.ob_display, self->right, border_px);
    XClearWindow(plugin.ob_display, self->right);

    XSetWindowBackground(plugin.ob_display, self->top, border_px);
    XClearWindow(plugin.ob_display, self->top);
    XSetWindowBackground(plugin.ob_display, self->bottom, border_px);
    XClearWindow(plugin.ob_display, self->bottom);

    XSetWindowBackground(plugin.ob_display, self->top_left, corner_px);
    XClearWindow(plugin.ob_display, self->top_left);
    XSetWindowBackground(plugin.ob_display, self->top_right, corner_px);
    XClearWindow(plugin.ob_display, self->top_right);

    XSetWindowBackground(plugin.ob_display, self->bottom_left, corner_px);
    XClearWindow(plugin.ob_display, self->bottom_left);
    XSetWindowBackground(plugin.ob_display, self->bottom_right, corner_px);
    XClearWindow(plugin.ob_display, self->bottom_right);

    XSetWindowBackground(plugin.ob_display, self->background, 0);
    XClearWindow(plugin.ob_display, self->background);

    XSetWindowBackground(plugin.ob_display, self->left_close, 0xff0000);
    XClearWindow(plugin.ob_display, self->left_close);
    XSetWindowBackground(plugin.ob_display, self->left_iconify, 0x00ff00);
    XClearWindow(plugin.ob_display, self->left_iconify);
    XSetWindowBackground(plugin.ob_display, self->left_maximize, 0x0000ff);
    XClearWindow(plugin.ob_display, self->left_maximize);
    XSetWindowBackground(plugin.ob_display, self->left_shade, 0xffff00);
    XClearWindow(plugin.ob_display, self->left_shade);

    XSetWindowBackground(plugin.ob_display, self->handle, 0x00ffff);
    XClearWindow(plugin.ob_display, self->handle);

    XFlush(plugin.ob_display);
}
