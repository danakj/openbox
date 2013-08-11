/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   theme.c for the Openbox window manager
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

#include "render.h"
#include "color.h"
#include "font.h"
#include "mask.h"
#include "theme.h"
#include "icon.h"
#include "obt/paths.h"

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static XrmDatabase loaddb(const gchar *name, gchar **path);
static gboolean read_int(XrmDatabase db, const gchar *rname, gint *value);
static gboolean read_string(XrmDatabase db, const gchar *rname, gchar **value);
static gboolean read_color(XrmDatabase db, const RrInstance *inst,
                           const gchar *rname, RrColor **value);
static gboolean read_mask(const RrInstance *inst, const gchar *path,
                          RrTheme *theme, const gchar *maskname,
                          RrPixmapMask **value);
static gboolean read_appearance(XrmDatabase db, const RrInstance *inst,
                                const gchar *rname, RrAppearance *value,
                                gboolean allow_trans);
static int parse_inline_number(const char *p);
static RrPixel32* read_c_image(gint width, gint height, const guint8 *data);
static void set_default_appearance(RrAppearance *a);
static void read_button_colors(XrmDatabase db, const RrInstance *inst, 
                               const RrTheme *theme, RrButton *btn, 
                               const gchar *btnname);

static RrFont *get_font(RrFont *target, RrFont **default_font,
                        const RrInstance *inst)
{
    if (target) {
        RrFontRef(target);
        return target;
    } else {
        /* Only load the default font once */
        if (*default_font) {
            RrFontRef(*default_font);
        } else {
            *default_font = RrFontOpenDefault(inst);
        }
        return *default_font;
    }
}

#define READ_INT(x_resstr, x_var, x_min, x_max, x_def) \
    if (!read_int(db, x_resstr, & x_var) || \
            x_var < x_min || x_var > x_max) \
        x_var = x_def;

#define READ_COLOR(x_resstr, x_var, x_def) \
    if (!read_color(db, inst, x_resstr, & x_var)) \
        x_var = x_def;

#define READ_COLOR_(x_res1, x_res2, x_var, x_def) \
    if (!read_color(db, inst, x_res1, & x_var) && \
        !read_color(db, inst, x_res2, & x_var)) \
        x_var = x_def;

#define READ_MASK_COPY(x_file, x_var, x_copysrc) \
    if (!read_mask(inst, path, theme, x_file, & x_var)) \
        x_var = RrPixmapMaskCopy(x_copysrc);

#define READ_APPEARANCE(x_resstr, x_var, x_parrel) \
    if (!read_appearance(db, inst, x_resstr, x_var, x_parrel)) \
        set_default_appearance(x_var);

#define READ_APPEARANCE_COPY(x_resstr, x_var, x_parrel, x_defval) \
    if (!read_appearance(db, inst, x_resstr, x_var, x_parrel)) {\
        RrAppearanceFree(x_var); \
        x_var = RrAppearanceCopy(x_defval); }

#define READ_APPEARANCE_COPY_TEXTURES(x_resstr, x_var, x_parrel, x_defval, n_tex) \
    if (!read_appearance(db, inst, x_resstr, x_var, x_parrel)) {\
        RrAppearanceFree(x_var); \
        x_var = RrAppearanceCopy(x_defval); \
        RrAppearanceRemoveTextures(x_var); \
        RrAppearanceAddTextures(x_var, 5); }

#define READ_APPEARANCE_(x_res1, x_res2, x_var, x_parrel, x_defval) \
    if (!read_appearance(db, inst, x_res1, x_var, x_parrel) && \
        !read_appearance(db, inst, x_res2, x_var, x_parrel)) {\
        RrAppearanceFree(x_var); \
        x_var = RrAppearanceCopy(x_defval); }

