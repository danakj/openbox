#include "render.h"
#include "color.h"
#include "font.h"
#include "mask.h"
#include "theme.h"

#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static XrmDatabase loaddb(RrTheme *theme, char *name);
static gboolean read_int(XrmDatabase db, char *rname, int *value);
static gboolean read_string(XrmDatabase db, char *rname, char **value);
static gboolean read_color(XrmDatabase db, const RrInstance *inst,
                           gchar *rname, RrColor **value);
static gboolean read_mask(const RrInstance *inst,
                          gchar *maskname, RrTheme *theme,
                          RrPixmapMask **value);
static gboolean read_appearance(XrmDatabase db, const RrInstance *inst,
                                gchar *rname, RrAppearance *value,
                                gboolean allow_trans);
static void set_default_appearance(RrAppearance *a);

RrTheme* RrThemeNew(const RrInstance *inst, gchar *name)
{
    XrmDatabase db = NULL;
    RrJustify winjust, mtitlejust, mjust;
    gchar *str;
    gchar *font_str;
    RrTheme *theme;

    theme = g_new0(RrTheme, 1);

    theme->inst = inst;

    theme->a_disabled_focused_max = RrAppearanceNew(inst, 1);
    theme->a_disabled_unfocused_max = RrAppearanceNew(inst, 1);
    theme->a_hover_focused_max = RrAppearanceNew(inst, 1);
    theme->a_hover_unfocused_max = RrAppearanceNew(inst, 1);
    theme->a_focused_unpressed_max = RrAppearanceNew(inst, 1);
    theme->a_focused_pressed_max = RrAppearanceNew(inst, 1);
    theme->a_focused_pressed_set_max = RrAppearanceNew(inst, 1);
    theme->a_unfocused_unpressed_max = RrAppearanceNew(inst, 1);
    theme->a_unfocused_pressed_max = RrAppearanceNew(inst, 1);
    theme->a_unfocused_pressed_set_max = RrAppearanceNew(inst, 1);
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
    theme->a_menu_title = RrAppearanceNew(inst, 1);
    theme->a_menu_item = RrAppearanceNew(inst, 1);
    theme->a_menu_disabled = RrAppearanceNew(inst, 1);
    theme->a_menu_hilite = RrAppearanceNew(inst, 1);

    theme->app_hilite_bg = RrAppearanceNew(inst, 0);
    theme->app_unhilite_bg = RrAppearanceNew(inst, 0);
    theme->app_hilite_label = RrAppearanceNew(inst, 1);
    theme->app_unhilite_label = RrAppearanceNew(inst, 1);
    theme->app_icon = RrAppearanceNew(inst, 1);

    if (name) {
	db = loaddb(theme, name);
        if (db == NULL) {
	    g_warning("Failed to load the theme '%s'\n"
                      "Falling back to the default: '%s'",
                      name, DEFAULT_THEME);
	} else
            theme->name = g_path_get_basename(name);
    }
    if (db == NULL) {
	db = loaddb(theme, DEFAULT_THEME);
	if (db == NULL) {
	    g_warning("Failed to load the theme '%s'.", DEFAULT_THEME);
	    return NULL;
	} else
            theme->name = g_path_get_basename(DEFAULT_THEME);
    }

    /* load the font stuff */
    if (!read_string(db, "window.title.xftfont", &font_str))
        font_str = "arial,sans:bold:pixelsize=10:shadow=y:shadowtint=50";

    if (!(theme->winfont = RrFontOpen(inst, font_str))) {
        RrThemeFree(theme);
        return NULL;
    }
    theme->winfont_height = RrFontHeight(theme->winfont);

    winjust = RR_JUSTIFY_LEFT;
    if (read_string(db, "window.justify", &str)) {
        if (!g_ascii_strcasecmp(str, "right"))
            winjust = RR_JUSTIFY_RIGHT;
        else if (!g_ascii_strcasecmp(str, "center"))
            winjust = RR_JUSTIFY_CENTER;
    }

    if (!read_string(db, "menu.title.xftfont", &font_str))
        font_str = "arial,sans:bold:pixelsize=12:shadow=y";

    if (!(theme->mtitlefont = RrFontOpen(inst, font_str))) {
        RrThemeFree(theme);
        return NULL;
    }
    theme->mtitlefont_height = RrFontHeight(theme->mtitlefont);

    mtitlejust = RR_JUSTIFY_LEFT;
    if (read_string(db, "menu.title.justify", &str)) {
        if (!g_ascii_strcasecmp(str, "right"))
            mtitlejust = RR_JUSTIFY_RIGHT;
        else if (!g_ascii_strcasecmp(str, "center"))
            mtitlejust = RR_JUSTIFY_CENTER;
    }

    if (!read_string(db, "menu.frame.xftfont", &font_str))
        font_str = "arial,sans:bold:pixelsize=11:shadow=y";

    if (!(theme->mfont = RrFontOpen(inst, font_str))) {
        RrThemeFree(theme);
        return NULL;
    }
    theme->mfont_height = RrFontHeight(theme->mfont);

    mjust = RR_JUSTIFY_LEFT;
    if (read_string(db, "menu.frame.justify", &str)) {
        if (!g_ascii_strcasecmp(str, "right"))
            mjust = RR_JUSTIFY_RIGHT;
        else if (!g_ascii_strcasecmp(str, "center"))
            mjust = RR_JUSTIFY_CENTER;
    }

    /* load direct dimensions */
    if (!read_int(db, "menuOverlap", &theme->menu_overlap) ||
	theme->menu_overlap < 0 || theme->menu_overlap > 20)
        theme->handle_height = 0;
    if (!read_int(db, "handleWidth", &theme->handle_height) ||
	theme->handle_height < 0 || theme->handle_height > 100)
        theme->handle_height = 6;
    if (!read_int(db, "bevelWidth", &theme->bevel) ||
	theme->bevel <= 0 || theme->bevel > 100) theme->bevel = 3;
    if (!read_int(db, "borderWidth", &theme->bwidth) ||
	theme->bwidth < 0 || theme->bwidth > 100) theme->bwidth = 1;
    if (!read_int(db, "frameWidth", &theme->cbwidth) ||
	theme->cbwidth < 0 || theme->cbwidth > 100)
        theme->cbwidth = theme->bevel;

    /* load colors */
    if (!read_color(db, inst,
                    "borderColor", &theme->b_color))
	theme->b_color = RrColorNew(inst, 0, 0, 0);
    if (!read_color(db, inst,
                    "window.frame.focusColor", &theme->cb_focused_color))
	theme->cb_focused_color = RrColorNew(inst, 0xff, 0xff, 0xff);
    if (!read_color(db, inst,
                    "window.frame.unfocusColor",&theme->cb_unfocused_color))
	theme->cb_unfocused_color = RrColorNew(inst, 0xff, 0xff, 0xff);
    if (!read_color(db, inst,
                    "window.label.focus.textColor",
                    &theme->title_focused_color))
	theme->title_focused_color = RrColorNew(inst, 0x0, 0x0, 0x0);
    if (!read_color(db, inst,
                    "window.label.unfocus.textColor",
                    &theme->title_unfocused_color))
	theme->title_unfocused_color = RrColorNew(inst, 0xff, 0xff, 0xff);
    if (!read_color(db, inst,
                    "window.button.focus.picColor",
                    &theme->titlebut_focused_unpressed_color))
	theme->titlebut_focused_unpressed_color = RrColorNew(inst, 0, 0, 0);
    if (!read_color(db, inst,
                    "window.button.unfocus.picColor",
                    &theme->titlebut_unfocused_unpressed_color))
	theme->titlebut_unfocused_unpressed_color =
            RrColorNew(inst, 0xff, 0xff, 0xff);
    if (!read_color(db, inst,
                    "window.button.pressed.focus.picColor",
                    &theme->titlebut_focused_pressed_color))
	theme->titlebut_focused_pressed_color =
            RrColorNew(inst,
                       theme->titlebut_focused_unpressed_color->r,
                       theme->titlebut_focused_unpressed_color->g,
                       theme->titlebut_focused_unpressed_color->b);
    if (!read_color(db, inst,
                    "window.button.pressed.unfocus.picColor",
                    &theme->titlebut_unfocused_pressed_color))
	theme->titlebut_unfocused_pressed_color =
            RrColorNew(inst,
                       theme->titlebut_unfocused_unpressed_color->r,
                       theme->titlebut_unfocused_unpressed_color->g,
                       theme->titlebut_unfocused_unpressed_color->b);
    if (!read_color(db, inst,
                    "window.button.disabled.focus.picColor",
                    &theme->titlebut_disabled_focused_color))
	theme->titlebut_disabled_focused_color =
            RrColorNew(inst, 0xff, 0xff, 0xff);
    if (!read_color(db, inst,
                    "window.button.disabled.unfocus.picColor",
                    &theme->titlebut_disabled_unfocused_color))
	theme->titlebut_disabled_unfocused_color = RrColorNew(inst, 0, 0, 0);
    if (!read_color(db, inst,
                    "window.button.hover.focus.picColor",
                    &theme->titlebut_hover_focused_color))
	theme->titlebut_hover_focused_color =
            RrColorNew(inst,
                       theme->titlebut_focused_unpressed_color->r,
                       theme->titlebut_focused_unpressed_color->g,
                       theme->titlebut_focused_unpressed_color->b);
    if (!read_color(db, inst,
                    "window.button.hover.unfocus.picColor",
                    &theme->titlebut_hover_unfocused_color))
	theme->titlebut_hover_unfocused_color =
            RrColorNew(inst,
                       theme->titlebut_unfocused_unpressed_color->r,
                       theme->titlebut_unfocused_unpressed_color->g,
                       theme->titlebut_unfocused_unpressed_color->b);
    if (!read_color(db, inst,
                    "menu.title.textColor", &theme->menu_title_color))
        theme->menu_title_color = RrColorNew(inst, 0, 0, 0);
    if (!read_color(db, inst,
                    "menu.frame.textColor", &theme->menu_color))
        theme->menu_color = RrColorNew(inst, 0xff, 0xff, 0xff);
    if (!read_color(db, inst,
                    "menu.frame.disableColor", &theme->menu_disabled_color))
        theme->menu_disabled_color = RrColorNew(inst, 0, 0, 0);
    if (!read_color(db, inst,
                    "menu.hilite.textColor", &theme->menu_hilite_color))
        theme->menu_hilite_color = RrColorNew(inst, 0, 0, 0);

    if (read_mask(inst, "max.xbm", theme, &theme->max_unset_mask)) {
        if (!read_mask(inst, "max_toggled.xbm", theme, &theme->max_set_mask)) {
            theme->max_set_mask = RrPixmapMaskCopy(theme->max_unset_mask);
        }
        if (!read_mask(inst, "max_disabled.xbm", theme,
                       &theme->max_disabled_mask)) {
            theme->max_disabled_mask = RrPixmapMaskCopy(theme->max_unset_mask);
        } 
        if (!read_mask(inst, "max_hover.xbm", theme, &theme->max_hover_mask)) {
            theme->max_hover_mask = RrPixmapMaskCopy(theme->max_unset_mask);
        }
   } else {
        {
            char data[] = { 0x7f, 0x7f, 0x7f, 0x41, 0x41, 0x41, 0x7f };
            theme->max_unset_mask = RrPixmapMaskNew(inst, 7, 7, data);
        }
        {
            char data[] = { 0x7c, 0x44, 0x47, 0x47, 0x7f, 0x1f, 0x1f };
            theme->max_set_mask = RrPixmapMaskNew(inst, 7, 7, data);
        }
        theme->max_disabled_mask = RrPixmapMaskCopy(theme->max_unset_mask);
        theme->max_hover_mask = RrPixmapMaskCopy(theme->max_unset_mask);
    }

    if (read_mask(inst, "iconify.xbm", theme, &theme->iconify_mask)) {
        if (!read_mask(inst, "iconify_disabled.xbm", theme,
                       &theme->iconify_disabled_mask)) {
            theme->iconify_disabled_mask =
                RrPixmapMaskCopy(theme->iconify_mask);
        } 
        if (!read_mask(inst, "iconify_hover.xbm", theme,
                       &theme->iconify_hover_mask)) {
            theme->iconify_hover_mask =
                RrPixmapMaskCopy(theme->iconify_mask);
        }
    } else {
        {
            char data[] = { 0x00, 0x00, 0x00, 0x00, 0x7f, 0x7f, 0x7f };
            theme->iconify_mask = RrPixmapMaskNew(inst, 7, 7, data);
        }
        theme->iconify_disabled_mask = RrPixmapMaskCopy(theme->iconify_mask);
        theme->iconify_hover_mask = RrPixmapMaskCopy(theme->iconify_mask);
    }

    if (read_mask(inst, "stick.xbm", theme, &theme->desk_unset_mask)) {
        if (!read_mask(inst, "stick_toggled.xbm", theme,
                       &theme->desk_set_mask)) {
            theme->desk_set_mask =
                RrPixmapMaskCopy(theme->desk_unset_mask);
        }
    } else {
        {
            char data[] = { 0x63, 0x63, 0x00, 0x00, 0x00, 0x63, 0x63 };
            theme->desk_unset_mask = RrPixmapMaskNew(inst, 7, 7, data);
        }
        {
            char data[] = { 0x00, 0x36, 0x36, 0x08, 0x36, 0x36, 0x00 };
            theme->desk_set_mask = RrPixmapMaskNew(inst, 7, 7, data);
        }
    }

    if (read_mask(inst, "shade.xbm", theme, &theme->shade_unset_mask)) {
        if (!read_mask(inst, "shade_toggled.xbm", theme,
                       &theme->shade_set_mask)) {
            theme->shade_set_mask =
                RrPixmapMaskCopy(theme->shade_unset_mask);
        }
        if (!read_mask(inst, "shade_disabled.xbm", theme,
                       &theme->shade_disabled_mask)) {
            theme->shade_disabled_mask =
                RrPixmapMaskCopy(theme->shade_unset_mask);
        } 
        if (!read_mask(inst, "shade_hover.xbm", theme, 
                       &theme->shade_hover_mask)) {
            theme->shade_hover_mask =
                RrPixmapMaskCopy(theme->shade_unset_mask);
        }
    } else {
        {
            char data[] = { 0x7f, 0x7f, 0x7f, 0x00, 0x00, 0x00, 0x00 };
            theme->shade_unset_mask = RrPixmapMaskNew(inst, 7, 7, data);
        }
        {
            char data[] = { 0x7f, 0x7f, 0x7f, 0x00, 0x00, 0x00, 0x7f };
            theme->shade_set_mask = RrPixmapMaskNew(inst, 7, 7, data);
        }
        theme->shade_disabled_mask = RrPixmapMaskCopy(theme->shade_unset_mask);
        theme->shade_hover_mask = RrPixmapMaskCopy(theme->shade_unset_mask);
    }

    if (read_mask(inst, "close.xbm", theme, &theme->close_mask)) {
        if (!read_mask(inst, "close_disabled.xbm", theme,
                       &theme->close_disabled_mask)) {
            theme->close_disabled_mask = RrPixmapMaskCopy(theme->close_mask);
        } 
        if (!read_mask(inst, "close_hover.xbm", theme,
                       &theme->close_hover_mask)) {
            theme->close_hover_mask = RrPixmapMaskCopy(theme->close_mask);
        }
    } else {
        {
            char data[] = { 0x63, 0x77, 0x3e, 0x1c, 0x3e, 0x77, 0x63 };
            theme->close_mask = RrPixmapMaskNew(inst, 7, 7, data);
        }
        theme->close_disabled_mask = RrPixmapMaskCopy(theme->close_mask);
        theme->close_hover_mask = RrPixmapMaskCopy(theme->close_mask);
    }        

    /* read the decoration textures */
    if (!read_appearance(db, inst,
                         "window.title.focus", theme->a_focused_title,
                         FALSE))
	set_default_appearance(theme->a_focused_title);
    if (!read_appearance(db, inst,
                         "window.title.unfocus", theme->a_unfocused_title,
                         FALSE))
	set_default_appearance(theme->a_unfocused_title);
    if (!read_appearance(db, inst,
                         "window.label.focus", theme->a_focused_label,
                         TRUE))
	set_default_appearance(theme->a_focused_label);
    if (!read_appearance(db, inst,
                         "window.label.unfocus", theme->a_unfocused_label,
                         TRUE))
	set_default_appearance(theme->a_unfocused_label);
    if (!read_appearance(db, inst,
                         "window.handle.focus", theme->a_focused_handle,
                         FALSE))
	set_default_appearance(theme->a_focused_handle);
    if (!read_appearance(db, inst,
                         "window.handle.unfocus",theme->a_unfocused_handle,
                         FALSE))
	set_default_appearance(theme->a_unfocused_handle);
    if (!read_appearance(db, inst,
                         "window.grip.focus", theme->a_focused_grip,
                         TRUE))
	set_default_appearance(theme->a_focused_grip);
    if (!read_appearance(db, inst,
                         "window.grip.unfocus", theme->a_unfocused_grip,
                         TRUE))
	set_default_appearance(theme->a_unfocused_grip);
    if (!read_appearance(db, inst,
                         "menu.frame", theme->a_menu,
                         FALSE))
	set_default_appearance(theme->a_menu);
    if (!read_appearance(db, inst,
                         "menu.title", theme->a_menu_title,
                         FALSE))
	set_default_appearance(theme->a_menu_title);
    if (!read_appearance(db, inst,
                         "menu.hilite", theme->a_menu_hilite,
                         TRUE))
	set_default_appearance(theme->a_menu_hilite);

    /* read the appearances for rendering non-decorations */
    if (!read_appearance(db, inst,
                         "window.title.focus", theme->app_hilite_bg,
                         FALSE))
        set_default_appearance(theme->app_hilite_bg);
    if (!read_appearance(db, inst,
                         "window.label.focus", theme->app_hilite_label,
                         TRUE))
        set_default_appearance(theme->app_hilite_label);
    if (!read_appearance(db, inst,
                         "window.title.unfocus", theme->app_unhilite_bg,
                         FALSE))
        set_default_appearance(theme->app_unhilite_bg);
    if (!read_appearance(db, inst,
                         "window.label.unfocus", theme->app_unhilite_label,
                         TRUE))
        set_default_appearance(theme->app_unhilite_label);

    /* read buttons textures */
    if (!read_appearance(db, inst,
                         "window.button.disabled.focus",
			 theme->a_disabled_focused_max,
                         TRUE))
        set_default_appearance(theme->a_disabled_focused_max);
    if (!read_appearance(db, inst,
                         "window.button.disabled.unfocus",
			 theme->a_disabled_unfocused_max,
                         TRUE))
        set_default_appearance(theme->a_disabled_unfocused_max);
    if (!read_appearance(db, inst,
                         "window.button.pressed.focus",
			 theme->a_focused_pressed_max,
                         TRUE))
	if (!read_appearance(db, inst,
                             "window.button.pressed",
                             theme->a_focused_pressed_max,
                         TRUE))
	    set_default_appearance(theme->a_focused_pressed_max);
    if (!read_appearance(db, inst,
                         "window.button.pressed.unfocus",
			 theme->a_unfocused_pressed_max,
                         TRUE))
	if (!read_appearance(db, inst,
                             "window.button.pressed",
			     theme->a_unfocused_pressed_max,
                             TRUE))
	    set_default_appearance(theme->a_unfocused_pressed_max);
    if (!read_appearance(db, inst,
                         "window.button.focus",
			 theme->a_focused_unpressed_max,
                         TRUE))
	set_default_appearance(theme->a_focused_unpressed_max);
    if (!read_appearance(db, inst,
                         "window.button.unfocus",
			 theme->a_unfocused_unpressed_max,
                         TRUE))
	set_default_appearance(theme->a_unfocused_unpressed_max);
    if (!read_appearance(db, inst,
                         "window.button.hover.focus",
			 theme->a_hover_focused_max,
                         TRUE))
        theme->a_hover_focused_max =
            RrAppearanceCopy(theme->a_focused_unpressed_max);
    if (!read_appearance(db, inst,
                         "window.button.hover.unfocus",
			 theme->a_hover_unfocused_max,
                         TRUE))
        theme->a_hover_unfocused_max =
            RrAppearanceCopy(theme->a_unfocused_unpressed_max);

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
    theme->a_unfocused_unpressed_desk =
        RrAppearanceCopy(theme->a_unfocused_unpressed_max);
    theme->a_unfocused_pressed_desk =
        RrAppearanceCopy(theme->a_unfocused_pressed_max);
    theme->a_unfocused_pressed_set_desk =
        RrAppearanceCopy(theme->a_unfocused_pressed_max);
    theme->a_focused_unpressed_desk =
        RrAppearanceCopy(theme->a_focused_unpressed_max);
    theme->a_focused_pressed_desk =
        RrAppearanceCopy(theme->a_focused_pressed_max);
    theme->a_focused_pressed_set_desk =
        RrAppearanceCopy(theme->a_focused_pressed_max);
    theme->a_disabled_focused_shade =
        RrAppearanceCopy(theme->a_disabled_focused_max);
    theme->a_disabled_unfocused_shade =
        RrAppearanceCopy(theme->a_disabled_unfocused_max);
    theme->a_hover_focused_shade =
        RrAppearanceCopy(theme->a_hover_focused_max);
    theme->a_hover_unfocused_shade =
        RrAppearanceCopy(theme->a_hover_unfocused_max);
    theme->a_unfocused_unpressed_shade =
        RrAppearanceCopy(theme->a_unfocused_unpressed_max);
    theme->a_unfocused_pressed_shade =
        RrAppearanceCopy(theme->a_unfocused_pressed_max);
    theme->a_unfocused_pressed_set_shade =
        RrAppearanceCopy(theme->a_unfocused_pressed_max);
    theme->a_focused_unpressed_shade =
        RrAppearanceCopy(theme->a_focused_unpressed_max);
    theme->a_focused_pressed_shade =
        RrAppearanceCopy(theme->a_focused_pressed_max);
    theme->a_focused_pressed_set_shade =
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
    theme->a_unfocused_pressed_set_max =
        RrAppearanceCopy(theme->a_unfocused_pressed_max);
    theme->a_focused_pressed_set_max =
        RrAppearanceCopy(theme->a_focused_pressed_max);

    theme->a_icon->surface.grad = RR_SURFACE_PARENTREL;

    /* set up the textures */
    theme->a_focused_label->texture[0].type = 
        theme->app_hilite_label->texture[0].type = RR_TEXTURE_TEXT;
    theme->a_focused_label->texture[0].data.text.justify = winjust;
    theme->app_hilite_label->texture[0].data.text.justify = RR_JUSTIFY_LEFT;
    theme->a_focused_label->texture[0].data.text.font =
        theme->app_hilite_label->texture[0].data.text.font = theme->winfont;
    theme->a_focused_label->texture[0].data.text.color =
        theme->app_hilite_label->texture[0].data.text.color =
        theme->title_focused_color;

    theme->a_unfocused_label->texture[0].type =
        theme->app_unhilite_label->texture[0].type = RR_TEXTURE_TEXT;
    theme->a_unfocused_label->texture[0].data.text.justify = winjust;
    theme->app_unhilite_label->texture[0].data.text.justify = RR_JUSTIFY_LEFT;
    theme->a_unfocused_label->texture[0].data.text.font =
        theme->app_unhilite_label->texture[0].data.text.font = theme->winfont;
    theme->a_unfocused_label->texture[0].data.text.color =
        theme->app_unhilite_label->texture[0].data.text.color =
        theme->title_unfocused_color;

    theme->a_menu_title->texture[0].type = RR_TEXTURE_TEXT;
    theme->a_menu_title->texture[0].data.text.justify = mtitlejust;
    theme->a_menu_title->texture[0].data.text.font = theme->mtitlefont;
    theme->a_menu_title->texture[0].data.text.color = theme->menu_title_color;

    theme->a_menu_item->surface.grad = 
        theme->a_menu_disabled->surface.grad =
        theme->app_icon->surface.grad = RR_SURFACE_PARENTREL;

    theme->a_menu_item->texture[0].type =
        theme->a_menu_disabled->texture[0].type = 
        theme->a_menu_hilite->texture[0].type = RR_TEXTURE_TEXT;
    theme->a_menu_item->texture[0].data.text.justify = 
        theme->a_menu_disabled->texture[0].data.text.justify = 
        theme->a_menu_hilite->texture[0].data.text.justify = mjust;
    theme->a_menu_item->texture[0].data.text.font =
        theme->a_menu_disabled->texture[0].data.text.font =
        theme->a_menu_hilite->texture[0].data.text.font = theme->mfont;
    theme->a_menu_item->texture[0].data.text.color = theme->menu_color;
    theme->a_menu_disabled->texture[0].data.text.color =
        theme->menu_disabled_color;
    theme->a_menu_hilite->texture[0].data.text.color =
        theme->menu_hilite_color;

    theme->a_disabled_focused_max->texture[0].type = 
        theme->a_disabled_unfocused_max->texture[0].type = 
        theme->a_hover_focused_max->texture[0].type = 
        theme->a_hover_unfocused_max->texture[0].type = 
        theme->a_focused_unpressed_max->texture[0].type = 
        theme->a_focused_pressed_max->texture[0].type = 
        theme->a_focused_pressed_set_max->texture[0].type =  
        theme->a_unfocused_unpressed_max->texture[0].type = 
        theme->a_unfocused_pressed_max->texture[0].type = 
        theme->a_unfocused_pressed_set_max->texture[0].type = 
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
        theme->a_focused_unpressed_desk->texture[0].type = 
        theme->a_focused_pressed_desk->texture[0].type = 
        theme->a_focused_pressed_set_desk->texture[0].type = 
        theme->a_unfocused_unpressed_desk->texture[0].type = 
        theme->a_unfocused_pressed_desk->texture[0].type = 
        theme->a_unfocused_pressed_set_desk->texture[0].type = 
        theme->a_disabled_focused_shade->texture[0].type = 
        theme->a_disabled_unfocused_shade->texture[0].type = 
        theme->a_hover_focused_shade->texture[0].type = 
        theme->a_hover_unfocused_shade->texture[0].type = 
        theme->a_focused_unpressed_shade->texture[0].type = 
        theme->a_focused_pressed_shade->texture[0].type = 
        theme->a_focused_pressed_set_shade->texture[0].type = 
        theme->a_unfocused_unpressed_shade->texture[0].type = 
        theme->a_unfocused_pressed_shade->texture[0].type = 
        theme->a_unfocused_pressed_set_shade->texture[0].type = 
        theme->a_disabled_focused_iconify->texture[0].type = 
        theme->a_disabled_unfocused_iconify->texture[0].type = 
        theme->a_hover_focused_iconify->texture[0].type = 
        theme->a_hover_unfocused_iconify->texture[0].type = 
        theme->a_focused_unpressed_iconify->texture[0].type = 
        theme->a_focused_pressed_iconify->texture[0].type = 
        theme->a_unfocused_unpressed_iconify->texture[0].type = 
        theme->a_unfocused_pressed_iconify->texture[0].type = RR_TEXTURE_MASK;
    theme->a_disabled_focused_max->texture[0].data.mask.mask = 
        theme->a_disabled_unfocused_max->texture[0].data.mask.mask = 
        theme->max_disabled_mask;
    theme->a_hover_focused_max->texture[0].data.mask.mask = 
        theme->a_hover_unfocused_max->texture[0].data.mask.mask = 
        theme->max_hover_mask;
    theme->a_focused_unpressed_max->texture[0].data.mask.mask = 
        theme->a_unfocused_unpressed_max->texture[0].data.mask.mask = 
        theme->a_focused_pressed_max->texture[0].data.mask.mask = 
        theme->a_unfocused_pressed_max->texture[0].data.mask.mask =
        theme->max_unset_mask;
    theme->a_focused_pressed_set_max->texture[0].data.mask.mask = 
        theme->a_unfocused_pressed_set_max->texture[0].data.mask.mask =
        theme->max_set_mask;
    theme->a_disabled_focused_close->texture[0].data.mask.mask = 
        theme->a_disabled_unfocused_close->texture[0].data.mask.mask = 
        theme->close_disabled_mask;
    theme->a_hover_focused_close->texture[0].data.mask.mask = 
        theme->a_hover_unfocused_close->texture[0].data.mask.mask = 
        theme->close_hover_mask;
    theme->a_focused_pressed_close->texture[0].data.mask.mask = 
        theme->a_unfocused_pressed_close->texture[0].data.mask.mask =
        theme->a_focused_unpressed_close->texture[0].data.mask.mask = 
        theme->a_unfocused_unpressed_close->texture[0].data.mask.mask =
        theme->close_mask;
    theme->a_disabled_focused_desk->texture[0].data.mask.mask = 
        theme->a_disabled_unfocused_desk->texture[0].data.mask.mask = 
        theme->desk_disabled_mask;
    theme->a_hover_focused_desk->texture[0].data.mask.mask = 
        theme->a_hover_unfocused_desk->texture[0].data.mask.mask = 
        theme->desk_hover_mask;
    theme->a_focused_unpressed_desk->texture[0].data.mask.mask = 
        theme->a_unfocused_unpressed_desk->texture[0].data.mask.mask = 
        theme->a_focused_pressed_desk->texture[0].data.mask.mask = 
        theme->a_unfocused_pressed_desk->texture[0].data.mask.mask =
        theme->desk_unset_mask;
    theme->a_focused_pressed_set_desk->texture[0].data.mask.mask = 
        theme->a_unfocused_pressed_set_desk->texture[0].data.mask.mask =
        theme->desk_set_mask;
    theme->a_disabled_focused_shade->texture[0].data.mask.mask = 
        theme->a_disabled_unfocused_shade->texture[0].data.mask.mask = 
        theme->shade_disabled_mask;
    theme->a_hover_focused_shade->texture[0].data.mask.mask = 
        theme->a_hover_unfocused_shade->texture[0].data.mask.mask = 
        theme->shade_hover_mask;
    theme->a_focused_unpressed_shade->texture[0].data.mask.mask = 
        theme->a_unfocused_unpressed_shade->texture[0].data.mask.mask = 
        theme->a_focused_pressed_shade->texture[0].data.mask.mask = 
        theme->a_unfocused_pressed_shade->texture[0].data.mask.mask =
        theme->shade_unset_mask;
    theme->a_focused_pressed_set_shade->texture[0].data.mask.mask = 
        theme->a_unfocused_pressed_set_shade->texture[0].data.mask.mask =
        theme->shade_set_mask;
    theme->a_disabled_focused_iconify->texture[0].data.mask.mask = 
        theme->a_disabled_unfocused_iconify->texture[0].data.mask.mask = 
        theme->iconify_disabled_mask;
    theme->a_hover_focused_iconify->texture[0].data.mask.mask = 
        theme->a_hover_unfocused_iconify->texture[0].data.mask.mask = 
        theme->iconify_hover_mask;
    theme->a_focused_unpressed_iconify->texture[0].data.mask.mask = 
        theme->a_unfocused_unpressed_iconify->texture[0].data.mask.mask = 
        theme->a_focused_pressed_iconify->texture[0].data.mask.mask = 
        theme->a_unfocused_pressed_iconify->texture[0].data.mask.mask =
        theme->iconify_mask;
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
    theme->a_focused_unpressed_max->texture[0].data.mask.color = 
        theme->a_focused_unpressed_close->texture[0].data.mask.color = 
        theme->a_focused_unpressed_desk->texture[0].data.mask.color = 
        theme->a_focused_unpressed_shade->texture[0].data.mask.color = 
        theme->a_focused_unpressed_iconify->texture[0].data.mask.color = 
        theme->titlebut_focused_unpressed_color;
    theme->a_focused_pressed_max->texture[0].data.mask.color = 
        theme->a_focused_pressed_set_max->texture[0].data.mask.color = 
        theme->a_focused_pressed_close->texture[0].data.mask.color = 
        theme->a_focused_pressed_desk->texture[0].data.mask.color = 
        theme->a_focused_pressed_set_desk->texture[0].data.mask.color = 
        theme->a_focused_pressed_shade->texture[0].data.mask.color = 
        theme->a_focused_pressed_set_shade->texture[0].data.mask.color = 
        theme->a_focused_pressed_iconify->texture[0].data.mask.color =
        theme->titlebut_focused_pressed_color;
    theme->a_unfocused_unpressed_max->texture[0].data.mask.color = 
        theme->a_unfocused_unpressed_close->texture[0].data.mask.color = 
        theme->a_unfocused_unpressed_desk->texture[0].data.mask.color = 
        theme->a_unfocused_unpressed_shade->texture[0].data.mask.color = 
        theme->a_unfocused_unpressed_iconify->texture[0].data.mask.color = 
        theme->titlebut_unfocused_unpressed_color;
        theme->a_unfocused_pressed_max->texture[0].data.mask.color = 
        theme->a_unfocused_pressed_set_max->texture[0].data.mask.color = 
        theme->a_unfocused_pressed_close->texture[0].data.mask.color = 
        theme->a_unfocused_pressed_desk->texture[0].data.mask.color = 
        theme->a_unfocused_pressed_set_desk->texture[0].data.mask.color = 
        theme->a_unfocused_pressed_shade->texture[0].data.mask.color = 
        theme->a_unfocused_pressed_set_shade->texture[0].data.mask.color = 
        theme->a_unfocused_pressed_iconify->texture[0].data.mask.color =
        theme->titlebut_unfocused_pressed_color;

    XrmDestroyDatabase(db);

    theme->label_height = theme->winfont_height;
    theme->title_height = theme->label_height + theme->bevel * 2;
    theme->button_size = theme->label_height - 2;
    theme->grip_width = theme->button_size * 2;

    return theme;
}

