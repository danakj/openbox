#ifndef __theme_h
#define __theme_h

#include "render.h"

typedef struct _RrTheme RrTheme;

struct _RrTheme {
    gchar *path;
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
    RrColor *b_color;
    RrColor *cb_focused_color;
    RrColor *cb_unfocused_color;
    RrColor *title_focused_color;
    RrColor *title_unfocused_color;
    RrColor *titlebut_focused_color;
    RrColor *titlebut_unfocused_color;
    RrColor *menu_title_color;
    RrColor *menu_color;
    RrColor *menu_disabled_color;
    RrColor *menu_hilite_color;

    /* style settings - fonts */
    gint winfont_height;
    RrFont *winfont;
    gint mtitlefont_height;
    RrFont *mtitlefont;
    gint mfont_height;
    RrFont *mfont;

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
