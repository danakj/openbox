#include "render.h"
#include "color.h"
#include "font.h"
#include "mask.h"

#include <X11/Xlib.h>
#include <X11/Xresource.h>

/* style settings - geometry */
int theme_bevel;
int theme_handle_height;
int theme_bwidth;
int theme_cbwidth;
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
int theme_winfont_height;
ObFont *theme_winfont;
gboolean theme_winfont_shadow;
int theme_winfont_shadow_offset;
int theme_winfont_shadow_tint;
int theme_mtitlefont_height;
ObFont *theme_mtitlefont;
gboolean theme_mtitlefont_shadow;
int theme_mtitlefont_shadow_offset;
int theme_mtitlefont_shadow_tint;
int theme_mfont_height;
ObFont *theme_mfont;
gboolean theme_mfont_shadow;
int theme_mfont_shadow_offset;
int theme_mfont_shadow_tint;
/* style settings - title layout */
char *theme_title_layout;
/* style settings - masks */
pixmap_mask *theme_max_set_mask;
pixmap_mask *theme_max_unset_mask;
pixmap_mask *theme_iconify_mask;
pixmap_mask *theme_desk_set_mask;
pixmap_mask *theme_desk_unset_mask;
pixmap_mask *theme_shade_set_mask;
pixmap_mask *theme_shade_unset_mask;
pixmap_mask *theme_close_mask;

/* global appearances */
Appearance *theme_a_focused_unpressed_max;
Appearance *theme_a_focused_pressed_max;
Appearance *theme_a_focused_pressed_set_max;
Appearance *theme_a_unfocused_unpressed_max;
Appearance *theme_a_unfocused_pressed_max;
Appearance *theme_a_unfocused_pressed_set_max;
Appearance *theme_a_focused_unpressed_close;
Appearance *theme_a_focused_pressed_close;
Appearance *theme_a_unfocused_unpressed_close;
Appearance *theme_a_unfocused_pressed_close;
Appearance *theme_a_focused_unpressed_desk;
Appearance *theme_a_focused_pressed_desk;
Appearance *theme_a_focused_pressed_set_desk;
Appearance *theme_a_unfocused_unpressed_desk;
Appearance *theme_a_unfocused_pressed_desk;
Appearance *theme_a_unfocused_pressed_set_desk;
Appearance *theme_a_focused_unpressed_shade;
Appearance *theme_a_focused_pressed_shade;
Appearance *theme_a_focused_pressed_set_shade;
Appearance *theme_a_unfocused_unpressed_shade;
Appearance *theme_a_unfocused_pressed_shade;
Appearance *theme_a_unfocused_pressed_set_shade;
Appearance *theme_a_focused_unpressed_iconify;
Appearance *theme_a_focused_pressed_iconify;
Appearance *theme_a_unfocused_unpressed_iconify;
Appearance *theme_a_unfocused_pressed_iconify;
Appearance *theme_a_focused_grip;
Appearance *theme_a_unfocused_grip;
Appearance *theme_a_focused_title;
Appearance *theme_a_unfocused_title;
Appearance *theme_a_focused_label;
Appearance *theme_a_unfocused_label;
Appearance *theme_a_icon; /* always parentrelative, so no focused/unfocused */
Appearance *theme_a_focused_handle;
Appearance *theme_a_unfocused_handle;
Appearance *theme_a_menu_title;
Appearance *theme_a_menu;
Appearance *theme_a_menu_item;
Appearance *theme_a_menu_disabled;
Appearance *theme_a_menu_hilite;

Appearance *theme_app_hilite_bg;
Appearance *theme_app_unhilite_bg;
Appearance *theme_app_hilite_label;
Appearance *theme_app_unhilite_label;
Appearance *theme_app_icon;

