#ifndef __engine_openbox_h
#define __engine_openbox_h

#include "../../kernel/frame.h"
#include "../../render/render.h"
#include "../../render/color.h"
#include "../../render/font.h"
#include "../../render/mask.h"

#define LABEL_HEIGHT    (ob_s_winfont_height + 2)
#define TITLE_HEIGHT    (LABEL_HEIGHT + ob_s_bevel * 2)
#define HANDLE_Y(f)     (f->innersize.top + f->frame.client->area.height + \
		         f->cbwidth)
#define BUTTON_SIZE     (LABEL_HEIGHT - 2)
#define GRIP_WIDTH      (BUTTON_SIZE * 2)

extern int ob_s_bevel;
extern int ob_s_handle_height;
extern int ob_s_bwidth;
extern int ob_s_cbwidth;

extern color_rgb *ob_s_b_color;
extern color_rgb *ob_s_cb_focused_color;
extern color_rgb *ob_s_cb_unfocused_color;
extern color_rgb *ob_s_title_focused_color;
extern color_rgb *ob_s_title_unfocused_color;
extern color_rgb *ob_s_titlebut_focused_color;
extern color_rgb *ob_s_titlebut_unfocused_color;

extern int ob_s_winfont_height;
extern int ob_s_winfont_shadow;
extern int ob_s_winfont_shadow_offset;
extern ObFont *ob_s_winfont;

extern pixmap_mask *ob_s_max_pressed_mask;
extern pixmap_mask *ob_s_max_unpressed_mask;
extern pixmap_mask *ob_s_iconify_pressed_mask;
extern pixmap_mask *ob_s_iconify_unpressed_mask;
extern pixmap_mask *ob_s_desk_pressed_mask;
extern pixmap_mask *ob_s_desk_unpressed_mask;
extern pixmap_mask *ob_s_close_pressed_mask;
extern pixmap_mask *ob_s_close_unpressed_mask;

extern Appearance *ob_a_focused_unpressed_max;
extern Appearance *ob_a_focused_pressed_max;
extern Appearance *ob_a_unfocused_unpressed_max;
extern Appearance *ob_a_unfocused_pressed_max;
extern Appearance *ob_a_focused_unpressed_close;
extern Appearance *ob_a_focused_pressed_close;
extern Appearance *ob_a_unfocused_unpressed_close;
extern Appearance *ob_a_unfocused_pressed_close;
extern Appearance *ob_a_focused_unpressed_desk;
extern Appearance *ob_a_focused_pressed_desk;
extern Appearance *ob_a_unfocused_unpressed_desk;
extern Appearance *ob_a_unfocused_pressed_desk;
extern Appearance *ob_a_focused_unpressed_iconify;
extern Appearance *ob_a_focused_pressed_iconify;
extern Appearance *ob_a_unfocused_unpressed_iconify;
extern Appearance *ob_a_unfocused_pressed_iconify;
extern Appearance *ob_a_focused_grip;
extern Appearance *ob_a_unfocused_grip;
extern Appearance *ob_a_focused_title;
extern Appearance *ob_a_unfocused_title;
extern Appearance *ob_a_focused_label;
extern Appearance *ob_a_unfocused_label;
extern Appearance *ob_a_icon;
extern Appearance *ob_a_focused_handle;
extern Appearance *ob_a_unfocused_handle;

typedef struct ObFrame {
    Frame frame;

    Window title;
    Window label;
    Window max;
    Window close;
    Window desk;
    Window icon;
    Window iconify;
    Window handle;
    Window lgrip;
    Window rgrip;

    Appearance *a_unfocused_title;
    Appearance *a_focused_title;
    Appearance *a_unfocused_label;
    Appearance *a_focused_label;
    Appearance *a_icon;
    Appearance *a_unfocused_handle;
    Appearance *a_focused_handle;

    Strut  innersize;

    GSList *clients;

    int width; /* title and handle */
    int label_width;
    int icon_x;        /* x-position of the window icon button */
    int label_x;       /* x-position of the window title */
    int iconify_x;     /* x-position of the window iconify button */
    int desk_x;         /* x-position of the window all-desktops button */
    int max_x;         /* x-position of the window maximize button */
    int close_x;       /* x-position of the window close button */
    int bwidth;        /* border width */
    int cbwidth;       /* client border width */

    gboolean max_press;
    gboolean close_press;
    gboolean desk_press;
    gboolean iconify_press;
} ObFrame;

#endif