RrTheme* RrThemeNew(const RrInstance *inst, const gchar *name,
                    gboolean allow_fallback,
                    RrFont *active_window_font, RrFont *inactive_window_font,
                    RrFont *menu_title_font, RrFont *menu_item_font,
                    RrFont *active_osd_font, RrFont *inactive_osd_font)
{
    XrmDatabase db = NULL;
    RrJustify winjust, mtitlejust;
    gchar *str;
    RrTheme *theme;
    RrFont *default_font = NULL;
    gchar *path;
    gboolean userdef;
    gint menu_overlap = 0;
    RrAppearance *a_disabled_focused_tmp;
    RrAppearance *a_disabled_unfocused_tmp;
    RrAppearance *a_hover_focused_tmp;
    RrAppearance *a_hover_unfocused_tmp;
    RrAppearance *a_focused_unpressed_tmp;
    RrAppearance *a_focused_pressed_tmp;
    RrAppearance *a_unfocused_unpressed_tmp;
    RrAppearance *a_unfocused_pressed_tmp;
    RrAppearance *a_toggled_hover_focused_tmp;
    RrAppearance *a_toggled_hover_unfocused_tmp;
    RrAppearance *a_toggled_focused_unpressed_tmp;
    RrAppearance *a_toggled_focused_pressed_tmp;
    RrAppearance *a_toggled_unfocused_unpressed_tmp;
    RrAppearance *a_toggled_unfocused_pressed_tmp;

    if (name) {
        db = loaddb(name, &path);
        if (db == NULL) {
            g_message("Unable to load the theme '%s'", name);
            if (allow_fallback)
                g_message("Falling back to the default theme '%s'",
                          DEFAULT_THEME);
            /* fallback to the default theme */
            name = NULL;
        }
    }
    if (name == NULL) {
        if (allow_fallback) {
            db = loaddb(DEFAULT_THEME, &path);
            if (db == NULL) {
                g_message("Unable to load the theme '%s'", DEFAULT_THEME);
                return NULL;
            }
        } else
            return NULL;
    }

    /* initialize temp reading textures */
    a_disabled_focused_tmp = RrAppearanceNew(inst, 1);
    a_disabled_unfocused_tmp = RrAppearanceNew(inst, 1);
    a_hover_focused_tmp = RrAppearanceNew(inst, 1);
    a_hover_unfocused_tmp = RrAppearanceNew(inst, 1);
    a_toggled_focused_unpressed_tmp = RrAppearanceNew(inst, 1);
    a_toggled_unfocused_unpressed_tmp = RrAppearanceNew(inst, 1);
    a_toggled_hover_focused_tmp = RrAppearanceNew(inst, 1);
    a_toggled_hover_unfocused_tmp = RrAppearanceNew(inst, 1);
    a_toggled_focused_pressed_tmp = RrAppearanceNew(inst, 1);
    a_toggled_unfocused_pressed_tmp = RrAppearanceNew(inst, 1);
    a_focused_unpressed_tmp = RrAppearanceNew(inst, 1);
    a_focused_pressed_tmp = RrAppearanceNew(inst, 1);
    a_unfocused_unpressed_tmp = RrAppearanceNew(inst, 1);
    a_unfocused_pressed_tmp = RrAppearanceNew(inst, 1);

    /* initialize theme */
    theme = g_slice_new0(RrTheme);

    theme->inst = inst;
    theme->name = g_strdup(name ? name : DEFAULT_THEME);

    /* init buttons */
    theme->btn_max = RrButtonNew(inst);
    theme->btn_close = RrButtonNew(inst);
    theme->btn_desk = RrButtonNew(inst);
    theme->btn_shade = RrButtonNew(inst);
    theme->btn_iconify = RrButtonNew(inst);

    /* init appearances */
    theme->a_focused_grip = RrAppearanceNew(inst, 0);
    theme->a_unfocused_grip = RrAppearanceNew(inst, 0);
    theme->a_focused_title = RrAppearanceNew(inst, 0);
    theme->a_unfocused_title = RrAppearanceNew(inst, 0);
    theme->a_focused_label = RrAppearanceNew(inst, 1);
    theme->a_unfocused_label = RrAppearanceNew(inst, 1);
    theme->a_icon = RrAppearanceNew(inst, 1);
    theme->a_focused_handle = RrAppearanceNew(inst, 0);
    theme->a_unfocused_handle = RrAppearanceNew(inst, 0);
    theme->a_menu = RrAppearanceNew(inst, 0);
    theme->a_menu_title = RrAppearanceNew(inst, 0);
    theme->a_menu_text_title = RrAppearanceNew(inst, 1);
    theme->a_menu_normal = RrAppearanceNew(inst, 0);
    theme->a_menu_selected = RrAppearanceNew(inst, 0);
    theme->a_menu_disabled = RrAppearanceNew(inst, 0);
    /* a_menu_disabled_selected is copied from a_menu_selected */
    theme->a_menu_text_normal = RrAppearanceNew(inst, 1);
    theme->a_menu_text_selected = RrAppearanceNew(inst, 1);
    theme->a_menu_text_disabled = RrAppearanceNew(inst, 1);
    theme->a_menu_text_disabled_selected = RrAppearanceNew(inst, 1);
    theme->a_menu_bullet_normal = RrAppearanceNew(inst, 1);
    theme->a_menu_bullet_selected = RrAppearanceNew(inst, 1);
    theme->a_clear = RrAppearanceNew(inst, 0);
    theme->a_clear_tex = RrAppearanceNew(inst, 1);
    theme->osd_bg = RrAppearanceNew(inst, 0);
    theme->osd_hilite_label = RrAppearanceNew(inst, 1);
    theme->osd_hilite_bg = RrAppearanceNew(inst, 0);
    theme->osd_unhilite_label = RrAppearanceNew(inst, 1);
    theme->osd_unhilite_bg = RrAppearanceNew(inst, 0);
    theme->osd_unpressed_button = RrAppearanceNew(inst, 1);
    theme->osd_pressed_button = RrAppearanceNew(inst, 5);
    theme->osd_focused_button = RrAppearanceNew(inst, 5);

    /* load the font stuff */
    theme->win_font_focused = get_font(active_window_font,
                                       &default_font, inst);
    theme->win_font_unfocused = get_font(inactive_window_font,
                                         &default_font, inst);

    winjust = RR_JUSTIFY_LEFT;
    if (read_string(db, "window.label.text.justify", &str)) {
        if (!g_ascii_strcasecmp(str, "right"))
            winjust = RR_JUSTIFY_RIGHT;
        else if (!g_ascii_strcasecmp(str, "center"))
            winjust = RR_JUSTIFY_CENTER;
    }

    theme->menu_title_font = get_font(menu_title_font, &default_font, inst);

    mtitlejust = RR_JUSTIFY_LEFT;
    if (read_string(db, "menu.title.text.justify", &str)) {
        if (!g_ascii_strcasecmp(str, "right"))
            mtitlejust = RR_JUSTIFY_RIGHT;
        else if (!g_ascii_strcasecmp(str, "center"))
            mtitlejust = RR_JUSTIFY_CENTER;
    }

    theme->menu_font = get_font(menu_item_font, &default_font, inst);

    theme->osd_font_hilite = get_font(active_osd_font, &default_font, inst);
    theme->osd_font_unhilite = get_font(inactive_osd_font, &default_font,inst);

    /* load direct dimensions */
    READ_INT("menu.overlap", menu_overlap, -100, 100, 0);
    READ_INT("menu.overlap.x", theme->menu_overlap_x, -100, 100, menu_overlap);
    READ_INT("menu.overlap.y", theme->menu_overlap_y, -100, 100, menu_overlap);
    READ_INT("window.handle.width", theme->handle_height, 0, 100, 6);
    READ_INT("padding.width", theme->paddingx, 0, 100, 3);
    READ_INT("padding.height", theme->paddingy, 0, 100, theme->paddingx);
    READ_INT("border.width", theme->fbwidth, 0, 100, 1);
    READ_INT("menu.border.width", theme->mbwidth, 0, 100, theme->fbwidth);
    READ_INT("osd.border.width", theme->obwidth, 0, 100, theme->fbwidth);
    READ_INT("undecorated.border.width", theme->ubwidth, 0, 100,
             theme->fbwidth);
    READ_INT("menu.separator.width", theme->menu_sep_width, 1, 100, 1);
    READ_INT("menu.separator.padding.width", theme->menu_sep_paddingx,
             0, 100, 6);
    READ_INT("menu.separator.padding.height", theme->menu_sep_paddingy,
             0, 100, 3);
    READ_INT("window.client.padding.width", theme->cbwidthx, 0, 100,
             theme->paddingx);
    READ_INT("window.client.padding.height", theme->cbwidthy, 0, 100,
             theme->cbwidthx);

    /* load colors */
    READ_COLOR_("window.active.border.color", "border.color",
                theme->frame_focused_border_color, RrColorNew(inst, 0, 0, 0));
    /* undecorated focused border color inherits from frame focused border
       color */
    READ_COLOR("window.undecorated.active.border.color",
               theme->frame_undecorated_focused_border_color,
               RrColorCopy(theme->frame_focused_border_color));
    /* title separator focused color inherits from focused border color */
    READ_COLOR("window.active.title.separator.color",
               theme->title_separator_focused_color,
               RrColorCopy(theme->frame_focused_border_color));

    /* unfocused border color inherits from frame focused border color */
    READ_COLOR("window.inactive.border.color",
               theme->frame_unfocused_border_color,
               RrColorCopy(theme->frame_focused_border_color));

    /* undecorated unfocused border color inherits from frame unfocused border
       color */
    READ_COLOR("window.undecorated.inactive.border.color",
               theme->frame_undecorated_unfocused_border_color,
               RrColorCopy(theme->frame_unfocused_border_color));

    /* title separator unfocused color inherits from unfocused border color */
    READ_COLOR("window.inactive.title.separator.color",
               theme->title_separator_unfocused_color,
               RrColorCopy(theme->frame_unfocused_border_color));

    /* menu border color inherits from frame focused border color */
    READ_COLOR("menu.border.color", theme->menu_border_color,
               RrColorCopy(theme->frame_focused_border_color));

    /* osd border color inherits from frame focused border color */
    READ_COLOR("osd.border.color", theme->osd_border_color,
               RrColorCopy(theme->frame_focused_border_color));

    READ_COLOR("window.active.client.color", theme->cb_focused_color,
               RrColorNew(inst, 0xff, 0xff, 0xff));

    READ_COLOR("window.inactive.client.color", theme->cb_unfocused_color,
               RrColorNew(inst, 0xff, 0xff, 0xff));

    READ_COLOR("window.active.label.text.color", theme->title_focused_color,
               RrColorNew(inst, 0x0, 0x0, 0x0));

    READ_COLOR("window.inactive.label.text.color", theme->title_unfocused_color,
               RrColorNew(inst, 0xff, 0xff, 0xff));

    READ_COLOR_("osd.active.label.text.color",
                "osd.label.text.color",
                theme->osd_text_active_color, RrColorCopy(theme->title_focused_color));

    READ_COLOR_("osd.inactive.label.text.color",
                "osd.label.text.color",
                theme->osd_text_inactive_color, RrColorCopy(theme->title_unfocused_color));

    READ_COLOR("window.active.button.unpressed.image.color",
               theme->titlebut_focused_unpressed_color,
               RrColorNew(inst, 0, 0, 0));

    READ_COLOR("window.inactive.button.unpressed.image.color",
               theme->titlebut_unfocused_unpressed_color,
               RrColorNew(inst, 0xff, 0xff, 0xff));

    READ_COLOR("window.active.button.pressed.image.color",
               theme->titlebut_focused_pressed_color,
               RrColorCopy(theme->titlebut_focused_unpressed_color));

    READ_COLOR("window.inactive.button.pressed.image.color",
               theme->titlebut_unfocused_pressed_color,
               RrColorCopy(theme->titlebut_unfocused_unpressed_color));

    READ_COLOR("window.active.button.disabled.image.color",
               theme->titlebut_disabled_focused_color,
               RrColorNew(inst, 0xff, 0xff, 0xff));

    READ_COLOR("window.inactive.button.disabled.image.color",
               theme->titlebut_disabled_unfocused_color,
               RrColorNew(inst, 0, 0, 0));

    READ_COLOR("window.active.button.hover.image.color",
               theme->titlebut_hover_focused_color,
               RrColorCopy(theme->titlebut_focused_unpressed_color));

    READ_COLOR("window.inactive.button.hover.image.color",
               theme->titlebut_hover_unfocused_color,
               RrColorCopy(theme->titlebut_unfocused_unpressed_color));

    READ_COLOR_("window.active.button.toggled.unpressed.image.color",
                "window.active.button.toggled.image.color",
                theme->titlebut_toggled_focused_unpressed_color,
                RrColorCopy(theme->titlebut_focused_pressed_color));

    READ_COLOR_("window.inactive.button.toggled.unpressed.image.color",
                "window.inactive.button.toggled.image.color",
                theme->titlebut_toggled_unfocused_unpressed_color,
                RrColorCopy(theme->titlebut_unfocused_pressed_color));

    READ_COLOR("window.active.button.toggled.hover.image.color",
               theme->titlebut_toggled_hover_focused_color,
               RrColorCopy(theme->titlebut_toggled_focused_unpressed_color));

    READ_COLOR("window.inactive.button.toggled.hover.image.color",
               theme->titlebut_toggled_hover_unfocused_color,
               RrColorCopy(theme->titlebut_toggled_unfocused_unpressed_color));

    READ_COLOR("window.active.button.toggled.pressed.image.color",
               theme->titlebut_toggled_focused_pressed_color,
               RrColorCopy(theme->titlebut_focused_pressed_color));

    READ_COLOR("window.inactive.button.toggled.pressed.image.color",
               theme->titlebut_toggled_unfocused_pressed_color,
               RrColorCopy(theme->titlebut_unfocused_pressed_color));

    READ_COLOR("menu.title.text.color", theme->menu_title_color,
               RrColorNew(inst, 0, 0, 0));

    READ_COLOR("menu.items.text.color", theme->menu_color,
               RrColorNew(inst, 0xff, 0xff, 0xff));

    READ_COLOR("menu.bullet.image.color", theme->menu_bullet_color,
               RrColorCopy(theme->menu_color));
   
    READ_COLOR("menu.items.disabled.text.color", theme->menu_disabled_color,
               RrColorNew(inst, 0, 0, 0));

    READ_COLOR("menu.items.active.disabled.text.color",
               theme->menu_disabled_selected_color,
               RrColorCopy(theme->menu_disabled_color));

    READ_COLOR("menu.items.active.text.color", theme->menu_selected_color,
               RrColorNew(inst, 0, 0, 0));

    READ_COLOR("menu.separator.color", theme->menu_sep_color,
               RrColorCopy(theme->menu_color));
    
    READ_COLOR("menu.bullet.selected.image.color", 
               theme->menu_bullet_selected_color,
               RrColorCopy(theme->menu_selected_color));

    READ_COLOR("osd.button.unpressed.text.color", theme->osd_unpressed_color,
               RrColorCopy(theme->osd_text_active_color));
    READ_COLOR("osd.button.pressed.text.color", theme->osd_pressed_color,
               RrColorCopy(theme->osd_text_active_color));
    READ_COLOR("osd.button.focused.text.color", theme->osd_focused_color,
               RrColorCopy(theme->osd_text_active_color));
    READ_COLOR("osd.button.pressed.box.color", theme->osd_pressed_lineart,
               RrColorCopy(theme->titlebut_focused_pressed_color));
    READ_COLOR("osd.button.focused.box.color", theme->osd_focused_lineart,
               RrColorCopy(theme->titlebut_hover_focused_color));
 
    /* load the image masks */

    /* maximize button masks */
    userdef = TRUE;
    if (!read_mask(inst, path, theme, "max.xbm", &theme->btn_max->mask)) {
            guchar data[] = { 0x3f, 0x3f, 0x21, 0x21, 0x21, 0x3f };
            theme->btn_max->mask = RrPixmapMaskNew(inst, 6, 6, (gchar*)data);
            userdef = FALSE;
    }
    if (!read_mask(inst, path, theme, "max_toggled.xbm",
                   &theme->btn_max->toggled_mask))
    {
        if (userdef)
            theme->btn_max->toggled_mask = RrPixmapMaskCopy(theme->btn_max->mask);
        else {
            guchar data[] = { 0x3e, 0x22, 0x2f, 0x29, 0x39, 0x0f };
            theme->btn_max->toggled_mask = RrPixmapMaskNew(inst, 6, 6,(gchar*)data);
        }
    }
    READ_MASK_COPY("max_pressed.xbm", theme->btn_max->pressed_mask,
                   theme->btn_max->mask);
    READ_MASK_COPY("max_disabled.xbm", theme->btn_max->disabled_mask,
                   theme->btn_max->mask);
    READ_MASK_COPY("max_hover.xbm", theme->btn_max->hover_mask, 
                   theme->btn_max->mask);
    READ_MASK_COPY("max_toggled_pressed.xbm", 
                   theme->btn_max->toggled_pressed_mask, 
                   theme->btn_max->toggled_mask);
    READ_MASK_COPY("max_toggled_hover.xbm", 
                   theme->btn_max->toggled_hover_mask,
                   theme->btn_max->toggled_mask);

    /* iconify button masks */
    if (!read_mask(inst, path, theme, "iconify.xbm", &theme->btn_iconify->mask)) {
        guchar data[] = { 0x00, 0x00, 0x00, 0x00, 0x3f, 0x3f };
        theme->btn_iconify->mask = RrPixmapMaskNew(inst, 6, 6, (gchar*)data);
    }
    READ_MASK_COPY("iconify_pressed.xbm", theme->btn_iconify->pressed_mask,
                   theme->btn_iconify->mask);
    READ_MASK_COPY("iconify_disabled.xbm", theme->btn_iconify->disabled_mask,
                   theme->btn_iconify->mask);
    READ_MASK_COPY("iconify_hover.xbm", theme->btn_iconify->hover_mask,
                   theme->btn_iconify->mask);

    /* all desktops button masks */
    userdef = TRUE;
    if (!read_mask(inst, path, theme, "desk.xbm", &theme->btn_desk->mask)) {
        guchar data[] = { 0x33, 0x33, 0x00, 0x00, 0x33, 0x33 };
        theme->btn_desk->mask = RrPixmapMaskNew(inst, 6, 6, (gchar*)data);
        userdef = FALSE;
    }
    if (!read_mask(inst, path, theme, "desk_toggled.xbm",
                   &theme->btn_desk->toggled_mask)) {
        if (userdef)
            theme->btn_desk->toggled_mask = RrPixmapMaskCopy(theme->btn_desk->mask);
        else {
            guchar data[] = { 0x00, 0x1e, 0x1a, 0x16, 0x1e, 0x00 };
            theme->btn_desk->toggled_mask =
                RrPixmapMaskNew(inst, 6, 6, (gchar*)data);
        }
    }
    READ_MASK_COPY("desk_pressed.xbm", theme->btn_desk->pressed_mask,
                   theme->btn_desk->mask);
    READ_MASK_COPY("desk_disabled.xbm", theme->btn_desk->disabled_mask,
                   theme->btn_desk->mask);
    READ_MASK_COPY("desk_hover.xbm", theme->btn_desk->hover_mask, theme->btn_desk->mask);
    READ_MASK_COPY("desk_toggled_pressed.xbm",
                   theme->btn_desk->toggled_pressed_mask, theme->btn_desk->toggled_mask);
    READ_MASK_COPY("desk_toggled_hover.xbm", theme->btn_desk->toggled_hover_mask,
                   theme->btn_desk->toggled_mask);

    /* shade button masks */
    if (!read_mask(inst, path, theme, "shade.xbm", &theme->btn_shade->mask)) {
        guchar data[] = { 0x3f, 0x3f, 0x00, 0x00, 0x00, 0x00 };
        theme->btn_shade->mask = RrPixmapMaskNew(inst, 6, 6, (gchar*)data);
    }
    READ_MASK_COPY("shade_toggled.xbm", theme->btn_shade->toggled_mask,
                   theme->btn_shade->mask);
    READ_MASK_COPY("shade_pressed.xbm", theme->btn_shade->pressed_mask,
                   theme->btn_shade->mask);
    READ_MASK_COPY("shade_disabled.xbm", theme->btn_shade->disabled_mask,
                   theme->btn_shade->mask);
    READ_MASK_COPY("shade_hover.xbm", theme->btn_shade->hover_mask,
                   theme->btn_shade->mask);
    READ_MASK_COPY("shade_toggled_pressed.xbm",
                   theme->btn_shade->toggled_pressed_mask,
                   theme->btn_shade->toggled_mask);
    READ_MASK_COPY("shade_toggled_hover.xbm",
                   theme->btn_shade->toggled_hover_mask, 
                   theme->btn_shade->toggled_mask);

    /* close button masks */
    if (!read_mask(inst, path, theme, "close.xbm", &theme->btn_close->mask)) {
        guchar data[] = { 0x33, 0x3f, 0x1e, 0x1e, 0x3f, 0x33 };
        theme->btn_close->mask = RrPixmapMaskNew(inst, 6, 6, (gchar*)data);
    }
    READ_MASK_COPY("close_pressed.xbm", theme->btn_close->pressed_mask,
                   theme->btn_close->mask);
    READ_MASK_COPY("close_disabled.xbm", theme->btn_close->disabled_mask,
                   theme->btn_close->mask);
    READ_MASK_COPY("close_hover.xbm", theme->btn_close->hover_mask,
                   theme->btn_close->mask);

    /* submenu bullet mask */
    if (!read_mask(inst, path, theme, "bullet.xbm", &theme->menu_bullet_mask))
    {
        guchar data[] = { 0x01, 0x03, 0x07, 0x0f, 0x07, 0x03, 0x01 };
        theme->menu_bullet_mask = RrPixmapMaskNew(inst, 4, 7, (gchar*)data);
    }

    /* up and down arrows */
    {
        guchar data[] = { 0xfe, 0x00, 0x7c, 0x00, 0x38, 0x00, 0x10, 0x00 };
        theme->down_arrow_mask = RrPixmapMaskNew(inst, 9, 4, (gchar*)data);
    }
    {
        guchar data[] = { 0x10, 0x00, 0x38, 0x00, 0x7c, 0x00, 0xfe, 0x00 };
        theme->up_arrow_mask = RrPixmapMaskNew(inst, 9, 4, (gchar*)data);
    }

    /* setup the default window icon */
    theme->def_win_icon = read_c_image(OB_DEFAULT_ICON_WIDTH,
                                       OB_DEFAULT_ICON_HEIGHT,
                                       OB_DEFAULT_ICON_pixel_data);
    theme->def_win_icon_w = OB_DEFAULT_ICON_WIDTH;
    theme->def_win_icon_h = OB_DEFAULT_ICON_HEIGHT;

    /* the toggled hover mask = the toggled unpressed mask (i.e. no change) */
    theme->btn_max->toggled_hover_mask =
        RrPixmapMaskCopy(theme->btn_max->toggled_mask);
    theme->btn_desk->toggled_hover_mask =
        RrPixmapMaskCopy(theme->btn_desk->toggled_mask);
    theme->btn_shade->toggled_hover_mask =
        RrPixmapMaskCopy(theme->btn_shade->toggled_mask);
    /* the toggled pressed mask = the toggled unpressed mask (i.e. no change)*/
    theme->btn_max->toggled_pressed_mask =
        RrPixmapMaskCopy(theme->btn_max->toggled_mask);
    theme->btn_desk->toggled_pressed_mask =
        RrPixmapMaskCopy(theme->btn_desk->toggled_mask);
    theme->btn_shade->toggled_pressed_mask =
        RrPixmapMaskCopy(theme->btn_shade->toggled_mask);

    /* read the decoration textures */
    READ_APPEARANCE("window.active.title.bg", theme->a_focused_title, FALSE);
    READ_APPEARANCE("window.inactive.title.bg", theme->a_unfocused_title,
                    FALSE);
    READ_APPEARANCE("window.active.label.bg", theme->a_focused_label, TRUE);
    READ_APPEARANCE("window.inactive.label.bg", theme->a_unfocused_label,
                    TRUE);
    READ_APPEARANCE("window.active.handle.bg", theme->a_focused_handle, FALSE);
    READ_APPEARANCE("window.inactive.handle.bg",theme->a_unfocused_handle,
                    FALSE);
    READ_APPEARANCE("window.active.grip.bg", theme->a_focused_grip, TRUE);
    READ_APPEARANCE("window.inactive.grip.bg", theme->a_unfocused_grip, TRUE);
    READ_APPEARANCE("menu.items.bg", theme->a_menu, FALSE);
    READ_APPEARANCE("menu.title.bg", theme->a_menu_title, TRUE);
    READ_APPEARANCE("menu.items.active.bg", theme->a_menu_selected, TRUE);

    theme->a_menu_disabled_selected =
        RrAppearanceCopy(theme->a_menu_selected);

    /* read appearances for non-decorations (on-screen-display) */
    if (!read_appearance(db, inst, "osd.bg", theme->osd_bg, FALSE)) {
        RrAppearanceFree(theme->osd_bg);
        theme->osd_bg = RrAppearanceCopy(theme->a_focused_title);
    }
    if (!read_appearance(db, inst, "osd.active.label.bg",
                         theme->osd_hilite_label, TRUE) &&
        !read_appearance(db, inst, "osd.label.bg",
                         theme->osd_hilite_label, TRUE)) {
        RrAppearanceFree(theme->osd_hilite_label);
        theme->osd_hilite_label = RrAppearanceCopy(theme->a_focused_label);
    }
    if (!read_appearance(db, inst, "osd.inactive.label.bg",
                         theme->osd_unhilite_label, TRUE)) {
        RrAppearanceFree(theme->osd_unhilite_label);
        theme->osd_unhilite_label = RrAppearanceCopy(theme->a_unfocused_label);
    }
    /* osd_hilite_fg can't be parentrel */
    if (!read_appearance(db, inst, "osd.hilight.bg",
                         theme->osd_hilite_bg, FALSE)) {
        RrAppearanceFree(theme->osd_hilite_bg);
        if (theme->a_focused_label->surface.grad != RR_SURFACE_PARENTREL)
            theme->osd_hilite_bg = RrAppearanceCopy(theme->a_focused_label);
        else
            theme->osd_hilite_bg = RrAppearanceCopy(theme->a_focused_title);
    }
    /* osd_unhilite_fg can't be parentrel either */
    if (!read_appearance(db, inst, "osd.unhilight.bg",
                         theme->osd_unhilite_bg, FALSE)) {
        RrAppearanceFree(theme->osd_unhilite_bg);
        if (theme->a_unfocused_label->surface.grad != RR_SURFACE_PARENTREL)
            theme->osd_unhilite_bg=RrAppearanceCopy(theme->a_unfocused_label);
        else
            theme->osd_unhilite_bg=RrAppearanceCopy(theme->a_unfocused_title);
    }

    /* read buttons textures */

    /* bases: unpressed, pressed, disabled */
    READ_APPEARANCE("window.active.button.unpressed.bg",
                    a_focused_unpressed_tmp, TRUE);
    READ_APPEARANCE("window.inactive.button.unpressed.bg",
                    a_unfocused_unpressed_tmp, TRUE);
    READ_APPEARANCE("window.active.button.pressed.bg",
                    a_focused_pressed_tmp, TRUE);
    READ_APPEARANCE("window.inactive.button.pressed.bg",
                    a_unfocused_pressed_tmp, TRUE);
    READ_APPEARANCE("window.active.button.disabled.bg",
                    a_disabled_focused_tmp, TRUE);
    READ_APPEARANCE("window.inactive.button.disabled.bg",
                    a_disabled_unfocused_tmp, TRUE);

    /* hover */
    READ_APPEARANCE_COPY("window.active.button.hover.bg",
                         a_hover_focused_tmp, TRUE,
                         a_focused_unpressed_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.hover.bg",
                         a_hover_unfocused_tmp, TRUE,
                         a_unfocused_unpressed_tmp);

    /* toggled unpressed */
    READ_APPEARANCE_("window.active.button.toggled.unpressed.bg",
                     "window.active.button.toggled.bg",
                     a_toggled_focused_unpressed_tmp, TRUE,
                     a_focused_pressed_tmp);
    READ_APPEARANCE_("window.inactive.button.toggled.unpressed.bg",
                     "window.inactive.button.toggled.bg",
                     a_toggled_unfocused_unpressed_tmp, TRUE,
                     a_unfocused_pressed_tmp);

    /* toggled pressed */
    READ_APPEARANCE_COPY("window.active.button.toggled.pressed.bg",
                         a_toggled_focused_pressed_tmp, TRUE,
                         a_focused_pressed_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.toggled.pressed.bg",
                         a_toggled_unfocused_pressed_tmp, TRUE,
                         a_unfocused_pressed_tmp);

    /* toggled hover */
    READ_APPEARANCE_COPY("window.active.button.toggled.hover.bg",
                         a_toggled_hover_focused_tmp, TRUE,
                         a_toggled_focused_unpressed_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.toggled.hover.bg",
                         a_toggled_hover_unfocused_tmp, TRUE,
                         a_toggled_unfocused_unpressed_tmp);


    /* now do individual buttons, if specified */

    /* max button */
    read_button_colors(db, inst, theme, theme->btn_max, "max");

    /* bases:  unpressed, pressed, disabled */
    READ_APPEARANCE_COPY("window.active.button.max.unpressed.bg",
                         theme->btn_max->a_focused_unpressed, TRUE,
                         a_focused_unpressed_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.max.unpressed.bg",
                         theme->btn_max->a_unfocused_unpressed, TRUE,
                         a_unfocused_unpressed_tmp);
    READ_APPEARANCE_COPY("window.active.button.max.pressed.bg",
                         theme->btn_max->a_focused_pressed, TRUE,
                         a_focused_pressed_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.max.pressed.bg",
                         theme->btn_max->a_unfocused_pressed, TRUE,
                         a_unfocused_pressed_tmp);
    READ_APPEARANCE_COPY("window.active.button.max.disabled.bg",
                         theme->btn_max->a_disabled_focused, TRUE,
                         a_disabled_focused_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.max.disabled.bg",
                         theme->btn_max->a_disabled_unfocused, TRUE,
                         a_disabled_unfocused_tmp);

    /* hover */
    READ_APPEARANCE_COPY("window.active.button.max.hover.bg",
                         theme->btn_max->a_hover_focused, TRUE,
                         a_hover_focused_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.max.hover.bg",
                         theme->btn_max->a_hover_unfocused, TRUE,
                         a_hover_unfocused_tmp);

    /* toggled unpressed */
    READ_APPEARANCE_("window.active.button.max.toggled.unpressed.bg",
                     "window.active.button.max.toggled.bg",
                     theme->btn_max->a_toggled_focused_unpressed, TRUE,
                     a_toggled_focused_unpressed_tmp);
    READ_APPEARANCE_("window.inactive.button.max.toggled.unpressed.bg",
                     "window.inactive.button.max.toggled.bg",
                     theme->btn_max->a_toggled_unfocused_unpressed, TRUE,
                     a_toggled_unfocused_unpressed_tmp);

    /* toggled pressed */
    READ_APPEARANCE_COPY("window.active.button.max.toggled.pressed.bg",
                         theme->btn_max->a_toggled_focused_pressed, TRUE,
                         a_toggled_focused_pressed_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.max.toggled.pressed.bg",
                         theme->btn_max->a_toggled_unfocused_pressed, TRUE,
                         a_toggled_unfocused_pressed_tmp);

    /* toggled hover */
    READ_APPEARANCE_COPY("window.active.button.max.toggled.hover.bg",
                         theme->btn_max->a_toggled_hover_focused, TRUE,
                         a_toggled_hover_focused_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.max.toggled.hover.bg",
                         theme->btn_max->a_toggled_hover_unfocused, TRUE,
                         a_toggled_hover_unfocused_tmp);

    /* close button */
    read_button_colors(db, inst, theme, theme->btn_close, "close");

    READ_APPEARANCE_COPY("window.active.button.close.unpressed.bg",
                         theme->btn_close->a_focused_unpressed, TRUE,
                         a_focused_unpressed_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.close.unpressed.bg",
                         theme->btn_close->a_unfocused_unpressed, TRUE,
                         a_unfocused_unpressed_tmp);
    READ_APPEARANCE_COPY("window.active.button.close.pressed.bg",
                         theme->btn_close->a_focused_pressed, TRUE,
                         a_focused_pressed_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.close.pressed.bg",
                         theme->btn_close->a_unfocused_pressed, TRUE,
                         a_unfocused_pressed_tmp);
    READ_APPEARANCE_COPY("window.active.button.close.disabled.bg",
                         theme->btn_close->a_disabled_focused, TRUE,
                         a_disabled_focused_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.close.disabled.bg",
                         theme->btn_close->a_disabled_unfocused, TRUE,
                         a_disabled_unfocused_tmp);
    READ_APPEARANCE_COPY("window.active.button.close.hover.bg",
                         theme->btn_close->a_hover_focused, TRUE,
                         a_hover_focused_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.close.hover.bg",
                         theme->btn_close->a_hover_unfocused, TRUE,
                         a_hover_unfocused_tmp);

    /* desk button */
    read_button_colors(db, inst, theme, theme->btn_desk, "desk");

    /* bases:  unpressed, pressed, disabled */
    READ_APPEARANCE_COPY("window.active.button.desk.unpressed.bg",
                         theme->btn_desk->a_focused_unpressed, TRUE,
                         a_focused_unpressed_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.desk.unpressed.bg",
                         theme->btn_desk->a_unfocused_unpressed, TRUE,
                         a_unfocused_unpressed_tmp);
    READ_APPEARANCE_COPY("window.active.button.desk.pressed.bg",
                         theme->btn_desk->a_focused_pressed, TRUE,
                         a_focused_pressed_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.desk.pressed.bg",
                         theme->btn_desk->a_unfocused_pressed, TRUE,
                         a_unfocused_pressed_tmp);
    READ_APPEARANCE_COPY("window.active.button.desk.disabled.bg",
                         theme->btn_desk->a_disabled_focused, TRUE,
                         a_disabled_focused_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.desk.disabled.bg",
                         theme->btn_desk->a_disabled_unfocused, TRUE,
                         a_disabled_unfocused_tmp);

    /* hover */
    READ_APPEARANCE_COPY("window.active.button.desk.hover.bg",
                         theme->btn_desk->a_hover_focused, TRUE,
                         a_hover_focused_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.desk.hover.bg",
                         theme->btn_desk->a_hover_unfocused, TRUE,
                         a_hover_unfocused_tmp);

    /* toggled unpressed */
    READ_APPEARANCE_("window.active.button.desk.toggled.unpressed.bg",
                     "window.active.button.desk.toggled.bg",
                     theme->btn_desk->a_toggled_focused_unpressed, TRUE,
                     a_toggled_focused_unpressed_tmp);
    READ_APPEARANCE_("window.inactive.button.desk.toggled.unpressed.bg",
                     "window.inactive.button.desk.toggled.bg",
                     theme->btn_desk->a_toggled_unfocused_unpressed, TRUE,
                     a_toggled_unfocused_unpressed_tmp);

    /* toggled pressed */
    READ_APPEARANCE_COPY("window.active.button.desk.toggled.pressed.bg",
                         theme->btn_desk->a_toggled_focused_pressed, TRUE,
                         a_toggled_focused_pressed_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.desk.toggled.pressed.bg",
                         theme->btn_desk->a_toggled_unfocused_pressed, TRUE,
                         a_toggled_unfocused_pressed_tmp);

    /* toggled hover */
    READ_APPEARANCE_COPY("window.active.button.desk.toggled.hover.bg",
                         theme->btn_desk->a_toggled_hover_focused, TRUE,
                         a_toggled_hover_focused_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.desk.toggled.hover.bg",
                         theme->btn_desk->a_toggled_hover_unfocused, TRUE,
                         a_toggled_hover_unfocused_tmp);

    /* shade button */
    read_button_colors(db, inst, theme, theme->btn_shade, "shade");

    /* bases:  unpressed, pressed, disabled */
    READ_APPEARANCE_COPY("window.active.button.shade.unpressed.bg",
                         theme->btn_shade->a_focused_unpressed, TRUE,
                         a_focused_unpressed_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.shade.unpressed.bg",
                         theme->btn_shade->a_unfocused_unpressed, TRUE,
                         a_unfocused_unpressed_tmp);
    READ_APPEARANCE_COPY("window.active.button.shade.pressed.bg",
                         theme->btn_shade->a_focused_pressed, TRUE,
                         a_focused_pressed_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.shade.pressed.bg",
                         theme->btn_shade->a_unfocused_pressed, TRUE,
                         a_unfocused_pressed_tmp);
    READ_APPEARANCE_COPY("window.active.button.shade.disabled.bg",
                         theme->btn_shade->a_disabled_focused, TRUE,
                         a_disabled_focused_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.shade.disabled.bg",
                         theme->btn_shade->a_disabled_unfocused, TRUE,
                         a_disabled_unfocused_tmp);

    /* hover */
    READ_APPEARANCE_COPY("window.active.button.shade.hover.bg",
                         theme->btn_shade->a_hover_focused, TRUE,
                         a_hover_focused_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.shade.hover.bg",
                         theme->btn_shade->a_hover_unfocused, TRUE,
                         a_hover_unfocused_tmp);

    /* toggled unpressed */
    READ_APPEARANCE_("window.active.button.shade.toggled.unpressed.bg",
                     "window.active.button.shade.toggled.bg",
                     theme->btn_shade->a_toggled_focused_unpressed, TRUE,
                     a_toggled_focused_unpressed_tmp);
    READ_APPEARANCE_("window.inactive.button.shade.toggled.unpressed.bg",
                     "window.inactive.button.shade.toggled.bg",
                     theme->btn_shade->a_toggled_unfocused_unpressed, TRUE,
                     a_toggled_unfocused_unpressed_tmp);

    /* toggled pressed */
    READ_APPEARANCE_COPY("window.active.button.shade.toggled.pressed.bg",
                         theme->btn_shade->a_toggled_focused_pressed, TRUE,
                         a_toggled_focused_pressed_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.shade.toggled.pressed.bg",
                         theme->btn_shade->a_toggled_unfocused_pressed, TRUE,
                         a_toggled_unfocused_pressed_tmp);

    /* toggled hover */
    READ_APPEARANCE_COPY("window.active.button.shade.toggled.hover.bg",
                         theme->btn_shade->a_toggled_hover_focused, TRUE,
                         a_toggled_hover_focused_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.shade.toggled.hover.bg",
                         theme->btn_shade->a_toggled_hover_unfocused, TRUE,
                         a_toggled_hover_unfocused_tmp);

    /* iconify button */
    read_button_colors(db, inst, theme, theme->btn_iconify, "iconify");

    READ_APPEARANCE_COPY("window.active.button.iconify.unpressed.bg",
                         theme->btn_iconify->a_focused_unpressed, TRUE,
                         a_focused_unpressed_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.iconify.unpressed.bg",
                         theme->btn_iconify->a_unfocused_unpressed, TRUE,
                         a_unfocused_unpressed_tmp);
    READ_APPEARANCE_COPY("window.active.button.iconify.pressed.bg",
                         theme->btn_iconify->a_focused_pressed, TRUE,
                         a_focused_pressed_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.iconify.pressed.bg",
                         theme->btn_iconify->a_unfocused_pressed, TRUE,
                         a_unfocused_pressed_tmp);
    READ_APPEARANCE_COPY("window.active.button.iconify.disabled.bg",
                         theme->btn_iconify->a_disabled_focused, TRUE,
                         a_disabled_focused_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.iconify.disabled.bg",
                         theme->btn_iconify->a_disabled_unfocused, TRUE,
                         a_disabled_unfocused_tmp);
    READ_APPEARANCE_COPY("window.active.button.iconify.hover.bg",
                         theme->btn_iconify->a_hover_focused, TRUE,
                         a_hover_focused_tmp);
    READ_APPEARANCE_COPY("window.inactive.button.iconify.hover.bg",
                         theme->btn_iconify->a_hover_unfocused, TRUE,
                         a_hover_unfocused_tmp);

    /* osd buttons */
    READ_APPEARANCE_COPY("osd.button.unpressed.bg", theme->osd_unpressed_button, TRUE, a_focused_unpressed_tmp);
    READ_APPEARANCE_COPY_TEXTURES("osd.button.pressed.bg", theme->osd_pressed_button, TRUE, a_focused_pressed_tmp, 5);
    READ_APPEARANCE_COPY_TEXTURES("osd.button.focused.bg", theme->osd_focused_button, TRUE, a_focused_unpressed_tmp, 5);

    theme->a_icon->surface.grad =
        theme->a_clear->surface.grad =
        theme->a_clear_tex->surface.grad =
        theme->a_menu_text_title->surface.grad =
        theme->a_menu_normal->surface.grad =
        theme->a_menu_disabled->surface.grad =
        theme->a_menu_text_normal->surface.grad =
        theme->a_menu_text_selected->surface.grad =
        theme->a_menu_text_disabled->surface.grad =
        theme->a_menu_text_disabled_selected->surface.grad =
        theme->a_menu_bullet_normal->surface.grad =
        theme->a_menu_bullet_selected->surface.grad = RR_SURFACE_PARENTREL;

    /* set up the textures */
    theme->a_focused_label->texture[0].type = RR_TEXTURE_TEXT;
    theme->a_focused_label->texture[0].data.text.justify = winjust;
    theme->a_focused_label->texture[0].data.text.font=theme->win_font_focused;
    theme->a_focused_label->texture[0].data.text.color =
        theme->title_focused_color;

    if (read_string(db, "window.active.label.text.font", &str)) {
        char *p;
        gint i = 0;
        gint j;
        if (strstr(str, "shadow=y")) {
            if ((p = strstr(str, "shadowoffset=")))
                i = parse_inline_number(p + strlen("shadowoffset="));
            else
                i = 1;
            theme->a_focused_label->texture[0].data.text.shadow_offset_x = i;
            theme->a_focused_label->texture[0].data.text.shadow_offset_y = i;
        }
        if ((p = strstr(str, "shadowtint=")))
        {
            i = parse_inline_number(p + strlen("shadowtint="));
            j = (i > 0 ? 0 : 255);
            i = ABS(i*255/100);

            theme->title_focused_shadow_color = RrColorNew(inst, j, j, j);
            theme->title_focused_shadow_alpha = i;
        } else {
            theme->title_focused_shadow_color = RrColorNew(inst, 0, 0, 0);
            theme->title_focused_shadow_alpha = 50;
        }
    }

    theme->a_focused_label->texture[0].data.text.shadow_color =
        theme->title_focused_shadow_color;
    theme->a_focused_label->texture[0].data.text.shadow_alpha =
        theme->title_focused_shadow_alpha;

    theme->osd_hilite_label->texture[0].type = RR_TEXTURE_TEXT;
    theme->osd_hilite_label->texture[0].data.text.justify = RR_JUSTIFY_LEFT;
    theme->osd_hilite_label->texture[0].data.text.font =
        theme->osd_font_hilite;
    theme->osd_hilite_label->texture[0].data.text.color =
        theme->osd_text_active_color;

    if (read_string(db, "osd.active.label.text.font", &str) ||
        read_string(db, "osd.label.text.font", &str))
    {
        char *p;
        gint i = 0;
        gint j;
        if (strstr(str, "shadow=y")) {
            if ((p = strstr(str, "shadowoffset=")))
                i = parse_inline_number(p + strlen("shadowoffset="));
            else
                i = 1;
            theme->osd_hilite_label->texture[0].data.text.shadow_offset_x = i;
            theme->osd_hilite_label->texture[0].data.text.shadow_offset_y = i;
        }
        if ((p = strstr(str, "shadowtint=")))
        {
            i = parse_inline_number(p + strlen("shadowtint="));
            j = (i > 0 ? 0 : 255);
            i = ABS(i*255/100);

            theme->osd_text_active_shadow_color = RrColorNew(inst, j, j, j);
            theme->osd_text_active_shadow_alpha = i;
        } else {
            theme->osd_text_active_shadow_color = RrColorNew(inst, 0, 0, 0);
            theme->osd_text_active_shadow_alpha = 50;
        }
    } else {
        /* inherit the font settings from the focused label */
        theme->osd_hilite_label->texture[0].data.text.shadow_offset_x =
            theme->a_focused_label->texture[0].data.text.shadow_offset_x;
        theme->osd_hilite_label->texture[0].data.text.shadow_offset_y =
            theme->a_focused_label->texture[0].data.text.shadow_offset_y;
        if (theme->title_focused_shadow_color)
            theme->osd_text_active_shadow_color =
                RrColorCopy(theme->title_focused_shadow_color);
        else
            theme->osd_text_active_shadow_color = RrColorNew(inst, 0, 0, 0);
        theme->osd_text_active_shadow_alpha =
            theme->title_focused_shadow_alpha;
    }

    theme->osd_hilite_label->texture[0].data.text.shadow_color =
        theme->osd_text_active_shadow_color;
    theme->osd_hilite_label->texture[0].data.text.shadow_alpha =
        theme->osd_text_active_shadow_alpha;

    theme->osd_unpressed_button->texture[0].type =
        theme->osd_pressed_button->texture[0].type =
        theme->osd_focused_button->texture[0].type =
        RR_TEXTURE_TEXT;

    theme->osd_unpressed_button->texture[0].data.text.justify =
        theme->osd_pressed_button->texture[0].data.text.justify =
        theme->osd_focused_button->texture[0].data.text.justify =
        RR_JUSTIFY_CENTER;

    theme->osd_unpressed_button->texture[0].data.text.font =
        theme->osd_pressed_button->texture[0].data.text.font =
        theme->osd_focused_button->texture[0].data.text.font =
        theme->osd_font_hilite;

    theme->osd_unpressed_button->texture[0].data.text.color =
        theme->osd_unpressed_color;
    theme->osd_pressed_button->texture[0].data.text.color =
        theme->osd_pressed_color;
    theme->osd_focused_button->texture[0].data.text.color =
        theme->osd_focused_color;

    theme->osd_pressed_button->texture[1].data.lineart.color =
        theme->osd_pressed_button->texture[2].data.lineart.color =
        theme->osd_pressed_button->texture[3].data.lineart.color =
        theme->osd_pressed_button->texture[4].data.lineart.color =
        theme->osd_pressed_lineart;

    theme->osd_focused_button->texture[1].data.lineart.color =
        theme->osd_focused_button->texture[2].data.lineart.color =
        theme->osd_focused_button->texture[3].data.lineart.color =
        theme->osd_focused_button->texture[4].data.lineart.color =
        theme->osd_focused_lineart;

    theme->a_unfocused_label->texture[0].type = RR_TEXTURE_TEXT;
    theme->a_unfocused_label->texture[0].data.text.justify = winjust;
    theme->a_unfocused_label->texture[0].data.text.font =
        theme->win_font_unfocused;
    theme->a_unfocused_label->texture[0].data.text.color =
        theme->title_unfocused_color;

    if (read_string(db, "window.inactive.label.text.font", &str)) {
        char *p;
        gint i = 0;
        gint j;
        if (strstr(str, "shadow=y")) {
            if ((p = strstr(str, "shadowoffset=")))
                i = parse_inline_number(p + strlen("shadowoffset="));
            else
                i = 1;
            theme->a_unfocused_label->texture[0].data.text.shadow_offset_x = i;
            theme->a_unfocused_label->texture[0].data.text.shadow_offset_y = i;
        }
        if ((p = strstr(str, "shadowtint=")))
        {
            i = parse_inline_number(p + strlen("shadowtint="));
            j = (i > 0 ? 0 : 255);
            i = ABS(i*255/100);

            theme->title_unfocused_shadow_color = RrColorNew(inst, j, j, j);
            theme->title_unfocused_shadow_alpha = i;
        } else {
            theme->title_unfocused_shadow_color = RrColorNew(inst, 0, 0, 0);
            theme->title_unfocused_shadow_alpha = 50;
        }
    }

    theme->a_unfocused_label->texture[0].data.text.shadow_color =
        theme->title_unfocused_shadow_color;
    theme->a_unfocused_label->texture[0].data.text.shadow_alpha =
        theme->title_unfocused_shadow_alpha;

    theme->osd_unhilite_label->texture[0].type = RR_TEXTURE_TEXT;
    theme->osd_unhilite_label->texture[0].data.text.justify = RR_JUSTIFY_LEFT;
    theme->osd_unhilite_label->texture[0].data.text.font =
        theme->osd_font_unhilite;
    theme->osd_unhilite_label->texture[0].data.text.color =
        theme->osd_text_inactive_color;

    if (read_string(db, "osd.inactive.label.text.font", &str))
    {
        char *p;
        gint i = 0;
        gint j;
        if (strstr(str, "shadow=y")) {
            if ((p = strstr(str, "shadowoffset=")))
                i = parse_inline_number(p + strlen("shadowoffset="));
            else
                i = 1;
            theme->osd_unhilite_label->texture[0].data.text.shadow_offset_x=i;
            theme->osd_unhilite_label->texture[0].data.text.shadow_offset_y=i;
        }
        if ((p = strstr(str, "shadowtint=")))
        {
            i = parse_inline_number(p + strlen("shadowtint="));
            j = (i > 0 ? 0 : 255);
            i = ABS(i*255/100);

            theme->osd_text_inactive_shadow_color = RrColorNew(inst, j, j, j);
            theme->osd_text_inactive_shadow_alpha = i;
        } else {
            theme->osd_text_inactive_shadow_color = RrColorNew(inst, 0, 0, 0);
            theme->osd_text_inactive_shadow_alpha = 50;
        }
    } else {
        /* inherit the font settings from the unfocused label */
        theme->osd_unhilite_label->texture[0].data.text.shadow_offset_x =
            theme->a_unfocused_label->texture[0].data.text.shadow_offset_x;
        theme->osd_unhilite_label->texture[0].data.text.shadow_offset_y =
            theme->a_unfocused_label->texture[0].data.text.shadow_offset_y;
        if (theme->title_unfocused_shadow_color)
            theme->osd_text_inactive_shadow_color =
                RrColorCopy(theme->title_unfocused_shadow_color);
        else
            theme->osd_text_inactive_shadow_color = RrColorNew(inst, 0, 0, 0);
        theme->osd_text_inactive_shadow_alpha =
            theme->title_unfocused_shadow_alpha;
    }

    theme->osd_unhilite_label->texture[0].data.text.shadow_color =
        theme->osd_text_inactive_shadow_color;
    theme->osd_unhilite_label->texture[0].data.text.shadow_alpha =
        theme->osd_text_inactive_shadow_alpha;

    theme->a_menu_text_title->texture[0].type = RR_TEXTURE_TEXT;
    theme->a_menu_text_title->texture[0].data.text.justify = mtitlejust;
    theme->a_menu_text_title->texture[0].data.text.font =
        theme->menu_title_font;
    theme->a_menu_text_title->texture[0].data.text.color =
        theme->menu_title_color;

    if (read_string(db, "menu.title.text.font", &str)) {
        char *p;
        gint i = 0;
        gint j;
        if (strstr(str, "shadow=y")) {
            if ((p = strstr(str, "shadowoffset=")))
                i = parse_inline_number(p + strlen("shadowoffset="));
            else
                i = 1;
            theme->a_menu_text_title->texture[0].data.text.shadow_offset_x = i;
            theme->a_menu_text_title->texture[0].data.text.shadow_offset_y = i;
        }
        if ((p = strstr(str, "shadowtint=")))
        {
            i = parse_inline_number(p + strlen("shadowtint="));
            j = (i > 0 ? 0 : 255);
            i = ABS(i*255/100);

            theme->menu_title_shadow_color = RrColorNew(inst, j, j, j);
            theme->menu_title_shadow_alpha = i;
        } else {
            theme->menu_title_shadow_color = RrColorNew(inst, 0, 0, 0);
            theme->menu_title_shadow_alpha = 50;
        }
    }

    theme->a_menu_text_title->texture[0].data.text.shadow_color =
        theme->menu_title_shadow_color;
    theme->a_menu_text_title->texture[0].data.text.shadow_alpha =
        theme->menu_title_shadow_alpha;

    theme->a_menu_text_normal->texture[0].type =
        theme->a_menu_text_selected->texture[0].type =
        theme->a_menu_text_disabled->texture[0].type =
        theme->a_menu_text_disabled_selected->texture[0].type =
        RR_TEXTURE_TEXT;
    theme->a_menu_text_normal->texture[0].data.text.justify =
        theme->a_menu_text_selected->texture[0].data.text.justify =
        theme->a_menu_text_disabled->texture[0].data.text.justify =
        theme->a_menu_text_disabled_selected->texture[0].data.text.justify =
        RR_JUSTIFY_LEFT;
    theme->a_menu_text_normal->texture[0].data.text.font =
        theme->a_menu_text_selected->texture[0].data.text.font =
        theme->a_menu_text_disabled->texture[0].data.text.font =
        theme->a_menu_text_disabled_selected->texture[0].data.text.font =
        theme->menu_font;
    theme->a_menu_text_normal->texture[0].data.text.color = theme->menu_color;
    theme->a_menu_text_selected->texture[0].data.text.color =
        theme->menu_selected_color;
    theme->a_menu_text_disabled->texture[0].data.text.color =
        theme->menu_disabled_color;
    theme->a_menu_text_disabled_selected->texture[0].data.text.color =
        theme->menu_disabled_selected_color;

    if (read_string(db, "menu.items.font", &str)) {
        char *p;
        gint i = 0;
        gint j;
        if (strstr(str, "shadow=y")) {
            if ((p = strstr(str, "shadowoffset=")))
                i = parse_inline_number(p + strlen("shadowoffset="));
            else
                i = 1;
            theme->a_menu_text_normal->
                texture[0].data.text.shadow_offset_x = i;
            theme->a_menu_text_normal->
                texture[0].data.text.shadow_offset_y = i;
            theme->a_menu_text_selected->
                texture[0].data.text.shadow_offset_x = i;
            theme->a_menu_text_selected->
                texture[0].data.text.shadow_offset_y = i;
            theme->a_menu_text_disabled->
                texture[0].data.text.shadow_offset_x = i;
            theme->a_menu_text_disabled->
                texture[0].data.text.shadow_offset_y = i;
            theme->a_menu_text_disabled_selected->
                texture[0].data.text.shadow_offset_x = i;
            theme->a_menu_text_disabled_selected->
                texture[0].data.text.shadow_offset_y = i;
        }
        if ((p = strstr(str, "shadowtint=")))
        {
            i = parse_inline_number(p + strlen("shadowtint="));
            j = (i > 0 ? 0 : 255);
            i = ABS(i*255/100);

            theme->menu_text_normal_shadow_color = RrColorNew(inst, j, j, j);
            theme->menu_text_selected_shadow_color = RrColorNew(inst, j, j, j);
            theme->menu_text_disabled_shadow_color = RrColorNew(inst, j, j, j);
            theme->menu_text_normal_shadow_alpha = i;
            theme->menu_text_selected_shadow_alpha = i;
            theme->menu_text_disabled_shadow_alpha = i;
            theme->menu_text_disabled_selected_shadow_alpha = i;
        } else {
            theme->menu_text_normal_shadow_color = RrColorNew(inst, 0, 0, 0);
            theme->menu_text_selected_shadow_color = RrColorNew(inst, 0, 0, 0);
            theme->menu_text_disabled_shadow_color = RrColorNew(inst, 0, 0, 0);
            theme->menu_text_normal_shadow_alpha = 50;
            theme->menu_text_selected_shadow_alpha = 50;
            theme->menu_text_disabled_selected_shadow_alpha = 50;
        }
    }

    theme->a_menu_text_normal->texture[0].data.text.shadow_color =
        theme->menu_text_normal_shadow_color;
    theme->a_menu_text_normal->texture[0].data.text.shadow_alpha =
        theme->menu_text_normal_shadow_alpha;
    theme->a_menu_text_selected->texture[0].data.text.shadow_color =
        theme->menu_text_selected_shadow_color;
    theme->a_menu_text_selected->texture[0].data.text.shadow_alpha =
        theme->menu_text_selected_shadow_alpha;
    theme->a_menu_text_disabled->texture[0].data.text.shadow_color =
        theme->menu_text_disabled_shadow_color;
    theme->a_menu_text_disabled->texture[0].data.text.shadow_alpha =
        theme->menu_text_disabled_shadow_alpha;
    theme->a_menu_text_disabled_selected->texture[0].data.text.shadow_color =
        theme->menu_text_disabled_shadow_color;
    theme->a_menu_text_disabled_selected->texture[0].data.text.shadow_alpha =
        theme->menu_text_disabled_shadow_alpha;

    theme->btn_max->a_disabled_focused->texture[0].type =
        theme->btn_max->a_disabled_unfocused->texture[0].type =
        theme->btn_max->a_hover_focused->texture[0].type =
        theme->btn_max->a_hover_unfocused->texture[0].type =
        theme->btn_max->a_toggled_hover_focused->texture[0].type =
        theme->btn_max->a_toggled_hover_unfocused->texture[0].type =
        theme->btn_max->a_toggled_focused_unpressed->texture[0].type =
        theme->btn_max->a_toggled_unfocused_unpressed->texture[0].type =
        theme->btn_max->a_toggled_focused_pressed->texture[0].type =
        theme->btn_max->a_toggled_unfocused_pressed->texture[0].type =
        theme->btn_max->a_focused_unpressed->texture[0].type =
        theme->btn_max->a_focused_pressed->texture[0].type =
        theme->btn_max->a_unfocused_unpressed->texture[0].type =
        theme->btn_max->a_unfocused_pressed->texture[0].type =
        theme->btn_close->a_disabled_focused->texture[0].type =
        theme->btn_close->a_disabled_unfocused->texture[0].type =
        theme->btn_close->a_hover_focused->texture[0].type =
        theme->btn_close->a_hover_unfocused->texture[0].type =
        theme->btn_close->a_focused_unpressed->texture[0].type =
        theme->btn_close->a_focused_pressed->texture[0].type =
        theme->btn_close->a_unfocused_unpressed->texture[0].type =
        theme->btn_close->a_unfocused_pressed->texture[0].type =
        theme->btn_desk->a_disabled_focused->texture[0].type =
        theme->btn_desk->a_disabled_unfocused->texture[0].type =
        theme->btn_desk->a_hover_focused->texture[0].type =
        theme->btn_desk->a_hover_unfocused->texture[0].type =
        theme->btn_desk->a_toggled_hover_focused->texture[0].type =
        theme->btn_desk->a_toggled_hover_unfocused->texture[0].type =
        theme->btn_desk->a_toggled_focused_unpressed->texture[0].type =
        theme->btn_desk->a_toggled_unfocused_unpressed->texture[0].type =
        theme->btn_desk->a_toggled_focused_pressed->texture[0].type =
        theme->btn_desk->a_toggled_unfocused_pressed->texture[0].type =
        theme->btn_desk->a_focused_unpressed->texture[0].type =
        theme->btn_desk->a_focused_pressed->texture[0].type =
        theme->btn_desk->a_unfocused_unpressed->texture[0].type =
        theme->btn_desk->a_unfocused_pressed->texture[0].type =
        theme->btn_shade->a_disabled_focused->texture[0].type =
        theme->btn_shade->a_disabled_unfocused->texture[0].type =
        theme->btn_shade->a_hover_focused->texture[0].type =
        theme->btn_shade->a_hover_unfocused->texture[0].type =
        theme->btn_shade->a_toggled_hover_focused->texture[0].type =
        theme->btn_shade->a_toggled_hover_unfocused->texture[0].type =
        theme->btn_shade->a_toggled_focused_unpressed->texture[0].type =
        theme->btn_shade->a_toggled_unfocused_unpressed->texture[0].type =
        theme->btn_shade->a_toggled_focused_pressed->texture[0].type =
        theme->btn_shade->a_toggled_unfocused_pressed->texture[0].type =
        theme->btn_shade->a_focused_unpressed->texture[0].type =
        theme->btn_shade->a_focused_pressed->texture[0].type =
        theme->btn_shade->a_unfocused_unpressed->texture[0].type =
        theme->btn_shade->a_unfocused_pressed->texture[0].type =
        theme->btn_iconify->a_disabled_focused->texture[0].type =
        theme->btn_iconify->a_disabled_unfocused->texture[0].type =
        theme->btn_iconify->a_hover_focused->texture[0].type =
        theme->btn_iconify->a_hover_unfocused->texture[0].type =
        theme->btn_iconify->a_focused_unpressed->texture[0].type =
        theme->btn_iconify->a_focused_pressed->texture[0].type =
        theme->btn_iconify->a_unfocused_unpressed->texture[0].type =
        theme->btn_iconify->a_unfocused_pressed->texture[0].type =
        theme->a_menu_bullet_normal->texture[0].type =
        theme->a_menu_bullet_selected->texture[0].type = RR_TEXTURE_MASK;

    theme->btn_max->a_disabled_focused->texture[0].data.mask.mask =
        theme->btn_max->a_disabled_unfocused->texture[0].data.mask.mask =
        theme->btn_max->disabled_mask;
    theme->btn_max->a_hover_focused->texture[0].data.mask.mask =
        theme->btn_max->a_hover_unfocused->texture[0].data.mask.mask =
        theme->btn_max->hover_mask;
    theme->btn_max->a_focused_pressed->texture[0].data.mask.mask =
        theme->btn_max->a_unfocused_pressed->texture[0].data.mask.mask =
        theme->btn_max->pressed_mask;
    theme->btn_max->a_focused_unpressed->texture[0].data.mask.mask =
        theme->btn_max->a_unfocused_unpressed->texture[0].data.mask.mask =
        theme->btn_max->mask;
    theme->btn_max->a_toggled_hover_focused->texture[0].data.mask.mask =
        theme->btn_max->a_toggled_hover_unfocused->texture[0].data.mask.mask =
        theme->btn_max->toggled_hover_mask;
    theme->btn_max->a_toggled_focused_unpressed->texture[0].data.mask.mask =
        theme->btn_max->a_toggled_unfocused_unpressed->
        texture[0].data.mask.mask = theme->btn_max->toggled_mask;
    theme->btn_max->a_toggled_focused_pressed->texture[0].data.mask.mask =
        theme->btn_max->a_toggled_unfocused_pressed->texture[0].data.mask.mask
        = theme->btn_max->toggled_pressed_mask;
    theme->btn_close->a_disabled_focused->texture[0].data.mask.mask =
        theme->btn_close->a_disabled_unfocused->texture[0].data.mask.mask =
        theme->btn_close->disabled_mask;
    theme->btn_close->a_hover_focused->texture[0].data.mask.mask =
        theme->btn_close->a_hover_unfocused->texture[0].data.mask.mask =
        theme->btn_close->hover_mask;
    theme->btn_close->a_focused_pressed->texture[0].data.mask.mask =
        theme->btn_close->a_unfocused_pressed->texture[0].data.mask.mask =
        theme->btn_close->pressed_mask;
    theme->btn_close->a_focused_unpressed->texture[0].data.mask.mask =
        theme->btn_close->a_unfocused_unpressed->texture[0].data.mask.mask =
        theme->btn_close->mask;
    theme->btn_desk->a_disabled_focused->texture[0].data.mask.mask =
        theme->btn_desk->a_disabled_unfocused->texture[0].data.mask.mask =
        theme->btn_desk->disabled_mask;
    theme->btn_desk->a_hover_focused->texture[0].data.mask.mask =
        theme->btn_desk->a_hover_unfocused->texture[0].data.mask.mask =
        theme->btn_desk->hover_mask;
    theme->btn_desk->a_focused_pressed->texture[0].data.mask.mask =
        theme->btn_desk->a_unfocused_pressed->texture[0].data.mask.mask =
        theme->btn_desk->pressed_mask;
    theme->btn_desk->a_focused_unpressed->texture[0].data.mask.mask =
        theme->btn_desk->a_unfocused_unpressed->texture[0].data.mask.mask =
        theme->btn_desk->mask;
    theme->btn_desk->a_toggled_hover_focused->texture[0].data.mask.mask =
        theme->btn_desk->a_toggled_hover_unfocused->texture[0].data.mask.mask =
        theme->btn_desk->toggled_hover_mask;
    theme->btn_desk->a_toggled_focused_unpressed->texture[0].data.mask.mask =
        theme->btn_desk->a_toggled_unfocused_unpressed->
        texture[0].data.mask.mask = theme->btn_desk->toggled_mask;
    theme->btn_desk->a_toggled_focused_pressed->texture[0].data.mask.mask =
        theme->btn_desk->a_toggled_unfocused_pressed->texture[0].data.mask.mask
        = theme->btn_desk->toggled_pressed_mask;
    theme->btn_shade->a_disabled_focused->texture[0].data.mask.mask =
        theme->btn_shade->a_disabled_unfocused->texture[0].data.mask.mask =
        theme->btn_shade->disabled_mask;
    theme->btn_shade->a_hover_focused->texture[0].data.mask.mask =
        theme->btn_shade->a_hover_unfocused->texture[0].data.mask.mask =
        theme->btn_shade->hover_mask;
    theme->btn_shade->a_focused_pressed->texture[0].data.mask.mask =
        theme->btn_shade->a_unfocused_pressed->texture[0].data.mask.mask =
        theme->btn_shade->pressed_mask;
    theme->btn_shade->a_focused_unpressed->texture[0].data.mask.mask =
        theme->btn_shade->a_unfocused_unpressed->texture[0].data.mask.mask =
        theme->btn_shade->mask;
    theme->btn_shade->a_toggled_hover_focused->texture[0].data.mask.mask =
        theme->btn_shade->a_toggled_hover_unfocused->texture[0].data.mask.mask
        = theme->btn_shade->toggled_hover_mask;
    theme->btn_shade->a_toggled_focused_unpressed->texture[0].data.mask.mask =
        theme->btn_shade->a_toggled_unfocused_unpressed->
        texture[0].data.mask.mask = theme->btn_shade->toggled_mask;
    theme->btn_shade->a_toggled_focused_pressed->texture[0].data.mask.mask =
        theme->btn_shade->a_toggled_unfocused_pressed->
        texture[0].data.mask.mask = theme->btn_shade->toggled_pressed_mask;
    theme->btn_iconify->a_disabled_focused->texture[0].data.mask.mask =
        theme->btn_iconify->a_disabled_unfocused->texture[0].data.mask.mask =
        theme->btn_iconify->disabled_mask;
    theme->btn_iconify->a_hover_focused->texture[0].data.mask.mask =
        theme->btn_iconify->a_hover_unfocused->texture[0].data.mask.mask =
        theme->btn_iconify->hover_mask;
    theme->btn_iconify->a_focused_pressed->texture[0].data.mask.mask =
        theme->btn_iconify->a_unfocused_pressed->texture[0].data.mask.mask =
        theme->btn_iconify->pressed_mask;
    theme->btn_iconify->a_focused_unpressed->texture[0].data.mask.mask =
        theme->btn_iconify->a_unfocused_unpressed->texture[0].data.mask.mask =
        theme->btn_iconify->mask;
    theme->a_menu_bullet_normal->texture[0].data.mask.mask =
    theme->a_menu_bullet_selected->texture[0].data.mask.mask =
        theme->menu_bullet_mask;
    theme->btn_max->a_disabled_focused->texture[0].data.mask.color = 
        theme->btn_max->disabled_focused_color;
    theme->btn_close->a_disabled_focused->texture[0].data.mask.color = 
        theme->btn_close->disabled_focused_color;
    theme->btn_desk->a_disabled_focused->texture[0].data.mask.color = 
        theme->btn_desk->disabled_focused_color;
    theme->btn_shade->a_disabled_focused->texture[0].data.mask.color = 
        theme->btn_shade->disabled_focused_color;
    theme->btn_iconify->a_disabled_focused->texture[0].data.mask.color = 
        theme->btn_iconify->disabled_focused_color;
    theme->btn_max->a_disabled_unfocused->texture[0].data.mask.color = 
        theme->btn_max->disabled_unfocused_color;
    theme->btn_close->a_disabled_unfocused->texture[0].data.mask.color = 
        theme->btn_close->disabled_unfocused_color;
    theme->btn_desk->a_disabled_unfocused->texture[0].data.mask.color = 
        theme->btn_desk->disabled_unfocused_color;
    theme->btn_shade->a_disabled_unfocused->texture[0].data.mask.color = 
        theme->btn_shade->disabled_unfocused_color;
    theme->btn_iconify->a_disabled_unfocused->texture[0].data.mask.color = 
        theme->btn_iconify->disabled_unfocused_color;
    theme->btn_max->a_hover_focused->texture[0].data.mask.color = 
        theme->btn_max->hover_focused_color;
    theme->btn_close->a_hover_focused->texture[0].data.mask.color = 
        theme->btn_close->hover_focused_color;
    theme->btn_desk->a_hover_focused->texture[0].data.mask.color = 
        theme->btn_desk->hover_focused_color;
    theme->btn_shade->a_hover_focused->texture[0].data.mask.color = 
        theme->btn_shade->hover_focused_color;
    theme->btn_iconify->a_hover_focused->texture[0].data.mask.color = 
        theme->btn_iconify->hover_focused_color;
    theme->btn_max->a_hover_unfocused->texture[0].data.mask.color = 
        theme->btn_max->hover_unfocused_color;
    theme->btn_close->a_hover_unfocused->texture[0].data.mask.color = 
        theme->btn_close->hover_unfocused_color;
    theme->btn_desk->a_hover_unfocused->texture[0].data.mask.color = 
        theme->btn_desk->hover_unfocused_color;
    theme->btn_shade->a_hover_unfocused->texture[0].data.mask.color = 
        theme->btn_shade->hover_unfocused_color;
    theme->btn_iconify->a_hover_unfocused->texture[0].data.mask.color = 
        theme->btn_iconify->hover_unfocused_color;
    theme->btn_max->a_toggled_hover_focused->texture[0].data.mask.color = 
        theme->btn_max->toggled_hover_focused_color;
    theme->btn_desk->a_toggled_hover_focused->texture[0].data.mask.color = 
        theme->btn_desk->toggled_hover_focused_color;
    theme->btn_shade->a_toggled_hover_focused->texture[0].data.mask.color = 
        theme->btn_shade->toggled_hover_focused_color;
    theme->btn_max->a_toggled_hover_unfocused->texture[0].data.mask.color = 
        theme->btn_max->toggled_hover_unfocused_color;
    theme->btn_desk->a_toggled_hover_unfocused->texture[0].data.mask.color = 
        theme->btn_desk->toggled_hover_unfocused_color;
    theme->btn_shade->a_toggled_hover_unfocused->texture[0].data.mask.color = 
        theme->btn_shade->toggled_hover_unfocused_color;
    theme->btn_max->a_toggled_focused_unpressed->texture[0].data.mask.color = 
        theme->btn_max->toggled_focused_unpressed_color;
    theme->btn_desk->a_toggled_focused_unpressed->texture[0].data.mask.color = 
        theme->btn_desk->toggled_focused_unpressed_color;
    theme->btn_shade->a_toggled_focused_unpressed->texture[0].data.mask.color = 
        theme->btn_shade->toggled_focused_unpressed_color;
    theme->btn_max->a_toggled_unfocused_unpressed->texture[0].data.mask.color = 
        theme->btn_max->toggled_unfocused_unpressed_color;
    theme->btn_desk->a_toggled_unfocused_unpressed->texture[0].data.mask.color
        = theme->btn_desk->toggled_unfocused_unpressed_color;
    theme->btn_shade->a_toggled_unfocused_unpressed->texture[0].data.mask.color
        = theme->btn_shade->toggled_unfocused_unpressed_color;
    theme->btn_max->a_toggled_focused_pressed->texture[0].data.mask.color = 
        theme->btn_max->toggled_focused_pressed_color;
    theme->btn_desk->a_toggled_focused_pressed->texture[0].data.mask.color = 
        theme->btn_desk->toggled_focused_pressed_color;
    theme->btn_shade->a_toggled_focused_pressed->texture[0].data.mask.color = 
        theme->btn_shade->toggled_focused_pressed_color;
    theme->btn_max->a_toggled_unfocused_pressed->texture[0].data.mask.color = 
        theme->btn_max->toggled_unfocused_pressed_color;
    theme->btn_desk->a_toggled_unfocused_pressed->texture[0].data.mask.color = 
        theme->btn_desk->toggled_unfocused_pressed_color;
    theme->btn_shade->a_toggled_unfocused_pressed->texture[0].data.mask.color = 
        theme->btn_shade->toggled_unfocused_pressed_color;
    theme->btn_max->a_focused_unpressed->texture[0].data.mask.color = 
        theme->btn_max->focused_unpressed_color;
    theme->btn_close->a_focused_unpressed->texture[0].data.mask.color = 
        theme->btn_close->focused_unpressed_color;
    theme->btn_desk->a_focused_unpressed->texture[0].data.mask.color = 
        theme->btn_desk->focused_unpressed_color;
    theme->btn_shade->a_focused_unpressed->texture[0].data.mask.color = 
        theme->btn_shade->focused_unpressed_color;
    theme->btn_iconify->a_focused_unpressed->texture[0].data.mask.color = 
        theme->btn_iconify->focused_unpressed_color;
    theme->btn_max->a_focused_pressed->texture[0].data.mask.color = 
        theme->btn_max->focused_pressed_color;
    theme->btn_close->a_focused_pressed->texture[0].data.mask.color = 
        theme->btn_close->focused_pressed_color;
    theme->btn_desk->a_focused_pressed->texture[0].data.mask.color = 
        theme->btn_desk->focused_pressed_color;
    theme->btn_shade->a_focused_pressed->texture[0].data.mask.color = 
        theme->btn_shade->focused_pressed_color;
    theme->btn_iconify->a_focused_pressed->texture[0].data.mask.color = 
        theme->btn_iconify->focused_pressed_color;
    theme->btn_max->a_unfocused_unpressed->texture[0].data.mask.color = 
        theme->btn_max->unfocused_unpressed_color;
    theme->btn_close->a_unfocused_unpressed->texture[0].data.mask.color = 
        theme->btn_close->unfocused_unpressed_color;
    theme->btn_desk->a_unfocused_unpressed->texture[0].data.mask.color = 
        theme->btn_desk->unfocused_unpressed_color;
    theme->btn_shade->a_unfocused_unpressed->texture[0].data.mask.color = 
        theme->btn_shade->unfocused_unpressed_color;
    theme->btn_iconify->a_unfocused_unpressed->texture[0].data.mask.color = 
        theme->btn_iconify->unfocused_unpressed_color;
    theme->btn_max->a_unfocused_pressed->texture[0].data.mask.color = 
        theme->btn_max->unfocused_pressed_color;
    theme->btn_close->a_unfocused_pressed->texture[0].data.mask.color = 
        theme->btn_close->unfocused_pressed_color;
    theme->btn_desk->a_unfocused_pressed->texture[0].data.mask.color = 
        theme->btn_desk->unfocused_pressed_color;
    theme->btn_shade->a_unfocused_pressed->texture[0].data.mask.color = 
        theme->btn_shade->unfocused_pressed_color;
    theme->btn_iconify->a_unfocused_pressed->texture[0].data.mask.color = 
        theme->btn_iconify->unfocused_pressed_color;
    theme->a_menu_bullet_normal->texture[0].data.mask.color =
        theme->menu_bullet_color;
    theme->a_menu_bullet_selected->texture[0].data.mask.color =
        theme->menu_bullet_selected_color;

    g_free(path);
    XrmDestroyDatabase(db);

    /* set the font heights */
    theme->win_font_height = RrFontHeight
        (theme->win_font_focused,
         theme->a_focused_label->texture[0].data.text.shadow_offset_y);
    theme->win_font_height =
        MAX(theme->win_font_height,
            RrFontHeight
            (theme->win_font_focused,
             theme->a_unfocused_label->texture[0].data.text.shadow_offset_y));
    theme->menu_title_font_height = RrFontHeight
        (theme->menu_title_font,
         theme->a_menu_text_title->texture[0].data.text.shadow_offset_y);
    theme->menu_font_height = RrFontHeight
        (theme->menu_font,
         theme->a_menu_text_normal->texture[0].data.text.shadow_offset_y);

    /* calculate some last extents */
    {
        gint ft, fb, fl, fr, ut, ub, ul, ur;

        RrMargins(theme->a_focused_label, &fl, &ft, &fr, &fb);
        RrMargins(theme->a_unfocused_label, &ul, &ut, &ur, &ub);
        theme->label_height = theme->win_font_height + MAX(ft + fb, ut + ub);
        theme->label_height += theme->label_height % 2;

        /* this would be nice I think, since padding.width can now be 0,
           but it breaks frame.c horribly and I don't feel like fixing that
           right now, so if anyone complains, here is how to keep text from
           going over the title's bevel/border with a padding.width of 0 and a
           bevelless/borderless label
           RrMargins(theme->a_focused_title, &fl, &ft, &fr, &fb);
           RrMargins(theme->a_unfocused_title, &ul, &ut, &ur, &ub);
           theme->title_height = theme->label_height +
           MAX(MAX(theme->padding * 2, ft + fb),
           MAX(theme->padding * 2, ut + ub));
        */
        theme->title_height = theme->label_height + theme->paddingy * 2;

        RrMargins(theme->a_menu_title, &ul, &ut, &ur, &ub);
        theme->menu_title_label_height = theme->menu_title_font_height+ut+ub;
        theme->menu_title_height = theme->menu_title_label_height +
            theme->paddingy * 2;
    }
    theme->button_size = theme->label_height - 2;
    theme->grip_width = 25;

    RrAppearanceFree(a_disabled_focused_tmp);
    RrAppearanceFree(a_disabled_unfocused_tmp);
    RrAppearanceFree(a_hover_focused_tmp);
    RrAppearanceFree(a_hover_unfocused_tmp);
    RrAppearanceFree(a_focused_unpressed_tmp);
    RrAppearanceFree(a_focused_pressed_tmp);
    RrAppearanceFree(a_unfocused_unpressed_tmp);
    RrAppearanceFree(a_unfocused_pressed_tmp);
    RrAppearanceFree(a_toggled_hover_focused_tmp);
    RrAppearanceFree(a_toggled_hover_unfocused_tmp);
    RrAppearanceFree(a_toggled_focused_unpressed_tmp);
    RrAppearanceFree(a_toggled_focused_pressed_tmp);
    RrAppearanceFree(a_toggled_unfocused_unpressed_tmp);
    RrAppearanceFree(a_toggled_unfocused_pressed_tmp);

    return theme;
}

