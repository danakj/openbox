#ifndef __theme_h
#define __theme_h

#include "render.h"
#include "color.h"
#include "font.h"
#include "mask.h"

typedef struct _RrTheme RrTheme;

struct _RrTheme {
    gchar *name;

    const RrInstance *inst;

    /* style settings - geometry */
    gint bevel;
    gint handle_height;
    gint bwidth;
    gint cbwidth;
    gint label_height;
    gint title_height;
    gint button_size;
    gint grip_width;

    /* style settings - colors */
    color_rgb *b_color;
    color_rgb *cb_focused_color;
    color_rgb *cb_unfocused_color;
    color_rgb *title_focused_color;
    color_rgb *title_unfocused_color;
    color_rgb *titlebut_focused_color;
    color_rgb *titlebut_unfocused_color;
    color_rgb *menu_title_color;
    color_rgb *menu_color;
    color_rgb *menu_disabled_color;
    color_rgb *menu_hilite_color;

    /* style settings - fonts */
    gint winfont_height;
    RrFont *winfont;
    gboolean winfont_shadow;
    gint winfont_shadow_offset;
    gint winfont_shadow_tint;
    gint mtitlefont_height;
    RrFont *mtitlefont;
    gboolean mtitlefont_shadow;
    gint mtitlefont_shadow_offset;
    gint mtitlefont_shadow_tint;
    gint mfont_height;
    RrFont *mfont;
    gboolean mfont_shadow;
    gint mfont_shadow_offset;
    gint mfont_shadow_tint;

    /* style settings - title layout */
    gchar *title_layout;

    /* style settings - masks */
    RrPixmapMask *max_set_mask;
    RrPixmapMask *max_unset_mask;
    RrPixmapMask *iconify_mask;
    RrPixmapMask *desk_set_mask;
    RrPixmapMask *desk_unset_mask;
    RrPixmapMask *shade_set_mask;
    RrPixmapMask *shade_unset_mask;
    RrPixmapMask *close_mask;

    /* global appearances */
    RrAppearance *a_focused_unpressed_max;
    RrAppearance *a_focused_pressed_max;
    RrAppearance *a_focused_pressed_set_max;
    RrAppearance *a_unfocused_unpressed_max;
    RrAppearance *a_unfocused_pressed_max;
    RrAppearance *a_unfocused_pressed_set_max;
    RrAppearance *a_focused_unpressed_close;
    RrAppearance *a_focused_pressed_close;
    RrAppearance *a_unfocused_unpressed_close;
    RrAppearance *a_unfocused_pressed_close;
    RrAppearance *a_focused_unpressed_desk;
    RrAppearance *a_focused_pressed_desk;
    RrAppearance *a_focused_pressed_set_desk;
    RrAppearance *a_unfocused_unpressed_desk;
    RrAppearance *a_unfocused_pressed_desk;
    RrAppearance *a_unfocused_pressed_set_desk;
    RrAppearance *a_focused_unpressed_shade;
    RrAppearance *a_focused_pressed_shade;
    RrAppearance *a_focused_pressed_set_shade;
    RrAppearance *a_unfocused_unpressed_shade;
    RrAppearance *a_unfocused_pressed_shade;
    RrAppearance *a_unfocused_pressed_set_shade;
    RrAppearance *a_focused_unpressed_iconify;
    RrAppearance *a_focused_pressed_iconify;
    RrAppearance *a_unfocused_unpressed_iconify;
    RrAppearance *a_unfocused_pressed_iconify;
    RrAppearance *a_focused_grip;
    RrAppearance *a_unfocused_grip;
    RrAppearance *a_focused_title;
    RrAppearance *a_unfocused_title;
    RrAppearance *a_focused_label;
    RrAppearance *a_unfocused_label;
    /* always parentrelative, so no focused/unfocused */
    RrAppearance *a_icon;
    RrAppearance *a_focused_handle;
    RrAppearance *a_unfocused_handle;
    RrAppearance *a_menu_title;
    RrAppearance *a_menu;
    RrAppearance *a_menu_item;
    RrAppearance *a_menu_disabled;
    RrAppearance *a_menu_hilite;

    RrAppearance *app_hilite_bg;
    RrAppearance *app_unhilite_bg;
    RrAppearance *app_hilite_label;
    RrAppearance *app_unhilite_label;
    RrAppearance *app_icon;
};

RrTheme *RrThemeNew(const RrInstance *inst, gchar *theme);
void RrThemeFree(RrTheme *theme);

#endif
