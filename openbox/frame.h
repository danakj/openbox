/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   frame.h for the Openbox window manager
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

#ifndef __frame_h
#define __frame_h

#include "geom.h"
#include "render/render.h"

typedef struct _ObFrame ObFrame;

struct _ObClient;

typedef void (*ObFrameIconifyAnimateFunc)(gpointer data);

typedef enum {
    OB_FRAME_CONTEXT_NONE,
    OB_FRAME_CONTEXT_DESKTOP,
    OB_FRAME_CONTEXT_ROOT,
    OB_FRAME_CONTEXT_CLIENT,
    OB_FRAME_CONTEXT_TITLEBAR,
    OB_FRAME_CONTEXT_FRAME,
    OB_FRAME_CONTEXT_BLCORNER,
    OB_FRAME_CONTEXT_BRCORNER,
    OB_FRAME_CONTEXT_TLCORNER,
    OB_FRAME_CONTEXT_TRCORNER,
    OB_FRAME_CONTEXT_TOP,
    OB_FRAME_CONTEXT_BOTTOM,
    OB_FRAME_CONTEXT_LEFT,
    OB_FRAME_CONTEXT_RIGHT,
    OB_FRAME_CONTEXT_MAXIMIZE,
    OB_FRAME_CONTEXT_ALLDESKTOPS,
    OB_FRAME_CONTEXT_SHADE,
    OB_FRAME_CONTEXT_ICONIFY,
    OB_FRAME_CONTEXT_ICON,
    OB_FRAME_CONTEXT_CLOSE,
    /*! This is a special context, which occurs while dragging a window in
      a move/resize */
    OB_FRAME_CONTEXT_MOVE_RESIZE,
    OB_FRAME_NUM_CONTEXTS
} ObFrameContext;

/*! The decorations the client window wants to be displayed on it */
typedef enum {
    OB_FRAME_DECOR_TITLEBAR    = 1 << 0, /*!< Display a titlebar */
    OB_FRAME_DECOR_HANDLE      = 1 << 1, /*!< Display a handle (bottom) */
    OB_FRAME_DECOR_GRIPS       = 1 << 2, /*!< Display grips in the handle */
    OB_FRAME_DECOR_BORDER      = 1 << 3, /*!< Display a border */
    OB_FRAME_DECOR_ICON        = 1 << 4, /*!< Display the window's icon */
    OB_FRAME_DECOR_ICONIFY     = 1 << 5, /*!< Display an iconify button */
    OB_FRAME_DECOR_MAXIMIZE    = 1 << 6, /*!< Display a maximize button */
    /*! Display a button to toggle the window's placement on
      all desktops */
    OB_FRAME_DECOR_ALLDESKTOPS = 1 << 7,
    OB_FRAME_DECOR_SHADE       = 1 << 8, /*!< Displays a shade button */
    OB_FRAME_DECOR_CLOSE       = 1 << 9  /*!< Display a close button */
} ObFrameDecorations;

struct _ObFrame
{
    struct _ObClient *client;

    Window    window;

    Strut     size;
    Rect      area;
    gboolean  visible;

    guint     functions;
    guint     decorations;

    Window    title;
    Window    label;
    Window    max;
    Window    close;
    Window    desk;
    Window    shade;
    Window    icon;
    Window    iconify;
    Window    handle;
    Window    lgrip;
    Window    rgrip;

    /* These are borders of the frame and its elements */
    Window    titleleft;
    Window    titletop;
    Window    titletopleft;
    Window    titletopright;
    Window    titleright;
    Window    titlebottom;
    Window    left;
    Window    right;
    Window    handleleft;
    Window    handletop;
    Window    handleright;
    Window    handlebottom;
    Window    lgriptop;
    Window    lgripleft;
    Window    lgripbottom;
    Window    rgriptop;
    Window    rgripright;
    Window    rgripbottom;
    Window    innerleft;    /*!< For drawing the inner client border */
    Window    innertop;     /*!< For drawing the inner client border */
    Window    innerright;   /*!< For drawing the inner client border */
    Window    innerbottom;  /*!< For drawing the inner client border */
    Window    backback;     /*!< A colored window shown while resizing */
    Window    backfront;    /*!< An undrawn-in window, to prevent flashing on
                                 unmap */