void RrThemeFree(RrTheme *theme)
{
    if (theme) {
        g_free(theme->name);

        RrButtonFree(theme->btn_max);
        RrButtonFree(theme->btn_close);
        RrButtonFree(theme->btn_desk);
        RrButtonFree(theme->btn_shade);
        RrButtonFree(theme->btn_iconify);

        RrColorFree(theme->menu_border_color);
        RrColorFree(theme->osd_border_color);
        RrColorFree(theme->frame_focused_border_color);
        RrColorFree(theme->frame_undecorated_focused_border_color);
        RrColorFree(theme->frame_unfocused_border_color);
        RrColorFree(theme->frame_undecorated_unfocused_border_color);
        RrColorFree(theme->title_separator_focused_color);
        RrColorFree(theme->title_separator_unfocused_color);
        RrColorFree(theme->cb_unfocused_color);
        RrColorFree(theme->cb_focused_color);
        RrColorFree(theme->title_focused_color);
        RrColorFree(theme->title_unfocused_color);
        RrColorFree(theme->titlebut_disabled_focused_color);
        RrColorFree(theme->titlebut_disabled_unfocused_color);
        RrColorFree(theme->titlebut_hover_focused_color);
        RrColorFree(theme->titlebut_hover_unfocused_color);
        RrColorFree(theme->titlebut_toggled_hover_focused_color);
        RrColorFree(theme->titlebut_toggled_hover_unfocused_color);
        RrColorFree(theme->titlebut_toggled_focused_pressed_color);
        RrColorFree(theme->titlebut_toggled_unfocused_pressed_color);
        RrColorFree(theme->titlebut_toggled_focused_unpressed_color);
        RrColorFree(theme->titlebut_toggled_unfocused_unpressed_color);
        RrColorFree(theme->titlebut_focused_pressed_color);
        RrColorFree(theme->titlebut_unfocused_pressed_color);
        RrColorFree(theme->titlebut_focused_unpressed_color);
        RrColorFree(theme->titlebut_unfocused_unpressed_color);
        RrColorFree(theme->menu_title_color);
        RrColorFree(theme->menu_sep_color);
        RrColorFree(theme->menu_color);
        RrColorFree(theme->menu_bullet_color);
        RrColorFree(theme->menu_bullet_selected_color);
        RrColorFree(theme->menu_selected_color);
        RrColorFree(theme->menu_disabled_color);
        RrColorFree(theme->menu_disabled_selected_color);
        RrColorFree(theme->title_focused_shadow_color);
        RrColorFree(theme->title_unfocused_shadow_color);
        RrColorFree(theme->osd_text_active_color);
        RrColorFree(theme->osd_text_inactive_color);
        RrColorFree(theme->osd_text_active_shadow_color);
        RrColorFree(theme->osd_text_inactive_shadow_color);
        RrColorFree(theme->osd_pressed_color);
        RrColorFree(theme->osd_unpressed_color);
        RrColorFree(theme->osd_focused_color);
        RrColorFree(theme->osd_pressed_lineart);
        RrColorFree(theme->osd_focused_lineart);
        RrColorFree(theme->menu_title_shadow_color);
        RrColorFree(theme->menu_text_normal_shadow_color);
        RrColorFree(theme->menu_text_selected_shadow_color);
        RrColorFree(theme->menu_text_disabled_shadow_color);
        RrColorFree(theme->menu_text_disabled_selected_shadow_color);

        g_free(theme->def_win_icon);
        
        RrPixmapMaskFree(theme->menu_bullet_mask);
        RrPixmapMaskFree(theme->down_arrow_mask);
        RrPixmapMaskFree(theme->up_arrow_mask);

        RrFontClose(theme->win_font_focused);
        RrFontClose(theme->win_font_unfocused);
        RrFontClose(theme->menu_title_font);
        RrFontClose(theme->menu_font);
        RrFontClose(theme->osd_font_hilite);
        RrFontClose(theme->osd_font_unhilite);

        RrAppearanceFree(theme->a_focused_grip);
        RrAppearanceFree(theme->a_unfocused_grip);
        RrAppearanceFree(theme->a_focused_title);
        RrAppearanceFree(theme->a_unfocused_title);
        RrAppearanceFree(theme->a_focused_label);
        RrAppearanceFree(theme->a_unfocused_label);
        RrAppearanceFree(theme->a_icon);
        RrAppearanceFree(theme->a_focused_handle);
        RrAppearanceFree(theme->a_unfocused_handle);
        RrAppearanceFree(theme->a_menu);
        RrAppearanceFree(theme->a_menu_title);
        RrAppearanceFree(theme->a_menu_text_title);
        RrAppearanceFree(theme->a_menu_normal);
        RrAppearanceFree(theme->a_menu_selected);
        RrAppearanceFree(theme->a_menu_disabled);
        RrAppearanceFree(theme->a_menu_disabled_selected);
        RrAppearanceFree(theme->a_menu_text_normal);
        RrAppearanceFree(theme->a_menu_text_selected);
        RrAppearanceFree(theme->a_menu_text_disabled);
        RrAppearanceFree(theme->a_menu_text_disabled_selected);
        RrAppearanceFree(theme->a_menu_bullet_normal);
        RrAppearanceFree(theme->a_menu_bullet_selected);
        RrAppearanceFree(theme->a_clear);
        RrAppearanceFree(theme->a_clear_tex);
        RrAppearanceFree(theme->osd_bg);
        RrAppearanceFree(theme->osd_hilite_bg);
        RrAppearanceFree(theme->osd_hilite_label);
        RrAppearanceFree(theme->osd_unhilite_bg);
        RrAppearanceFree(theme->osd_unhilite_label);
        RrAppearanceFree(theme->osd_pressed_button);
        RrAppearanceFree(theme->osd_unpressed_button);
        RrAppearanceFree(theme->osd_focused_button);

        g_slice_free(RrTheme, theme);
    }
}

