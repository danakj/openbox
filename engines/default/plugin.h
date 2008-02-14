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
#ifndef FRAME_DEFAULT_PLUGIN_H_
#define FRAME_DEFAULT_PLUGIN_H_

#include "config.h"
#include "render/render.h"
#include "openbox/engine_interface.h"

ObFrameThemeConfig theme_config;

struct _ObDefaultFrame
{
    /* PUBLIC : */

    /* PRIVATE: */
    /* You are free to add what you want here */

    Window window;

    gboolean visible;

    gboolean max_horz; /* when maxed some decorations are hidden */
    gboolean max_vert; /* when maxed some decorations are hidden */

    struct _ObClient *client;
    guint decorations;

    Strut size;
    Rect area;
    Rect client_area;

    gint bwidth;

    ObFrameButton hover_flag;
    ObFrameButton press_flag;

    gint iconify_animation_going;
    ObStyle style;

    guint functions;

    Window title;
    Window label;
    Window max;
    Window close;
    Window desk;
    Window shade;
    Window icon;
    Window iconify;
    Window handle;
    Window lgrip;
    Window rgrip;

    /* These are borders of the frame and its elements */
    Window titleleft;
    Window titletop;
    Window titletopleft;
    Window titletopright;
    Window titleright;
    Window titlebottom;
    Window left;
    Window right;
    Window handleleft;
    Window handletop;
    Window handleright;
    Window handlebottom;
    Window lgriptop;
    Window lgripleft;
    Window lgripbottom;
    Window rgriptop;
    Window rgripright;
    Window rgripbottom;
    Window innerleft; /*!< For drawing the inner client border */
    Window innertop; /*!< For drawing the inner client border */
    Window innerright; /*!< For drawing the inner client border */
    Window innerbottom; /*!< For drawing the inner client border */
    Window innerblb;
    Window innerbll;
    Window innerbrb;
    Window innerbrr;
    Window backback; /*!< A colored window shown while resizing */
    Window backfront; /*!< An undrawn-in window, to prevent flashing on unmap */

    /* These are resize handles inside the titlebar */
    Window topresize;
    Window tltresize;
    Window tllresize;
    Window trtresize;
    Window trrresize;

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

typedef struct _ObDefaultFrame ObDefaultFrame;

/* Function use for interface */
gint init(Display *, gint);
gpointer frame_new(struct _ObClient *c);
void frame_free(gpointer self);
void frame_show(gpointer self);
gint frame_hide(gpointer self);
void frame_adjust_theme(gpointer self);
void frame_adjust_shape(gpointer self);

void frame_grab(gpointer self, GHashTable *);
void frame_ungrab(gpointer self, GHashTable *);

ObFrameContext frame_context(gpointer, Window, gint, gint);

void frame_set_is_visible(gpointer, gboolean);
void frame_set_is_focus(gpointer, gboolean);
void frame_set_is_max_vert(gpointer, gboolean);
void frame_set_is_max_horz(gpointer, gboolean);
void frame_set_is_shaded(gpointer, gboolean);

void frame_update_layout(gpointer, gboolean, gboolean);
void frame_adjust_client_area(gpointer self);
void frame_adjust_state(gpointer self);
void frame_adjust_focus(gpointer self, gboolean hilite);
void frame_adjust_title(gpointer self);
void frame_adjust_icon(gpointer self);

static gulong frame_animate_iconify_time_left(gpointer _self,
        const GTimeVal *now);

ObFrameContext frame_context(gpointer, Window win, gint x, gint y);
//void frame_client_gravity(gpointer self, gint *x, gint *y);
//void frame_frame_gravity(gpointer self, gint *x, gint *y);
//void frame_rect_to_frame(gpointer self, Rect *r);
//void frame_rect_to_client(gpointer self, Rect *r);
void frame_flash_start(gpointer self);
void frame_flash_stop(gpointer self);
void frame_begin_iconify_animation(gpointer self, gboolean iconifying);
void frame_end_iconify_animation(gpointer self);
gboolean frame_iconify_animating(gpointer _self);

void frame_set_hover_flag(gpointer, ObFrameButton);
void frame_set_press_flag(gpointer, ObFrameButton);

Window frame_get_window(gpointer);

Strut frame_get_size(gpointer self);
Rect frame_get_area(gpointer self);
gint frame_get_decorations(gpointer self);

gboolean frame_is_visible(gpointer self);
gboolean frame_is_max_horz(gpointer self);
gboolean frame_is_max_vert(gpointer self);

void flash_done(gpointer data);
gboolean flash_timeout(gpointer data);

void layout_title(ObDefaultFrame *);
void set_theme_statics(gpointer self);
void free_theme_statics(gpointer self);
gboolean frame_animate_iconify(gpointer self);
void frame_adjust_cursors(gpointer self);

/* Global for renderframe.c only */
extern ObFramePlugin plugin;
#define OBDEFAULTFRAME(x) ((ObDefaultFrame *)(x))

#endif /*FRAME_DEFAULT_PLUGIN_H_*/