void RrThemeFree(RrTheme *theme)
{
    if (theme) {
        g_free(theme->name);

        RrColorFree(theme->b_color);
        RrColorFree(theme->cb_unfocused_color);
        RrColorFree(theme->cb_focused_color);
        RrColorFree(theme->title_unfocused_color);
        RrColorFree(theme->title_focused_color);
        RrColorFree(theme->titlebut_disabled_focused_color);
        RrColorFree(theme->titlebut_disabled_unfocused_color);
        RrColorFree(theme->titlebut_hover_focused_color);
        RrColorFree(theme->titlebut_hover_unfocused_color);
        RrColorFree(theme->titlebut_unfocused_pressed_color);
        RrColorFree(theme->titlebut_focused_pressed_color);
        RrColorFree(theme->titlebut_unfocused_unpressed_color);
        RrColorFree(theme->titlebut_focused_unpressed_color);
        RrColorFree(theme->menu_color);
        RrColorFree(theme->menu_title_color);
        RrColorFree(theme->menu_disabled_color);
        RrColorFree(theme->menu_hilite_color);

        RrPixmapMaskFree(theme->max_set_mask);
        RrPixmapMaskFree(theme->max_unset_mask);
        RrPixmapMaskFree(theme->max_disabled_mask);
        RrPixmapMaskFree(theme->max_hover_mask);
        RrPixmapMaskFree(theme->desk_set_mask);
        RrPixmapMaskFree(theme->desk_unset_mask);
        RrPixmapMaskFree(theme->desk_disabled_mask);
        RrPixmapMaskFree(theme->desk_hover_mask);
        RrPixmapMaskFree(theme->shade_set_mask);
        RrPixmapMaskFree(theme->shade_unset_mask);
        RrPixmapMaskFree(theme->shade_disabled_mask);
        RrPixmapMaskFree(theme->shade_hover_mask);
        RrPixmapMaskFree(theme->iconify_mask);
        RrPixmapMaskFree(theme->iconify_disabled_mask);
        RrPixmapMaskFree(theme->iconify_hover_mask);
        RrPixmapMaskFree(theme->close_mask);
        RrPixmapMaskFree(theme->close_disabled_mask);
        RrPixmapMaskFree(theme->close_hover_mask);

        RrFontClose(theme->winfont);
        RrFontClose(theme->mtitlefont);
        RrFontClose(theme->mfont);

        RrAppearanceFree(theme->a_disabled_focused_max);
        RrAppearanceFree(theme->a_disabled_unfocused_max);
        RrAppearanceFree(theme->a_hover_focused_max);
        RrAppearanceFree(theme->a_hover_unfocused_max);
        RrAppearanceFree(theme->a_focused_unpressed_max);
        RrAppearanceFree(theme->a_focused_pressed_max);
        RrAppearanceFree(theme->a_focused_pressed_set_max);
        RrAppearanceFree(theme->a_unfocused_unpressed_max);
        RrAppearanceFree(theme->a_unfocused_pressed_max);
        RrAppearanceFree(theme->a_unfocused_pressed_set_max);
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
        RrAppearanceFree(theme->a_disabled_focused_shade);
        RrAppearanceFree(theme->a_disabled_unfocused_shade);
        RrAppearanceFree(theme->a_hover_focused_shade);
        RrAppearanceFree(theme->a_hover_unfocused_shade);
        RrAppearanceFree(theme->a_focused_unpressed_shade);
        RrAppearanceFree(theme->a_focused_pressed_shade);
        RrAppearanceFree(theme->a_unfocused_unpressed_shade);
        RrAppearanceFree(theme->a_unfocused_pressed_shade);
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
        RrAppearanceFree(theme->a_menu_item);
        RrAppearanceFree(theme->a_menu_disabled);
        RrAppearanceFree(theme->a_menu_hilite);
        RrAppearanceFree(theme->app_hilite_bg);
        RrAppearanceFree(theme->app_unhilite_bg);
        RrAppearanceFree(theme->app_hilite_label);
        RrAppearanceFree(theme->app_unhilite_label);
        RrAppearanceFree(theme->app_icon);
    }
}

