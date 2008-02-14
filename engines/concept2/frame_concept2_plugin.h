/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

 frame_default_plugin.h for the Openbox window manager
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
#ifndef FRAME_CONCEPT2_PLUGIN_H_
#define FRAME_CONCEPT2_PLUGIN_H_

#include "frame_concept2_config.h"

#include "render/render.h"
#include "openbox/engine_interface.h"

ObFrameThemeConfig theme_config;

struct _ObConceptFrame
{
    // PUBLIC :
    struct _ObClient *client;

    Window window;

    Strut size;
    Rect area;
    gint bwidth; /* border width */
    guint decorations;

    gboolean visible;

    gboolean max_horz; /* when maxed some decorations are hidden */
    gboolean max_vert; /* when maxed some decorations are hidden */

    gboolean max_press;
    gboolean close_press;
    gboolean desk_press;
    gboolean shade_press;
    gboolean iconify_press;

    gboolean max_hover;
    gboolean close_hover;
    gboolean desk_hover;
    gboolean shade_hover;
    gboolean iconify_hover;

    gint iconify_animation_going;

    /* PRIVATE: */
    /* You are free to add what you want here */

    ObStyle style;

    guint functions;

    /* These are borders of the frame and its elements */

    Window top;
    Window bottom;
    Window left;
    Window right;

    Window top_left;
    Window top_right;

    Window bottom_left;
    Window bottom_right;

    Window background;

    Window left_shade;
    Window left_close;
    Window left_iconify;
    Window left_maximize;

    Window handle;

    Colormap colormap;

    RrAppearance *a_unfocused_title;
    RrAppearance *a_focused_title;
    RrAppearance *a_unfocused_label;
    RrAppearance *a_focused_label;
    RrAppearance *a_icon;
    RrAppearance *a_unfocused_handle;
    RrAppearance *a_focused_handle;

    gint icon_on; /* if the window icon button is on */
    gint label_on; /* if the window title is on */
    gint iconify_on; /* if the window iconify button is on */
    gint desk_on; /* if the window all-desktops button is on */
    gint shade_on; /* if the window shade button is on */
    gint max_on; /* if the window maximize button is on */
    gint close_on; /* if the window close button is on */

    gint width; /* width of the titlebar and handle */

    gint label_width; /* width of the label in the titlebar */
    gint icon_x; /* x-position of the window icon button */
    gint label_x; /* x-position of the window title */
    gint iconify_x; /* x-position of the window iconify button */
    gint desk_x; /* x-position of the window all-desktops button */
    gint shade_x; /* x-position of the window shade button */
    gint max_x; /* x-position of the window maximize button */
    gint close_x; /* x-position of the window close button */

    gint cbwidth_l; /* client border width */
    gint cbwidth_t; /* client border width */
    gint cbwidth_r; /* client border width */
    gint cbwidth_b; /* client border width */
    gboolean shaded; /* decorations adjust when shaded */

    /* the leftmost and rightmost elements in the titlebar */
    ObFrameContext leftmost;
    ObFrameContext rightmost;

    gboolean focused;
    gboolean need_render;

    gboolean flashing;
    gboolean flash_on;
    GTimeVal flash_end;

    GTimeVal iconify_animation_end;

};

typedef struct _ObConceptFrame ObConceptFrame;

/* Function use for interface */
gint init(Display *, gint);
gpointer frame_new(struct _ObClient *c);
void frame_free(gpointer self);
void frame_show(gpointer self);
void frame_hide(gpointer self);
void frame_adjust_theme(gpointer self);
void frame_adjust_shape(gpointer self);
void frame_adjust_area(gpointer self, gboolean moved, gboolean resized,
        gboolean fake);
void frame_adjust_client_area(gpointer self);
void frame_adjust_state(gpointer self);
void frame_adjust_focus(gpointer self, gboolean hilite);
void frame_adjust_title(gpointer self);
void frame_adjust_icon(gpointer self);
void frame_grab_client(gpointer self, GHashTable *);
void frame_release_client(gpointer self, GHashTable *);

ObFrameContext frame_context(gpointer, Window, gint, gint);
void frame_client_gravity(gpointer self, gint *x, gint *y);
void frame_frame_gravity(gpointer self, gint *x, gint *y);
void frame_rect_to_frame(gpointer self, Rect *r);
void frame_rect_to_client(gpointer self, Rect *r);
void frame_flash_start(gpointer self);
void frame_flash_stop(gpointer self);
void frame_begin_iconify_animation(gpointer self, gboolean iconifying);
void frame_end_iconify_animation(gpointer self);
gboolean frame_iconify_animating(gpointer _self);

void flash_done(gpointer data);
gboolean flash_timeout(gpointer data);

void set_theme_statics(gpointer self);
void free_theme_statics(gpointer self);
gboolean frame_animate_iconify(gpointer self);
void frame_adjust_cursors(gpointer self);

/* Global for frame_concept_render.c only */
extern ObFramePlugin plugin;
#define OBCONCEPTFRAME(x) ((ObConceptFrame *)(x))

#endif /*FRAME_CONCEPT2_PLUGIN_H_*/
