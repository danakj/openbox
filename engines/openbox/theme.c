#include "openbox.h"
#include "../../kernel/themerc.h"

#include <glib.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif
#ifdef HAVE_CTYPE_H
#  include <ctype.h>
#endif
#ifdef HAVE_STRING_H
#  include <string.h>
#endif

static XrmDatabase loaddb(char *theme)
{
    XrmDatabase db;

    db = XrmGetFileDatabase(theme);
    if (db == NULL) {
	char *s = g_build_filename(g_get_home_dir(), ".openbox", "themes",
				   "openbox", theme, NULL);
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

gboolean read_bool(XrmDatabase db, char *rname, gboolean *value)
{
    gboolean ret = FALSE;
    char *rclass = create_class_name(rname);
    char *rettype;
    XrmValue retvalue;
  
    if (XrmGetResource(db, rname, rclass, &rettype, &retvalue) &&
	retvalue.addr != NULL) {
	if (!g_ascii_strcasecmp(retvalue.addr, "true"))
	    *value = TRUE;
	else
	    *value = FALSE;
	ret = TRUE;
    }

    g_free(rclass);
    return ret;
}

gboolean read_int(XrmDatabase db, char *rname, int *value)
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

gboolean read_string(XrmDatabase db, char *rname, char **value)
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

gboolean read_color(XrmDatabase db, char *rname, color_rgb **value)
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

gboolean read_mask(XrmDatabase db, char *rname, pixmap_mask **value)
{
    gboolean ret = FALSE;
    char *rclass = create_class_name(rname);
    char *rettype;
    XrmValue retvalue;
  
    if (XrmGetResource(db, rname, rclass, &rettype, &retvalue) &&
	retvalue.addr != NULL) {
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
		*grad = Background_Elliptic;
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


gboolean read_appearance(XrmDatabase db, char *rname, Appearance *value)
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

void set_default_appearance(Appearance *a)
{
    a->surface.data.planar.grad = Background_Solid;
    a->surface.data.planar.relief = Flat;
    a->surface.data.planar.bevel = Bevel1;
    a->surface.data.planar.interlaced = FALSE;
    a->surface.data.planar.border = FALSE;
    a->surface.data.planar.primary = color_new(0, 0, 0);
    a->surface.data.planar.secondary = color_new(0, 0, 0);
}

gboolean load()
{
    XrmDatabase db = NULL;
    Justify winjust;
    char *winjuststr;

    if (themerc_theme != NULL) {
	db = loaddb(themerc_theme);
        if (db == NULL) {
	    g_warning("Failed to load the theme '%s'", themerc_theme);
	    g_message("Falling back to the default: '%s'", DEFAULT_THEME);
	}
    }
    if (db == NULL) {
	db = loaddb(DEFAULT_THEME);
	if (db == NULL) {
	    g_warning("Failed to load the theme '%s'.", DEFAULT_THEME);
	    return FALSE;
	}
    }

    /* load the font, not from the theme file tho, its in themerc_font */
    s_winfont_shadow = 1; /* XXX read from themrc */
    s_winfont_shadow_offset = 2; /* XXX read from themerc */
    s_winfont = font_open(themerc_font);
    s_winfont_height = font_height(s_winfont, s_winfont_shadow,
                                   s_winfont_shadow_offset);

    winjust = Justify_Left;
    if (read_string(db, "window.justify", &winjuststr)) {
        if (!g_ascii_strcasecmp(winjuststr, "right"))
            winjust = Justify_Right;
        else if (!g_ascii_strcasecmp(winjuststr, "center"))
            winjust = Justify_Center;
        g_free(winjuststr);
    }

    if (!read_int(db, "handleWidth", &s_handle_height) ||
	s_handle_height < 0 || s_handle_height > 100) s_handle_height = 6;
    if (!read_int(db, "bevelWidth", &s_bevel) ||
	s_bevel <= 0 || s_bevel > 100) s_bevel = 3;
    if (!read_int(db, "borderWidth", &s_bwidth) ||
	s_bwidth < 0 || s_bwidth > 100) s_bwidth = 1;
    if (!read_int(db, "frameWidth", &s_cbwidth) ||
	s_cbwidth < 0 || s_cbwidth > 100) s_cbwidth = s_bevel;

    if (!read_color(db, "borderColor", &s_b_color))
	s_b_color = color_new(0, 0, 0);
    if (!read_color(db, "window.frame.focusColor", &s_cb_focused_color))
	s_cb_focused_color = color_new(0xff, 0xff, 0xff);
    if (!read_color(db, "window.frame.unfocusColor", &s_cb_unfocused_color))
	s_cb_unfocused_color = color_new(0xff, 0xff, 0xff);
    if (!read_color(db, "window.label.focus.textColor",
                    &s_title_focused_color))
	s_title_focused_color = color_new(0xff, 0xff, 0xff);
    if (!read_color(db, "window.label.unfocus.textColor",
                    &s_title_unfocused_color))
	s_title_unfocused_color = color_new(0xff, 0xff, 0xff);
    if (!read_color(db, "window.button.focus.picColor",
                    &s_titlebut_focused_color))
	s_titlebut_focused_color = color_new(0, 0, 0);
    if (!read_color(db, "window.button.unfocus.picColor",
                    &s_titlebut_unfocused_color))
	s_titlebut_unfocused_color = color_new(0xff, 0xff, 0xff);

    if (!read_mask(db, "window.button.max.mask", &s_max_mask)) {
        char data[] = { 0x7c, 0x44, 0x47, 0x47, 0x7f, 0x1f, 0x1f  };
        s_max_mask = pixmap_mask_new(7, 7, data);
    }
    if (!read_mask(db, "window.button.icon.mask", &s_icon_mask)) {
        char data[] = { 0x00, 0x00, 0x00, 0x00, 0x3e, 0x3e, 0x3e };
        s_icon_mask = pixmap_mask_new(7, 7, data);
    }
    if (!read_mask(db, "window.button.stick.mask", &s_desk_mask)) {
        char data[] = { 0x00, 0x36, 0x36, 0x00, 0x36, 0x36, 0x00 };
        s_desk_mask = pixmap_mask_new(7, 7, data);
    }
    if (!read_mask(db, "window.button.close.mask", &s_close_mask)) {
        char data[] = { 0x22, 0x77, 0x3e, 0x1c, 0x3e, 0x77, 0x22 };
        s_close_mask = pixmap_mask_new(7, 7, data);
    }        

    if (!read_appearance(db, "window.title.focus", a_focused_title))
	set_default_appearance(a_focused_title);
    if (!read_appearance(db, "window.title.unfocus", a_unfocused_title))
	set_default_appearance(a_unfocused_title);
    if (!read_appearance(db, "window.label.focus", a_focused_label))
	set_default_appearance(a_focused_label);
    if (!read_appearance(db, "window.label.unfocus", a_unfocused_label))
	set_default_appearance(a_unfocused_label);
    if (!read_appearance(db, "window.handle.focus", a_focused_handle))
	set_default_appearance(a_focused_handle);
    if (!read_appearance(db, "window.handle.unfocus", a_unfocused_handle))
	set_default_appearance(a_unfocused_handle);
    if (!read_appearance(db, "window.grip.focus", a_focused_grip))
	set_default_appearance(a_focused_grip);
    if (!read_appearance(db, "window.grip.unfocus", a_unfocused_grip))
	set_default_appearance(a_unfocused_grip);

    if (!read_appearance(db, "window.button.pressed.focus",
			 a_focused_pressed_max))
	if (!read_appearance(db, "window.button.pressed",
			     a_focused_pressed_max))
	    set_default_appearance(a_focused_pressed_max);
    if (!read_appearance(db, "window.button.pressed.unfocus",
			 a_unfocused_pressed_max))
	if (!read_appearance(db, "window.button.pressed",
			     a_unfocused_pressed_max))
	    set_default_appearance(a_unfocused_pressed_max);
    if (!read_appearance(db, "window.button.focus",
			 a_focused_unpressed_max))
	    set_default_appearance(a_focused_unpressed_max);
    if (!read_appearance(db, "window.button.unfocus",
			 a_unfocused_unpressed_max))
	    set_default_appearance(a_unfocused_unpressed_max);

    a_unfocused_unpressed_close = appearance_copy(a_unfocused_unpressed_max);
    a_unfocused_pressed_close = appearance_copy(a_unfocused_pressed_max);
    a_focused_unpressed_close = appearance_copy(a_focused_unpressed_max);
    a_focused_pressed_close = appearance_copy(a_focused_pressed_max);
    a_unfocused_unpressed_desk = appearance_copy(a_unfocused_unpressed_max);
    a_unfocused_pressed_desk = appearance_copy(a_unfocused_pressed_max);
    a_focused_unpressed_desk = appearance_copy(a_focused_unpressed_max);
    a_focused_pressed_desk = appearance_copy(a_focused_pressed_max);
    a_unfocused_unpressed_iconify = appearance_copy(a_unfocused_unpressed_max);
    a_unfocused_pressed_iconify = appearance_copy(a_unfocused_pressed_max);
    a_focused_unpressed_iconify = appearance_copy(a_focused_unpressed_max);
    a_focused_pressed_iconify = appearance_copy(a_focused_pressed_max);

    a_icon->surface.data.planar.grad = Background_ParentRelative;

    /* set up the textures */
    a_focused_label->texture[0].type = Text;
    a_focused_label->texture[0].data.text.justify = winjust;
    a_focused_label->texture[0].data.text.font = s_winfont;
    a_focused_label->texture[0].data.text.shadow = s_winfont_shadow;
    a_focused_label->texture[0].data.text.offset = s_winfont_shadow_offset;
    a_focused_label->texture[0].data.text.color = s_title_focused_color;

    a_unfocused_label->texture[0].type = Text;
    a_unfocused_label->texture[0].data.text.justify = winjust;
    a_unfocused_label->texture[0].data.text.font = s_winfont;
    a_unfocused_label->texture[0].data.text.shadow = s_winfont_shadow;
    a_unfocused_label->texture[0].data.text.offset = s_winfont_shadow_offset;
    a_unfocused_label->texture[0].data.text.color = s_title_unfocused_color;

    a_focused_unpressed_max->texture[0].type = 
        a_focused_pressed_max->texture[0].type = 
        a_unfocused_unpressed_max->texture[0].type = 
        a_unfocused_pressed_max->texture[0].type = 
        a_focused_unpressed_close->texture[0].type = 
        a_focused_pressed_close->texture[0].type = 
        a_unfocused_unpressed_close->texture[0].type = 
        a_unfocused_pressed_close->texture[0].type = 
        a_focused_unpressed_desk->texture[0].type = 
        a_focused_pressed_desk->texture[0].type = 
        a_unfocused_unpressed_desk->texture[0].type = 
        a_unfocused_pressed_desk->texture[0].type = 
        a_focused_unpressed_iconify->texture[0].type = 
        a_focused_pressed_iconify->texture[0].type = 
        a_unfocused_unpressed_iconify->texture[0].type = 
        a_unfocused_pressed_iconify->texture[0].type = Bitmask;
    a_focused_unpressed_max->texture[0].data.mask.mask = 
        a_focused_pressed_max->texture[0].data.mask.mask = 
        a_unfocused_unpressed_max->texture[0].data.mask.mask = 
        a_unfocused_pressed_max->texture[0].data.mask.mask = s_max_mask;
    a_focused_unpressed_close->texture[0].data.mask.mask = 
        a_focused_pressed_close->texture[0].data.mask.mask = 
        a_unfocused_unpressed_close->texture[0].data.mask.mask = 
        a_unfocused_pressed_close->texture[0].data.mask.mask = s_close_mask;
    a_focused_unpressed_desk->texture[0].data.mask.mask = 
        a_focused_pressed_desk->texture[0].data.mask.mask = 
        a_unfocused_unpressed_desk->texture[0].data.mask.mask = 
        a_unfocused_pressed_desk->texture[0].data.mask.mask = s_desk_mask;
    a_focused_unpressed_iconify->texture[0].data.mask.mask = 
        a_focused_pressed_iconify->texture[0].data.mask.mask = 
        a_unfocused_unpressed_iconify->texture[0].data.mask.mask = 
        a_unfocused_pressed_iconify->texture[0].data.mask.mask = s_icon_mask;
    a_focused_unpressed_max->texture[0].data.mask.color = 
        a_focused_pressed_max->texture[0].data.mask.color = 
        a_focused_unpressed_close->texture[0].data.mask.color = 
        a_focused_pressed_close->texture[0].data.mask.color = 
        a_focused_unpressed_desk->texture[0].data.mask.color = 
        a_focused_pressed_desk->texture[0].data.mask.color = 
        a_focused_unpressed_iconify->texture[0].data.mask.color = 
        a_focused_pressed_iconify->texture[0].data.mask.color =
        s_titlebut_focused_color;
    a_unfocused_unpressed_max->texture[0].data.mask.color = 
        a_unfocused_pressed_max->texture[0].data.mask.color = 
        a_unfocused_unpressed_close->texture[0].data.mask.color = 
        a_unfocused_pressed_close->texture[0].data.mask.color = 
        a_unfocused_unpressed_desk->texture[0].data.mask.color = 
        a_unfocused_pressed_desk->texture[0].data.mask.color = 
        a_unfocused_unpressed_iconify->texture[0].data.mask.color = 
        a_unfocused_pressed_iconify->texture[0].data.mask.color =
        s_titlebut_unfocused_color;

    XrmDestroyDatabase(db);
    return TRUE;
}


