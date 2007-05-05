/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   theme.h for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
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

#ifndef __theme_h
#define __theme_h

#include "render.h"

G_BEGIN_DECLS

typedef struct _RrTheme RrTheme;

struct _RrTheme {
    const RrInstance *inst;

    /* style settings - fonts */
    RrFont *win_font_focused;
    RrFont *win_font_unfocused;
    RrFont *menu_title_font;
    RrFont *menu_font;
    RrFont *osd_font;

    /* style settings - geometry */
    gint paddingx;
    gint paddingy;
    gint handle_height;
    gint fbwidth; /*!< frame border width */
    gint mbwidth; /*!< menu border width */
    gint cbwidthx;
    gint cbwidthy;
    gint menu_overlap;
    /* these ones are calculated, not set directly by the theme file */
    gint win_font_height;
    gint menu_title_font_height;
    gint menu_font_height;
    gint label_height;
    gint title_height;
    gint menu_title_height;
    gint button_size;
    gint grip_width;

    /* style settings - colors */
    RrColor *menu_b_color;
    RrColor *frame_b_color;
    RrColor *cb_focused_color;
    RrColor *cb_unfocused_color;
    RrColor *title_focused_color;
    RrColor *title_unfocused_color;
    RrColor *titlebut_disabled_focused_color;
    RrColor *titlebut_disabled_unfocused_color;
    RrColor *titlebut_hover_focused_color;
    RrColor *titlebut_hover_unfocused_color;
    RrColor *titlebut_toggled_hover_focused_color;
    RrColor *titlebut_toggled_hover_unfocused_color;
    RrColor *titlebut_toggled_focused_pressed_color;
    RrColor *titlebut_toggled_unfocused_pressed_color;
    RrColor *titlebut_toggled_focused_unpressed_color;
    RrColor *titlebut_toggled_unfocused_unpressed_color;
    RrColor *titlebut_focused_pressed_color;
    RrColor *titlebut_unfocused_pressed_color;
    RrColor *titlebut_focused_unpressed_color;
    RrColor *titlebut_unfocused_unpressed_color;
    RrColor *menu_title_color;
    RrColor *menu_color;
    RrColor *menu_selected_color;
    RrColor *menu_disabled_color;
    RrColor *menu_disabled_selected_color;
    RrColor *title_focused_shadow_color;
    gchar    title_focused_shadow_alpha;
    RrColor *title_unfocused_shadow_color;
    gchar    title_unfocused_shadow_alpha;
    RrColor *osd_color;
    RrColor *osd_shadow_color;
    gchar    osd_shadow_alpha;
    RrColor *menu_title_shadow_color;
    gchar    menu_title_shadow_alpha;
    RrColor *menu_text_normal_shadow_color;
    gchar    menu_text_normal_shadow_alpha;
    RrColor *menu_text_selected_shadow_color;
    gchar    menu_text_selected_shadow_alpha;
    RrColor *menu_text_disabled_shadow_color;
    gchar    menu_text_disabled_shadow_alpha;
    RrColor *menu_text_disabled_selected_shadow_color;
    gchar    menu_text_disabled_selected_shadow_alpha;

    /* style settings - pics */
    RrPixel32 *def_win_icon; /* 48x48 RGBA */