static XrmDatabase loaddb(const gchar *name, gchar **path)
{
    GSList *it;
    XrmDatabase db = NULL;
    gchar *s;

    if (name[0] == '/') {
        s = g_build_filename(name, "openbox-3", "themerc", NULL);
        if ((db = XrmGetFileDatabase(s)))
            *path = g_path_get_dirname(s);
        g_free(s);
    } else {
        ObtPaths *p;

        p = obt_paths_new();

        /* XXX backwards compatibility, remove me sometime later */
        s = g_build_filename(g_get_home_dir(), ".themes", name,
                             "openbox-3", "themerc", NULL);
        if ((db = XrmGetFileDatabase(s)))
            *path = g_path_get_dirname(s);
        g_free(s);

        for (it = obt_paths_data_dirs(p); !db && it; it = g_slist_next(it))
        {
            s = g_build_filename(it->data, "themes", name,
                                 "openbox-3", "themerc", NULL);
            if ((db = XrmGetFileDatabase(s)))
                *path = g_path_get_dirname(s);
            g_free(s);
        }

        obt_paths_unref(p);
    }

    if (db == NULL) {
        s = g_build_filename(name, "themerc", NULL);
        if ((db = XrmGetFileDatabase(s)))
            *path = g_path_get_dirname(s);
        g_free(s);
    }

    return db;
}