void theme_startup()
{
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

    theme_a_focused_unpressed_max = appearance_new(Surface_Planar, 1);
    theme_a_focused_pressed_max = appearance_new(Surface_Planar, 1);
    theme_a_focused_pressed_set_max = appearance_new(Surface_Planar, 1);
    theme_a_unfocused_unpressed_max = appearance_new(Surface_Planar, 1);
    theme_a_unfocused_pressed_max = appearance_new(Surface_Planar, 1);
    theme_a_unfocused_pressed_set_max = appearance_new(Surface_Planar, 1);
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
    theme_a_focused_grip = appearance_new(Surface_Planar, 0);
    theme_a_unfocused_grip = appearance_new(Surface_Planar, 0);
    theme_a_focused_title = appearance_new(Surface_Planar, 0);
    theme_a_unfocused_title = appearance_new(Surface_Planar, 0);
    theme_a_focused_label = appearance_new(Surface_Planar, 1);
    theme_a_unfocused_label = appearance_new(Surface_Planar, 1);
    theme_a_icon = appearance_new(Surface_Planar, 1);
    theme_a_focused_handle = appearance_new(Surface_Planar, 0);
    theme_a_unfocused_handle = appearance_new(Surface_Planar, 0);
    theme_a_menu = appearance_new(Surface_Planar, 0);
    theme_a_menu_title = appearance_new(Surface_Planar, 1);
    theme_a_menu_item = appearance_new(Surface_Planar, 1);
    theme_a_menu_disabled = appearance_new(Surface_Planar, 1);
    theme_a_menu_hilite = appearance_new(Surface_Planar, 1);

    theme_app_hilite_bg = appearance_new(Surface_Planar, 0);
    theme_app_unhilite_bg = appearance_new(Surface_Planar, 0);
    theme_app_hilite_label = appearance_new(Surface_Planar, 1);
    theme_app_unhilite_label = appearance_new(Surface_Planar, 1);
    theme_app_icon = appearance_new(Surface_Planar, 1);

}

void theme_shutdown()
{
    color_free(theme_b_color);
    color_free(theme_cb_unfocused_color);
    color_free(theme_cb_focused_color);
    color_free(theme_title_unfocused_color);
    color_free(theme_title_focused_color);
    color_free(theme_titlebut_unfocused_color);
    color_free(theme_titlebut_focused_color);
    color_free(theme_menu_color);
    color_free(theme_menu_title_color);
    color_free(theme_menu_disabled_color);
    color_free(theme_menu_hilite_color);

    pixmap_mask_free(theme_max_set_mask);
    pixmap_mask_free(theme_max_unset_mask);
    pixmap_mask_free(theme_desk_set_mask);
    pixmap_mask_free(theme_desk_unset_mask);
    pixmap_mask_free(theme_shade_set_mask);
    pixmap_mask_free(theme_shade_unset_mask);
    pixmap_mask_free(theme_iconify_mask);
    pixmap_mask_free(theme_close_mask);

    font_close(theme_winfont);
    font_close(theme_mtitlefont);
    font_close(theme_mfont);

    g_free(theme_title_layout);

    appearance_free(theme_a_focused_unpressed_max);
    appearance_free(theme_a_focused_pressed_max);
    appearance_free(theme_a_focused_pressed_set_max);
    appearance_free(theme_a_unfocused_unpressed_max);
    appearance_free(theme_a_unfocused_pressed_max);
    appearance_free(theme_a_unfocused_pressed_set_max);
    appearance_free(theme_a_focused_unpressed_close);
    appearance_free(theme_a_focused_pressed_close);
    appearance_free(theme_a_unfocused_unpressed_close);
    appearance_free(theme_a_unfocused_pressed_close);
    appearance_free(theme_a_focused_unpressed_desk);
    appearance_free(theme_a_focused_pressed_desk);
    appearance_free(theme_a_unfocused_unpressed_desk);
    appearance_free(theme_a_unfocused_pressed_desk);
    appearance_free(theme_a_focused_unpressed_shade);
    appearance_free(theme_a_focused_pressed_shade);
    appearance_free(theme_a_unfocused_unpressed_shade);
    appearance_free(theme_a_unfocused_pressed_shade);
    appearance_free(theme_a_focused_unpressed_iconify);
    appearance_free(theme_a_focused_pressed_iconify);
    appearance_free(theme_a_unfocused_unpressed_iconify);
    appearance_free(theme_a_unfocused_pressed_iconify);
    appearance_free(theme_a_focused_grip);
    appearance_free(theme_a_unfocused_grip);
    appearance_free(theme_a_focused_title);
    appearance_free(theme_a_unfocused_title);
    appearance_free(theme_a_focused_label);
    appearance_free(theme_a_unfocused_label);
    appearance_free(theme_a_icon);
    appearance_free(theme_a_focused_handle);
    appearance_free(theme_a_unfocused_handle);
    appearance_free(theme_a_menu);
    appearance_free(theme_a_menu_title);
    appearance_free(theme_a_menu_item);
    appearance_free(theme_a_menu_disabled);
    appearance_free(theme_a_menu_hilite);
    appearance_free(theme_app_hilite_bg);
    appearance_free(theme_app_unhilite_bg);
    appearance_free(theme_app_hilite_label);
    appearance_free(theme_app_unhilite_label);
    appearance_free(theme_app_icon);
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

static gboolean read_color(XrmDatabase db, char *rname, color_rgb **value)
{
    gboolean ret = FALSE;
    char *rclass = create_class_name(rname);
    char *rettype;
    XrmValue retvalue;
  
    if (XrmGetResource(db, rname, rclass, &rettype, &retvalue) &&
	retvalue.addr != NULL) {
	color_rgb *c = color_parse(retvalue.addr);
	if (c != NULL) {
	    *value = c;
	    ret = TRUE;
	}
    }

    g_free(rclass);
    return ret;
}

static gboolean read_mask(XrmDatabase db, char *rname, char *theme,
                          pixmap_mask **value)
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
            *value = pixmap_mask_new(w, h, (char*)b);
            XFree(b);
        }
      
        g_free(s);
        g_free(button_dir);
    }

    g_free(rclass);
    return ret;
}

