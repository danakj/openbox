#ifndef __engine_openbox_h
#define __engine_openbox_h

#include "../../render/render.h"
#include "../../render/color.h"

extern int s_font_height;
extern int s_bevel;
extern int s_handle_height;
extern int s_bwidth;
extern int s_cbwidth;

extern color_rgb *s_b_color;
extern color_rgb *s_cb_focused_color;
extern color_rgb *s_cb_unfocused_color;

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
