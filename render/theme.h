#ifndef __theme_h
#define __theme_h

#include "render.h"
#include "color.h"
#include "font.h"
#include "mask.h"

extern gint theme_bevel;
extern gint theme_handle_height;
extern gint theme_bwidth;
extern gint theme_cbwidth;

#define theme_label_height (theme_winfont_height)
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

extern gint theme_winfont_height;
extern RrFont *theme_winfont;
extern gchar *theme_title_layout;

extern RrPixmapMask *theme_max_set_mask;
extern RrPixmapMask *theme_max_unset_mask;
extern RrPixmapMask *theme_iconify_mask;
extern RrPixmapMask *theme_desk_set_mask;
extern RrPixmapMask *theme_desk_unset_mask;
extern RrPixmapMask *theme_shade_set_mask;
extern RrPixmapMask *theme_shade_unset_mask;
extern RrPixmapMask *theme_close_mask;

extern RrAppearance *theme_a_focused_unpressed_max;
extern RrAppearance *theme_a_focused_pressed_max;
extern RrAppearance *theme_a_focused_pressed_set_max;
extern RrAppearance *theme_a_unfocused_unpressed_max;
extern RrAppearance *theme_a_unfocused_pressed_max;
extern RrAppearance *theme_a_unfocused_pressed_set_max;
extern RrAppearance *theme_a_focused_unpressed_close;
extern RrAppearance *theme_a_focused_pressed_close;
extern RrAppearance *theme_a_unfocused_unpressed_close;
extern RrAppearance *theme_a_unfocused_pressed_close;
extern RrAppearance *theme_a_focused_unpressed_desk;
extern RrAppearance *theme_a_focused_pressed_desk;
extern RrAppearance *theme_a_focused_pressed_set_desk;
extern RrAppearance *theme_a_unfocused_unpressed_desk;
extern RrAppearance *theme_a_unfocused_pressed_desk;
extern RrAppearance *theme_a_unfocused_pressed_set_desk;
extern RrAppearance *theme_a_focused_unpressed_shade;
extern RrAppearance *theme_a_focused_pressed_shade;
extern RrAppearance *theme_a_focused_pressed_set_shade;
extern RrAppearance *theme_a_unfocused_unpressed_shade;
extern RrAppearance *theme_a_unfocused_pressed_shade;
extern RrAppearance *theme_a_unfocused_pressed_set_shade;
extern RrAppearance *theme_a_focused_unpressed_iconify;
extern RrAppearance *theme_a_focused_pressed_iconify;
extern RrAppearance *theme_a_unfocused_unpressed_iconify;
extern RrAppearance *theme_a_unfocused_pressed_iconify;
extern RrAppearance *theme_a_focused_grip;
extern RrAppearance *theme_a_unfocused_grip;
extern RrAppearance *theme_a_focused_title;
extern RrAppearance *theme_a_unfocused_title;
extern RrAppearance *theme_a_focused_label;
extern RrAppearance *theme_a_unfocused_label;
extern RrAppearance *theme_a_icon;
extern RrAppearance *theme_a_focused_handle;
extern RrAppearance *theme_a_unfocused_handle;
extern RrAppearance *theme_a_menu_title;
extern RrAppearance *theme_a_menu;
extern RrAppearance *theme_a_menu_item;
extern RrAppearance *theme_a_menu_disabled;
extern RrAppearance *theme_a_menu_hilite;

extern RrAppearance *theme_app_hilite_bg;
extern RrAppearance *theme_app_unhilite_bg;
extern RrAppearance *theme_app_hilite_label;
extern RrAppearance *theme_app_unhilite_label;
extern RrAppearance *theme_app_icon;

void theme_startup(const RrInstance *inst);
void theme_shutdown();

gchar *theme_load(gchar *theme);

#endif