static gchar *create_class_name(const gchar *rname)
{
    gchar *rclass = g_strdup(rname);
    gchar *p = rclass;

    while (TRUE) {
        *p = toupper(*p);
        p = strchr(p+1, '.');
        if (p == NULL) break;
        ++p;
        if (*p == '\0') break;
    }
    return rclass;
}

static gboolean read_int(XrmDatabase db, const gchar *rname, gint *value)
{
    gboolean ret = FALSE;
    gchar *rclass = create_class_name(rname);
    gchar *rettype, *end;
    XrmValue retvalue;

    if (XrmGetResource(db, rname, rclass, &rettype, &retvalue) &&
        retvalue.addr != NULL) {
        *value = (gint)strtol(retvalue.addr, &end, 10);
        if (end != retvalue.addr)
            ret = TRUE;
    }

    g_free(rclass);
    return ret;
}

static gboolean read_string(XrmDatabase db, const gchar *rname, gchar **value)
{
    gboolean ret = FALSE;
    gchar *rclass = create_class_name(rname);
    gchar *rettype;
    XrmValue retvalue;

    if (XrmGetResource(db, rname, rclass, &rettype, &retvalue) &&
        retvalue.addr != NULL) {
        g_strstrip(retvalue.addr);
        *value = retvalue.addr;
        ret = TRUE;
    }

    g_free(rclass);
    return ret;
}