static void parse_appearance(char *tex, SurfaceColorType *grad,
                             ReliefType *relief, BevelType *bevel,
                             gboolean *interlaced, gboolean *border)
{
    char *t;

    /* convert to all lowercase */
    for (t = tex; *t != '\0'; ++t)
	*t = g_ascii_tolower(*t);

    if (strstr(tex, "parentrelative") != NULL) {
	*grad = Background_ParentRelative;
    } else {
	if (strstr(tex, "gradient") != NULL) {
	    if (strstr(tex, "crossdiagonal") != NULL)
		*grad = Background_CrossDiagonal;
	    else if (strstr(tex, "rectangle") != NULL)
		*grad = Background_Rectangle;
	    else if (strstr(tex, "pyramid") != NULL)
		*grad = Background_Pyramid;
	    else if (strstr(tex, "pipecross") != NULL)
		*grad = Background_PipeCross;
	    else if (strstr(tex, "elliptic") != NULL)
		*grad = Background_Rectangle;
	    else if (strstr(tex, "horizontal") != NULL)
		*grad = Background_Horizontal;
	    else if (strstr(tex, "vertical") != NULL)
		*grad = Background_Vertical;
	    else
		*grad = Background_Diagonal;
	} else {
	    *grad = Background_Solid;
	}

	if (strstr(tex, "sunken") != NULL)
	    *relief = Sunken;
	else if (strstr(tex, "flat") != NULL)
	    *relief = Flat;
	else
	    *relief = Raised;
	
	*border = FALSE;
	if (*relief == Flat) {
	    if (strstr(tex, "border") != NULL)
		*border = TRUE;
	} else {
	    if (strstr(tex, "bevel2") != NULL)
		*bevel = Bevel2;
	    else
		*bevel = Bevel1;
	}

	if (strstr(tex, "interlaced") != NULL)
	    *interlaced = TRUE;
	else
	    *interlaced = FALSE;
    }
}


static gboolean read_appearance(XrmDatabase db, char *rname, Appearance *value)
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
			 &value->surface.data.planar.grad,
			 &value->surface.data.planar.relief,
			 &value->surface.data.planar.bevel,
			 &value->surface.data.planar.interlaced,
			 &value->surface.data.planar.border);
	if (!read_color(db, cname, &value->surface.data.planar.primary))
	    value->surface.data.planar.primary = color_new(0, 0, 0);
	if (!read_color(db, ctoname, &value->surface.data.planar.secondary))
	    value->surface.data.planar.secondary = color_new(0, 0, 0);
	if (value->surface.data.planar.border)
	    if (!read_color(db, bcname,
			    &value->surface.data.planar.border_color))
		value->surface.data.planar.border_color = color_new(0, 0, 0);
	ret = TRUE;
    }

    g_free(bcname);
    g_free(ctoname);
    g_free(cname);
    g_free(rclass);
    return ret;
}

static void set_default_appearance(Appearance *a)
{
    a->surface.data.planar.grad = Background_Solid;
    a->surface.data.planar.relief = Flat;
    a->surface.data.planar.bevel = Bevel1;
    a->surface.data.planar.interlaced = FALSE;
    a->surface.data.planar.border = FALSE;
    a->surface.data.planar.primary = color_new(0, 0, 0);
    a->surface.data.planar.secondary = color_new(0, 0, 0);
}

