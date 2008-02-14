/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

 plugin.h for the Openbox window manager
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
#ifndef ENGINE_INTERFACE_H_
#define ENGINE_INTERFACE_H_
/* Thanks to BMP, XMMS, Audacious and glib project to enable plugin */
#include "geom.h"
#include "render/render.h"
#include "render/theme.h"
#include "obt/parse.h"
#include "client.h"

#include "obt/mainloop.h"

#include <X11/Xresource.h>
#include <gmodule.h>

struct _RrTheme;
struct _ObClient;

typedef enum
{
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
typedef enum
{
    OB_FRAME_DECOR_TITLEBAR = 1 << 0, /*!< Display a titlebar */
    OB_FRAME_DECOR_HANDLE = 1 << 1, /*!< Display a handle (bottom) */
    OB_FRAME_DECOR_GRIPS = 1 << 2, /*!< Display grips in the handle */
    OB_FRAME_DECOR_BORDER = 1 << 3, /*!< Display a border */
    OB_FRAME_DECOR_ICON = 1 << 4, /*!< Display the window's icon */
    OB_FRAME_DECOR_ICONIFY = 1 << 5, /*!< Display an iconify button */
    OB_FRAME_DECOR_MAXIMIZE = 1 << 6, /*!< Display a maximize button */
    /*! Display a button to toggle the window's placement on
     all desktops */
    OB_FRAME_DECOR_ALLDESKTOPS = 1 << 7,
    OB_FRAME_DECOR_SHADE = 1 << 8, /*!< Displays a shade button */
    OB_FRAME_DECOR_CLOSE = 1 << 9 /*!< Display a close button */
} ObFrameDecorations;

typedef enum
{
    OB_BUTTON_NONE = 0,
    OB_BUTTON_MAX = 1,
    OB_BUTTON_CLOSE = 2,
    OB_BUTTON_DESK = 3,
    OB_BUTTON_SHADE = 4,
    OB_BUTTON_ICONIFY = 5
} ObFrameButton;

/* The plugin must implement standars triggers and is free to use other
 * It's mimic signal (USR1 and USR2) */
typedef enum
{
    OB_TRIGGER_NORMAL,
    OB_TRIGGER_INCONIFIED,
    OB_TRIGGER_SHADED,
    OB_TRIGGER_MAX,
    OB_TRIGGER_MAX_VERT,
    OB_TRIGGER_MAX_HORZ,
    OB_TRIGGER_NO_BORDER,
    OB_TRIGGER_PLUGIN1,
    OB_TRIGGER_PLUGIN2,
    OB_TRIGGER_PLUGIN3,
    OB_TRIGGER_PLUGIN4, /* ... */
} ObFrameTrigger;

struct _ObFramePlugin
{
    gpointer handler; // Currently not used.

    gchar * filename;
    gchar * name;

    /* Function to init module */
    gint (*init)(Display * display, gint screen);
    /* Not curently used */
    gint (*release)(void);

    /* create a new frame, return the ID of frame */
    gpointer (*frame_new)(struct _ObClient *c);
    /* Free the frame */
    void (*frame_free)(gpointer self);

    void (*frame_show)(gpointer self);
    gint (*frame_hide)(gpointer self);

    void (*frame_adjust_theme)(gpointer self);
    void (*frame_adjust_shape)(gpointer self);

    /* Grab or Ungrab event */
    void (*frame_grab)(gpointer self, GHashTable *);
    void (*frame_ungrab)(gpointer self, GHashTable *);

    /* Provide the context of the mouse */
    ObFrameContext (*frame_context)(gpointer self, Window win, gint x, gint y);

    void (*frame_set_is_visible)(gpointer, gboolean);
    void (*frame_set_is_focus)(gpointer, gboolean);
    void (*frame_set_is_max_vert)(gpointer, gboolean);
    void (*frame_set_is_max_horz)(gpointer, gboolean);
    void (*frame_set_is_shaded)(gpointer, gboolean);

    void (*frame_flash_start)(gpointer self);
    void (*frame_flash_stop)(gpointer self);
    void (*frame_begin_iconify_animation)(gpointer self, gboolean iconifying);
    void (*frame_end_iconify_animation)(gpointer self);
    gboolean (*frame_iconify_animating)(gpointer p);

    /* Set the layout wanted by client */
    void (*frame_set_decorations)(gpointer, ObFrameDecorations);

    /* get the current window area */
    Rect (*frame_get_window_area)(gpointer);
    /* set the requested client area */
    void (*frame_set_client_area)(gpointer, Rect);
    /* Update size, move/resize windows */
    void (*frame_update_layout)(gpointer self, gboolean is_resize,
            gboolean is_fake);
    /* Update skin, color/texture windows */ 
    void (*frame_update_skin)(gpointer);

    void (*frame_set_hover_flag)(gpointer, ObFrameButton);
    void (*frame_set_press_flag)(gpointer, ObFrameButton);

    Window (*frame_get_window)(gpointer);

    Strut (*frame_get_size)(gpointer);
    gint (*frame_get_decorations)(gpointer);

    gboolean (*frame_is_visible)(gpointer);
    gboolean (*frame_is_max_horz)(gpointer);
    gboolean (*frame_is_max_vert)(gpointer);
    
    /* This must implement triggers (currently not used) */
    void (*frame_trigger)(gpointer, ObFrameTrigger);

    gint (*load_theme_config)(const RrInstance *inst, const gchar *name,
            const gchar * path, XrmDatabase db, RrFont *active_window_font,
            RrFont *inactive_window_font, RrFont *menu_title_font,
            RrFont *menu_item_font, RrFont *osd_font);

    /* Filled by openbox-core */
    Display * ob_display;
    gint ob_screen;
    RrInstance *ob_rr_inst;
    // Not more needed
    //struct _RrTheme * ob_rr_theme;
    gboolean config_theme_keepborder;
    struct _ObClient *focus_cycle_target;
    gchar *config_title_layout;
    gboolean moveresize_in_progress;
    struct _ObtMainLoop *ob_main_loop;
};
/* Define how to draw the current windows */
enum _ObStyle
{
    OBSTYLE_DECOR,
    OBSTYLE_NODECOR,
    OBSTYLE_SHADE,
    OBSTYLE_ICON,
    OBSTYLE_MAXVERT,
    OBSTYLE_MAXHORIZ,
    OBSTYLE_MAX,
};

typedef enum _ObStyle ObStyle;
typedef struct _ObFramePlugin ObFramePlugin;
typedef ObFramePlugin * (*ObFramePluginFunc)(void);

#define OBFRAME(x) (ObFrame *) (x);

/* initialize theme plugin, it read themerc and load
 * the plugin needed */
ObFramePlugin * init_frame_plugin(const gchar *name, gboolean allow_fallback,
        RrFont *active_window_font, RrFont *inactive_window_font,
        RrFont *menu_title_font, RrFont *menu_item_font, RrFont *osd_font);

/* Update plugin data */
void update_frame_plugin(ObFramePlugin *);

/* Load modules specified in filename */
ObFramePlugin * load_frame_plugin(const gchar * filename);

/* Give context from string, it's used to parse config file */
ObFrameContext frame_context_from_string(const gchar *name);
ObFrameContext plugin_frame_context(ObClient *, Window, gint, gint);

void frame_client_gravity(ObClient * self, gint *x, gint *y);
void frame_frame_gravity(ObClient * self, gint *x, gint *y);

void frame_rect_to_frame(ObClient * self, Rect *r);
void frame_rect_to_client(ObClient * self, Rect *r);

#endif /*FRAME_PLUGIN_H_*/
