#include "render.h"
#include "color.h"
#include "font.h"
#include "mask.h"

#include <X11/Xlib.h>
#include <X11/Xresource.h>

/* style settings - geometry */
gint theme_bevel;
gint theme_handle_height;
gint theme_bwidth;
gint theme_cbwidth;
/* style settings - colors */
color_rgb *theme_b_color;
color_rgb *theme_cb_focused_color;
color_rgb *theme_cb_unfocused_color;
color_rgb *theme_title_focused_color;
color_rgb *theme_title_unfocused_color;
color_rgb *theme_titlebut_focused_color;
color_rgb *theme_titlebut_unfocused_color;
color_rgb *theme_menu_title_color;
color_rgb *theme_menu_color;
color_rgb *theme_menu_disabled_color;
color_rgb *theme_menu_hilite_color;
/* style settings - fonts */
gint theme_winfont_height;
RrFont *theme_winfont;
gboolean theme_winfont_shadow;
gint theme_winfont_shadow_offset;
gint theme_winfont_shadow_tint;
gint theme_mtitlefont_height;
RrFont *theme_mtitlefont;
gboolean theme_mtitlefont_shadow;
gint theme_mtitlefont_shadow_offset;
gint theme_mtitlefont_shadow_tint;
gint theme_mfont_height;
RrFont *theme_mfont;
gboolean theme_mfont_shadow;
gint theme_mfont_shadow_offset;
gint theme_mfont_shadow_tint;
/* style settings - title layout */
gchar *theme_title_layout;
/* style settings - masks */
RrPixmapMask *theme_max_set_mask;
RrPixmapMask *theme_max_unset_mask;
RrPixmapMask *theme_iconify_mask;
RrPixmapMask *theme_desk_set_mask;
RrPixmapMask *theme_desk_unset_mask;
RrPixmapMask *theme_shade_set_mask;
RrPixmapMask *theme_shade_unset_mask;
RrPixmapMask *theme_close_mask;

/* global appearances */
RrAppearance *theme_a_focused_unpressed_max;
RrAppearance *theme_a_focused_pressed_max;
RrAppearance *theme_a_focused_pressed_set_max;
RrAppearance *theme_a_unfocused_unpressed_max;
RrAppearance *theme_a_unfocused_pressed_max;
RrAppearance *theme_a_unfocused_pressed_set_max;
RrAppearance *theme_a_focused_unpressed_close;
RrAppearance *theme_a_focused_pressed_close;
RrAppearance *theme_a_unfocused_unpressed_close;
RrAppearance *theme_a_unfocused_pressed_close;
RrAppearance *theme_a_focused_unpressed_desk;
RrAppearance *theme_a_focused_pressed_desk;
RrAppearance *theme_a_focused_pressed_set_desk;
RrAppearance *theme_a_unfocused_unpressed_desk;
RrAppearance *theme_a_unfocused_pressed_desk;
RrAppearance *theme_a_unfocused_pressed_set_desk;
RrAppearance *theme_a_focused_unpressed_shade;
RrAppearance *theme_a_focused_pressed_shade;
RrAppearance *theme_a_focused_pressed_set_shade;
RrAppearance *theme_a_unfocused_unpressed_shade;
RrAppearance *theme_a_unfocused_pressed_shade;
RrAppearance *theme_a_unfocused_pressed_set_shade;
RrAppearance *theme_a_focused_unpressed_iconify;
RrAppearance *theme_a_focused_pressed_iconify;
RrAppearance *theme_a_unfocused_unpressed_iconify;
RrAppearance *theme_a_unfocused_pressed_iconify;
RrAppearance *theme_a_focused_grip;
RrAppearance *theme_a_unfocused_grip;
RrAppearance *theme_a_focused_title;
RrAppearance *theme_a_unfocused_title;
RrAppearance *theme_a_focused_label;
RrAppearance *theme_a_unfocused_label;
RrAppearance *theme_a_icon; /* always parentrelative, so no focused/unfocused */
RrAppearance *theme_a_focused_handle;
RrAppearance *theme_a_unfocused_handle;
RrAppearance *theme_a_menu_title;
RrAppearance *theme_a_menu;
RrAppearance *theme_a_menu_item;
RrAppearance *theme_a_menu_disabled;
RrAppearance *theme_a_menu_hilite;

RrAppearance *theme_app_hilite_bg;
RrAppearance *theme_app_unhilite_bg;
RrAppearance *theme_app_hilite_label;
RrAppearance *theme_app_unhilite_label;
RrAppearance *theme_app_icon;

static const RrInstance *theme_inst = NULL;