static gboolean read_color(XrmDatabase db, const RrInstance *inst,
                           const gchar *rname, RrColor **value)
{
    gboolean ret = FALSE;
    gchar *rclass = create_class_name(rname);
    gchar *rettype;
    XrmValue retvalue;

    if (XrmGetResource(db, rname, rclass, &rettype, &retvalue) &&
        retvalue.addr != NULL) {
        RrColor *c;

        /* retvalue.addr is inside the xrdb database so we can't destroy it
           but we can edit it in place, as g_strstrip does. */
        g_strstrip(retvalue.addr);
        c = RrColorParse(inst, retvalue.addr);
        if (c != NULL) {
            *value = c;
            ret = TRUE;
        }
    }

    g_free(rclass);
    return ret;
}

static gboolean read_mask(const RrInstance *inst, const gchar *path,
                          RrTheme *theme, const gchar *maskname,
                          RrPixmapMask **value)
{
    gboolean ret = FALSE;
    gchar *s;
    gint hx, hy; /* ignored */
    guint w, h;
    guchar *b;

    s = g_build_filename(path, maskname, NULL);
    if (XReadBitmapFileData(s, &w, &h, &b, &hx, &hy) == BitmapSuccess) {
        ret = TRUE;
        *value = RrPixmapMaskNew(inst, w, h, (gchar*)b);
        XFree(b);
    }
    g_free(s);

    return ret;
}

