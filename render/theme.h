#ifndef __theme_h
#define __theme_h

#include "render.h"
#include "color.h"
#include "font.h"
#include "mask.h"

extern int theme_bevel;
extern int theme_handle_height;
extern int theme_bwidth;
extern int theme_cbwidth;

#define theme_label_height (theme_winfont_height + 2)
#define theme_title_height (theme_label_height + theme_bevel * 2)
#define theme_button_size  (theme_label_height - 2)
#define theme_grip_width   (theme_button_size * 2)

extern color_rgb *theme_b_color;
extern color_rgb *theme_cb_focused_color;
extern color_rgb *theme_cb_unfocused_color;
extern color_rgb *theme_title_focused_color;
extern color_rgb *theme_title_unfocused_color;
extern color_rgb *theme_titlebut_focused_color;
extern color_rgb *theme_titlebut_unfocused_color;

extern int theme_winfont_height;
extern ObFont *theme_winfont;
extern char *theme_title_layout;

extern pixmap_mask *theme_max_set_mask;
extern pixmap_mask *theme_max_unset_mask;
extern pixmap_mask *theme_iconify_mask;
extern pixmap_mask *theme_desk_set_mask;
extern pixmap_mask *theme_desk_unset_mask;
extern pixmap_mask *theme_shade_set_mask;
extern pixmap_mask *theme_shade_unset_mask;
extern pixmap_mask *theme_close_mask;

extern Appearance *theme_a_focused_unpressed_max;
extern Appearance *theme_a_focused_pressed_max;
extern Appearance *theme_a_focused_pressed_set_max;
extern Appearance *theme_a_unfocused_unpressed_max;
extern Appearance *theme_a_unfocused_pressed_max;
extern Appearance *theme_a_unfocused_pressed_set_max;
extern Appearance *theme_a_focused_unpressed_close;
extern Appearance *theme_a_focused_pressed_close;
extern Appearance *theme_a_unfocused_unpressed_close;
extern Appearance *theme_a_unfocused_pressed_close;
extern Appearance *theme_a_focused_unpressed_desk;
extern Appearance *theme_a_focused_pressed_desk;
extern Appearance *theme_a_focused_pressed_set_desk;
extern Appearance *theme_a_unfocused_unpressed_desk;
extern Appearance *theme_a_unfocused_pressed_desk;
extern Appearance *theme_a_unfocused_pressed_set_desk;
extern Appearance *theme_a_focused_unpressed_shade;
extern Appearance *theme_a_focused_pressed_shade;
extern Appearance *theme_a_focused_pressed_set_shade;
extern Appearance *theme_a_unfocused_unpressed_shade;
extern Appearance *theme_a_unfocused_pressed_shade;
extern Appearance *theme_a_unfocused_pressed_set_shade;
extern Appearance *theme_a_focused_unpressed_iconify;
extern Appearance *theme_a_focused_pressed_iconify;
extern Appearance *theme_a_unfocused_unpressed_iconify;
extern Appearance *theme_a_unfocused_pressed_iconify;
extern Appearance *theme_a_focused_grip;
extern Appearance *theme_a_unfocused_grip;
extern Appearance *theme_a_focused_title;
extern Appearance *theme_a_unfocused_title;
extern Appearance *theme_a_focused_label;
extern Appearance *theme_a_unfocused_label;
extern Appearance *theme_a_icon;
extern Appearance *theme_a_focused_handle;
extern Appearance *theme_a_unfocused_handle;
extern Appearance *theme_a_menu_title;
extern Appearance *theme_a_menu;
extern Appearance *theme_a_menu_item;
extern Appearance *theme_a_menu_disabled;
extern Appearance *theme_a_menu_hilite;

extern Appearance *theme_app_hilite_bg;
extern Appearance *theme_app_unhilite_bg;
extern Appearance *theme_app_hilite_label;
extern Appearance *theme_app_unhilite_label;
extern Appearance *theme_app_icon;

void theme_startup();
void theme_shutdown();

char *theme_load(char *theme);

#endif
