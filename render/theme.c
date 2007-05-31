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
#include "parser/parse.h"

#include <X11/Xlib.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    xmlDocPtr doc;
    const RrInstance *inst;
    gchar *path;
} ParseState;

static void parse_style(gchar *tex, RrSurfaceColorType *grad,
                        RrReliefType *relief, RrBevelType *bevel,
                        gboolean *interlaced, gboolean *border,
                        gboolean allow_trans);
static gboolean read_mask(ParseState *ps, const gchar *maskname,
                          RrPixmapMask **value);
static RrPixel32* read_c_image(gint width, gint height, const guint8 *data);
static void set_default_appearance(RrAppearance *a);
static xmlNodePtr find_node(xmlNodePtr n, const gchar *names[]);
static gboolean find_int(ParseState *ps, xmlNodePtr n, const gchar *names[],
                         gint *integer, gint lower, gint upper);
static gboolean find_string(ParseState *ps, xmlNodePtr n, const gchar *names[],
                            gchar **string);
static gboolean find_color(ParseState *ps, xmlNodePtr n, const gchar *names[],
                           RrColor **color, gchar *alpha);
    static gboolean find_point(ParseState *ps, xmlNodePtr n, const gchar *names[],
                           gint *x, gint *y,
                           gint lowx, gint lowy, gint upx, gint upy);
static gboolean find_shadow(ParseState *ps, xmlNodePtr n, const gchar *names[],
                            RrAppearance *a);
static gboolean find_appearance(ParseState *ps, xmlNodePtr n, const gchar *names[],
                                RrAppearance *a, gboolean allow_trans);

/* make a null terminated array out of a list of strings */
#define L(args...) (const gchar*[]){args,NULL}
/* shortcut to the various find_* functions */
#define FIND(type, args...) find_##type(&ps, root, args)