static XrmDatabase loaddb(RrTheme *theme, char *name)
{
    XrmDatabase db;

    char *s = g_build_filename(name, "themerc", NULL);
    if ((db = XrmGetFileDatabase(s)))
        theme->path = g_path_get_dirname(s);
    g_free(s);
    if (db == NULL) {
	char *s = g_build_filename(g_get_home_dir(), ".openbox", "themes",
				   name, "themerc", NULL);
	if ((db = XrmGetFileDatabase(s)))
            theme->path = g_path_get_dirname(s);
	g_free(s);
    }
    if (db == NULL) {
	char *s = g_build_filename(THEMEDIR, name, "themerc", NULL);
	if ((db = XrmGetFileDatabase(s)))
            theme->path = g_path_get_dirname(s);
        g_free(s);
    }

    return db;
}

static char *create_class_name(char *rname)
{
    char *rclass = g_strdup(rname);
    char *p = rclass;

    while (TRUE) {
	*p = toupper(*p);
	p = strchr(p+1, '.');
	if (p == NULL) break;
	++p;
	if (*p == '\0') break;
    }
    return rclass;
}

static gboolean read_int(XrmDatabase db, char *rname, int *value)
{
    gboolean ret = FALSE;
    char *rclass = create_class_name(rname);
    char *rettype, *end;
    XrmValue retvalue;
  
    if (XrmGetResource(db, rname, rclass, &rettype, &retvalue) &&
	retvalue.addr != NULL) {
	*value = (int)strtol(retvalue.addr, &end, 10);
	if (end != retvalue.addr)
	    ret = TRUE;
    }

    g_free(rclass);
    return ret;
}