char *theme_load(char *theme)
{
    XrmDatabase db = NULL;
    char *loaded = NULL;
    Justify winjust, mtitlejust, mjust;
    char *str;
    char *font_str;

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
    font_str = "arial-8:bold";

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

    winjust = Justify_Left;
    if (read_string(db, "window.justify", &str)) {
        if (!g_ascii_strcasecmp(str, "right"))
            winjust = Justify_Right;
        else if (!g_ascii_strcasecmp(str, "center"))
            winjust = Justify_Center;
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

    mtitlejust = Justify_Left;
    if (read_string(db, "menu.title.justify", &str)) {
        if (!g_ascii_strcasecmp(str, "right"))
            mtitlejust = Justify_Right;
        else if (!g_ascii_strcasecmp(str, "center"))
            mtitlejust = Justify_Center;
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

    mjust = Justify_Left;
    if (read_string(db, "menu.frame.justify", &str)) {
        if (!g_ascii_strcasecmp(str, "right"))
            mjust = Justify_Right;
        else if (!g_ascii_strcasecmp(str, "center"))
            mjust = Justify_Center;
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
    if (!read_color(db, "borderColor", &theme_b_color))
	theme_b_color = color_new(0, 0, 0);
    if (!read_color(db, "window.frame.focusColor", &theme_cb_focused_color))
	theme_cb_focused_color = color_new(0xff, 0xff, 0xff);
    if (!read_color(db, "window.frame.unfocusColor",&theme_cb_unfocused_color))
	theme_cb_unfocused_color = color_new(0xff, 0xff, 0xff);
    if (!read_color(db, "window.label.focus.textColor",
                    &theme_title_focused_color))
	theme_title_focused_color = color_new(0x0, 0x0, 0x0);
    if (!read_color(db, "window.label.unfocus.textColor",
                    &theme_title_unfocused_color))
	theme_title_unfocused_color = color_new(0xff, 0xff, 0xff);
    if (!read_color(db, "window.button.focus.picColor",
                    &theme_titlebut_focused_color))
	theme_titlebut_focused_color = color_new(0, 0, 0);
    if (!read_color(db, "window.button.unfocus.picColor",
                    &theme_titlebut_unfocused_color))
	theme_titlebut_unfocused_color = color_new(0xff, 0xff, 0xff);
    if (!read_color(db, "menu.title.textColor", &theme_menu_title_color))
        theme_menu_title_color = color_new(0, 0, 0);
    if (!read_color(db, "menu.frame.textColor", &theme_menu_color))
        theme_menu_color = color_new(0xff, 0xff, 0xff);
    if (!read_color(db, "menu.frame.disableColor", &theme_menu_disabled_color))
        theme_menu_disabled_color = color_new(0, 0, 0);
    if (!read_color(db, "menu.hilite.textColor", &theme_menu_hilite_color))
        theme_menu_hilite_color = color_new(0, 0, 0);

    if (read_mask(db, "window.button.max.mask", theme, &theme_max_unset_mask)){
        if (!read_mask(db, "window.button.max.toggled.mask", theme,
                       &theme_max_set_mask)) {
            theme_max_set_mask = pixmap_mask_copy(theme_max_unset_mask);
        }
    } else {
        {
            char data[] = { 0x7f, 0x7f, 0x7f, 0x41, 0x41, 0x41, 0x7f };
            theme_max_unset_mask = pixmap_mask_new(7, 7, data);
        }
        {
            char data[] = { 0x7c, 0x44, 0x47, 0x47, 0x7f, 0x1f, 0x1f };
            theme_max_set_mask = pixmap_mask_new(7, 7, data);
        }
    }

    if (!read_mask(db, "window.button.icon.mask", theme,
                   &theme_iconify_mask)) {
        char data[] = { 0x00, 0x00, 0x00, 0x00, 0x7f, 0x7f, 0x7f };
        theme_iconify_mask = pixmap_mask_new(7, 7, data);
    }

    if (read_mask(db, "window.button.stick.mask", theme,
                   &theme_desk_unset_mask)) {
        if (!read_mask(db, "window.button.stick.toggled.mask", theme,
                       &theme_desk_set_mask)) {
            theme_desk_set_mask =
                pixmap_mask_copy(theme_desk_unset_mask);
        }
    } else {
        {
            char data[] = { 0x63, 0x63, 0x00, 0x00, 0x00, 0x63, 0x63 };
            theme_desk_unset_mask = pixmap_mask_new(7, 7, data);
        }
        {
            char data[] = { 0x00, 0x36, 0x36, 0x08, 0x36, 0x36, 0x00 };
            theme_desk_set_mask = pixmap_mask_new(7, 7, data);
        }
    }

    if (read_mask(db, "window.button.shade.mask", theme,
                   &theme_shade_unset_mask)) {
        if (!read_mask(db, "window.button.shade.toggled.mask", theme,
                       &theme_shade_set_mask)) {
            theme_shade_set_mask =
                pixmap_mask_copy(theme_shade_unset_mask);
        }
    } else {
        {
            char data[] = { 0x7f, 0x7f, 0x7f, 0x00, 0x00, 0x00, 0x00 };
            theme_shade_unset_mask = pixmap_mask_new(7, 7, data);
        }
        {
            char data[] = { 0x7f, 0x7f, 0x7f, 0x00, 0x00, 0x00, 0x7f };
            theme_shade_set_mask = pixmap_mask_new(7, 7, data);
        }
    }

    if (!read_mask(db, "window.button.close.mask", theme,
                   &theme_close_mask)) {
        char data[] = { 0x63, 0x77, 0x3e, 0x1c, 0x3e, 0x77, 0x63 };
        theme_close_mask = pixmap_mask_new(7, 7, data);
    }        

    /* read the decoration textures */
    if (!read_appearance(db, "window.title.focus", theme_a_focused_title))
	set_default_appearance(theme_a_focused_title);
    if (!read_appearance(db, "window.title.unfocus", theme_a_unfocused_title))
	set_default_appearance(theme_a_unfocused_title);
    if (!read_appearance(db, "window.label.focus", theme_a_focused_label))
	set_default_appearance(theme_a_focused_label);
    if (!read_appearance(db, "window.label.unfocus", theme_a_unfocused_label))
	set_default_appearance(theme_a_unfocused_label);
    if (!read_appearance(db, "window.handle.focus", theme_a_focused_handle))
	set_default_appearance(theme_a_focused_handle);
    if (!read_appearance(db, "window.handle.unfocus",theme_a_unfocused_handle))
	set_default_appearance(theme_a_unfocused_handle);
    if (!read_appearance(db, "window.grip.focus", theme_a_focused_grip))
	set_default_appearance(theme_a_focused_grip);
    if (!read_appearance(db, "window.grip.unfocus", theme_a_unfocused_grip))
	set_default_appearance(theme_a_unfocused_grip);
    if (!read_appearance(db, "menu.frame", theme_a_menu))
	set_default_appearance(theme_a_menu);
    if (!read_appearance(db, "menu.title", theme_a_menu_title))
	set_default_appearance(theme_a_menu_title);
    if (!read_appearance(db, "menu.hilite", theme_a_menu_hilite))
	set_default_appearance(theme_a_menu_hilite);

    /* read the appearances for rendering non-decorations */
    if (!read_appearance(db, "window.title.focus", theme_app_hilite_bg))
        set_default_appearance(theme_app_hilite_bg);
    if (!read_appearance(db, "window.label.focus", theme_app_hilite_label))
        set_default_appearance(theme_app_hilite_label);
    if (!read_appearance(db, "window.title.unfocus", theme_app_unhilite_bg))
        set_default_appearance(theme_app_unhilite_bg);
    if (!read_appearance(db, "window.label.unfocus", theme_app_unhilite_label))
        set_default_appearance(theme_app_unhilite_label);

    /* read buttons textures */
    if (!read_appearance(db, "window.button.pressed.focus",
			 theme_a_focused_pressed_max))
	if (!read_appearance(db, "window.button.pressed",
                             theme_a_focused_pressed_max))
	    set_default_appearance(theme_a_focused_pressed_max);
    if (!read_appearance(db, "window.button.pressed.unfocus",
			 theme_a_unfocused_pressed_max))
	if (!read_appearance(db, "window.button.pressed",
			     theme_a_unfocused_pressed_max))
	    set_default_appearance(theme_a_unfocused_pressed_max);
    if (!read_appearance(db, "window.button.focus",
			 theme_a_focused_unpressed_max))
	set_default_appearance(theme_a_focused_unpressed_max);
    if (!read_appearance(db, "window.button.unfocus",
			 theme_a_unfocused_unpressed_max))
	set_default_appearance(theme_a_unfocused_unpressed_max);

    theme_a_unfocused_unpressed_close =
        appearance_copy(theme_a_unfocused_unpressed_max);
    theme_a_unfocused_pressed_close =
        appearance_copy(theme_a_unfocused_pressed_max);
    theme_a_focused_unpressed_close =
        appearance_copy(theme_a_focused_unpressed_max);
    theme_a_focused_pressed_close =
        appearance_copy(theme_a_focused_pressed_max);
    theme_a_unfocused_unpressed_desk =
        appearance_copy(theme_a_unfocused_unpressed_max);
    theme_a_unfocused_pressed_desk =
        appearance_copy(theme_a_unfocused_pressed_max);
    theme_a_unfocused_pressed_set_desk =
        appearance_copy(theme_a_unfocused_pressed_max);
    theme_a_focused_unpressed_desk =
        appearance_copy(theme_a_focused_unpressed_max);
    theme_a_focused_pressed_desk =
        appearance_copy(theme_a_focused_pressed_max);
    theme_a_focused_pressed_set_desk =
        appearance_copy(theme_a_focused_pressed_max);
    theme_a_unfocused_unpressed_shade =
        appearance_copy(theme_a_unfocused_unpressed_max);
    theme_a_unfocused_pressed_shade =
        appearance_copy(theme_a_unfocused_pressed_max);
    theme_a_unfocused_pressed_set_shade =
        appearance_copy(theme_a_unfocused_pressed_max);
    theme_a_focused_unpressed_shade =
        appearance_copy(theme_a_focused_unpressed_max);
    theme_a_focused_pressed_shade =
        appearance_copy(theme_a_focused_pressed_max);
    theme_a_focused_pressed_set_shade =
        appearance_copy(theme_a_focused_pressed_max);
    theme_a_unfocused_unpressed_iconify =
        appearance_copy(theme_a_unfocused_unpressed_max);
    theme_a_unfocused_pressed_iconify =
        appearance_copy(theme_a_unfocused_pressed_max);
    theme_a_focused_unpressed_iconify =
        appearance_copy(theme_a_focused_unpressed_max);
    theme_a_focused_pressed_iconify =
        appearance_copy(theme_a_focused_pressed_max);
    theme_a_unfocused_pressed_set_max =
        appearance_copy(theme_a_unfocused_pressed_max);
    theme_a_focused_pressed_set_max =
        appearance_copy(theme_a_focused_pressed_max);

    theme_a_icon->surface.data.planar.grad = Background_ParentRelative;

    /* set up the textures */
    theme_a_focused_label->texture[0].type = 
        theme_app_hilite_label->texture[0].type = Text;
    theme_a_focused_label->texture[0].data.text.justify = winjust;
    theme_app_hilite_label->texture[0].data.text.justify = Justify_Left;
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
        theme_app_unhilite_label->texture[0].type = Text;
    theme_a_unfocused_label->texture[0].data.text.justify = winjust;
    theme_app_unhilite_label->texture[0].data.text.justify = Justify_Left;
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

    theme_a_menu_title->texture[0].type = Text;
    theme_a_menu_title->texture[0].data.text.justify = mtitlejust;
    theme_a_menu_title->texture[0].data.text.font = theme_mtitlefont;
    theme_a_menu_title->texture[0].data.text.shadow = theme_mtitlefont_shadow;
    theme_a_menu_title->texture[0].data.text.offset =
        theme_mtitlefont_shadow_offset;
    theme_a_menu_title->texture[0].data.text.tint =
        theme_mtitlefont_shadow_tint;
    theme_a_menu_title->texture[0].data.text.color = theme_menu_title_color;

    theme_a_menu_item->surface.data.planar.grad = 
        theme_a_menu_disabled->surface.data.planar.grad =
        theme_app_icon->surface.data.planar.grad =
        Background_ParentRelative;

    theme_a_menu_item->texture[0].type =
        theme_a_menu_disabled->texture[0].type = 
        theme_a_menu_hilite->texture[0].type = Text;
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
        theme_a_unfocused_pressed_iconify->texture[0].type = Bitmask;
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