void theme_startup(const RrInstance *inst)
{
    theme_inst = inst;

    theme_b_color = theme_cb_unfocused_color = theme_cb_focused_color = 
        theme_title_unfocused_color = theme_title_focused_color = 
        theme_titlebut_unfocused_color = theme_titlebut_focused_color = 
        theme_menu_color = theme_menu_title_color = theme_menu_disabled_color =
        theme_menu_hilite_color = NULL;
    theme_winfont = theme_mtitlefont = theme_mfont = NULL;
    theme_title_layout = NULL;
    theme_max_set_mask = theme_max_unset_mask = NULL;
    theme_desk_set_mask = theme_desk_unset_mask = NULL;
    theme_shade_set_mask = theme_shade_unset_mask = NULL;
    theme_iconify_mask = theme_close_mask = NULL;

    theme_a_focused_unpressed_max = RrAppearanceNew(inst, 1);
    theme_a_focused_pressed_max = RrAppearanceNew(inst, 1);
    theme_a_focused_pressed_set_max = RrAppearanceNew(inst, 1);
    theme_a_unfocused_unpressed_max = RrAppearanceNew(inst, 1);
    theme_a_unfocused_pressed_max = RrAppearanceNew(inst, 1);
    theme_a_unfocused_pressed_set_max = RrAppearanceNew(inst, 1);
    theme_a_focused_unpressed_close = NULL;
    theme_a_focused_pressed_close = NULL;
    theme_a_unfocused_unpressed_close = NULL;
    theme_a_unfocused_pressed_close = NULL;
    theme_a_focused_unpressed_desk = NULL;
    theme_a_focused_pressed_desk = NULL;
    theme_a_focused_pressed_set_desk = NULL;
    theme_a_unfocused_unpressed_desk = NULL;
    theme_a_unfocused_pressed_desk = NULL;
    theme_a_unfocused_pressed_set_desk = NULL;
    theme_a_focused_unpressed_shade = NULL;
    theme_a_focused_pressed_shade = NULL;
    theme_a_focused_pressed_set_shade = NULL;
    theme_a_unfocused_unpressed_shade = NULL;
    theme_a_unfocused_pressed_shade = NULL;
    theme_a_unfocused_pressed_set_shade = NULL;
    theme_a_focused_unpressed_iconify = NULL;
    theme_a_focused_pressed_iconify = NULL;
    theme_a_unfocused_unpressed_iconify = NULL;
    theme_a_unfocused_pressed_iconify = NULL;
    theme_a_focused_grip = RrAppearanceNew(inst, 0);
    theme_a_unfocused_grip = RrAppearanceNew(inst, 0);
    theme_a_focused_title = RrAppearanceNew(inst, 0);
    theme_a_unfocused_title = RrAppearanceNew(inst, 0);
    theme_a_focused_label = RrAppearanceNew(inst, 1);
    theme_a_unfocused_label = RrAppearanceNew(inst, 1);
    theme_a_icon = RrAppearanceNew(inst, 1);
    theme_a_focused_handle = RrAppearanceNew(inst, 0);
    theme_a_unfocused_handle = RrAppearanceNew(inst, 0);
    theme_a_menu = RrAppearanceNew(inst, 0);
    theme_a_menu_title = RrAppearanceNew(inst, 1);
    theme_a_menu_item = RrAppearanceNew(inst, 1);
    theme_a_menu_disabled = RrAppearanceNew(inst, 1);
    theme_a_menu_hilite = RrAppearanceNew(inst, 1);

    theme_app_hilite_bg = RrAppearanceNew(inst, 0);
    theme_app_unhilite_bg = RrAppearanceNew(inst, 0);
    theme_app_hilite_label = RrAppearanceNew(inst, 1);
    theme_app_unhilite_label = RrAppearanceNew(inst, 1);
    theme_app_icon = RrAppearanceNew(inst, 1);

}

void theme_shutdown()
{
    RrColorFree(theme_b_color);
    RrColorFree(theme_cb_unfocused_color);
    RrColorFree(theme_cb_focused_color);
    RrColorFree(theme_title_unfocused_color);
    RrColorFree(theme_title_focused_color);
    RrColorFree(theme_titlebut_unfocused_color);
    RrColorFree(theme_titlebut_focused_color);
    RrColorFree(theme_menu_color);
    RrColorFree(theme_menu_title_color);
    RrColorFree(theme_menu_disabled_color);
    RrColorFree(theme_menu_hilite_color);

    RrPixmapMaskFree(theme_max_set_mask);
    RrPixmapMaskFree(theme_max_unset_mask);
    RrPixmapMaskFree(theme_desk_set_mask);
    RrPixmapMaskFree(theme_desk_unset_mask);
    RrPixmapMaskFree(theme_shade_set_mask);
    RrPixmapMaskFree(theme_shade_unset_mask);
    RrPixmapMaskFree(theme_iconify_mask);
    RrPixmapMaskFree(theme_close_mask);

    font_close(theme_winfont);
    font_close(theme_mtitlefont);
    font_close(theme_mfont);

    g_free(theme_title_layout);

    RrAppearanceFree(theme_a_focused_unpressed_max);
    RrAppearanceFree(theme_a_focused_pressed_max);
    RrAppearanceFree(theme_a_focused_pressed_set_max);
    RrAppearanceFree(theme_a_unfocused_unpressed_max);
    RrAppearanceFree(theme_a_unfocused_pressed_max);
    RrAppearanceFree(theme_a_unfocused_pressed_set_max);
    RrAppearanceFree(theme_a_focused_unpressed_close);
    RrAppearanceFree(theme_a_focused_pressed_close);
    RrAppearanceFree(theme_a_unfocused_unpressed_close);
    RrAppearanceFree(theme_a_unfocused_pressed_close);
    RrAppearanceFree(theme_a_focused_unpressed_desk);
    RrAppearanceFree(theme_a_focused_pressed_desk);
    RrAppearanceFree(theme_a_unfocused_unpressed_desk);
    RrAppearanceFree(theme_a_unfocused_pressed_desk);
    RrAppearanceFree(theme_a_focused_unpressed_shade);
    RrAppearanceFree(theme_a_focused_pressed_shade);
    RrAppearanceFree(theme_a_unfocused_unpressed_shade);
    RrAppearanceFree(theme_a_unfocused_pressed_shade);
    RrAppearanceFree(theme_a_focused_unpressed_iconify);
    RrAppearanceFree(theme_a_focused_pressed_iconify);
    RrAppearanceFree(theme_a_unfocused_unpressed_iconify);
    RrAppearanceFree(theme_a_unfocused_pressed_iconify);
    RrAppearanceFree(theme_a_focused_grip);
    RrAppearanceFree(theme_a_unfocused_grip);
    RrAppearanceFree(theme_a_focused_title);
    RrAppearanceFree(theme_a_unfocused_title);
    RrAppearanceFree(theme_a_focused_label);
    RrAppearanceFree(theme_a_unfocused_label);
    RrAppearanceFree(theme_a_icon);
    RrAppearanceFree(theme_a_focused_handle);
    RrAppearanceFree(theme_a_unfocused_handle);
    RrAppearanceFree(theme_a_menu);
    RrAppearanceFree(theme_a_menu_title);
    RrAppearanceFree(theme_a_menu_item);
    RrAppearanceFree(theme_a_menu_disabled);
    RrAppearanceFree(theme_a_menu_hilite);
    RrAppearanceFree(theme_app_hilite_bg);
    RrAppearanceFree(theme_app_unhilite_bg);
    RrAppearanceFree(theme_app_hilite_label);
    RrAppearanceFree(theme_app_unhilite_label);
    RrAppearanceFree(theme_app_icon);
}