static void parse_appearance(gchar *tex, RrSurfaceColorType *grad,
                             RrReliefType *relief, RrBevelType *bevel,
                             gboolean *interlaced, gboolean *border,
                             gboolean allow_trans)
{
    gchar *t;

    /* convert to all lowercase */
    for (t = tex; *t != '\0'; ++t)
        *t = g_ascii_tolower(*t);

    if (allow_trans && strstr(tex, "parentrelative") != NULL) {
        *grad = RR_SURFACE_PARENTREL;
    } else {
        if (strstr(tex, "gradient") != NULL) {
            if (strstr(tex, "crossdiagonal") != NULL)
                *grad = RR_SURFACE_CROSS_DIAGONAL;
            else if (strstr(tex, "pyramid") != NULL)
                *grad = RR_SURFACE_PYRAMID;
            else if (strstr(tex, "mirrorhorizontal") != NULL)
                *grad = RR_SURFACE_MIRROR_HORIZONTAL;
            else if (strstr(tex, "horizontal") != NULL)
                *grad = RR_SURFACE_HORIZONTAL;
            else if (strstr(tex, "splitvertical") != NULL)
                *grad = RR_SURFACE_SPLIT_VERTICAL;
            else if (strstr(tex, "vertical") != NULL)
                *grad = RR_SURFACE_VERTICAL;
            else
                *grad = RR_SURFACE_DIAGONAL;
        } else {
            *grad = RR_SURFACE_SOLID;
        }
    }

    if (strstr(tex, "sunken") != NULL)
        *relief = RR_RELIEF_SUNKEN;
    else if (strstr(tex, "flat") != NULL)
        *relief = RR_RELIEF_FLAT;
    else if (strstr(tex, "raised") != NULL)
        *relief = RR_RELIEF_RAISED;
    else
        *relief = (*grad == RR_SURFACE_PARENTREL) ?
                  RR_RELIEF_FLAT : RR_RELIEF_RAISED;

    *border = FALSE;
    if (*relief == RR_RELIEF_FLAT) {
        if (strstr(tex, "border") != NULL)
            *border = TRUE;
    } else {
        if (strstr(tex, "bevel2") != NULL)
            *bevel = RR_BEVEL_2;
        else
            *bevel = RR_BEVEL_1;
    }

    if (strstr(tex, "interlaced") != NULL)
        *interlaced = TRUE;
    else
        *interlaced = FALSE;
}