    /* style settings - masks */
    RrPixmapMask *max_mask;
    RrPixmapMask *max_hover_mask;
    RrPixmapMask *max_pressed_mask;
    RrPixmapMask *max_toggled_mask;
    RrPixmapMask *max_toggled_hover_mask;
    RrPixmapMask *max_toggled_pressed_mask;
    RrPixmapMask *max_disabled_mask;
    RrPixmapMask *iconify_mask;
    RrPixmapMask *iconify_hover_mask;
    RrPixmapMask *iconify_pressed_mask;
    RrPixmapMask *iconify_disabled_mask;
    RrPixmapMask *desk_mask;
    RrPixmapMask *desk_hover_mask;
    RrPixmapMask *desk_pressed_mask;
    RrPixmapMask *desk_toggled_mask;
    RrPixmapMask *desk_toggled_hover_mask;
    RrPixmapMask *desk_toggled_pressed_mask;
    RrPixmapMask *desk_disabled_mask;
    RrPixmapMask *shade_mask;
    RrPixmapMask *shade_hover_mask;
    RrPixmapMask *shade_pressed_mask;
    RrPixmapMask *shade_toggled_mask;
    RrPixmapMask *shade_toggled_hover_mask;
    RrPixmapMask *shade_toggled_pressed_mask;
    RrPixmapMask *shade_disabled_mask;
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
    RrAppearance *a_focused_unpressed_max;
    RrAppearance *a_focused_pressed_max;
    RrAppearance *a_unfocused_unpressed_max;
    RrAppearance *a_unfocused_pressed_max;
    RrAppearance *a_toggled_hover_focused_max;
    RrAppearance *a_toggled_hover_unfocused_max;
    RrAppearance *a_toggled_focused_unpressed_max;
    RrAppearance *a_toggled_focused_pressed_max;
    RrAppearance *a_toggled_unfocused_unpressed_max;
    RrAppearance *a_toggled_unfocused_pressed_max;
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
    RrAppearance *a_focused_unpressed_desk;
    RrAppearance *a_focused_pressed_desk;
    RrAppearance *a_unfocused_unpressed_desk;
    RrAppearance *a_unfocused_pressed_desk;
    RrAppearance *a_toggled_hover_focused_desk;
    RrAppearance *a_toggled_hover_unfocused_desk;
    RrAppearance *a_toggled_focused_unpressed_desk;
    RrAppearance *a_toggled_focused_pressed_desk;
    RrAppearance *a_toggled_unfocused_unpressed_desk;
    RrAppearance *a_toggled_unfocused_pressed_desk;
    RrAppearance *a_disabled_focused_shade;
    RrAppearance *a_disabled_unfocused_shade;
    RrAppearance *a_hover_focused_shade;
    RrAppearance *a_hover_unfocused_shade;
    RrAppearance *a_focused_unpressed_shade;
    RrAppearance *a_focused_pressed_shade;
    RrAppearance *a_unfocused_unpressed_shade;
    RrAppearance *a_unfocused_pressed_shade;
    RrAppearance *a_toggled_hover_focused_shade;
    RrAppearance *a_toggled_hover_unfocused_shade;
    RrAppearance *a_toggled_focused_unpressed_shade;
    RrAppearance *a_toggled_focused_pressed_shade;
    RrAppearance *a_toggled_unfocused_unpressed_shade;
    RrAppearance *a_toggled_unfocused_pressed_shade;
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
    RrAppearance *a_menu_text_title;
    RrAppearance *a_menu_title;
    RrAppearance *a_menu;
    RrAppearance *a_menu_normal;
    RrAppearance *a_menu_selected;
    RrAppearance *a_menu_disabled;
    RrAppearance *a_menu_disabled_selected;
    RrAppearance *a_menu_text_normal;
    RrAppearance *a_menu_text_disabled;
    RrAppearance *a_menu_text_disabled_selected;
    RrAppearance *a_menu_text_selected;
    RrAppearance *a_menu_bullet_normal;
    RrAppearance *a_menu_bullet_selected;
    RrAppearance *a_clear;     /* clear with no texture */
    RrAppearance *a_clear_tex; /* clear with a texture */

    RrAppearance *osd_hilite_bg; /* can never be parent relative */
    RrAppearance *osd_hilite_fg; /* can never be parent relative */
    RrAppearance *osd_hilite_label; /* can be parent relative */
    RrAppearance *osd_unhilite_fg; /* can never be parent relative */

};

/*! The font values are all optional. If a NULL is used for any of them, then
  the default font will be used. */
RrTheme* RrThemeNew(const RrInstance *inst, gchar *theme,
                    RrFont *active_window_font, RrFont *inactive_window_font,
                    RrFont *menu_title_font, RrFont *menu_item_font,
                    RrFont *osd_font);
void RrThemeFree(RrTheme *theme);

G_END_DECLS

#endif