static XrmDatabase loaddb(char *theme)
{
    XrmDatabase db;

    db = XrmGetFileDatabase(theme);
    if (db == NULL) {
	char *s = g_build_filename(g_get_home_dir(), ".openbox", "themes",
				   theme, NULL);
	db = XrmGetFileDatabase(s);
	g_free(s);
    }
    if (db == NULL) {
	char *s = g_build_filename(THEMEDIR, theme, NULL);
	db = XrmGetFileDatabase(s);
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
	*value = g_strdup(retvalue.addr);
	ret = TRUE;
    }

    g_free(rclass);
    return ret;
}

static gboolean read_color(XrmDatabase db, const RrInstance *inst,
                           gchar *rname, color_rgb **value)
{
    gboolean ret = FALSE;
    char *rclass = create_class_name(rname);
    char *rettype;
    XrmValue retvalue;
  
    if (XrmGetResource(db, rname, rclass, &rettype, &retvalue) &&
	retvalue.addr != NULL) {
	color_rgb *c = RrColorParse(inst, retvalue.addr);
	if (c != NULL) {
	    *value = c;
	    ret = TRUE;
	}
    }

    g_free(rclass);
    return ret;
}

static gboolean read_mask(XrmDatabase db, const RrInstance *inst,
                           gchar *rname, gchar *theme,
                          RrPixmapMask **value)
{
    gboolean ret = FALSE;
    char *rclass = create_class_name(rname);
    char *rettype;
    char *s;
    char *button_dir;
    XrmValue retvalue;
    int hx, hy; /* ignored */
    unsigned int w, h;
    unsigned char *b;
  
    if (XrmGetResource(db, rname, rclass, &rettype, &retvalue) &&
        retvalue.addr != NULL) {

	button_dir = g_strdup_printf("%s_data", theme);

        s = g_build_filename(g_get_home_dir(), ".openbox", "themes",
                             button_dir, retvalue.addr, NULL);

        if (XReadBitmapFileData(s, &w, &h, &b, &hx, &hy) == BitmapSuccess)
            ret = TRUE;
        else {
            g_free(s);
            s = g_build_filename(THEMEDIR, button_dir, retvalue.addr, NULL);
	
            if (XReadBitmapFileData(s, &w, &h, &b, &hx, &hy) == BitmapSuccess) 
                ret = TRUE;
            else {
                char *themename;

                g_free(s);
                themename = g_path_get_basename(theme);
                s = g_strdup_printf("%s/%s_data/%s", theme,
                                    themename, retvalue.addr);
                g_free(themename);
                if (XReadBitmapFileData(s, &w, &h, &b, &hx, &hy) ==
                    BitmapSuccess) 
                    ret = TRUE;
                else
                    g_message("Unable to find bitmap '%s'", retvalue.addr);
            }
        }

        if (ret) {
            *value = RrPixmapMaskNew(inst, w, h, (char*)b);
            XFree(b);
        }
      
        g_free(s);
        g_free(button_dir);
    }

    g_free(rclass);
    return ret;
}