static gboolean read_appearance(XrmDatabase db, const RrInstance *inst,
                                const gchar *rname, RrAppearance *value,
                                gboolean allow_trans)
{
    gboolean ret = FALSE;
    gchar *rclass = create_class_name(rname);
    gchar *cname, *ctoname, *bcname, *icname, *hname, *sname;
    gchar *csplitname, *ctosplitname;
    gchar *rettype;
    XrmValue retvalue;
    gint i;

    cname = g_strconcat(rname, ".color", NULL);
    ctoname = g_strconcat(rname, ".colorTo", NULL);
    bcname = g_strconcat(rname, ".border.color", NULL);
    icname = g_strconcat(rname, ".interlace.color", NULL);
    hname = g_strconcat(rname, ".highlight", NULL);
    sname = g_strconcat(rname, ".shadow", NULL);
    csplitname = g_strconcat(rname, ".color.splitTo", NULL);
    ctosplitname = g_strconcat(rname, ".colorTo.splitTo", NULL);

    if (XrmGetResource(db, rname, rclass, &rettype, &retvalue) &&
        retvalue.addr != NULL) {
        parse_appearance(retvalue.addr,
                         &value->surface.grad,
                         &value->surface.relief,
                         &value->surface.bevel,
                         &value->surface.interlaced,
                         &value->surface.border,
                         allow_trans);
        if (!read_color(db, inst, cname, &value->surface.primary))
            value->surface.primary = RrColorNew(inst, 0, 0, 0);
        if (!read_color(db, inst, ctoname, &value->surface.secondary))
            value->surface.secondary = RrColorNew(inst, 0, 0, 0);
        if (value->surface.border)
            if (!read_color(db, inst, bcname,
                            &value->surface.border_color))
                value->surface.border_color = RrColorNew(inst, 0, 0, 0);
        if (value->surface.interlaced)
            if (!read_color(db, inst, icname,
                            &value->surface.interlace_color))
                value->surface.interlace_color = RrColorNew(inst, 0, 0, 0);
        if (read_int(db, hname, &i) && i >= 0)
            value->surface.bevel_light_adjust = i;
        if (read_int(db, sname, &i) && i >= 0 && i <= 256)
            value->surface.bevel_dark_adjust = i;

        if (value->surface.grad == RR_SURFACE_SPLIT_VERTICAL) {
            gint r, g, b;

            if (!read_color(db, inst, csplitname,
                            &value->surface.split_primary))
            {
                r = value->surface.primary->r;
                r += r >> 2;
                g = value->surface.primary->g;
                g += g >> 2;
                b = value->surface.primary->b;
                b += b >> 2;
                if (r > 0xFF) r = 0xFF;
                if (g > 0xFF) g = 0xFF;
                if (b > 0xFF) b = 0xFF;
                value->surface.split_primary = RrColorNew(inst, r, g, b);
            }

            if (!read_color(db, inst, ctosplitname,
                            &value->surface.split_secondary))
            {
                r = value->surface.secondary->r;
                r += r >> 4;
                g = value->surface.secondary->g;
                g += g >> 4;
                b = value->surface.secondary->b;
                b += b >> 4;
                if (r > 0xFF) r = 0xFF;
                if (g > 0xFF) g = 0xFF;
                if (b > 0xFF) b = 0xFF;
                value->surface.split_secondary = RrColorNew(inst, r, g, b);
            }
        }

        ret = TRUE;
    }

    g_free(ctosplitname);
    g_free(csplitname);
    g_free(sname);
    g_free(hname);
    g_free(icname);
    g_free(bcname);
    g_free(ctoname);
    g_free(cname);
    g_free(rclass);
    return ret;
}

static int parse_inline_number(const char *p)
{
    int neg = 1;
    int res = 0;
    if (*p == '-') {
        neg = -1;
        ++p;
    }
    for (; isdigit(*p); ++p)
        res = res * 10 + *p - '0';
    res *= neg;
    return res;
}

static void set_default_appearance(RrAppearance *a)
{
    a->surface.grad = RR_SURFACE_SOLID;
    a->surface.relief = RR_RELIEF_FLAT;
    a->surface.bevel = RR_BEVEL_1;
    a->surface.interlaced = FALSE;
    a->surface.border = FALSE;
    a->surface.primary = RrColorNew(a->inst, 0, 0, 0);
    a->surface.secondary = RrColorNew(a->inst, 0, 0, 0);
}

/* Reads the output from gimp's C-Source file format into valid RGBA data for
   an RrTextureRGBA. */
static RrPixel32* read_c_image(gint width, gint height, const guint8 *data)
{
    RrPixel32 *im, *p;
    gint i;

    p = im = g_memdup(data, width * height * sizeof(RrPixel32));

    for (i = 0; i < width * height; ++i) {
        guchar a = ((*p >> 24) & 0xff);
        guchar b = ((*p >> 16) & 0xff);
        guchar g = ((*p >>  8) & 0xff);
        guchar r = ((*p >>  0) & 0xff);

        *p = ((r << RrDefaultRedOffset) +
              (g << RrDefaultGreenOffset) +
              (b << RrDefaultBlueOffset) +
              (a << RrDefaultAlphaOffset));
        p++;
    }

    return im;
}

static void read_button_colors(XrmDatabase db, const RrInstance *inst, 
                               const RrTheme *theme, RrButton *btn, 
                               const gchar *btnname)
{
    gchar *name;

    /* active unpressed */
    name = g_strdup_printf("window.active.button.%s.unpressed.image.color",
                           btnname);
    READ_COLOR(name, btn->focused_unpressed_color,
               RrColorCopy(theme->titlebut_focused_unpressed_color));
    g_free(name);

    /* inactive unpressed */
    name = g_strdup_printf("window.inactive.button.%s.unpressed.image.color",
                           btnname);
    READ_COLOR(name, btn->unfocused_unpressed_color,
               RrColorCopy(theme->titlebut_unfocused_unpressed_color));
    g_free(name);

    /* active pressed */
    name = g_strdup_printf("window.active.button.%s.pressed.image.color",
                           btnname);
    READ_COLOR(name, btn->focused_pressed_color,
               RrColorCopy(theme->titlebut_focused_pressed_color));
    g_free(name);

    /* inactive pressed */
    name = g_strdup_printf("window.inactive.button.%s.pressed.image.color",
                          btnname);
    READ_COLOR(name, btn->unfocused_pressed_color,
               RrColorCopy(theme->titlebut_unfocused_pressed_color));
    g_free(name);

    /* active disabled */
    name = g_strdup_printf("window.active.button.%s.disabled.image.color",
                           btnname);
    READ_COLOR(name, btn->disabled_focused_color,
               RrColorCopy(theme->titlebut_disabled_focused_color));
    g_free(name);

    /* inactive disabled */
    name = g_strdup_printf("window.inactive.button.%s.disabled.image.color",
                           btnname);
    READ_COLOR(name, btn->disabled_unfocused_color,
               RrColorCopy(theme->titlebut_disabled_unfocused_color));
    g_free(name);

    /* active hover */
    name = g_strdup_printf("window.active.button.%s.hover.image.color",
                           btnname);
    READ_COLOR(name, btn->hover_focused_color, 
               RrColorCopy(theme->titlebut_hover_focused_color));
    g_free(name);

    /* inactive hover */
    name = g_strdup_printf("window.inactive.button.%s.hover.image.color",
                           btnname);
    READ_COLOR(name, btn->hover_unfocused_color,
               RrColorCopy(theme->titlebut_hover_unfocused_color));
    g_free(name);

    /* active toggled unpressed */
    name = g_strdup_printf("window.active.button.%s.toggled."
                          "unpressed.image.color", btnname);
    READ_COLOR(name, btn->toggled_focused_unpressed_color,
               RrColorCopy(theme->titlebut_toggled_focused_unpressed_color));
    g_free(name);

    /* inactive toggled unpressed */
    name = g_strdup_printf("window.inactive.button.%s.toggled."
                           "unpressed.image.color", btnname);
    READ_COLOR(name, btn->toggled_unfocused_unpressed_color,
               RrColorCopy(theme->titlebut_toggled_unfocused_unpressed_color));
    g_free(name);

    /* active toggled hover */
    name = g_strdup_printf("window.active.button.%s.toggled.hover.image.color",
                           btnname);
    READ_COLOR(name, btn->toggled_hover_focused_color,
               RrColorCopy(theme->titlebut_toggled_hover_focused_color));

    g_free(name);

    /* inactive toggled hover */
    name = g_strdup_printf("window.inactive.button.%s.toggled.hover."
                           "image.color", btnname);
    READ_COLOR(name, btn->toggled_hover_unfocused_color,
               RrColorCopy(theme->titlebut_toggled_hover_unfocused_color));
    g_free(name);

    /* active toggled pressed */
    name = g_strdup_printf("window.active.button.%s.toggled.pressed."
                           "image.color", btnname);
    READ_COLOR(name, btn->toggled_focused_pressed_color, 
               RrColorCopy(theme->titlebut_toggled_focused_pressed_color));
    g_free(name);
   
    /* inactive toggled pressed */
    name = g_strdup_printf("window.inactive.button.%s.toggled.pressed."
                           "image.color", btnname);
    READ_COLOR(name, btn->toggled_unfocused_pressed_color,
               RrColorCopy(theme->titlebut_toggled_unfocused_pressed_color));
    g_free(name);
}


