#ifndef __theme_h
#define __theme_h

#include "render.h"

typedef struct _RrTheme RrTheme;

struct _RrTheme {
    gchar *path;
    gchar *name;

    const RrInstance *inst;

    /* style settings - optional decor */
    gboolean show_handle;

    /* style settings - geometry */
    gint bevel;
    gint handle_height;
    gint bwidth;
    gint cbwidth;
    gint label_height;
    gint title_height;
    gint button_size;
    gint grip_width;
    gint menu_overlap;

    /* style settings - colors */
    RrColor *b_color;
    RrColor *cb_focused_color;
    RrColor *cb_unfocused_color;
    RrColor *title_focused_color;
    RrColor *title_unfocused_color;
    RrColor *titlebut_disabled_focused_color;
    RrColor *titlebut_disabled_unfocused_color;
    RrColor *titlebut_hover_focused_color;
    RrColor *titlebut_hover_unfocused_color;
    RrColor *titlebut_toggled_focused_color;
    RrColor *titlebut_toggled_unfocused_color;
    RrColor *titlebut_focused_pressed_color;
    RrColor *titlebut_unfocused_pressed_color;
    RrColor *titlebut_focused_unpressed_color;
    RrColor *titlebut_unfocused_unpressed_color;
    RrColor *menu_title_color;
    RrColor *menu_color;
    RrColor *menu_bullet_color;
    RrColor *menu_disabled_color;
    RrColor *menu_hilite_color;

    /* style settings - fonts */
    gint winfont_height;
    RrFont *winfont_focused;
    RrFont *winfont_unfocused;
    gint mtitlefont_height;
    RrFont *mtitlefont;
    gint mfont_height;
    RrFont *mfont;

    /* style settings - masks */
    RrPixmapMask *max_mask;
    RrPixmapMask *max_toggled_mask;
    RrPixmapMask *max_hover_mask;
    RrPixmapMask *max_disabled_mask;
    RrPixmapMask *max_pressed_mask;
    RrPixmapMask *iconify_mask;
    RrPixmapMask *iconify_hover_mask;
    RrPixmapMask *iconify_disabled_mask;
    RrPixmapMask *iconify_pressed_mask;
    RrPixmapMask *desk_mask;
    RrPixmapMask *desk_toggled_mask;
    RrPixmapMask *desk_hover_mask;
    RrPixmapMask *desk_disabled_mask;
    RrPixmapMask *desk_pressed_mask;
    RrPixmapMask *shade_mask;
    RrPixmapMask *shade_toggled_mask;
    RrPixmapMask *shade_hover_mask;
    RrPixmapMask *shade_disabled_mask;
    RrPixmapMask *shade_pressed_mask;
    RrPixmapMask *close_mask;
    RrPixmapMask *close_hover_mask;
    RrPixmapMask *close_disabled_mask;
    RrPixmapMask *close_pressed_mask;

    RrPixmapMask *menu_bullet_mask; /* submenu pointer */
    RrPixmapMask *menu_toggle_mask; /* menu boolean */

    /* global appearances */
    RrAppearance *a_disabled_focused_max;
    RrAppearance *a_disabled_unfocused_max;
    RrAppearance *a_hover_focused_max;
    RrAppearance *a_hover_unfocused_max;
    RrAppearance *a_toggled_focused_max;
    RrAppearance *a_toggled_unfocused_max;
    RrAppearance *a_focused_unpressed_max;
    RrAppearance *a_focused_pressed_max;
    RrAppearance *a_unfocused_unpressed_max;
    RrAppearance *a_unfocused_pressed_max;
    RrAppearance *a_disabled_focused_close;
    RrAppearance *a_disabled_unfocused_close;
    RrAppearance *a_hover_focused_close;
    RrAppearance *a_hover_unfocused_close;
    RrAppearance *a_focused_unpressed_close;
    RrAppearance *a_focused_pressed_close;
    RrAppearance *a_unfocused_unpressed_close;
    RrAppearance *a_unfocused_pressed_close;
    RrAppearance *a_disabled_focused_desk;
    RrAppearance *a_disabled_unfocused_desk;
    RrAppearance *a_hover_focused_desk;
    RrAppearance *a_hover_unfocused_desk;
    RrAppearance *a_toggled_focused_desk;
    RrAppearance *a_toggled_unfocused_desk;
    RrAppearance *a_focused_unpressed_desk;
    RrAppearance *a_focused_pressed_desk;
    RrAppearance *a_unfocused_unpressed_desk;
    RrAppearance *a_unfocused_pressed_desk;
    RrAppearance *a_disabled_focused_shade;
    RrAppearance *a_disabled_unfocused_shade;
    RrAppearance *a_hover_focused_shade;
    RrAppearance *a_hover_unfocused_shade;
    RrAppearance *a_toggled_focused_shade;
    RrAppearance *a_toggled_unfocused_shade;
    RrAppearance *a_focused_unpressed_shade;
    RrAppearance *a_focused_pressed_shade;
    RrAppearance *a_unfocused_unpressed_shade;
    RrAppearance *a_unfocused_pressed_shade;
    RrAppearance *a_disabled_focused_iconify;
    RrAppearance *a_disabled_unfocused_iconify;
    RrAppearance *a_hover_focused_iconify;
    RrAppearance *a_hover_unfocused_iconify;
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
    RrAppearance *a_menu_text_item;
    RrAppearance *a_menu_text_disabled;
    RrAppearance *a_menu_text_hilite;
    RrAppearance *a_menu_bullet;
    RrAppearance *a_clear;     /* clear with no texture */
    RrAppearance *a_clear_tex; /* clear with a texture */

    RrAppearance *app_hilite_bg;
    RrAppearance *app_unhilite_bg;
    RrAppearance *app_hilite_label;
    RrAppearance *app_unhilite_label;

};

RrTheme *RrThemeNew(const RrInstance *inst, gchar *theme);
void RrThemeFree(RrTheme *theme);

#endif