static void parse_appearance(gchar *tex, RrSurfaceColorType *grad,
                             RrReliefType *relief, RrBevelType *bevel,
                             gboolean *interlaced, gboolean *border)
{
    char *t;

    /* convert to all lowercase */
    for (t = tex; *t != '\0'; ++t)
	*t = g_ascii_tolower(*t);

    if (strstr(tex, "parentrelative") != NULL) {
	*grad = RR_SURFACE_PARENTREL;
    } else {
	if (strstr(tex, "gradient") != NULL) {
	    if (strstr(tex, "crossdiagonal") != NULL)
		*grad = RR_SURFACE_CROSS_DIAGONAL;
	    else if (strstr(tex, "rectangle") != NULL)
		*grad = RR_SURFACE_RECTANGLE;
	    else if (strstr(tex, "pyramid") != NULL)
		*grad = RR_SURFACE_PYRAMID;
	    else if (strstr(tex, "pipecross") != NULL)
		*grad = RR_SURFACE_PIPECROSS;
	    else if (strstr(tex, "elliptic") != NULL)
		*grad = RR_SURFACE_PIPECROSS;
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
                           gchar *rname, RrAppearance *value)
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
			 &value->surface.border);
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

gchar *theme_load(gchar *theme)
{
    XrmDatabase db = NULL;
    gchar *loaded = NULL;
    RrJustify winjust, mtitlejust, mjust;
    gchar *str;
    gchar *font_str;
    const RrInstance *inst = theme_inst;

    if (theme) {
	db = loaddb(theme);
        if (db == NULL) {
	    g_warning("Failed to load the theme '%s'", theme);
	    g_message("Falling back to the default: '%s'", DEFAULT_THEME);
	} else
            loaded = g_strdup(theme);
    }
    if (db == NULL) {
	db = loaddb(DEFAULT_THEME);
	if (db == NULL) {
	    g_warning("Failed to load the theme '%s'.", DEFAULT_THEME);
	    return NULL;
	} else
            loaded = g_strdup(DEFAULT_THEME);
    }

    /* load the font stuff */
    font_str = "arial:bold:pixelsize=10";

    theme_winfont_shadow = FALSE;
    if (read_string(db, "window.xft.flags", &str)) {
        if (g_strrstr(str, "shadow"))
            theme_winfont_shadow = TRUE;
        g_free(str);
    }
 
    if (!read_int(db, "window.xft.shadow.offset",
                  &theme_winfont_shadow_offset))
        theme_winfont_shadow_offset = 1;
    if (!read_int(db, "window.xft.shadow.tint",
                  &theme_winfont_shadow_tint) ||
        theme_winfont_shadow_tint < 100 || theme_winfont_shadow_tint > 100)
        theme_winfont_shadow_tint = 25;

    theme_winfont = font_open(font_str);
    theme_winfont_height = font_height(theme_winfont, theme_winfont_shadow,
                                       theme_winfont_shadow_offset);

    winjust = RR_JUSTIFY_LEFT;
    if (read_string(db, "window.justify", &str)) {
        if (!g_ascii_strcasecmp(str, "right"))
            winjust = RR_JUSTIFY_RIGHT;
        else if (!g_ascii_strcasecmp(str, "center"))
            winjust = RR_JUSTIFY_CENTER;
        g_free(str);
    }

    font_str = "arial-10:bold";

    theme_mtitlefont_shadow = FALSE;
    if (read_string(db, "menu.title.xft.flags", &str)) {
        if (g_strrstr(str, "shadow"))
            theme_mtitlefont_shadow = TRUE;
        g_free(str);
    }
 
    if (!read_int(db, "menu.title.xft.shadow.offset",
                  &theme_mtitlefont_shadow_offset))
        theme_mtitlefont_shadow_offset = 1;
    if (!read_int(db, "menu.title.xft.shadow.tint",
                  &theme_mtitlefont_shadow_tint) ||
        theme_mtitlefont_shadow_tint < 100 ||
        theme_mtitlefont_shadow_tint > 100)
        theme_mtitlefont_shadow_tint = 25;

    theme_mtitlefont = font_open(font_str);
    theme_mtitlefont_height = font_height(theme_mtitlefont,
                                          theme_mtitlefont_shadow,
                                          theme_mtitlefont_shadow_offset);

    mtitlejust = RR_JUSTIFY_LEFT;
    if (read_string(db, "menu.title.justify", &str)) {
        if (!g_ascii_strcasecmp(str, "right"))
            mtitlejust = RR_JUSTIFY_RIGHT;
        else if (!g_ascii_strcasecmp(str, "center"))
            mtitlejust = RR_JUSTIFY_CENTER;
        g_free(str);
    }

    font_str = "arial-8";

    theme_mfont_shadow = FALSE;
    if (read_string(db, "menu.frame.xft.flags", &str)) {
        if (g_strrstr(str, "shadow"))
            theme_mfont_shadow = TRUE;
        g_free(str);
    }
 
    if (!read_int(db, "menu.frame.xft.shadow.offset",
                  &theme_mfont_shadow_offset))
        theme_mfont_shadow_offset = 1;
    if (!read_int(db, "menu.frame.xft.shadow.tint",
                  &theme_mfont_shadow_tint) ||
        theme_mfont_shadow_tint < 100 ||
        theme_mfont_shadow_tint > 100)
        theme_mfont_shadow_tint = 25;

    theme_mfont = font_open(font_str);
    theme_mfont_height = font_height(theme_mfont, theme_mfont_shadow,
                                     theme_mfont_shadow_offset);

    mjust = RR_JUSTIFY_LEFT;
    if (read_string(db, "menu.frame.justify", &str)) {
        if (!g_ascii_strcasecmp(str, "right"))
            mjust = RR_JUSTIFY_RIGHT;
        else if (!g_ascii_strcasecmp(str, "center"))
            mjust = RR_JUSTIFY_CENTER;
        g_free(str);
    }

    /* load the title layout */
    theme_title_layout = g_strdup("NLIMC");

    if (!read_int(db, "handleWidth", &theme_handle_height) ||
	theme_handle_height < 0 || theme_handle_height > 100)
        theme_handle_height = 6;
    if (!read_int(db, "bevelWidth", &theme_bevel) ||
	theme_bevel <= 0 || theme_bevel > 100) theme_bevel = 3;
    if (!read_int(db, "borderWidth", &theme_bwidth) ||
	theme_bwidth < 0 || theme_bwidth > 100) theme_bwidth = 1;
    if (!read_int(db, "frameWidth", &theme_cbwidth) ||
	theme_cbwidth < 0 || theme_cbwidth > 100) theme_cbwidth = theme_bevel;

    /* load colors */
    if (!read_color(db, inst,
                    "borderColor", &theme_b_color))
	theme_b_color = RrColorNew(inst, 0, 0, 0);
    if (!read_color(db, inst,
                    "window.frame.focusColor", &theme_cb_focused_color))
	theme_cb_focused_color = RrColorNew(inst, 0xff, 0xff, 0xff);
    if (!read_color(db, inst,
                    "window.frame.unfocusColor",&theme_cb_unfocused_color))
	theme_cb_unfocused_color = RrColorNew(inst, 0xff, 0xff, 0xff);
    if (!read_color(db, inst,
                    "window.label.focus.textColor",
                    &theme_title_focused_color))
	theme_title_focused_color = RrColorNew(inst, 0x0, 0x0, 0x0);
    if (!read_color(db, inst,
                    "window.label.unfocus.textColor",
                    &theme_title_unfocused_color))
	theme_title_unfocused_color = RrColorNew(inst, 0xff, 0xff, 0xff);
    if (!read_color(db, inst,
                    "window.button.focus.picColor",
                    &theme_titlebut_focused_color))
	theme_titlebut_focused_color = RrColorNew(inst, 0, 0, 0);
    if (!read_color(db, inst,
                    "window.button.unfocus.picColor",
                    &theme_titlebut_unfocused_color))
	theme_titlebut_unfocused_color = RrColorNew(inst, 0xff, 0xff, 0xff);
    if (!read_color(db, inst,
                    "menu.title.textColor", &theme_menu_title_color))
        theme_menu_title_color = RrColorNew(inst, 0, 0, 0);
    if (!read_color(db, inst,
                    "menu.frame.textColor", &theme_menu_color))
        theme_menu_color = RrColorNew(inst, 0xff, 0xff, 0xff);
    if (!read_color(db, inst,
                    "menu.frame.disableColor", &theme_menu_disabled_color))
        theme_menu_disabled_color = RrColorNew(inst, 0, 0, 0);
    if (!read_color(db, inst,
                    "menu.hilite.textColor", &theme_menu_hilite_color))
        theme_menu_hilite_color = RrColorNew(inst, 0, 0, 0);

    if (read_mask(db, inst,
                  "window.button.max.mask", theme, &theme_max_unset_mask)){
        if (!read_mask(db, inst,
                       "window.button.max.toggled.mask", theme,
                       &theme_max_set_mask)) {
            theme_max_set_mask = RrPixmapMaskCopy(theme_max_unset_mask);
        }
    } else {
        {
            char data[] = { 0x7f, 0x7f, 0x7f, 0x41, 0x41, 0x41, 0x7f };
            theme_max_unset_mask = RrPixmapMaskNew(inst, 7, 7, data);
        }
        {
            char data[] = { 0x7c, 0x44, 0x47, 0x47, 0x7f, 0x1f, 0x1f };
            theme_max_set_mask = RrPixmapMaskNew(inst, 7, 7, data);
        }
    }

    if (!read_mask(db, inst,
                   "window.button.icon.mask", theme,
                   &theme_iconify_mask)) {
        char data[] = { 0x00, 0x00, 0x00, 0x00, 0x7f, 0x7f, 0x7f };
        theme_iconify_mask = RrPixmapMaskNew(inst, 7, 7, data);
    }

    if (read_mask(db, inst,
                  "window.button.stick.mask", theme,
                   &theme_desk_unset_mask)) {
        if (!read_mask(db, inst, "window.button.stick.toggled.mask", theme,
                       &theme_desk_set_mask)) {
            theme_desk_set_mask =
                RrPixmapMaskCopy(theme_desk_unset_mask);
        }
    } else {
        {
            char data[] = { 0x63, 0x63, 0x00, 0x00, 0x00, 0x63, 0x63 };
            theme_desk_unset_mask = RrPixmapMaskNew(inst, 7, 7, data);
        }
        {
            char data[] = { 0x00, 0x36, 0x36, 0x08, 0x36, 0x36, 0x00 };
            theme_desk_set_mask = RrPixmapMaskNew(inst, 7, 7, data);
        }
    }

    if (read_mask(db, inst, "window.button.shade.mask", theme,
                   &theme_shade_unset_mask)) {
        if (!read_mask(db, inst, "window.button.shade.toggled.mask", theme,
                       &theme_shade_set_mask)) {
            theme_shade_set_mask =
                RrPixmapMaskCopy(theme_shade_unset_mask);
        }
    } else {
        {
            char data[] = { 0x7f, 0x7f, 0x7f, 0x00, 0x00, 0x00, 0x00 };
            theme_shade_unset_mask = RrPixmapMaskNew(inst, 7, 7, data);
        }
        {
            char data[] = { 0x7f, 0x7f, 0x7f, 0x00, 0x00, 0x00, 0x7f };
            theme_shade_set_mask = RrPixmapMaskNew(inst, 7, 7, data);
        }
    }

    if (!read_mask(db, inst, "window.button.close.mask", theme,
                   &theme_close_mask)) {
        char data[] = { 0x63, 0x77, 0x3e, 0x1c, 0x3e, 0x77, 0x63 };
        theme_close_mask = RrPixmapMaskNew(inst, 7, 7, data);
    }        

    /* read the decoration textures */
    if (!read_appearance(db, inst,
                         "window.title.focus", theme_a_focused_title))
	set_default_appearance(theme_a_focused_title);
    if (!read_appearance(db, inst,
                         "window.title.unfocus", theme_a_unfocused_title))
	set_default_appearance(theme_a_unfocused_title);
    if (!read_appearance(db, inst,
                         "window.label.focus", theme_a_focused_label))
	set_default_appearance(theme_a_focused_label);
    if (!read_appearance(db, inst,
                         "window.label.unfocus", theme_a_unfocused_label))
	set_default_appearance(theme_a_unfocused_label);
    if (!read_appearance(db, inst,
                         "window.handle.focus", theme_a_focused_handle))
	set_default_appearance(theme_a_focused_handle);
    if (!read_appearance(db, inst,
                         "window.handle.unfocus",theme_a_unfocused_handle))
	set_default_appearance(theme_a_unfocused_handle);
    if (!read_appearance(db, inst,
                         "window.grip.focus", theme_a_focused_grip))
	set_default_appearance(theme_a_focused_grip);
    if (!read_appearance(db, inst,
                         "window.grip.unfocus", theme_a_unfocused_grip))
	set_default_appearance(theme_a_unfocused_grip);
    if (!read_appearance(db, inst,
                         "menu.frame", theme_a_menu))
	set_default_appearance(theme_a_menu);
    if (!read_appearance(db, inst,
                         "menu.title", theme_a_menu_title))
	set_default_appearance(theme_a_menu_title);
    if (!read_appearance(db, inst,
                         "menu.hilite", theme_a_menu_hilite))
	set_default_appearance(theme_a_menu_hilite);

    /* read the appearances for rendering non-decorations */
    if (!read_appearance(db, inst,
                         "window.title.focus", theme_app_hilite_bg))
        set_default_appearance(theme_app_hilite_bg);
    if (!read_appearance(db, inst,
                         "window.label.focus", theme_app_hilite_label))
        set_default_appearance(theme_app_hilite_label);
    if (!read_appearance(db, inst,
                         "window.title.unfocus", theme_app_unhilite_bg))
        set_default_appearance(theme_app_unhilite_bg);
    if (!read_appearance(db, inst,
                         "window.label.unfocus", theme_app_unhilite_label))
        set_default_appearance(theme_app_unhilite_label);

    /* read buttons textures */
    if (!read_appearance(db, inst,
                         "window.button.pressed.focus",
			 theme_a_focused_pressed_max))
	if (!read_appearance(db, inst,
                             "window.button.pressed",
                             theme_a_focused_pressed_max))
	    set_default_appearance(theme_a_focused_pressed_max);
    if (!read_appearance(db, inst,
                         "window.button.pressed.unfocus",
			 theme_a_unfocused_pressed_max))
	if (!read_appearance(db, inst,
                             "window.button.pressed",
			     theme_a_unfocused_pressed_max))
	    set_default_appearance(theme_a_unfocused_pressed_max);
    if (!read_appearance(db, inst,
                         "window.button.focus",
			 theme_a_focused_unpressed_max))
	set_default_appearance(theme_a_focused_unpressed_max);
    if (!read_appearance(db, inst,
                         "window.button.unfocus",
			 theme_a_unfocused_unpressed_max))
	set_default_appearance(theme_a_unfocused_unpressed_max);

    theme_a_unfocused_unpressed_close =
        RrAppearanceCopy(theme_a_unfocused_unpressed_max);
    theme_a_unfocused_pressed_close =
        RrAppearanceCopy(theme_a_unfocused_pressed_max);
    theme_a_focused_unpressed_close =
        RrAppearanceCopy(theme_a_focused_unpressed_max);
    theme_a_focused_pressed_close =
        RrAppearanceCopy(theme_a_focused_pressed_max);
    theme_a_unfocused_unpressed_desk =
        RrAppearanceCopy(theme_a_unfocused_unpressed_max);
    theme_a_unfocused_pressed_desk =
        RrAppearanceCopy(theme_a_unfocused_pressed_max);
    theme_a_unfocused_pressed_set_desk =
        RrAppearanceCopy(theme_a_unfocused_pressed_max);
    theme_a_focused_unpressed_desk =
        RrAppearanceCopy(theme_a_focused_unpressed_max);
    theme_a_focused_pressed_desk =
        RrAppearanceCopy(theme_a_focused_pressed_max);
    theme_a_focused_pressed_set_desk =
        RrAppearanceCopy(theme_a_focused_pressed_max);
    theme_a_unfocused_unpressed_shade =
        RrAppearanceCopy(theme_a_unfocused_unpressed_max);
    theme_a_unfocused_pressed_shade =
        RrAppearanceCopy(theme_a_unfocused_pressed_max);
    theme_a_unfocused_pressed_set_shade =
        RrAppearanceCopy(theme_a_unfocused_pressed_max);
    theme_a_focused_unpressed_shade =
        RrAppearanceCopy(theme_a_focused_unpressed_max);
    theme_a_focused_pressed_shade =
        RrAppearanceCopy(theme_a_focused_pressed_max);
    theme_a_focused_pressed_set_shade =
        RrAppearanceCopy(theme_a_focused_pressed_max);
    theme_a_unfocused_unpressed_iconify =
        RrAppearanceCopy(theme_a_unfocused_unpressed_max);
    theme_a_unfocused_pressed_iconify =
        RrAppearanceCopy(theme_a_unfocused_pressed_max);
    theme_a_focused_unpressed_iconify =
        RrAppearanceCopy(theme_a_focused_unpressed_max);
    theme_a_focused_pressed_iconify =
        RrAppearanceCopy(theme_a_focused_pressed_max);
    theme_a_unfocused_pressed_set_max =
        RrAppearanceCopy(theme_a_unfocused_pressed_max);
    theme_a_focused_pressed_set_max =
        RrAppearanceCopy(theme_a_focused_pressed_max);

    theme_a_icon->surface.grad = RR_SURFACE_PARENTREL;

    /* set up the textures */
    theme_a_focused_label->texture[0].type = 
        theme_app_hilite_label->texture[0].type = RR_TEXTURE_TEXT;
    theme_a_focused_label->texture[0].data.text.justify = winjust;
    theme_app_hilite_label->texture[0].data.text.justify = RR_JUSTIFY_LEFT;
    theme_a_focused_label->texture[0].data.text.font =
        theme_app_hilite_label->texture[0].data.text.font = theme_winfont;
    theme_a_focused_label->texture[0].data.text.shadow =
        theme_app_hilite_label->texture[0].data.text.shadow =
        theme_winfont_shadow;
    theme_a_focused_label->texture[0].data.text.offset =
        theme_app_hilite_label->texture[0].data.text.offset =
        theme_winfont_shadow_offset;
    theme_a_focused_label->texture[0].data.text.tint =
        theme_app_hilite_label->texture[0].data.text.tint =
        theme_winfont_shadow_tint;
    theme_a_focused_label->texture[0].data.text.color =
        theme_app_hilite_label->texture[0].data.text.color =
        theme_title_focused_color;

    theme_a_unfocused_label->texture[0].type =
        theme_app_unhilite_label->texture[0].type = RR_TEXTURE_TEXT;
    theme_a_unfocused_label->texture[0].data.text.justify = winjust;
    theme_app_unhilite_label->texture[0].data.text.justify = RR_JUSTIFY_LEFT;
    theme_a_unfocused_label->texture[0].data.text.font =
        theme_app_unhilite_label->texture[0].data.text.font = theme_winfont;
    theme_a_unfocused_label->texture[0].data.text.shadow =
        theme_app_unhilite_label->texture[0].data.text.shadow =
        theme_winfont_shadow;
    theme_a_unfocused_label->texture[0].data.text.offset =
        theme_app_unhilite_label->texture[0].data.text.offset =
        theme_winfont_shadow_offset;
    theme_a_unfocused_label->texture[0].data.text.tint =
        theme_app_unhilite_label->texture[0].data.text.tint =
        theme_winfont_shadow_tint;
    theme_a_unfocused_label->texture[0].data.text.color =
        theme_app_unhilite_label->texture[0].data.text.color =
        theme_title_unfocused_color;

    theme_a_menu_title->texture[0].type = RR_TEXTURE_TEXT;
    theme_a_menu_title->texture[0].data.text.justify = mtitlejust;
    theme_a_menu_title->texture[0].data.text.font = theme_mtitlefont;
    theme_a_menu_title->texture[0].data.text.shadow = theme_mtitlefont_shadow;
    theme_a_menu_title->texture[0].data.text.offset =
        theme_mtitlefont_shadow_offset;
    theme_a_menu_title->texture[0].data.text.tint =
        theme_mtitlefont_shadow_tint;
    theme_a_menu_title->texture[0].data.text.color = theme_menu_title_color;

    theme_a_menu_item->surface.grad = 
        theme_a_menu_disabled->surface.grad =
        theme_app_icon->surface.grad = RR_SURFACE_PARENTREL;

    theme_a_menu_item->texture[0].type =
        theme_a_menu_disabled->texture[0].type = 
        theme_a_menu_hilite->texture[0].type = RR_TEXTURE_TEXT;
    theme_a_menu_item->texture[0].data.text.justify = 
        theme_a_menu_disabled->texture[0].data.text.justify = 
        theme_a_menu_hilite->texture[0].data.text.justify = mjust;
    theme_a_menu_item->texture[0].data.text.font =
        theme_a_menu_disabled->texture[0].data.text.font =
        theme_a_menu_hilite->texture[0].data.text.font = theme_mfont;
    theme_a_menu_item->texture[0].data.text.shadow = 
        theme_a_menu_disabled->texture[0].data.text.shadow = 
        theme_a_menu_hilite->texture[0].data.text.shadow = theme_mfont_shadow;
    theme_a_menu_item->texture[0].data.text.offset =
        theme_a_menu_disabled->texture[0].data.text.offset = 
        theme_a_menu_hilite->texture[0].data.text.offset = 
        theme_mfont_shadow_offset;
    theme_a_menu_item->texture[0].data.text.tint =
        theme_a_menu_disabled->texture[0].data.text.tint =
        theme_a_menu_hilite->texture[0].data.text.tint =
        theme_mfont_shadow_tint;
    theme_a_menu_item->texture[0].data.text.color = theme_menu_color;
    theme_a_menu_disabled->texture[0].data.text.color =
        theme_menu_disabled_color;
    theme_a_menu_hilite->texture[0].data.text.color =  theme_menu_hilite_color;

    theme_a_focused_unpressed_max->texture[0].type = 
        theme_a_focused_pressed_max->texture[0].type = 
        theme_a_focused_pressed_set_max->texture[0].type =  
        theme_a_unfocused_unpressed_max->texture[0].type = 
        theme_a_unfocused_pressed_max->texture[0].type = 
        theme_a_unfocused_pressed_set_max->texture[0].type = 
        theme_a_focused_unpressed_close->texture[0].type = 
        theme_a_focused_pressed_close->texture[0].type = 
        theme_a_unfocused_unpressed_close->texture[0].type = 
        theme_a_unfocused_pressed_close->texture[0].type = 
        theme_a_focused_unpressed_desk->texture[0].type = 
        theme_a_focused_pressed_desk->texture[0].type = 
        theme_a_focused_pressed_set_desk->texture[0].type = 
        theme_a_unfocused_unpressed_desk->texture[0].type = 
        theme_a_unfocused_pressed_desk->texture[0].type = 
        theme_a_unfocused_pressed_set_desk->texture[0].type = 
        theme_a_focused_unpressed_shade->texture[0].type = 
        theme_a_focused_pressed_shade->texture[0].type = 
        theme_a_focused_pressed_set_shade->texture[0].type = 
        theme_a_unfocused_unpressed_shade->texture[0].type = 
        theme_a_unfocused_pressed_shade->texture[0].type = 
        theme_a_unfocused_pressed_set_shade->texture[0].type = 
        theme_a_focused_unpressed_iconify->texture[0].type = 
        theme_a_focused_pressed_iconify->texture[0].type = 
        theme_a_unfocused_unpressed_iconify->texture[0].type = 
        theme_a_unfocused_pressed_iconify->texture[0].type = RR_TEXTURE_MASK;
    theme_a_focused_unpressed_max->texture[0].data.mask.mask = 
        theme_a_unfocused_unpressed_max->texture[0].data.mask.mask = 
        theme_a_focused_pressed_max->texture[0].data.mask.mask = 
        theme_a_unfocused_pressed_max->texture[0].data.mask.mask =
        theme_max_unset_mask;
    theme_a_focused_pressed_set_max->texture[0].data.mask.mask = 
        theme_a_unfocused_pressed_set_max->texture[0].data.mask.mask =
        theme_max_set_mask;
    theme_a_focused_pressed_close->texture[0].data.mask.mask = 
        theme_a_unfocused_pressed_close->texture[0].data.mask.mask =
        theme_a_focused_unpressed_close->texture[0].data.mask.mask = 
        theme_a_unfocused_unpressed_close->texture[0].data.mask.mask =
        theme_close_mask;
    theme_a_focused_unpressed_desk->texture[0].data.mask.mask = 
        theme_a_unfocused_unpressed_desk->texture[0].data.mask.mask = 
        theme_a_focused_pressed_desk->texture[0].data.mask.mask = 
        theme_a_unfocused_pressed_desk->texture[0].data.mask.mask =
        theme_desk_unset_mask;
    theme_a_focused_pressed_set_desk->texture[0].data.mask.mask = 
        theme_a_unfocused_pressed_set_desk->texture[0].data.mask.mask =
        theme_desk_set_mask;
    theme_a_focused_unpressed_shade->texture[0].data.mask.mask = 
        theme_a_unfocused_unpressed_shade->texture[0].data.mask.mask = 
        theme_a_focused_pressed_shade->texture[0].data.mask.mask = 
        theme_a_unfocused_pressed_shade->texture[0].data.mask.mask =
        theme_shade_unset_mask;
    theme_a_focused_pressed_set_shade->texture[0].data.mask.mask = 
        theme_a_unfocused_pressed_set_shade->texture[0].data.mask.mask =
        theme_shade_set_mask;
    theme_a_focused_unpressed_iconify->texture[0].data.mask.mask = 
        theme_a_unfocused_unpressed_iconify->texture[0].data.mask.mask = 
        theme_a_focused_pressed_iconify->texture[0].data.mask.mask = 
        theme_a_unfocused_pressed_iconify->texture[0].data.mask.mask =
        theme_iconify_mask;
    theme_a_focused_unpressed_max->texture[0].data.mask.color = 
        theme_a_focused_pressed_max->texture[0].data.mask.color = 
        theme_a_focused_pressed_set_max->texture[0].data.mask.color = 
        theme_a_focused_unpressed_close->texture[0].data.mask.color = 
        theme_a_focused_pressed_close->texture[0].data.mask.color = 
        theme_a_focused_unpressed_desk->texture[0].data.mask.color = 
        theme_a_focused_pressed_desk->texture[0].data.mask.color = 
        theme_a_focused_pressed_set_desk->texture[0].data.mask.color = 
        theme_a_focused_unpressed_shade->texture[0].data.mask.color = 
        theme_a_focused_pressed_shade->texture[0].data.mask.color = 
        theme_a_focused_pressed_set_shade->texture[0].data.mask.color = 
        theme_a_focused_unpressed_iconify->texture[0].data.mask.color = 
        theme_a_focused_pressed_iconify->texture[0].data.mask.color =
        theme_titlebut_focused_color;
    theme_a_unfocused_unpressed_max->texture[0].data.mask.color = 
        theme_a_unfocused_pressed_max->texture[0].data.mask.color = 
        theme_a_unfocused_pressed_set_max->texture[0].data.mask.color = 
        theme_a_unfocused_unpressed_close->texture[0].data.mask.color = 
        theme_a_unfocused_pressed_close->texture[0].data.mask.color = 
        theme_a_unfocused_unpressed_desk->texture[0].data.mask.color = 
        theme_a_unfocused_pressed_desk->texture[0].data.mask.color = 
        theme_a_unfocused_pressed_set_desk->texture[0].data.mask.color = 
        theme_a_unfocused_unpressed_shade->texture[0].data.mask.color = 
        theme_a_unfocused_pressed_shade->texture[0].data.mask.color = 
        theme_a_unfocused_pressed_set_shade->texture[0].data.mask.color = 
        theme_a_unfocused_unpressed_iconify->texture[0].data.mask.color = 
        theme_a_unfocused_pressed_iconify->texture[0].data.mask.color =
        theme_titlebut_unfocused_color;

    XrmDestroyDatabase(db);

    return loaded;
}
