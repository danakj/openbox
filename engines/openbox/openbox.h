#ifndef __engine_openbox_h
#define __engine_openbox_h

#include "../../render/render.h"
#include "../../render/color.h"
#include "../../render/font.h"
#include "../../render/mask.h"

extern int s_bevel;
extern int s_handle_height;
extern int s_bwidth;
extern int s_cbwidth;

extern color_rgb *s_b_color;
extern color_rgb *s_cb_focused_color;
extern color_rgb *s_cb_unfocused_color;
extern color_rgb *s_title_focused_color;
extern color_rgb *s_title_unfocused_color;
extern color_rgb *s_titlebut_focused_color;
extern color_rgb *s_titlebut_unfocused_color;

extern int s_winfont_height;
extern int s_winfont_shadow;
extern int s_winfont_shadow_offset;
extern ObFont *s_winfont;

extern pixmap_mask *s_max_mask;
extern pixmap_mask *s_icon_mask;
extern pixmap_mask *s_desk_mask;
extern pixmap_mask *s_close_mask;

extern Appearance *a_focused_unpressed_max;
extern Appearance *a_focused_pressed_max;
extern Appearance *a_unfocused_unpressed_max;
extern Appearance *a_unfocused_pressed_max;
extern Appearance *a_focused_unpressed_close;
extern Appearance *a_focused_pressed_close;
extern Appearance *a_unfocused_unpressed_close;
extern Appearance *a_unfocused_pressed_close;
extern Appearance *a_focused_unpressed_desk;
extern Appearance *a_focused_pressed_desk;
extern Appearance *a_unfocused_unpressed_desk;
extern Appearance *a_unfocused_pressed_desk;
extern Appearance *a_focused_unpressed_iconify;
extern Appearance *a_focused_pressed_iconify;
extern Appearance *a_unfocused_unpressed_iconify;
extern Appearance *a_unfocused_pressed_iconify;
extern Appearance *a_focused_grip;
extern Appearance *a_unfocused_grip;
extern Appearance *a_focused_title;
extern Appearance *a_unfocused_title;
extern Appearance *a_focused_label;
extern Appearance *a_unfocused_label;
extern Appearance *a_icon;
extern Appearance *a_focused_handle;
extern Appearance *a_unfocused_handle;

#endif
