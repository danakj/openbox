#include "obengine.h"
#include "../../kernel/config.h"
#include "../../kernel/openbox.h"

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

static gboolean read_mask(XrmDatabase db, char *rname, pixmap_mask **value)
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
    ConfigValue theme;
  
    if (XrmGetResource(db, rname, rclass, &rettype, &retvalue) &&
        retvalue.addr != NULL) {
        if (!config_get("theme", Config_String, &theme))
            g_assert_not_reached(); /* where's the default!? its not set? */

	button_dir = g_strdup_printf("%s_buttons", theme.string);

        s = g_build_filename(g_get_home_dir(), ".openbox", "themes",
                             "openbox", button_dir, retvalue.addr, NULL);

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
                themename = g_path_get_basename(theme.string);
                s = g_strdup_printf("%s_buttons/%s", theme.string,
                                    themename);
                g_free(themename);
                if (XReadBitmapFileData(s, &w, &h, &b, &hx, &hy) ==
                    BitmapSuccess) 
                    ret = TRUE;
                else
                    g_message("Unable to find bitmap '%s'", s);
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

gboolean obtheme_load()
{
    XrmDatabase db = NULL;
    Justify winjust;
    char *winjuststr;
    ConfigValue theme, shadow, offset, font;

    if (config_get("theme", Config_String, &theme)) {
	db = loaddb(theme.string);
        if (db == NULL) {
	    g_warning("Failed to load the theme '%s'", theme.string);
	    g_message("Falling back to the default: '%s'", DEFAULT_THEME);
	}
    }
    if (db == NULL) {
	db = loaddb(DEFAULT_THEME);
	if (db == NULL) {
	    g_warning("Failed to load the theme '%s'.", DEFAULT_THEME);
	    return FALSE;
	}
        /* change to reflect what was actually loaded */
        theme.string = DEFAULT_THEME;
        config_set("theme", Config_String, theme);
    }

    /* load the font, not from the theme file tho, its in the config */

    if (!config_get("font.shadow", Config_Bool, &shadow)) {
        shadow.bool = TRUE; /* default */
        config_set("font.shadow", Config_Bool, shadow);
    }
    ob_s_winfont_shadow = shadow.bool;
    if (!config_get("font.shadow.offset", Config_Integer, &offset) ||
        offset.integer < 0 || offset.integer >= 10) {
        offset.integer = 1; /* default */
        config_set("font.shadow.offset", Config_Integer, offset);
    }
    ob_s_winfont_shadow_offset = offset.integer;
    if (!config_get("font", Config_String, &font)) {
        font.string = DEFAULT_FONT;
        config_set("font", Config_String, font);
    }
    ob_s_winfont = font_open(font.string);
    ob_s_winfont_height = font_height(ob_s_winfont, ob_s_winfont_shadow,
                                      ob_s_winfont_shadow_offset);

    winjust = Justify_Left;
    if (read_string(db, "window.justify", &winjuststr)) {
        if (!g_ascii_strcasecmp(winjuststr, "right"))
            winjust = Justify_Right;
        else if (!g_ascii_strcasecmp(winjuststr, "center"))
            winjust = Justify_Center;
        g_free(winjuststr);
    }

    if (!read_int(db, "handleWidth", &ob_s_handle_height) ||
	ob_s_handle_height < 0 || ob_s_handle_height > 100) ob_s_handle_height = 6;
    if (!read_int(db, "bevelWidth", &ob_s_bevel) ||
	ob_s_bevel <= 0 || ob_s_bevel > 100) ob_s_bevel = 3;
    if (!read_int(db, "borderWidth", &ob_s_bwidth) ||
	ob_s_bwidth < 0 || ob_s_bwidth > 100) ob_s_bwidth = 1;
    if (!read_int(db, "frameWidth", &ob_s_cbwidth) ||
	ob_s_cbwidth < 0 || ob_s_cbwidth > 100) ob_s_cbwidth = ob_s_bevel;

    if (!read_color(db, "borderColor", &ob_s_b_color))
	ob_s_b_color = color_new(0, 0, 0);
    if (!read_color(db, "window.frame.focusColor", &ob_s_cb_focused_color))
	ob_s_cb_focused_color = color_new(0xff, 0xff, 0xff);
    if (!read_color(db, "window.frame.unfocusColor", &ob_s_cb_unfocused_color))
	ob_s_cb_unfocused_color = color_new(0xff, 0xff, 0xff);
    if (!read_color(db, "window.label.focus.textColor",
                    &ob_s_title_focused_color))
	ob_s_title_focused_color = color_new(0xff, 0xff, 0xff);
    if (!read_color(db, "window.label.unfocus.textColor",
                    &ob_s_title_unfocused_color))
	ob_s_title_unfocused_color = color_new(0xff, 0xff, 0xff);
    if (!read_color(db, "window.button.focus.picColor",
                    &ob_s_titlebut_focused_color))
	ob_s_titlebut_focused_color = color_new(0, 0, 0);
    if (!read_color(db, "window.button.unfocus.picColor",
                    &ob_s_titlebut_unfocused_color))
	ob_s_titlebut_unfocused_color = color_new(0xff, 0xff, 0xff);

    if (read_mask(db, "window.button.max.mask", &ob_s_max_unset_mask)) {
        if (!read_mask(db, "window.button.max.toggled.mask",
                       &ob_s_max_set_mask)) {
            ob_s_max_set_mask = pixmap_mask_copy(ob_s_max_unset_mask);
        }
    } else {
        {
            char data[] = { 0x3f, 0x3f, 0x21, 0x21, 0x21, 0x3f };
            ob_s_max_unset_mask = pixmap_mask_new(6, 6, data);
        }
        {
            char data[] = { 0x3c, 0x24, 0x27, 0x3f, 0x0f, 0x0f };
            ob_s_max_set_mask = pixmap_mask_new(6, 6, data);
        }
    }

    if (!read_mask(db, "window.button.icon.mask",
                   &ob_s_iconify_mask)) {
        char data[] = { 0x00, 0x00, 0x00, 0x3f, 0x3f, 0x3f };
        ob_s_iconify_mask = pixmap_mask_new(6, 6, data);
    }

    if (read_mask(db, "window.button.stick.mask",
                   &ob_s_desk_unset_mask)) {
        if (!read_mask(db, "window.button.stick.toggled.mask",
                       &ob_s_desk_set_mask)) {
            ob_s_desk_set_mask =
                pixmap_mask_copy(ob_s_desk_unset_mask);
        }
    } else {
        {
            char data[] = { 0x33, 0x33, 0x00, 0x00, 0x33, 0x33 };
            ob_s_desk_unset_mask = pixmap_mask_new(6, 6, data);
        }
        {
            char data[] = { 0x0c, 0x0c, 0x3f, 0x3f, 0x0c, 0x0c };
            ob_s_desk_set_mask = pixmap_mask_new(6, 6, data);
        }
    }

    if (!read_mask(db, "window.button.close.mask",
                   &ob_s_close_mask)) {
        char data[] = { 0x33, 0x3f, 0x1e, 0x1e, 0x3f, 0x33 };
        ob_s_close_mask = pixmap_mask_new(6, 6, data);
    }        

    if (!read_appearance(db, "window.title.focus", ob_a_focused_title))
	set_default_appearance(ob_a_focused_title);
    if (!read_appearance(db, "window.title.unfocus", ob_a_unfocused_title))
	set_default_appearance(ob_a_unfocused_title);
    if (!read_appearance(db, "window.label.focus", ob_a_focused_label))
	set_default_appearance(ob_a_focused_label);
    if (!read_appearance(db, "window.label.unfocus", ob_a_unfocused_label))
	set_default_appearance(ob_a_unfocused_label);
    if (!read_appearance(db, "window.handle.focus", ob_a_focused_handle))
	set_default_appearance(ob_a_focused_handle);
    if (!read_appearance(db, "window.handle.unfocus", ob_a_unfocused_handle))
	set_default_appearance(ob_a_unfocused_handle);
    if (!read_appearance(db, "window.grip.focus", ob_a_focused_grip))
	set_default_appearance(ob_a_focused_grip);
    if (!read_appearance(db, "window.grip.unfocus", ob_a_unfocused_grip))
	set_default_appearance(ob_a_unfocused_grip);

    if (!read_appearance(db, "window.button.pressed.focus",
			 ob_a_focused_pressed_max))
	if (!read_appearance(db, "window.button.pressed",
                             ob_a_focused_pressed_max))
	    set_default_appearance(ob_a_focused_pressed_max);
    if (!read_appearance(db, "window.button.pressed.unfocus",
			 ob_a_unfocused_pressed_max))
	if (!read_appearance(db, "window.button.pressed",
			     ob_a_unfocused_pressed_max))
	    set_default_appearance(ob_a_unfocused_pressed_max);
    if (!read_appearance(db, "window.button.focus",
			 ob_a_focused_unpressed_max))
	set_default_appearance(ob_a_focused_unpressed_max);
    if (!read_appearance(db, "window.button.unfocus",
			 ob_a_unfocused_unpressed_max))
	set_default_appearance(ob_a_unfocused_unpressed_max);

    ob_a_unfocused_unpressed_close =
        appearance_copy(ob_a_unfocused_unpressed_max);
    ob_a_unfocused_pressed_close = appearance_copy(ob_a_unfocused_pressed_max);
    ob_a_focused_unpressed_close = appearance_copy(ob_a_focused_unpressed_max);
    ob_a_focused_pressed_close = appearance_copy(ob_a_focused_pressed_max);
    ob_a_unfocused_unpressed_desk =
        appearance_copy(ob_a_unfocused_unpressed_max);
    ob_a_unfocused_pressed_desk = appearance_copy(ob_a_unfocused_pressed_max);
    ob_a_unfocused_pressed_set_desk =
        appearance_copy(ob_a_unfocused_pressed_max);
    ob_a_focused_unpressed_desk = appearance_copy(ob_a_focused_unpressed_max);
    ob_a_focused_pressed_desk = appearance_copy(ob_a_focused_pressed_max);
    ob_a_focused_pressed_set_desk = appearance_copy(ob_a_focused_pressed_max);
    ob_a_unfocused_unpressed_iconify =
        appearance_copy(ob_a_unfocused_unpressed_max);
    ob_a_unfocused_pressed_iconify =
        appearance_copy(ob_a_unfocused_pressed_max);
    ob_a_focused_unpressed_iconify =
        appearance_copy(ob_a_focused_unpressed_max);
    ob_a_focused_pressed_iconify = appearance_copy(ob_a_focused_pressed_max);
    ob_a_unfocused_pressed_set_max =
        appearance_copy(ob_a_unfocused_pressed_max);
    ob_a_focused_pressed_set_max = appearance_copy(ob_a_focused_pressed_max);

    ob_a_icon->surface.data.planar.grad = Background_ParentRelative;

    /* set up the textures */
    ob_a_focused_label->texture[0].type = Text;
    ob_a_focused_label->texture[0].data.text.justify = winjust;
    ob_a_focused_label->texture[0].data.text.font = ob_s_winfont;
    ob_a_focused_label->texture[0].data.text.shadow = ob_s_winfont_shadow;
    ob_a_focused_label->texture[0].data.text.offset =
        ob_s_winfont_shadow_offset;
    ob_a_focused_label->texture[0].data.text.color = ob_s_title_focused_color;

    ob_a_unfocused_label->texture[0].type = Text;
    ob_a_unfocused_label->texture[0].data.text.justify = winjust;
    ob_a_unfocused_label->texture[0].data.text.font = ob_s_winfont;
    ob_a_unfocused_label->texture[0].data.text.shadow = ob_s_winfont_shadow;
    ob_a_unfocused_label->texture[0].data.text.offset =
        ob_s_winfont_shadow_offset;
    ob_a_unfocused_label->texture[0].data.text.color =
        ob_s_title_unfocused_color;

    ob_a_focused_unpressed_max->texture[0].type = 
        ob_a_focused_pressed_max->texture[0].type = 
        ob_a_focused_pressed_set_max->texture[0].type =  
        ob_a_unfocused_unpressed_max->texture[0].type = 
        ob_a_unfocused_pressed_max->texture[0].type = 
        ob_a_unfocused_pressed_set_max->texture[0].type = 
        ob_a_focused_unpressed_close->texture[0].type = 
        ob_a_focused_pressed_close->texture[0].type = 
        ob_a_unfocused_unpressed_close->texture[0].type = 
        ob_a_unfocused_pressed_close->texture[0].type = 
        ob_a_focused_unpressed_desk->texture[0].type = 
        ob_a_focused_pressed_desk->texture[0].type = 
        ob_a_focused_pressed_set_desk->texture[0].type = 
        ob_a_unfocused_unpressed_desk->texture[0].type = 
        ob_a_unfocused_pressed_desk->texture[0].type = 
        ob_a_unfocused_pressed_set_desk->texture[0].type = 
        ob_a_focused_unpressed_iconify->texture[0].type = 
        ob_a_focused_pressed_iconify->texture[0].type = 
        ob_a_unfocused_unpressed_iconify->texture[0].type = 
        ob_a_unfocused_pressed_iconify->texture[0].type = Bitmask;
    ob_a_focused_unpressed_max->texture[0].data.mask.mask = 
        ob_a_unfocused_unpressed_max->texture[0].data.mask.mask = 
        ob_a_focused_pressed_max->texture[0].data.mask.mask = 
        ob_a_unfocused_pressed_max->texture[0].data.mask.mask =
        ob_s_max_unset_mask;
    ob_a_focused_pressed_set_max->texture[0].data.mask.mask = 
        ob_a_unfocused_pressed_set_max->texture[0].data.mask.mask =
        ob_s_max_set_mask;
    ob_a_focused_pressed_close->texture[0].data.mask.mask = 
        ob_a_unfocused_pressed_close->texture[0].data.mask.mask =
        ob_a_focused_unpressed_close->texture[0].data.mask.mask = 
        ob_a_unfocused_unpressed_close->texture[0].data.mask.mask =
        ob_s_close_mask;
    ob_a_focused_unpressed_desk->texture[0].data.mask.mask = 
        ob_a_unfocused_unpressed_desk->texture[0].data.mask.mask = 
        ob_a_focused_pressed_desk->texture[0].data.mask.mask = 
        ob_a_unfocused_pressed_desk->texture[0].data.mask.mask =
        ob_s_desk_unset_mask;
    ob_a_focused_pressed_set_desk->texture[0].data.mask.mask = 
        ob_a_unfocused_pressed_set_desk->texture[0].data.mask.mask =
        ob_s_desk_set_mask;
    ob_a_focused_unpressed_iconify->texture[0].data.mask.mask = 
        ob_a_unfocused_unpressed_iconify->texture[0].data.mask.mask = 
        ob_a_focused_pressed_iconify->texture[0].data.mask.mask = 
        ob_a_unfocused_pressed_iconify->texture[0].data.mask.mask =
        ob_s_iconify_mask;
    ob_a_focused_unpressed_max->texture[0].data.mask.color = 
        ob_a_focused_pressed_max->texture[0].data.mask.color = 
        ob_a_focused_pressed_set_max->texture[0].data.mask.color = 
        ob_a_focused_unpressed_close->texture[0].data.mask.color = 
        ob_a_focused_pressed_close->texture[0].data.mask.color = 
        ob_a_focused_unpressed_desk->texture[0].data.mask.color = 
        ob_a_focused_pressed_desk->texture[0].data.mask.color = 
        ob_a_focused_pressed_set_desk->texture[0].data.mask.color = 
        ob_a_focused_unpressed_iconify->texture[0].data.mask.color = 
        ob_a_focused_pressed_iconify->texture[0].data.mask.color =
        ob_s_titlebut_focused_color;
    ob_a_unfocused_unpressed_max->texture[0].data.mask.color = 
        ob_a_unfocused_pressed_max->texture[0].data.mask.color = 
        ob_a_unfocused_pressed_set_max->texture[0].data.mask.color = 
        ob_a_unfocused_unpressed_close->texture[0].data.mask.color = 
        ob_a_unfocused_pressed_close->texture[0].data.mask.color = 
        ob_a_unfocused_unpressed_desk->texture[0].data.mask.color = 
        ob_a_unfocused_pressed_desk->texture[0].data.mask.color = 
        ob_a_unfocused_pressed_set_desk->texture[0].data.mask.color = 
        ob_a_unfocused_unpressed_iconify->texture[0].data.mask.color = 
        ob_a_unfocused_pressed_iconify->texture[0].data.mask.color =
        ob_s_titlebut_unfocused_color;

    XrmDestroyDatabase(db);
    return TRUE;
}