static gboolean read_string(XrmDatabase db, char *rname, char **value)
{
    gboolean ret = FALSE;
    char *rclass = create_class_name(rname);
    char *rettype;
    XrmValue retvalue;
  
    if (XrmGetResource(db, rname, rclass, &rettype, &retvalue) &&
	retvalue.addr != NULL) {
	*value = retvalue.addr;
	ret = TRUE;
    }

    g_free(rclass);
    return ret;
}

static gboolean read_color(XrmDatabase db, const RrInstance *inst,
                           gchar *rname, RrColor **value)
{
    gboolean ret = FALSE;
    char *rclass = create_class_name(rname);
    char *rettype;
    XrmValue retvalue;
  
    if (XrmGetResource(db, rname, rclass, &rettype, &retvalue) &&
	retvalue.addr != NULL) {
	RrColor *c = RrColorParse(inst, retvalue.addr);
	if (c != NULL) {
	    *value = c;
	    ret = TRUE;
	}
    }

    g_free(rclass);
    return ret;
}

static gboolean read_mask(const RrInstance *inst,
                          gchar *maskname, RrTheme *theme,
                          RrPixmapMask **value)
{
    gboolean ret = FALSE;
    char *s;
    int hx, hy; /* ignored */
    unsigned int w, h;
    unsigned char *b;

    s = g_build_filename(g_get_home_dir(), ".openbox", "themes",
                         theme->name, maskname, NULL);
    if (XReadBitmapFileData(s, &w, &h, &b, &hx, &hy) == BitmapSuccess)
        ret = TRUE;
    else {
        g_free(s);
        s = g_build_filename(THEMEDIR, theme->name, maskname, NULL);
        if (XReadBitmapFileData(s, &w, &h, &b, &hx, &hy) == BitmapSuccess) 
            ret = TRUE;
        else {
            g_free(s);
            s = g_build_filename(theme->path, maskname, NULL);
            if (XReadBitmapFileData(s, &w, &h, &b, &hx, &hy) == BitmapSuccess) 
                ret = TRUE;
        }
    }

    if (ret) {
        *value = RrPixmapMaskNew(inst, w, h, (char*)b);
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
    char *t;

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
	    else if (strstr(tex, "horizontal") != NULL)
		*grad = RR_SURFACE_HORIZONTAL;
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


static gboolean read_appearance(XrmDatabase db, const RrInstance *inst,
                                gchar *rname, RrAppearance *value,
                                gboolean allow_trans)
{
    gboolean ret = FALSE;
    char *rclass = create_class_name(rname), *cname, *ctoname, *bcname;
    char *rettype;
    XrmValue retvalue;

    cname = g_strconcat(rname, ".color", NULL);
    ctoname = g_strconcat(rname, ".colorTo", NULL);
    bcname = g_strconcat(rname, ".borderColor", NULL);

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
	ret = TRUE;
    }

    g_free(bcname);
    g_free(ctoname);
    g_free(cname);
    g_free(rclass);
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
