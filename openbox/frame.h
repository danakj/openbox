#ifndef __frame_h
#define __frame_h

#include "geom.h"
#include "render/render.h"

typedef struct _ObFrame ObFrame;

struct _ObClient;

typedef enum {
    OB_FRAME_CONTEXT_NONE,
    OB_FRAME_CONTEXT_ROOT,
    OB_FRAME_CONTEXT_CLIENT,
    OB_FRAME_CONTEXT_TITLEBAR,
    OB_FRAME_CONTEXT_HANDLE,
    OB_FRAME_CONTEXT_FRAME,
    OB_FRAME_CONTEXT_BLCORNER,
    OB_FRAME_CONTEXT_BRCORNER,
    OB_FRAME_CONTEXT_TLCORNER,
    OB_FRAME_CONTEXT_TRCORNER,
    OB_FRAME_CONTEXT_MAXIMIZE,
    OB_FRAME_CONTEXT_ALLDESKTOPS,
    OB_FRAME_CONTEXT_SHADE,
    OB_FRAME_CONTEXT_ICONIFY,
    OB_FRAME_CONTEXT_ICON,
    OB_FRAME_CONTEXT_CLOSE,
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
    Window    plate;

    Strut     size;
    Rect      area;
    gboolean  visible;
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

    Window    tlresize;
    Window    trresize;

    RrAppearance *a_unfocused_title;
    RrAppearance *a_focused_title;
    RrAppearance *a_unfocused_label;
    RrAppearance *a_focused_label;
    RrAppearance *a_icon;
    RrAppearance *a_unfocused_handle;
    RrAppearance *a_focused_handle;

    Strut     innersize;

    GSList   *clients;

    gint      width;         /* title and handle */
    gint      label_width;
    gint      icon_x;        /* x-position of the window icon button */
    gint      label_x;       /* x-position of the window title */
    gint      iconify_x;     /* x-position of the window iconify button */
    gint      desk_x;        /* x-position of the window all-desktops button */
    gint      shade_x;       /* x-position of the window shade button */
    gint      max_x;         /* x-position of the window maximize button */
    gint      close_x;       /* x-position of the window close button */
    gint      bwidth;        /* border width */
    gint      cbwidth;       /* client border width */

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
};

ObFrame *frame_new();
void frame_show(ObFrame *self);
void frame_hide(ObFrame *self);
void frame_adjust_shape(ObFrame *self);
void frame_adjust_area(ObFrame *self, gboolean moved, gboolean resized);
void frame_adjust_state(ObFrame *self);
void frame_adjust_focus(ObFrame *self, gboolean hilite);
void frame_adjust_title(ObFrame *self);
void frame_adjust_icon(ObFrame *self);
void frame_grab_client(ObFrame *self, struct _ObClient *client);
void frame_release_client(ObFrame *self, struct _ObClient *client);

ObFrameContext frame_context_from_string(char *name);

ObFrameContext frame_context(struct _ObClient *self, Window win);

/*! Applies gravity to the client's position to find where the frame should
  be positioned.
  @return The proper coordinates for the frame, based on the client.
*/
void frame_client_gravity(ObFrame *self, int *x, int *y);

/*! Reversly applies gravity to the frame's position to find where the client
  should be positioned.
    @return The proper coordinates for the client, based on the frame.
*/
void frame_frame_gravity(ObFrame *self, int *x, int *y);


#endif