RrTheme* RrThemeNew(const RrInstance *inst, const gchar *name,
                    gboolean allow_fallback,
                    RrFont *active_window_font, RrFont *inactive_window_font,
                    RrFont *menu_title_font, RrFont *menu_item_font,
                    RrFont *osd_font)
{
    ParseState ps;
    xmlNodePtr root;
    RrJustify winjust, mtitlejust;
    gchar *str;
    RrTheme *theme;
    gboolean userdef;

    if (name) {
        if (!parse_load_theme(name, &ps.doc, &root, &ps.path)) {
            g_message("Unable to load the theme '%s'", name);
            g_message("Falling back to the default theme '%s'",
                      DEFAULT_THEME);
            /* make it fall back to default theme */
            name = NULL;
        }
    }
    if (name == NULL && allow_fallback) {
        if (!parse_load_theme(DEFAULT_THEME, &ps.doc, &root, &ps.path)) {
            g_message("Unable to load the theme '%s'", DEFAULT_THEME);
            return NULL;
        }
    }
    if (name == NULL)
        return NULL;

    ps.inst = inst;

    theme = g_new0(RrTheme, 1);
    theme->inst = inst;
    theme->name = g_strdup(name ? name : DEFAULT_THEME);

    theme->a_disabled_focused_max = RrAppearanceNew(inst, 1);
    theme->a_disabled_unfocused_max = RrAppearanceNew(inst, 1);
    theme->a_hover_focused_max = RrAppearanceNew(inst, 1);
    theme->a_hover_unfocused_max = RrAppearanceNew(inst, 1);
    theme->a_toggled_focused_pressed_max = RrAppearanceNew(inst, 1);
    theme->a_toggled_unfocused_pressed_max = RrAppearanceNew(inst, 1);
    theme->a_toggled_focused_unpressed_max = RrAppearanceNew(inst, 1);
    theme->a_toggled_unfocused_unpressed_max = RrAppearanceNew(inst, 1);
    theme->a_toggled_hover_focused_max = RrAppearanceNew(inst, 1);
    theme->a_toggled_hover_unfocused_max = RrAppearanceNew(inst, 1);
    theme->a_focused_unpressed_max = RrAppearanceNew(inst, 1);
    theme->a_focused_pressed_max = RrAppearanceNew(inst, 1);
    theme->a_unfocused_unpressed_max = RrAppearanceNew(inst, 1);
    theme->a_unfocused_pressed_max = RrAppearanceNew(inst, 1);
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
    theme->a_menu_disabled = RrAppearanceNew(inst, 0);
    theme->a_menu_disabled_selected = RrAppearanceNew(inst, 0);
    theme->a_menu_selected = RrAppearanceNew(inst, 0);
    theme->a_menu_text_normal = RrAppearanceNew(inst, 1);
    theme->a_menu_text_selected = RrAppearanceNew(inst, 1);
    theme->a_menu_text_disabled = RrAppearanceNew(inst, 1);
    theme->a_menu_text_disabled_selected = RrAppearanceNew(inst, 1);
    theme->a_menu_bullet_normal = RrAppearanceNew(inst, 1);
    theme->a_menu_bullet_selected = RrAppearanceNew(inst, 1);
    theme->a_clear = RrAppearanceNew(inst, 0);
    theme->a_clear_tex = RrAppearanceNew(inst, 1);

    /* load the font stuff */

    if (active_window_font) {
        theme->win_font_focused = active_window_font;
        RrFontRef(active_window_font);
    } else
        theme->win_font_focused = RrFontOpenDefault(inst);

    if (inactive_window_font) {
        theme->win_font_unfocused = inactive_window_font;
        RrFontRef(inactive_window_font);
    } else
        theme->win_font_unfocused = RrFontOpenDefault(inst);

    winjust = RR_JUSTIFY_LEFT;
    if (FIND(string, L( "window", "justify"), &str)) {
        if (strcmp(str, "right") == 0)
            winjust = RR_JUSTIFY_RIGHT;
        else if (strcmp(str, "center") == 0)
            winjust = RR_JUSTIFY_CENTER;
        g_free(str);
    }

    if (menu_title_font) {
        theme->menu_title_font = menu_title_font;
        RrFontRef(menu_title_font);
    } else
        theme->menu_title_font = RrFontOpenDefault(inst);

    mtitlejust = RR_JUSTIFY_LEFT;
    if (FIND(string, L("menu", "justify"), &str)) {
        if (strcmp(str, "right") == 0)
            mtitlejust = RR_JUSTIFY_RIGHT;
        else if (strcmp(str, "center") == 0)
            mtitlejust = RR_JUSTIFY_CENTER;
        g_free(str);
    }

    if (menu_item_font) {
        theme->menu_font = menu_item_font;
        RrFontRef(menu_item_font);
    } else
        theme->menu_font = RrFontOpenDefault(inst);

    if (osd_font) {
        theme->osd_font = osd_font;
        RrFontRef(osd_font);
    } else
        theme->osd_font = RrFontOpenDefault(inst);

    /* load direct dimensions */
    if (!FIND(int, L("menu","overlap"),
              &theme->menu_overlap, -100, 100))
        theme->menu_overlap = 0;

    if (!FIND(int, L("dimensions","handle"), &theme->handle_height, 0, 100))
        theme->handle_height = 6;

    if (!FIND(point, L("dimensions","padding"),
              &theme->paddingx, &theme->paddingy, 0, 100, 0, 100))
        theme->paddingx = theme->paddingy = 3;

    if (!FIND(int, L("dimensions","window","border"),
              &theme->fbwidth, 0, 100))
        theme->fbwidth = 1;

    /* menu border width inherits from frame border width */
    if (!FIND(int, L("dimensions","menu","border"),
              &theme->mbwidth, 0, 100))
        theme->mbwidth = theme->fbwidth;

    if (!FIND(point, L("dimensions","window","clientpadding"),
              &theme->cbwidthx, &theme->cbwidthy, 0, 100, 0, 100))
        theme->cbwidthx = theme->cbwidthy = 1;

    /* load colors */
    if (!FIND(color, L("window","active","border"),
              &theme->frame_focused_border_color, NULL))
        theme->frame_focused_border_color = RrColorNew(inst, 0, 0, 0);
    /* frame unfocused border color inherits from frame focused border color */
    if (!FIND(color, L("window","inactive","border"),
              &theme->frame_unfocused_border_color, NULL))
        theme->frame_unfocused_border_color =
            RrColorNew(inst,
                       theme->frame_focused_border_color->r,
                       theme->frame_focused_border_color->g,
                       theme->frame_focused_border_color->b);

    /* menu border color inherits from frame focused border color */
    if (!FIND(color, L("menu","border"),
              &theme->menu_border_color, NULL))
        theme->menu_border_color =
            RrColorNew(inst,
                       theme->frame_focused_border_color->r,
                       theme->frame_focused_border_color->g,
                       theme->frame_focused_border_color->b);
    if (!FIND(color, L("window","active","clientpadding"),
              &theme->cb_focused_color, NULL))
        theme->cb_focused_color = RrColorNew(inst, 255, 255, 255);
    if (!FIND(color, L("window","inactive","clientpadding"),
              &theme->cb_unfocused_color, NULL))
        theme->cb_unfocused_color = RrColorNew(inst, 255, 255, 255);
    if (!FIND(color, L("window","active","label","text","primary"),
              &theme->title_focused_color, NULL))
        theme->title_focused_color = RrColorNew(inst, 0x0, 0x0, 0x0);
    if (!FIND(color, L("osd","text","primary"),
              &theme->osd_color, NULL))
        theme->osd_color = RrColorNew(inst,
                                      theme->title_focused_color->r,
                                      theme->title_focused_color->g,
                                      theme->title_focused_color->b);
    if (!FIND(color, L("window","inactive","label","text","primary"),
              &theme->title_unfocused_color, NULL))
        theme->title_unfocused_color = RrColorNew(inst, 0xff, 0xff, 0xff);
    if (!FIND(color, L("window","active","buttons","unpressed","image"),
              &theme->titlebut_focused_unpressed_color, NULL))
        theme->titlebut_focused_unpressed_color = RrColorNew(inst, 0, 0, 0);
    if (!FIND(color, L("window","inactive","buttons", "unpressed","image"),
              &theme->titlebut_unfocused_unpressed_color, NULL))
        theme->titlebut_unfocused_unpressed_color =
            RrColorNew(inst, 0xff, 0xff, 0xff);
    if (!FIND(color, L("window","active","buttons","pressed","image"),
              &theme->titlebut_focused_pressed_color, NULL))
        theme->titlebut_focused_pressed_color =
            RrColorNew(inst,
                       theme->titlebut_focused_unpressed_color->r,
                       theme->titlebut_focused_unpressed_color->g,
                       theme->titlebut_focused_unpressed_color->b);
    if (!FIND(color, L("window","inactive","buttons","pressed","image"),
              &theme->titlebut_unfocused_pressed_color, NULL))
        theme->titlebut_unfocused_pressed_color =
            RrColorNew(inst,
                       theme->titlebut_unfocused_unpressed_color->r,
                       theme->titlebut_unfocused_unpressed_color->g,
                       theme->titlebut_unfocused_unpressed_color->b);
    if (!FIND(color, L("window","active","buttons","disabled","image"),
              &theme->titlebut_disabled_focused_color, NULL))
        theme->titlebut_disabled_focused_color =
            RrColorNew(inst, 0xff, 0xff, 0xff);
    if (!FIND(color, L("window","inactive","buttons","disabled","image"),
              &theme->titlebut_disabled_unfocused_color, NULL))
        theme->titlebut_disabled_unfocused_color = RrColorNew(inst, 0, 0, 0);
    if (!FIND(color,
              L("window","active","buttons","hover","image"),
              &theme->titlebut_hover_focused_color, NULL))
        theme->titlebut_hover_focused_color =
            RrColorNew(inst,
                       theme->titlebut_focused_unpressed_color->r,
                       theme->titlebut_focused_unpressed_color->g,
                       theme->titlebut_focused_unpressed_color->b);
    if (!FIND(color, L("window","inactive","buttons","hover","image"),
              &theme->titlebut_hover_unfocused_color, NULL))
        theme->titlebut_hover_unfocused_color =
            RrColorNew(inst,
                       theme->titlebut_unfocused_unpressed_color->r,
                       theme->titlebut_unfocused_unpressed_color->g,
                       theme->titlebut_unfocused_unpressed_color->b);
    if (!FIND(color,
              L("window","active","buttons","toggled-pressed","image"),
              &theme->titlebut_toggled_focused_pressed_color, NULL))
        theme->titlebut_toggled_focused_pressed_color =
            RrColorNew(inst,
                       theme->titlebut_focused_pressed_color->r,
                       theme->titlebut_focused_pressed_color->g,
                       theme->titlebut_focused_pressed_color->b);
    if (!FIND(color,
              L("window","inactive","buttons","toggled-pressed","image"),
              &theme->titlebut_toggled_unfocused_pressed_color, NULL))
        theme->titlebut_toggled_unfocused_pressed_color =
            RrColorNew(inst,
                       theme->titlebut_unfocused_pressed_color->r,
                       theme->titlebut_unfocused_pressed_color->g,
                       theme->titlebut_unfocused_pressed_color->b);
    if (!FIND(color,
              L("window","active","buttons","toggled-unpressed","image"),
              &theme->titlebut_toggled_focused_unpressed_color, NULL))
        theme->titlebut_toggled_focused_unpressed_color =
            RrColorNew(inst,
                       theme->titlebut_focused_pressed_color->r,
                       theme->titlebut_focused_pressed_color->g,
                       theme->titlebut_focused_pressed_color->b);
    if (!FIND(color,
              L("window","inactive","buttons","toggled-unpressed","image"),
              &theme->titlebut_toggled_unfocused_unpressed_color, NULL))
        theme->titlebut_toggled_unfocused_unpressed_color =
            RrColorNew(inst,
                       theme->titlebut_unfocused_pressed_color->r,
                       theme->titlebut_unfocused_pressed_color->g,
                       theme->titlebut_unfocused_pressed_color->b);
    if (!FIND(color,
              L("window","active","buttons","toggled-hover","image"),
              &theme->titlebut_toggled_hover_focused_color, NULL))
        theme->titlebut_toggled_hover_focused_color =
            RrColorNew(inst,
                       theme->titlebut_toggled_focused_unpressed_color->r,
                       theme->titlebut_toggled_focused_unpressed_color->g,
                       theme->titlebut_toggled_focused_unpressed_color->b);
    if (!FIND(color,
              L("window","inactive","buttons","toggled-hover","image"),
              &theme->titlebut_toggled_hover_unfocused_color, NULL))
        theme->titlebut_toggled_hover_unfocused_color =
            RrColorNew(inst,
                       theme->titlebut_toggled_unfocused_unpressed_color->r,
                       theme->titlebut_toggled_unfocused_unpressed_color->g,
                       theme->titlebut_toggled_unfocused_unpressed_color->b);
    if (!FIND(color, L("menu","title","text","primary"),
              &theme->menu_title_color, NULL))
        theme->menu_title_color = RrColorNew(inst, 0, 0, 0);
    if (!FIND(color, L("menu","inactive","primary"), &theme->menu_color, NULL))
        theme->menu_color = RrColorNew(inst, 0xff, 0xff, 0xff);
    if (!FIND(color, L("menu","disabled","primary"),
              &theme->menu_disabled_color, NULL))
        theme->menu_disabled_color = RrColorNew(inst, 0, 0, 0);
    if (!FIND(color, L("menu","activedisabled","text","primary"),
              &theme->menu_disabled_selected_color, NULL))
        theme->menu_disabled_selected_color =
            RrColorNew(inst,
                       theme->menu_disabled_color->r,
                       theme->menu_disabled_color->g,
                       theme->menu_disabled_color->b);
    if (!FIND(color, L("menu","active","text","primary"),
              &theme->menu_selected_color, NULL))
        theme->menu_selected_color = RrColorNew(inst, 0, 0, 0);
    if (!FIND(color, L("window","active","label","text","shadow","primary"),
              &theme->title_focused_shadow_color,
              &theme->title_focused_shadow_alpha))
    {
        theme->title_focused_shadow_color = RrColorNew(inst, 0, 0, 0);
        theme->title_focused_shadow_alpha = 50;
    }
    if (!FIND(color, L("osd","text","shadow","primary"),
              &theme->osd_shadow_color, &theme->osd_shadow_alpha))
    {
        theme->osd_shadow_color = 
            RrColorNew(inst, theme->title_focused_shadow_color->r,
                       theme->title_focused_shadow_color->g,
                       theme->title_focused_shadow_color->b);
        theme->osd_shadow_alpha = theme->title_focused_shadow_alpha;
    }
    if (!FIND(color, L("window","inactive","label","text","shadow","primary"),
              &theme->title_unfocused_shadow_color,
              &theme->title_unfocused_shadow_alpha))
    {
        theme->title_unfocused_shadow_color = RrColorNew(inst, 0, 0, 0);
        theme->title_unfocused_shadow_alpha = 50;
    }
    if (!FIND(color, L("menu","title","text","shadow","primary"),
              &theme->menu_title_shadow_color,
              &theme->menu_title_shadow_alpha))
    {
        theme->menu_title_shadow_color = RrColorNew(inst, 0, 0, 0);
        theme->menu_title_shadow_alpha = 50;
    }
    if (!FIND(color, L("menu","inactive","shadow","primary"),
              &theme->menu_text_normal_shadow_color,
              &theme->menu_text_normal_shadow_alpha))
    {
        theme->menu_text_normal_shadow_color = RrColorNew(inst, 0, 0, 0);
        theme->menu_text_normal_shadow_alpha = 50;
    }
    if (!FIND(color, L("menu","active","text","shadow","primary"),
              &theme->menu_text_selected_shadow_color,
              &theme->menu_text_selected_shadow_alpha))
    {
        theme->menu_text_selected_shadow_color = RrColorNew(inst, 0, 0, 0);
        theme->menu_text_selected_shadow_alpha = 50;
    }
    if (!FIND(color, L("menu","disabled","shadow","primary"),
              &theme->menu_text_disabled_shadow_color,
              &theme->menu_text_disabled_shadow_alpha))
    {
        theme->menu_text_disabled_shadow_color =
            RrColorNew(inst, theme->menu_text_normal_shadow_color->r,
                       theme->menu_text_normal_shadow_color->g,
                       theme->menu_text_normal_shadow_color->b);
        theme->menu_text_disabled_shadow_alpha = 
            theme->menu_text_normal_shadow_alpha;
    }
    if (!FIND(color, L("menu","activedisabled","shadow","primary"),
              &theme->menu_text_disabled_selected_shadow_color,
              &theme->menu_text_disabled_selected_shadow_alpha))
    {
        theme->menu_text_disabled_selected_shadow_color =
            RrColorNew(inst, theme->menu_text_disabled_shadow_color->r,
                       theme->menu_text_disabled_shadow_color->g,
                       theme->menu_text_disabled_shadow_color->b);
        theme->menu_text_disabled_selected_shadow_alpha = 
            theme->menu_text_disabled_shadow_alpha;
    }
    
    /* load the image masks */

    /* maximize button masks */
    userdef = TRUE;
    if (!read_mask(&ps, "max.xbm", &theme->max_mask)) {
            guchar data[] = { 0x3f, 0x3f, 0x21, 0x21, 0x21, 0x3f };
            theme->max_mask = RrPixmapMaskNew(inst, 6, 6, (gchar*)data);
            userdef = FALSE;
    }
    if (!read_mask(&ps, "max_toggled.xbm",  &theme->max_toggled_mask)) {
        if (userdef)
            theme->max_toggled_mask = RrPixmapMaskCopy(theme->max_mask);
        else {
            guchar data[] = { 0x3e, 0x22, 0x2f, 0x29, 0x39, 0x0f };
            theme->max_toggled_mask = RrPixmapMaskNew(inst, 6, 6,(gchar*)data);
        }
    }
    if (!read_mask(&ps, "max_pressed.xbm", &theme->max_pressed_mask))
        theme->max_pressed_mask = RrPixmapMaskCopy(theme->max_mask);
    if (!read_mask(&ps, "max_disabled.xbm", &theme->max_disabled_mask))
        theme->max_disabled_mask = RrPixmapMaskCopy(theme->max_mask);
    if (!read_mask(&ps, "max_hover.xbm", &theme->max_hover_mask))
        theme->max_hover_mask = RrPixmapMaskCopy(theme->max_mask);
    if (!read_mask(&ps, "max_toggled_pressed.xbm",
                   &theme->max_toggled_pressed_mask))
        theme->max_toggled_pressed_mask =
            RrPixmapMaskCopy(theme->max_toggled_mask);
    if (!read_mask(&ps, "max_toggled_hover.xbm",
                   &theme->max_toggled_hover_mask))
        theme->max_toggled_hover_mask =
            RrPixmapMaskCopy(theme->max_toggled_mask);

    /* iconify button masks */
    if (!read_mask(&ps, "iconify.xbm", &theme->iconify_mask)) {
        guchar data[] = { 0x00, 0x00, 0x00, 0x00, 0x3f, 0x3f };
        theme->iconify_mask = RrPixmapMaskNew(inst, 6, 6, (gchar*)data);
    }
    if (!read_mask(&ps, "iconify_pressed.xbm", &theme->iconify_pressed_mask))
        theme->iconify_pressed_mask = RrPixmapMaskCopy(theme->iconify_mask);
    if (!read_mask(&ps, "iconify_disabled.xbm", &theme->iconify_disabled_mask))
        theme->iconify_disabled_mask = RrPixmapMaskCopy(theme->iconify_mask);
    if (!read_mask(&ps, "iconify_hover.xbm", &theme->iconify_hover_mask))
        theme->iconify_hover_mask = RrPixmapMaskCopy(theme->iconify_mask);

    /* all desktops button masks */
    userdef = TRUE;
    if (!read_mask(&ps, "desk.xbm", &theme->desk_mask)) {
        guchar data[] = { 0x33, 0x33, 0x00, 0x00, 0x33, 0x33 };
        theme->desk_mask = RrPixmapMaskNew(inst, 6, 6, (gchar*)data);
        userdef = FALSE;
    }
    if (!read_mask(&ps, "desk_toggled.xbm", &theme->desk_toggled_mask)) {
        if (userdef)
            theme->desk_toggled_mask = RrPixmapMaskCopy(theme->desk_mask);
        else {
            guchar data[] = { 0x00, 0x1e, 0x1a, 0x16, 0x1e, 0x00 };
            theme->desk_toggled_mask =
                RrPixmapMaskNew(inst, 6, 6, (gchar*)data);
        }
    }
    if (!read_mask(&ps, "desk_pressed.xbm", &theme->desk_pressed_mask))
        theme->desk_pressed_mask = RrPixmapMaskCopy(theme->desk_mask);
    if (!read_mask(&ps, "desk_disabled.xbm", &theme->desk_disabled_mask))
        theme->desk_disabled_mask = RrPixmapMaskCopy(theme->desk_mask);
    if (!read_mask(&ps, "desk_hover.xbm", &theme->desk_hover_mask))
        theme->desk_hover_mask = RrPixmapMaskCopy(theme->desk_mask);
    if (!read_mask(&ps, "desk_toggled_pressed.xbm",
                   &theme->desk_toggled_pressed_mask))
        theme->desk_toggled_pressed_mask =
            RrPixmapMaskCopy(theme->desk_toggled_mask);
    if (!read_mask(&ps, "desk_toggled_hover.xbm",
                   &theme->desk_toggled_hover_mask))
        theme->desk_toggled_hover_mask =
            RrPixmapMaskCopy(theme->desk_toggled_mask);

    /* shade button masks */
    if (!read_mask(&ps, "shade.xbm", &theme->shade_mask)) {
        guchar data[] = { 0x3f, 0x3f, 0x00, 0x00, 0x00, 0x00 };
        theme->shade_mask = RrPixmapMaskNew(inst, 6, 6, (gchar*)data);
    }
    if (!read_mask(&ps, "shade_toggled.xbm", &theme->shade_toggled_mask))
        theme->shade_toggled_mask = RrPixmapMaskCopy(theme->shade_mask);
    if (!read_mask(&ps, "shade_pressed.xbm", &theme->shade_pressed_mask))
        theme->shade_pressed_mask = RrPixmapMaskCopy(theme->shade_mask);
    if (!read_mask(&ps, "shade_disabled.xbm", &theme->shade_disabled_mask))
        theme->shade_disabled_mask = RrPixmapMaskCopy(theme->shade_mask);
    if (!read_mask(&ps, "shade_hover.xbm", &theme->shade_hover_mask))
        theme->shade_hover_mask = RrPixmapMaskCopy(theme->shade_mask);
    if (!read_mask(&ps, "shade_toggled_pressed.xbm",
                   &theme->shade_toggled_pressed_mask))
        theme->shade_toggled_pressed_mask =
            RrPixmapMaskCopy(theme->shade_toggled_mask);
    if (!read_mask(&ps, "shade_toggled_hover.xbm",
                   &theme->shade_toggled_hover_mask))
        theme->shade_toggled_hover_mask =
            RrPixmapMaskCopy(theme->shade_toggled_mask);

    /* close button masks */
    if (!read_mask(&ps, "close.xbm", &theme->close_mask)) {
        guchar data[] = { 0x33, 0x3f, 0x1e, 0x1e, 0x3f, 0x33 };
        theme->close_mask = RrPixmapMaskNew(inst, 6, 6, (gchar*)data);
    }
    if (!read_mask(&ps, "close_pressed.xbm", &theme->close_pressed_mask))
        theme->close_pressed_mask = RrPixmapMaskCopy(theme->close_mask);
    if (!read_mask(&ps, "close_disabled.xbm", &theme->close_disabled_mask))
        theme->close_disabled_mask = RrPixmapMaskCopy(theme->close_mask);
    if (!read_mask(&ps, "close_hover.xbm", &theme->close_hover_mask))
        theme->close_hover_mask = RrPixmapMaskCopy(theme->close_mask);

    /* submenu bullet mask */
    if (!read_mask(&ps, "bullet.xbm", &theme->menu_bullet_mask)) {
        guchar data[] = { 0x01, 0x03, 0x07, 0x0f, 0x07, 0x03, 0x01 };
        theme->menu_bullet_mask = RrPixmapMaskNew(inst, 4, 7, (gchar*)data);
    }

    /* setup the default window icon */
    theme->def_win_icon = read_c_image(OB_DEFAULT_ICON_WIDTH,
                                       OB_DEFAULT_ICON_HEIGHT,
                                       OB_DEFAULT_ICON_pixel_data);

    /* read the decoration textures */
    if (!FIND(appearance, L("window","active","titlebar"),
              theme->a_focused_title, FALSE))
        set_default_appearance(theme->a_focused_title);
    if (!FIND(appearance, L("window","inactive","titlebar"),
                         theme->a_unfocused_title, FALSE))
        set_default_appearance(theme->a_unfocused_title);
    if (!FIND(appearance, L("window","active","label"),
                         theme->a_focused_label, TRUE))
        set_default_appearance(theme->a_focused_label);
    if (!FIND(appearance, L("window","inactive","label"),
                         theme->a_unfocused_label, TRUE))
        set_default_appearance(theme->a_unfocused_label);
    if (!FIND(appearance, L("window","active","handle"),
                         theme->a_focused_handle, FALSE))
        set_default_appearance(theme->a_focused_handle);
    if (!FIND(appearance, L("window","inactive","handle"),
                         theme->a_unfocused_handle, FALSE))
        set_default_appearance(theme->a_unfocused_handle);
    if (!FIND(appearance, L("window","active","grip"),
                         theme->a_focused_grip, TRUE))
        set_default_appearance(theme->a_focused_grip);
    if (!FIND(appearance, L("window","inactive","grip"),
                         theme->a_unfocused_grip, TRUE))
        set_default_appearance(theme->a_unfocused_grip);
    if (!FIND(appearance, L("menu","entries"), theme->a_menu, FALSE))
        set_default_appearance(theme->a_menu);
    if (!FIND(appearance, L("menu","title"), theme->a_menu_title, TRUE))
        set_default_appearance(theme->a_menu_title);
    if (!FIND(appearance, L("menu", "active"), theme->a_menu_selected, TRUE))
        set_default_appearance(theme->a_menu_selected);
    if (!FIND(appearance, L("menu", "activedisabled"),
              theme->a_menu_disabled_selected, TRUE))
        theme->a_menu_disabled_selected =
            RrAppearanceCopy(theme->a_menu_selected);

    /* read the appearances for rendering non-decorations */
    theme->osd_hilite_bg = RrAppearanceCopy(theme->a_focused_title);
    theme->osd_hilite_label = RrAppearanceCopy(theme->a_focused_label);
    if (theme->a_focused_label->surface.grad != RR_SURFACE_PARENTREL)
        theme->osd_hilite_fg = RrAppearanceCopy(theme->a_focused_label);
    else
        theme->osd_hilite_fg = RrAppearanceCopy(theme->a_focused_title);
    if (theme->a_unfocused_label->surface.grad != RR_SURFACE_PARENTREL)
        theme->osd_unhilite_fg = RrAppearanceCopy(theme->a_unfocused_label);
    else
        theme->osd_unhilite_fg = RrAppearanceCopy(theme->a_unfocused_title);

    /* read buttons textures */
    if (!FIND(appearance, L("window","active","buttons","disabled"),
                         theme->a_disabled_focused_max, TRUE))
        set_default_appearance(theme->a_disabled_focused_max);
    if (!FIND(appearance, L("window","inactive","buttons","disabled"),
                         theme->a_disabled_unfocused_max, TRUE))
        set_default_appearance(theme->a_disabled_unfocused_max);
    if (!FIND(appearance, L("window","active","buttons","pressed"),
              theme->a_focused_pressed_max, TRUE))
        set_default_appearance(theme->a_focused_pressed_max);
    if (!FIND(appearance, L("window","inactive","buttons","pressed"),
                         theme->a_unfocused_pressed_max, TRUE))
        set_default_appearance(theme->a_unfocused_pressed_max);
    if (!FIND(appearance, L("window","active","buttons","unpressed"),
                         theme->a_focused_unpressed_max, TRUE))
        set_default_appearance(theme->a_focused_unpressed_max);
    if (!FIND(appearance, L("window","inactive","buttons","unpressed"),
                         theme->a_unfocused_unpressed_max, TRUE))
        set_default_appearance(theme->a_unfocused_unpressed_max);
    if (!FIND(appearance, L("window","active","buttons","hover"),
                         theme->a_hover_focused_max, TRUE))
    {
        RrAppearanceFree(theme->a_hover_focused_max);
        theme->a_hover_focused_max =
            RrAppearanceCopy(theme->a_focused_unpressed_max);
    }
    if (!FIND(appearance, L("window","inactive","buttons","hover"),
                         theme->a_hover_unfocused_max, TRUE))
    {
        RrAppearanceFree(theme->a_hover_unfocused_max);
        theme->a_hover_unfocused_max =
            RrAppearanceCopy(theme->a_unfocused_unpressed_max);
    }
    if (!FIND(appearance, L("window","active","buttons","toggled-pressed"),
              theme->a_toggled_focused_pressed_max, TRUE))
    {
        RrAppearanceFree(theme->a_toggled_focused_pressed_max);
        theme->a_toggled_focused_pressed_max =
            RrAppearanceCopy(theme->a_focused_pressed_max);
    }
    if (!FIND(appearance, L("window","inactive","buttons","toggled-pressed"),
              theme->a_toggled_unfocused_pressed_max, TRUE))
    {
        RrAppearanceFree(theme->a_toggled_unfocused_pressed_max);
        theme->a_toggled_unfocused_pressed_max =
            RrAppearanceCopy(theme->a_unfocused_pressed_max);
    }
    if (!FIND(appearance, L("window","active","buttons","toggled-unpressed"),
              theme->a_toggled_focused_unpressed_max, TRUE))
    {
        RrAppearanceFree(theme->a_toggled_focused_unpressed_max);
        theme->a_toggled_focused_unpressed_max =
            RrAppearanceCopy(theme->a_focused_pressed_max);
    }
    if (!FIND(appearance, L("window","inactive","buttons","toggled-unpressed"),
              theme->a_toggled_unfocused_unpressed_max, TRUE))
    {
        RrAppearanceFree(theme->a_toggled_unfocused_unpressed_max);
        theme->a_toggled_unfocused_unpressed_max =
            RrAppearanceCopy(theme->a_unfocused_pressed_max);
    }
    if (!FIND(appearance, L("window","active","buttons","toggled-hover"),
              theme->a_toggled_hover_focused_max, TRUE))
    {
        RrAppearanceFree(theme->a_toggled_hover_focused_max);
        theme->a_toggled_hover_focused_max =
            RrAppearanceCopy(theme->a_toggled_focused_unpressed_max);
    }
    if (!FIND(appearance, L("window","inactive","buttons","toggled-hover"),
              theme->a_toggled_hover_unfocused_max, TRUE))
    {
        RrAppearanceFree(theme->a_toggled_hover_unfocused_max);
        theme->a_toggled_hover_unfocused_max =
            RrAppearanceCopy(theme->a_toggled_unfocused_unpressed_max);
    }

   theme->a_disabled_focused_close =
        RrAppearanceCopy(theme->a_disabled_focused_max);
    theme->a_disabled_unfocused_close =
        RrAppearanceCopy(theme->a_disabled_unfocused_max);
    theme->a_hover_focused_close =
        RrAppearanceCopy(theme->a_hover_focused_max);
    theme->a_hover_unfocused_close =
        RrAppearanceCopy(theme->a_hover_unfocused_max);
    theme->a_unfocused_unpressed_close =
        RrAppearanceCopy(theme->a_unfocused_unpressed_max);
    theme->a_unfocused_pressed_close =
        RrAppearanceCopy(theme->a_unfocused_pressed_max);
    theme->a_focused_unpressed_close =
        RrAppearanceCopy(theme->a_focused_unpressed_max);
    theme->a_focused_pressed_close =
        RrAppearanceCopy(theme->a_focused_pressed_max);
    theme->a_disabled_focused_desk =
        RrAppearanceCopy(theme->a_disabled_focused_max);
    theme->a_disabled_unfocused_desk =
        RrAppearanceCopy(theme->a_disabled_unfocused_max);
    theme->a_hover_focused_desk =
        RrAppearanceCopy(theme->a_hover_focused_max);
    theme->a_hover_unfocused_desk =
        RrAppearanceCopy(theme->a_hover_unfocused_max); 
    theme->a_toggled_focused_pressed_desk =
        RrAppearanceCopy(theme->a_toggled_focused_pressed_max);
    theme->a_toggled_unfocused_pressed_desk =
        RrAppearanceCopy(theme->a_toggled_unfocused_pressed_max);
    theme->a_toggled_focused_unpressed_desk =
        RrAppearanceCopy(theme->a_toggled_focused_unpressed_max);
    theme->a_toggled_unfocused_unpressed_desk =
        RrAppearanceCopy(theme->a_toggled_unfocused_unpressed_max);
    theme->a_toggled_hover_focused_desk =
        RrAppearanceCopy(theme->a_toggled_hover_focused_max);
    theme->a_toggled_hover_unfocused_desk =
        RrAppearanceCopy(theme->a_toggled_hover_unfocused_max);
    theme->a_unfocused_unpressed_desk =
        RrAppearanceCopy(theme->a_unfocused_unpressed_max);
    theme->a_unfocused_pressed_desk =
        RrAppearanceCopy(theme->a_unfocused_pressed_max);
    theme->a_focused_unpressed_desk =
        RrAppearanceCopy(theme->a_focused_unpressed_max);
    theme->a_focused_pressed_desk =
        RrAppearanceCopy(theme->a_focused_pressed_max);
    theme->a_disabled_focused_shade =
        RrAppearanceCopy(theme->a_disabled_focused_max);
    theme->a_disabled_unfocused_shade =
        RrAppearanceCopy(theme->a_disabled_unfocused_max);
    theme->a_hover_focused_shade =
        RrAppearanceCopy(theme->a_hover_focused_max);
    theme->a_hover_unfocused_shade =
        RrAppearanceCopy(theme->a_hover_unfocused_max);
    theme->a_toggled_focused_pressed_shade =
        RrAppearanceCopy(theme->a_toggled_focused_pressed_max);
    theme->a_toggled_unfocused_pressed_shade =
        RrAppearanceCopy(theme->a_toggled_unfocused_pressed_max);
    theme->a_toggled_focused_unpressed_shade =
        RrAppearanceCopy(theme->a_toggled_focused_unpressed_max);
    theme->a_toggled_unfocused_unpressed_shade =
        RrAppearanceCopy(theme->a_toggled_unfocused_unpressed_max);
    theme->a_toggled_hover_focused_shade =
        RrAppearanceCopy(theme->a_toggled_hover_focused_max);
    theme->a_toggled_hover_unfocused_shade =
        RrAppearanceCopy(theme->a_toggled_hover_unfocused_max);
    theme->a_unfocused_unpressed_shade =
        RrAppearanceCopy(theme->a_unfocused_unpressed_max);
    theme->a_unfocused_pressed_shade =
        RrAppearanceCopy(theme->a_unfocused_pressed_max);
    theme->a_focused_unpressed_shade =
        RrAppearanceCopy(theme->a_focused_unpressed_max);
    theme->a_focused_pressed_shade =
        RrAppearanceCopy(theme->a_focused_pressed_max);
    theme->a_disabled_focused_iconify =
        RrAppearanceCopy(theme->a_disabled_focused_max);
    theme->a_disabled_unfocused_iconify =
        RrAppearanceCopy(theme->a_disabled_focused_max);
    theme->a_hover_focused_iconify =
        RrAppearanceCopy(theme->a_hover_focused_max);
    theme->a_hover_unfocused_iconify =
        RrAppearanceCopy(theme->a_hover_unfocused_max);
    theme->a_unfocused_unpressed_iconify =
        RrAppearanceCopy(theme->a_unfocused_unpressed_max);
    theme->a_unfocused_pressed_iconify =
        RrAppearanceCopy(theme->a_unfocused_pressed_max);
    theme->a_focused_unpressed_iconify =
        RrAppearanceCopy(theme->a_focused_unpressed_max);
    theme->a_focused_pressed_iconify =
        RrAppearanceCopy(theme->a_focused_pressed_max);

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
    theme->a_focused_label->texture[0].type = 
        theme->osd_hilite_label->texture[0].type = RR_TEXTURE_TEXT;
    theme->a_focused_label->texture[0].data.text.justify = winjust;
    theme->osd_hilite_label->texture[0].data.text.justify = RR_JUSTIFY_LEFT;
    theme->a_focused_label->texture[0].data.text.font =
        theme->win_font_focused;
    theme->osd_hilite_label->texture[0].data.text.font = theme->osd_font;
    theme->a_focused_label->texture[0].data.text.color =
        theme->title_focused_color;
    theme->osd_hilite_label->texture[0].data.text.color =
        theme->osd_color;

    if (!FIND(shadow, L("window","active","label","text","shadow","offset"),
              theme->a_focused_label))
        theme->a_focused_label->texture[0].data.text.shadow_offset_x =
            theme->a_focused_label->texture[0].data.text.shadow_offset_y = 0;
    theme->a_focused_label->texture[0].data.text.shadow_color =
        theme->title_focused_shadow_color;
    theme->a_focused_label->texture[0].data.text.shadow_alpha =
        theme->title_focused_shadow_alpha;

    if (!FIND(shadow, L("osd","text","shadow","offset"),
              theme->osd_hilite_label))
    {
        theme->osd_hilite_label->texture[0].data.text.shadow_offset_x =
            theme->a_focused_label->texture[0].data.text.shadow_offset_x;
        theme->osd_hilite_label->texture[0].data.text.shadow_offset_y =
            theme->a_focused_label->texture[0].data.text.shadow_offset_y;
    }
    theme->osd_hilite_label->texture[0].data.text.shadow_color =
        theme->osd_shadow_color;
    theme->osd_hilite_label->texture[0].data.text.shadow_alpha =
        theme->osd_shadow_alpha;

    theme->a_unfocused_label->texture[0].type = RR_TEXTURE_TEXT;
    theme->a_unfocused_label->texture[0].data.text.justify = winjust;
    theme->a_unfocused_label->texture[0].data.text.font =
        theme->win_font_unfocused;
    theme->a_unfocused_label->texture[0].data.text.color =
        theme->title_unfocused_color;

    if (!FIND(shadow, L("window","inactive","label","text","shadow","offset"),
              theme->a_unfocused_label))
        theme->a_unfocused_label->texture[0].data.text.shadow_offset_x =
            theme->a_unfocused_label->texture[0].data.text.shadow_offset_y = 0;
    theme->a_unfocused_label->texture[0].data.text.shadow_color =
        theme->title_unfocused_shadow_color;
    theme->a_unfocused_label->texture[0].data.text.shadow_alpha =
        theme->title_unfocused_shadow_alpha;

    theme->a_menu_text_title->texture[0].type = RR_TEXTURE_TEXT;
    theme->a_menu_text_title->texture[0].data.text.justify = mtitlejust;
    theme->a_menu_text_title->texture[0].data.text.font =
        theme->menu_title_font;
    theme->a_menu_text_title->texture[0].data.text.color =
        theme->menu_title_color;

    if (!FIND(shadow, L("menu","title","text","shadow","offset"),
              theme->a_menu_text_title))
        theme->a_menu_text_title->texture[0].data.text.shadow_offset_x =
            theme->a_menu_text_title->texture[0].data.text.shadow_offset_y = 0;
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

    if (!FIND(shadow, L("menu","inactive","shadow","offset"),
              theme->a_menu_text_normal))
        theme->a_menu_text_normal->texture[0].data.text.shadow_offset_x =
            theme->a_menu_text_normal->texture[0].data.text.shadow_offset_y =
            0;
    if (!FIND(shadow, L("menu","active","text","shadow","offset"),
              theme->a_menu_text_selected))
        theme->a_menu_text_selected->texture[0].data.text.shadow_offset_x =
            theme->a_menu_text_selected->texture[0].data.text.shadow_offset_y =
            0;
    if (!FIND(shadow, L("menu","disabled","shadow","offset"),
              theme->a_menu_text_disabled))
        theme->a_menu_text_disabled->texture[0].data.text.shadow_offset_x =
            theme->a_menu_text_disabled->texture[0].data.text.shadow_offset_y =
            0;
    if (!FIND(shadow, L("menu","activedisabled","shadow","offset"),
              theme->a_menu_text_disabled_selected))
        theme->a_menu_text_disabled_selected->
            texture[0].data.text.shadow_offset_x = 0;
    theme->a_menu_text_disabled_selected->
        texture[0].data.text.shadow_offset_y = 0;
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
        theme->menu_text_disabled_selected_shadow_color;
    theme->a_menu_text_disabled_selected->texture[0].data.text.shadow_alpha =
        theme->menu_text_disabled_selected_shadow_alpha;

    theme->a_disabled_focused_max->texture[0].type = 
        theme->a_disabled_unfocused_max->texture[0].type = 
        theme->a_hover_focused_max->texture[0].type = 
        theme->a_hover_unfocused_max->texture[0].type = 
        theme->a_toggled_focused_pressed_max->texture[0].type = 
        theme->a_toggled_unfocused_pressed_max->texture[0].type = 
        theme->a_toggled_focused_unpressed_max->texture[0].type = 
        theme->a_toggled_unfocused_unpressed_max->texture[0].type = 
        theme->a_toggled_hover_focused_max->texture[0].type = 
        theme->a_toggled_hover_unfocused_max->texture[0].type = 
        theme->a_focused_unpressed_max->texture[0].type = 
        theme->a_focused_pressed_max->texture[0].type = 
        theme->a_unfocused_unpressed_max->texture[0].type = 
        theme->a_unfocused_pressed_max->texture[0].type = 
        theme->a_disabled_focused_close->texture[0].type = 
        theme->a_disabled_unfocused_close->texture[0].type = 
        theme->a_hover_focused_close->texture[0].type = 
        theme->a_hover_unfocused_close->texture[0].type = 
        theme->a_focused_unpressed_close->texture[0].type = 
        theme->a_focused_pressed_close->texture[0].type = 
        theme->a_unfocused_unpressed_close->texture[0].type = 
        theme->a_unfocused_pressed_close->texture[0].type = 
        theme->a_disabled_focused_desk->texture[0].type = 
        theme->a_disabled_unfocused_desk->texture[0].type = 
        theme->a_hover_focused_desk->texture[0].type = 
        theme->a_hover_unfocused_desk->texture[0].type = 
        theme->a_toggled_focused_pressed_desk->texture[0].type = 
        theme->a_toggled_unfocused_pressed_desk->texture[0].type = 
        theme->a_toggled_focused_unpressed_desk->texture[0].type = 
        theme->a_toggled_unfocused_unpressed_desk->texture[0].type = 
        theme->a_toggled_hover_focused_desk->texture[0].type = 
        theme->a_toggled_hover_unfocused_desk->texture[0].type = 
        theme->a_focused_unpressed_desk->texture[0].type = 
        theme->a_focused_pressed_desk->texture[0].type = 
        theme->a_unfocused_unpressed_desk->texture[0].type = 
        theme->a_unfocused_pressed_desk->texture[0].type = 
        theme->a_disabled_focused_shade->texture[0].type = 
        theme->a_disabled_unfocused_shade->texture[0].type = 
        theme->a_hover_focused_shade->texture[0].type = 
        theme->a_hover_unfocused_shade->texture[0].type = 
        theme->a_toggled_focused_pressed_shade->texture[0].type = 
        theme->a_toggled_unfocused_pressed_shade->texture[0].type = 
        theme->a_toggled_focused_unpressed_shade->texture[0].type = 
        theme->a_toggled_unfocused_unpressed_shade->texture[0].type = 
        theme->a_toggled_hover_focused_shade->texture[0].type = 
        theme->a_toggled_hover_unfocused_shade->texture[0].type = 
        theme->a_focused_unpressed_shade->texture[0].type = 
        theme->a_focused_pressed_shade->texture[0].type = 
        theme->a_unfocused_unpressed_shade->texture[0].type = 
        theme->a_unfocused_pressed_shade->texture[0].type = 
        theme->a_disabled_focused_iconify->texture[0].type = 
        theme->a_disabled_unfocused_iconify->texture[0].type = 
        theme->a_hover_focused_iconify->texture[0].type = 
        theme->a_hover_unfocused_iconify->texture[0].type = 
        theme->a_focused_unpressed_iconify->texture[0].type = 
        theme->a_focused_pressed_iconify->texture[0].type = 
        theme->a_unfocused_unpressed_iconify->texture[0].type = 
        theme->a_unfocused_pressed_iconify->texture[0].type =
        theme->a_menu_bullet_normal->texture[0].type =
        theme->a_menu_bullet_selected->texture[0].type = RR_TEXTURE_MASK;

    theme->a_disabled_focused_max->texture[0].data.mask.mask = 
        theme->a_disabled_unfocused_max->texture[0].data.mask.mask = 
        theme->max_disabled_mask;
    theme->a_hover_focused_max->texture[0].data.mask.mask = 
        theme->a_hover_unfocused_max->texture[0].data.mask.mask = 
        theme->max_hover_mask;
    theme->a_focused_pressed_max->texture[0].data.mask.mask = 
        theme->a_unfocused_pressed_max->texture[0].data.mask.mask =
        theme->max_pressed_mask;
    theme->a_focused_unpressed_max->texture[0].data.mask.mask = 
        theme->a_unfocused_unpressed_max->texture[0].data.mask.mask = 
        theme->max_mask;
    theme->a_toggled_focused_pressed_max->texture[0].data.mask.mask = 
        theme->a_toggled_unfocused_pressed_max->texture[0].data.mask.mask =
        theme->max_toggled_pressed_mask;
    theme->a_toggled_focused_unpressed_max->texture[0].data.mask.mask = 
        theme->a_toggled_unfocused_unpressed_max->texture[0].data.mask.mask =
        theme->max_toggled_mask;
    theme->a_toggled_hover_focused_max->texture[0].data.mask.mask = 
        theme->a_toggled_hover_unfocused_max->texture[0].data.mask.mask =
        theme->max_toggled_hover_mask;
    theme->a_disabled_focused_close->texture[0].data.mask.mask = 
        theme->a_disabled_unfocused_close->texture[0].data.mask.mask = 
        theme->close_disabled_mask;
    theme->a_hover_focused_close->texture[0].data.mask.mask = 
        theme->a_hover_unfocused_close->texture[0].data.mask.mask = 
        theme->close_hover_mask;
    theme->a_focused_pressed_close->texture[0].data.mask.mask = 
        theme->a_unfocused_pressed_close->texture[0].data.mask.mask =
        theme->close_pressed_mask;
    theme->a_focused_unpressed_close->texture[0].data.mask.mask = 
        theme->a_unfocused_unpressed_close->texture[0].data.mask.mask =
        theme->close_mask;
    theme->a_disabled_focused_desk->texture[0].data.mask.mask = 
        theme->a_disabled_unfocused_desk->texture[0].data.mask.mask = 
        theme->desk_disabled_mask;
    theme->a_hover_focused_desk->texture[0].data.mask.mask = 
        theme->a_hover_unfocused_desk->texture[0].data.mask.mask = 
        theme->desk_hover_mask;
    theme->a_focused_pressed_desk->texture[0].data.mask.mask = 
        theme->a_unfocused_pressed_desk->texture[0].data.mask.mask =
        theme->desk_pressed_mask;
    theme->a_focused_unpressed_desk->texture[0].data.mask.mask = 
        theme->a_unfocused_unpressed_desk->texture[0].data.mask.mask = 
        theme->desk_mask;
    theme->a_toggled_focused_pressed_desk->texture[0].data.mask.mask = 
        theme->a_toggled_unfocused_pressed_desk->texture[0].data.mask.mask =
        theme->desk_toggled_pressed_mask;
    theme->a_toggled_focused_unpressed_desk->texture[0].data.mask.mask = 
        theme->a_toggled_unfocused_unpressed_desk->texture[0].data.mask.mask =
        theme->desk_toggled_mask;
    theme->a_toggled_hover_focused_desk->texture[0].data.mask.mask = 
        theme->a_toggled_hover_unfocused_desk->texture[0].data.mask.mask =
        theme->desk_toggled_hover_mask;
    theme->a_disabled_focused_shade->texture[0].data.mask.mask = 
        theme->a_disabled_unfocused_shade->texture[0].data.mask.mask = 
        theme->shade_disabled_mask;
    theme->a_hover_focused_shade->texture[0].data.mask.mask = 
        theme->a_hover_unfocused_shade->texture[0].data.mask.mask = 
        theme->shade_hover_mask;
    theme->a_focused_pressed_shade->texture[0].data.mask.mask = 
        theme->a_unfocused_pressed_shade->texture[0].data.mask.mask =
        theme->shade_pressed_mask;
    theme->a_focused_unpressed_shade->texture[0].data.mask.mask = 
        theme->a_unfocused_unpressed_shade->texture[0].data.mask.mask = 
        theme->shade_mask;
    theme->a_toggled_focused_pressed_shade->texture[0].data.mask.mask = 
        theme->a_toggled_unfocused_pressed_shade->texture[0].data.mask.mask =
        theme->shade_toggled_pressed_mask;
    theme->a_toggled_focused_unpressed_shade->texture[0].data.mask.mask = 
        theme->a_toggled_unfocused_unpressed_shade->texture[0].data.mask.mask =
        theme->shade_toggled_mask;
    theme->a_toggled_hover_focused_shade->texture[0].data.mask.mask = 
        theme->a_toggled_hover_unfocused_shade->texture[0].data.mask.mask =
        theme->shade_toggled_hover_mask;
    theme->a_disabled_focused_iconify->texture[0].data.mask.mask = 
        theme->a_disabled_unfocused_iconify->texture[0].data.mask.mask = 
        theme->iconify_disabled_mask;
    theme->a_hover_focused_iconify->texture[0].data.mask.mask = 
        theme->a_hover_unfocused_iconify->texture[0].data.mask.mask = 
        theme->iconify_hover_mask;
    theme->a_focused_pressed_iconify->texture[0].data.mask.mask = 
        theme->a_unfocused_pressed_iconify->texture[0].data.mask.mask =
        theme->iconify_pressed_mask;
    theme->a_focused_unpressed_iconify->texture[0].data.mask.mask = 
        theme->a_unfocused_unpressed_iconify->texture[0].data.mask.mask = 
        theme->iconify_mask;
    theme->a_menu_bullet_normal->texture[0].data.mask.mask = 
    theme->a_menu_bullet_selected->texture[0].data.mask.mask = 
        theme->menu_bullet_mask;
    theme->a_disabled_focused_max->texture[0].data.mask.color = 
        theme->a_disabled_focused_close->texture[0].data.mask.color = 
        theme->a_disabled_focused_desk->texture[0].data.mask.color = 
        theme->a_disabled_focused_shade->texture[0].data.mask.color = 
        theme->a_disabled_focused_iconify->texture[0].data.mask.color = 
        theme->titlebut_disabled_focused_color;
    theme->a_disabled_unfocused_max->texture[0].data.mask.color = 
        theme->a_disabled_unfocused_close->texture[0].data.mask.color = 
        theme->a_disabled_unfocused_desk->texture[0].data.mask.color = 
        theme->a_disabled_unfocused_shade->texture[0].data.mask.color = 
        theme->a_disabled_unfocused_iconify->texture[0].data.mask.color = 
        theme->titlebut_disabled_unfocused_color;
    theme->a_hover_focused_max->texture[0].data.mask.color = 
        theme->a_hover_focused_close->texture[0].data.mask.color = 
        theme->a_hover_focused_desk->texture[0].data.mask.color = 
        theme->a_hover_focused_shade->texture[0].data.mask.color = 
        theme->a_hover_focused_iconify->texture[0].data.mask.color = 
        theme->titlebut_hover_focused_color;
    theme->a_hover_unfocused_max->texture[0].data.mask.color = 
        theme->a_hover_unfocused_close->texture[0].data.mask.color = 
        theme->a_hover_unfocused_desk->texture[0].data.mask.color = 
        theme->a_hover_unfocused_shade->texture[0].data.mask.color = 
        theme->a_hover_unfocused_iconify->texture[0].data.mask.color = 
        theme->titlebut_hover_unfocused_color;
    theme->a_toggled_hover_focused_max->texture[0].data.mask.color = 
        theme->a_toggled_hover_focused_desk->texture[0].data.mask.color = 
        theme->a_toggled_hover_focused_shade->texture[0].data.mask.color = 
        theme->titlebut_toggled_hover_focused_color;
    theme->a_toggled_hover_unfocused_max->texture[0].data.mask.color = 
        theme->a_toggled_hover_unfocused_desk->texture[0].data.mask.color = 
        theme->a_toggled_hover_unfocused_shade->texture[0].data.mask.color = 
        theme->titlebut_toggled_hover_unfocused_color;
    theme->a_toggled_focused_pressed_max->texture[0].data.mask.color = 
        theme->a_toggled_focused_pressed_desk->texture[0].data.mask.color = 
        theme->a_toggled_focused_pressed_shade->texture[0].data.mask.color = 
        theme->titlebut_toggled_focused_pressed_color;
    theme->a_toggled_unfocused_pressed_max->texture[0].data.mask.color = 
        theme->a_toggled_unfocused_pressed_desk->texture[0].data.mask.color = 
        theme->a_toggled_unfocused_pressed_shade->texture[0].data.mask.color = 
        theme->titlebut_toggled_unfocused_pressed_color;
    theme->a_toggled_focused_unpressed_max->texture[0].data.mask.color = 
        theme->a_toggled_focused_unpressed_desk->texture[0].data.mask.color = 
        theme->a_toggled_focused_unpressed_shade->texture[0].data.mask.color = 
        theme->titlebut_toggled_focused_unpressed_color;
    theme->a_toggled_unfocused_unpressed_max->texture[0].data.mask.color = 
        theme->a_toggled_unfocused_unpressed_desk->texture[0].data.mask.color =
        theme->a_toggled_unfocused_unpressed_shade->texture[0].data.mask.color=
        theme->titlebut_toggled_unfocused_unpressed_color;
    theme->a_focused_unpressed_max->texture[0].data.mask.color = 
        theme->a_focused_unpressed_close->texture[0].data.mask.color = 
        theme->a_focused_unpressed_desk->texture[0].data.mask.color = 
        theme->a_focused_unpressed_shade->texture[0].data.mask.color = 
        theme->a_focused_unpressed_iconify->texture[0].data.mask.color = 
        theme->titlebut_focused_unpressed_color;
    theme->a_focused_pressed_max->texture[0].data.mask.color = 
        theme->a_focused_pressed_close->texture[0].data.mask.color = 
        theme->a_focused_pressed_desk->texture[0].data.mask.color = 
        theme->a_focused_pressed_shade->texture[0].data.mask.color = 
        theme->a_focused_pressed_iconify->texture[0].data.mask.color =
        theme->titlebut_focused_pressed_color;
    theme->a_unfocused_unpressed_max->texture[0].data.mask.color = 
        theme->a_unfocused_unpressed_close->texture[0].data.mask.color = 
        theme->a_unfocused_unpressed_desk->texture[0].data.mask.color = 
        theme->a_unfocused_unpressed_shade->texture[0].data.mask.color = 
        theme->a_unfocused_unpressed_iconify->texture[0].data.mask.color = 
        theme->titlebut_unfocused_unpressed_color;
    theme->a_unfocused_pressed_max->texture[0].data.mask.color = 
        theme->a_unfocused_pressed_close->texture[0].data.mask.color = 
        theme->a_unfocused_pressed_desk->texture[0].data.mask.color = 
        theme->a_unfocused_pressed_shade->texture[0].data.mask.color = 
        theme->a_unfocused_pressed_iconify->texture[0].data.mask.color =
        theme->titlebut_unfocused_pressed_color;
    theme->a_menu_bullet_normal->texture[0].data.mask.color = 
        theme->menu_color;
    theme->a_menu_bullet_selected->texture[0].data.mask.color = 
        theme->menu_selected_color;

    g_free(ps.path);
    parse_close(ps.doc);

    {
        gint ft, fb, fl, fr, ut, ub, ul, ur;
        RrAppearance *a, *b, *c, *d;

        /* caluclate the font heights*/
        a = theme->a_focused_label;
        theme->win_font_height =
            RrFontHeight(theme->win_font_focused,
                         a->texture[0].data.text.shadow_offset_y);
        a = theme->a_unfocused_label;
        theme->win_font_height =
            MAX(theme->win_font_height,
                RrFontHeight(theme->win_font_unfocused,
                             a->texture[0].data.text.shadow_offset_y));
        a = theme->a_menu_text_title;
        theme->menu_title_font_height =
            RrFontHeight(theme->menu_title_font,
                         a->texture[0].data.text.shadow_offset_y);
        a = theme->a_menu_text_normal;
        b = theme->a_menu_text_selected;
        c = theme->a_menu_text_disabled;
        d = theme->a_menu_text_disabled_selected;
        theme->menu_font_height =
            RrFontHeight(theme->menu_font,
                         MAX(a->texture[0].data.text.shadow_offset_y,
                             MAX(b->texture[0].data.text.shadow_offset_y,
                                 MAX(c->texture[0].data.text.shadow_offset_y,
                                     d->texture[0].data.text.shadow_offset_y
                                     ))));

        RrMargins(theme->a_focused_label, &fl, &ft, &fr, &fb);
        RrMargins(theme->a_unfocused_label, &ul, &ut, &ur, &ub);
        theme->label_height = theme->win_font_height + MAX(ft + fb, ut + ub);
        theme->label_height += theme->label_height & 1;

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

    return theme;
}

void RrThemeFree(RrTheme *theme)
{
    if (theme) {
        g_free(theme->name);

        RrColorFree(theme->menu_border_color);
        RrColorFree(theme->frame_focused_border_color);
        RrColorFree(theme->frame_unfocused_border_color);
        RrColorFree(theme->cb_unfocused_color);
        RrColorFree(theme->cb_focused_color);
        RrColorFree(theme->title_focused_color);
        RrColorFree(theme->title_unfocused_color);
        RrColorFree(theme->titlebut_disabled_focused_color);
        RrColorFree(theme->titlebut_disabled_unfocused_color);
        RrColorFree(theme->titlebut_hover_focused_color);
        RrColorFree(theme->titlebut_hover_unfocused_color);
        RrColorFree(theme->titlebut_focused_pressed_color);
        RrColorFree(theme->titlebut_unfocused_pressed_color);
        RrColorFree(theme->titlebut_focused_unpressed_color);
        RrColorFree(theme->titlebut_unfocused_unpressed_color);
        RrColorFree(theme->titlebut_toggled_hover_focused_color);
        RrColorFree(theme->titlebut_toggled_hover_unfocused_color);
        RrColorFree(theme->titlebut_toggled_focused_pressed_color);
        RrColorFree(theme->titlebut_toggled_unfocused_pressed_color);
        RrColorFree(theme->titlebut_toggled_focused_unpressed_color);
        RrColorFree(theme->titlebut_toggled_unfocused_unpressed_color);
        RrColorFree(theme->menu_title_color);
        RrColorFree(theme->menu_color);
        RrColorFree(theme->menu_selected_color);
        RrColorFree(theme->menu_disabled_color);
        RrColorFree(theme->menu_disabled_selected_color);
        RrColorFree(theme->title_focused_shadow_color);
        RrColorFree(theme->title_unfocused_shadow_color);
        RrColorFree(theme->osd_color);
        RrColorFree(theme->osd_shadow_color);
        RrColorFree(theme->menu_title_shadow_color);
        RrColorFree(theme->menu_text_normal_shadow_color);
        RrColorFree(theme->menu_text_selected_shadow_color);
        RrColorFree(theme->menu_text_disabled_shadow_color);
        RrColorFree(theme->menu_text_disabled_selected_shadow_color);

        g_free(theme->def_win_icon);

        RrPixmapMaskFree(theme->max_mask);
        RrPixmapMaskFree(theme->max_toggled_mask);
        RrPixmapMaskFree(theme->max_toggled_hover_mask);
        RrPixmapMaskFree(theme->max_toggled_pressed_mask);
        RrPixmapMaskFree(theme->max_disabled_mask);
        RrPixmapMaskFree(theme->max_hover_mask);
        RrPixmapMaskFree(theme->max_pressed_mask);
        RrPixmapMaskFree(theme->desk_mask);
        RrPixmapMaskFree(theme->desk_toggled_mask);
        RrPixmapMaskFree(theme->desk_toggled_hover_mask);
        RrPixmapMaskFree(theme->desk_toggled_pressed_mask);
        RrPixmapMaskFree(theme->desk_disabled_mask);
        RrPixmapMaskFree(theme->desk_hover_mask);
        RrPixmapMaskFree(theme->desk_pressed_mask);
        RrPixmapMaskFree(theme->shade_mask);
        RrPixmapMaskFree(theme->shade_toggled_mask);
        RrPixmapMaskFree(theme->shade_toggled_hover_mask);
        RrPixmapMaskFree(theme->shade_toggled_pressed_mask);
        RrPixmapMaskFree(theme->shade_disabled_mask);
        RrPixmapMaskFree(theme->shade_hover_mask);
        RrPixmapMaskFree(theme->shade_pressed_mask);
        RrPixmapMaskFree(theme->iconify_mask);
        RrPixmapMaskFree(theme->iconify_disabled_mask);
        RrPixmapMaskFree(theme->iconify_hover_mask);
        RrPixmapMaskFree(theme->iconify_pressed_mask);
        RrPixmapMaskFree(theme->close_mask);
        RrPixmapMaskFree(theme->close_disabled_mask);
        RrPixmapMaskFree(theme->close_hover_mask);
        RrPixmapMaskFree(theme->close_pressed_mask);
        RrPixmapMaskFree(theme->menu_bullet_mask);

        RrFontClose(theme->win_font_focused); 
        RrFontClose(theme->win_font_unfocused);
        RrFontClose(theme->menu_title_font);
        RrFontClose(theme->menu_font);

        RrAppearanceFree(theme->a_disabled_focused_max);
        RrAppearanceFree(theme->a_disabled_unfocused_max);
        RrAppearanceFree(theme->a_hover_focused_max);
        RrAppearanceFree(theme->a_hover_unfocused_max);
        RrAppearanceFree(theme->a_focused_unpressed_max);
        RrAppearanceFree(theme->a_focused_pressed_max);
        RrAppearanceFree(theme->a_unfocused_unpressed_max);
        RrAppearanceFree(theme->a_unfocused_pressed_max);
        RrAppearanceFree(theme->a_toggled_hover_focused_max);
        RrAppearanceFree(theme->a_toggled_hover_unfocused_max);
        RrAppearanceFree(theme->a_toggled_focused_unpressed_max);
        RrAppearanceFree(theme->a_toggled_focused_pressed_max);
        RrAppearanceFree(theme->a_toggled_unfocused_unpressed_max);
        RrAppearanceFree(theme->a_toggled_unfocused_pressed_max);
        RrAppearanceFree(theme->a_disabled_focused_close);
        RrAppearanceFree(theme->a_disabled_unfocused_close);
        RrAppearanceFree(theme->a_hover_focused_close);
        RrAppearanceFree(theme->a_hover_unfocused_close);
        RrAppearanceFree(theme->a_focused_unpressed_close);
        RrAppearanceFree(theme->a_focused_pressed_close);
        RrAppearanceFree(theme->a_unfocused_unpressed_close);
        RrAppearanceFree(theme->a_unfocused_pressed_close);
        RrAppearanceFree(theme->a_disabled_focused_desk);
        RrAppearanceFree(theme->a_disabled_unfocused_desk);
        RrAppearanceFree(theme->a_hover_focused_desk);
        RrAppearanceFree(theme->a_hover_unfocused_desk);
        RrAppearanceFree(theme->a_focused_unpressed_desk);
        RrAppearanceFree(theme->a_focused_pressed_desk);
        RrAppearanceFree(theme->a_unfocused_unpressed_desk);
        RrAppearanceFree(theme->a_unfocused_pressed_desk);
        RrAppearanceFree(theme->a_toggled_hover_focused_desk);
        RrAppearanceFree(theme->a_toggled_hover_unfocused_desk);
        RrAppearanceFree(theme->a_toggled_focused_unpressed_desk);
        RrAppearanceFree(theme->a_toggled_focused_pressed_desk);
        RrAppearanceFree(theme->a_toggled_unfocused_unpressed_desk);
        RrAppearanceFree(theme->a_toggled_unfocused_pressed_desk);
        RrAppearanceFree(theme->a_disabled_focused_shade);
        RrAppearanceFree(theme->a_disabled_unfocused_shade);
        RrAppearanceFree(theme->a_hover_focused_shade);
        RrAppearanceFree(theme->a_hover_unfocused_shade);
        RrAppearanceFree(theme->a_focused_unpressed_shade);
        RrAppearanceFree(theme->a_focused_pressed_shade);
        RrAppearanceFree(theme->a_unfocused_unpressed_shade);
        RrAppearanceFree(theme->a_unfocused_pressed_shade);
        RrAppearanceFree(theme->a_toggled_hover_focused_shade);
        RrAppearanceFree(theme->a_toggled_hover_unfocused_shade);
        RrAppearanceFree(theme->a_toggled_focused_unpressed_shade);
        RrAppearanceFree(theme->a_toggled_focused_pressed_shade);
        RrAppearanceFree(theme->a_toggled_unfocused_unpressed_shade);
        RrAppearanceFree(theme->a_toggled_unfocused_pressed_shade);
        RrAppearanceFree(theme->a_disabled_focused_iconify);
        RrAppearanceFree(theme->a_disabled_unfocused_iconify);
        RrAppearanceFree(theme->a_hover_focused_iconify);
        RrAppearanceFree(theme->a_hover_unfocused_iconify);
        RrAppearanceFree(theme->a_focused_unpressed_iconify);
        RrAppearanceFree(theme->a_focused_pressed_iconify);
        RrAppearanceFree(theme->a_unfocused_unpressed_iconify);
        RrAppearanceFree(theme->a_unfocused_pressed_iconify);
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
        RrAppearanceFree(theme->osd_hilite_bg);
        RrAppearanceFree(theme->osd_hilite_fg);
        RrAppearanceFree(theme->osd_hilite_label);
        RrAppearanceFree(theme->osd_unhilite_fg);

        g_free(theme);
    }
}

static gboolean read_mask(ParseState *ps, const gchar *maskname,
                          RrPixmapMask **value)
{
    gboolean ret = FALSE;
    gchar *s;
    gint hx, hy; /* ignored */
    guint w, h;
    guchar *b;

    s = g_build_filename(ps->path, maskname, NULL);
    if (XReadBitmapFileData(s, &w, &h, &b, &hx, &hy) == BitmapSuccess) {
        ret = TRUE;
        *value = RrPixmapMaskNew(ps->inst, w, h, (gchar*)b);
        XFree(b);
    }
    g_free(s);

    return ret;
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

static void parse_style(gchar *tex, RrSurfaceColorType *grad,
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

        if (strstr(tex, "sunken") != NULL)
            *relief = RR_RELIEF_SUNKEN;
        else if (strstr(tex, "flat") != NULL)
            *relief = RR_RELIEF_FLAT;
        else
            *relief = RR_RELIEF_RAISED;

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
}

static xmlNodePtr find_node(xmlNodePtr n, const gchar *names[])
{
    gint i;

    for (i = 0; names[i] && n; ++i)
        n = parse_find_node(names[i], n->children);
    return n;
}

static gboolean find_int(ParseState *ps, xmlNodePtr n, const gchar *names[],
                         gint *integer, gint lower, gint upper)
{
    gint i;

    if ((n = find_node(n, names))) {
        i = parse_int(ps->doc, n);
        if (i >= lower && i <= upper) {
            *integer = i;
            return TRUE;
        }
    }
    return FALSE;
}

static gboolean find_string(ParseState *ps, xmlNodePtr n, const gchar *names[],
                            gchar **string)
{
    if ((n = find_node(n, names))) {
        *string = parse_string(ps->doc, n);
        return TRUE;
    }
    return FALSE;
}

static gboolean find_color(ParseState *ps, xmlNodePtr n, const gchar *names[],
                           RrColor **color, gchar *alpha)
{
    if ((n = find_node(n, names))) {
        int r,g,b,a;
        if (parse_attr_int("r", n, &r) &&
            parse_attr_int("g", n, &g) &&
            parse_attr_int("b", n, &b) &&
            parse_attr_int("a", n, &a) &&
            r >= 0 && g >= 0 && b >= 0 && a >= 0 &&
            r < 256 && g < 256 && b < 256 && a < 256)
        {
            *color = RrColorNew(ps->inst, r, g, b);
            if (alpha) *alpha = a;
            return TRUE;
        }
    }
    return FALSE;
}

static gboolean find_point(ParseState *ps, xmlNodePtr n, const gchar *names[],
                           gint *x, gint *y,
                           gint lowx, gint upx, gint lowy, gint upy)
{
    if ((n = find_node(n, names))) {
        gint a, b;
        if (parse_attr_int("x", n, &a) &&
            parse_attr_int("y", n, &b) &&
            a >= lowx && a <= upx && b >= lowy && b <= upy)
        {
            *x = a; *y = b;
            return TRUE;
        }
    }
    return FALSE;
}

static gboolean find_shadow(ParseState *ps, xmlNodePtr n, const gchar *names[],
                            RrAppearance *a)
{
    return find_point(ps, n, names,
                      &a->texture[0].data.text.shadow_offset_x,
                      &a->texture[0].data.text.shadow_offset_y,
                      -20, 20, -20, 20);
}

static gboolean find_appearance(ParseState *ps, xmlNodePtr n, const gchar *names[],
                                RrAppearance *a, gboolean allow_trans)
{
    xmlNodePtr n2;

    if (!(n = find_node(n, names)))
        return FALSE;

    if ((n2 = find_node(n, L("style")))) {
        gchar *s = parse_string(ps->doc, n2);
        parse_style(s, &a->surface.grad, &a->surface.relief,
                    &a->surface.bevel, &a->surface.interlaced,
                    &a->surface.border, allow_trans);
        g_free(s);
    } else
        return FALSE;

    if (!find_color(ps, n, L("primary"), &a->surface.primary, NULL))
        a->surface.primary = RrColorNew(ps->inst, 0, 0, 0);
    if (!find_color(ps, n, L("secondary"), &a->surface.secondary, NULL))
        a->surface.secondary = RrColorNew(ps->inst, 0, 0, 0);
    if (a->surface.border)
        if (!find_color(ps, n, L("border"),
                        &a->surface.border_color, NULL))
            a->surface.border_color = RrColorNew(ps->inst, 0, 0, 0);
    if (a->surface.interlaced)
        if (!find_color(ps, n, L("interlace"),
                        &a->surface.interlace_color, NULL))
            a->surface.interlace_color = RrColorNew(ps->inst, 0, 0, 0);

    return TRUE;
}