    /* These are resize handles inside the titlebar */
    Window    topresize;
    Window    tltresize;
    Window    tllresize;
    Window    trtresize;
    Window    trrresize;

    Colormap  colormap;

    RrAppearance *a_unfocused_title;
    RrAppearance *a_focused_title;
    RrAppearance *a_unfocused_label;
    RrAppearance *a_focused_label;
    RrAppearance *a_icon;
    RrAppearance *a_unfocused_handle;
    RrAppearance *a_focused_handle;

    GSList   *clients;

    gint      icon_on;    /* if the window icon button is on */
    gint      label_on;   /* if the window title is on */
    gint      iconify_on; /* if the window iconify button is on */
    gint      desk_on;    /* if the window all-desktops button is on */
    gint      shade_on;   /* if the window shade button is on */
    gint      max_on;     /* if the window maximize button is on */
    gint      close_on;   /* if the window close button is on */

    gint      width;         /* width of the titlebar and handle */
    gint      label_width;   /* width of the label in the titlebar */
    gint      icon_x;        /* x-position of the window icon button */
    gint      label_x;       /* x-position of the window title */
    gint      iconify_x;     /* x-position of the window iconify button */
    gint      desk_x;        /* x-position of the window all-desktops button */
    gint      shade_x;       /* x-position of the window shade button */
    gint      max_x;         /* x-position of the window maximize button */
    gint      close_x;       /* x-position of the window close button */
    gint      bwidth;        /* border width */
    gint      rbwidth;       /* border width between the title and client */
    gint      cbwidth_x;     /* client border width */
    gint      cbwidth_y;     /* client border width */
    gboolean  max_horz;      /* when maxed some decorations are hidden */
    gboolean  max_vert;      /* when maxed some decorations are hidden */

    /* the leftmost and rightmost elements in the titlebar */
    ObFrameContext leftmost;
    ObFrameContext rightmost;

    gboolean  max_press;
    gboolean  close_press;
    gboolean  desk_press;
    gboolean  shade_press;
    gboolean  iconify_press;
    gboolean  max_hover;
    gboolean  close_hover;
    gboolean  desk_hover;
    gboolean  shade_hover;
    gboolean  iconify_hover;

    gboolean  focused;

    gboolean  flashing;
    gboolean  flash_on;
    GTimeVal  flash_end;

    /*! Is the frame currently in an animation for iconify or restore.
      0 means that it is not animating. > 0 means it is animating an iconify.
      < 0 means it is animating a restore.
    */
    gint iconify_animation_going;
    GTimeVal  iconify_animation_end;
};

ObFrame *frame_new(struct _ObClient *c);
void frame_free(ObFrame *self);

void frame_show(ObFrame *self);
void frame_hide(ObFrame *self);
void frame_adjust_theme(ObFrame *self);
void frame_adjust_shape(ObFrame *self);
void frame_adjust_area(ObFrame *self, gboolean moved,
                       gboolean resized, gboolean fake);
void frame_adjust_client_area(ObFrame *self);
void frame_adjust_state(ObFrame *self);
void frame_adjust_focus(ObFrame *self, gboolean hilite);
void frame_adjust_title(ObFrame *self);
void frame_adjust_icon(ObFrame *self);
void frame_grab_client(ObFrame *self);
void frame_release_client(ObFrame *self);

ObFrameContext frame_context_from_string(const gchar *name);

ObFrameContext frame_context(struct _ObClient *self, Window win,
                             gint x, gint y);

/*! Applies gravity to the client's position to find where the frame should
  be positioned.
  @return The proper coordinates for the frame, based on the client.
*/
void frame_client_gravity(ObFrame *self, gint *x, gint *y, gint w, gint h);

/*! Reversly applies gravity to the frame's position to find where the client
  should be positioned.
    @return The proper coordinates for the client, based on the frame.
*/
void frame_frame_gravity(ObFrame *self, gint *x, gint *y, gint w, gint h);

void frame_flash_start(ObFrame *self);
void frame_flash_stop(ObFrame *self);

/*! Start an animation for iconifying or restoring a frame. The callback
  will be called when the animation finishes. But if another animation is
  started in the meantime, the callback will never get called. */
void frame_begin_iconify_animation(ObFrame *self, gboolean iconifying);
void frame_end_iconify_animation(ObFrame *self);

#define frame_iconify_animating(f) (f->iconify_animation_going != 0)

#endif
