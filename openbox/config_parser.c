/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   config_parser.c for the Openbox window manager
   Copyright (c) 2011        Dana Jansens

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

#include "config_parser.h"
#include "config.h"
#include "geom.h"
#include "obt/paths.h"
#include "obt/xml.h"

typedef struct {
    gchar *name;
    ObConfigValue *def;
    ObConfigValue *value;
    ObConfigValueDataType type;
    ObConfigValueDataPtr data;
    const ObConfigValueEnum *enum_choices;
    gchar* (*string_modify_func)(const gchar *s);
} ObConfigParserEntity;

static GHashTable *entities;

static void entity_free(ObConfigParserEntity *e)
{
    g_free(e->name);
    config_value_unref(e->def);
    config_value_unref(e->value);
    e->data.pointer = NULL; /* if it was holding a pointer, then NULL it */
    g_slice_free(ObConfigParserEntity, e);
}

void config_parser_startup(gboolean reconfig)
{
    if (reconfig) return;

    entities = g_hash_table_new_full(
        g_str_hash, g_str_equal, NULL, (GDestroyNotify)entity_free);
}

void config_parser_shutdown(gboolean reconfig)
{
    if (reconfig) return;

    g_hash_table_unref(entities);
    entities = NULL;
}

static void copy_value(ObConfigParserEntity *e)
{
    gboolean b;

    if (e->string_modify_func && config_value_is_string(e->value)) {
        gchar *s = e->string_modify_func(config_value_string(e->value));
        if (s) {
            config_value_unref(e->value);
            e->value = config_value_new_string_steal(s);
        }
    }

    switch (e->type) {
    case OB_CONFIG_VALUE_ENUM:
        b = config_value_copy_ptr(e->value, e->type, e->data, e->enum_choices);
        break;
    default:
        b = config_value_copy_ptr(e->value, e->type, e->data, NULL);
        break;
    }

    /* replace the bad value with the default (if it's not the default) */
    if (!b && e->value != e->def) {
        config_value_unref(e->value);
        e->value = e->def;
        config_value_ref(e->value);
        copy_value(e);
    }
}

static ObConfigParserEntity* add(const gchar *name, ObConfigValue *def,
                                 ObConfigValueDataType type,
                                 ObConfigValueDataPtr v)
{
    ObConfigParserEntity *e;

    g_return_val_if_fail(def != NULL, NULL);
    g_return_val_if_fail(g_hash_table_lookup(entities, name) != NULL, NULL);

    e = g_slice_new(ObConfigParserEntity);
    e->name = g_strdup(name);
    e->value = def;
    e->def = def;
    config_value_ref(def);
    config_value_ref(def);
    e->type = type;
    e->data = v;
    g_hash_table_replace(entities, e->name, e);

    copy_value(e);
    return e;
}

void config_parser_bool(const gchar *name, const gchar *def, gboolean *v)
{
    ObConfigValue *cv = config_value_new_string(def);
    add(name, cv, OB_CONFIG_VALUE_BOOLEAN, (ObConfigValueDataPtr)v);
    config_value_unref(cv);
}

void config_parser_int(const gchar *name, const gchar *def, gint *v)
{
    ObConfigValue *cv = config_value_new_string(def);
    add(name, cv, OB_CONFIG_VALUE_INTEGER, (ObConfigValueDataPtr)v);
    config_value_unref(cv);
}

void config_parser_string(const gchar *name, const gchar *def, const gchar **v)
{
    ObConfigValue *cv = config_value_new_string(def);
    add(name, cv, OB_CONFIG_VALUE_STRING, (ObConfigValueDataPtr)v);
    config_value_unref(cv);
}

void config_parser_enum(const gchar *name, const gchar *def, guint *v,
                        const ObConfigValueEnum choices[])
{
    ObConfigValue *cv;
    ObConfigParserEntity *e;

    cv = config_value_new_string(def);
    e = add(name, cv, OB_CONFIG_VALUE_ENUM, (ObConfigValueDataPtr)v);
    config_value_unref(cv);
    e->enum_choices = choices;
}

void config_parser_list(const gchar *name, GList *def, const GList **v)
{
    ObConfigValue *cv = config_value_new_list(def);
    add(name, cv, OB_CONFIG_VALUE_LIST, (ObConfigValueDataPtr)v);
    config_value_unref(cv);
}

gchar *modify_uniq(const gchar *s)
{
    gchar *c, *d, *e;
    gchar *str = g_strdup(s);
    /* look at each char c.
       scan ahead and look at each char d past c.
       if d == c, then move everything past d up one position, destroying d
    */
    for (c = str; *c != '\0'; ++c)
        for (d = c+1; *d != '\0'; ++d)
            if (*c == *d)
                for (e = d; *e != '\0'; ++e)
                    *e = *(e+1);
    return str;
}

void config_parser_string_uniq(const gchar *name, const gchar *def,
                               const gchar **v)
{
    ObConfigValue *cv;
    ObConfigParserEntity *e;

    cv = config_value_new_string(def);
    e = add(name, cv, OB_CONFIG_VALUE_STRING, (ObConfigValueDataPtr)v);
    config_value_unref(cv);
    e->string_modify_func = modify_uniq;
}

gchar *modify_path(const gchar *s)
{
    return obt_paths_expand_tilde(s);
}

void config_parser_string_path(const gchar *name, const gchar *def,
                               const gchar **v)
{
    ObConfigValue *cv;
    ObConfigParserEntity *e;

    cv = config_value_new_string(def);
    e = add(name, cv, OB_CONFIG_VALUE_STRING, (ObConfigValueDataPtr)v);
    config_value_unref(cv);
    e->string_modify_func = modify_path;
}
void config_parser_key(const gchar *name, const gchar *def,
                       const gchar **v)
{
    ObConfigValue *cv;
    ObConfigParserEntity *e;

    cv = config_value_new_string(def);
    e = add(name, cv, OB_CONFIG_VALUE_KEY, (ObConfigValueDataPtr)v);
    config_value_unref(cv);
}


#if 0

void config_parser_options(void)
{
    ObtXmlInst *i = obt_xml_instance_new();
    ObtPaths *paths;
    gchar *fpath, *dpath;
    xmlNodePtr root, n;

    paths = obt_paths_new();
    dpath = g_build_filename(obt_paths_cache_home(paths), "openbox", NULL);
    fpath = g_build_filename(dpath, "config", NULL);

    obt_paths_mkdir_path(dpath, 0777);
    if (!obt_xml_load_file(i, fpath, "openbox_config"))
        obt_xml_new_file(i, "openbox_config");
    root = obt_xml_root(i);

    n = obt_xml_tree_get_node("focus");
    config_focus_new = read_bool(n, "focusnew", TRUE);
    config_focus_follow = read_bool(n, "followmouse", FALSE);
    config_focus_delay = read_int(n, "focusdelay", 0);
    config_focus_raise = read_bool(n, "raiseonfocus", FALSE);
    config_focus_last = read_bool(n, "focuslast", TRUE);
    config_focus_under_mouse = read_bool(n, "undermouse", FALSE);
    config_unfocus_leave = read_bool(n, "unfocusonleave", FALSE);

    n = obt_xml_tree_get_node("placement");
    config_place_policy = read_enum(
        n, "policy",
        {
            {"smart", OB_PLACE_POLICY_SMART},
            {"undermouse", OB_PLACE_POLICY_MOUSE},
            {0, 0},
        }
        "smart");
    config_place_center = read_bool(n "center", TRUE);
    config_place_monitor = read_enum(
        n, "monitor",
        {
            {"any", OB_PLACE_MONITOR_ANY},
            {"active", OB_PLACE_MONITOR_ACTIVE},
            {"mouse", OB_PLACE_MONITOR_MOUSE},
            {"primary", OB_PLACE_MONITOR_PRIMARY},
            {0, 0}
        },
        "primary");
    config_primary_monitor_index = read_int(n, "primarymonitor" 1);
    if (!config_primary_monitor_index)
        config_primary_monitor = read_enum(
            n, "primarymonitor",
            {
                {"active", OB_PLACE_MONITOR_ACTIVE},
                {"mouse", OB_PLACE_MONITOR_MOUSE},
                {0, 0}
            },
            "active");

    n = obt_xml_tree_get_node("margins");
    STRUT_PARTIAL_SET(config_margins, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    config_margins.top = MAX(0, read_int(n, "top", 0));
    config_margins.left = MAX(0, read_int(n, "left", 0));
    config_margins.bottom = MAX(0, read_int(n, "bottom", 0));
    config_margins.right = MAX(0, read_int(n, "right", 0));

    n = obt_xml_tree_get_node("theme");
    config_theme = read_path(n, "name", NULL);
    config_animate_iconify = read_bool(n, "animateiconify", TRUE);
    config_title_layout = read_string_uniq(n, "titlelayout", "NLIMC");
    config_theme_keepborder = read_bool(n, "keepborder", TRUE);
    config_theme_window_list_icon_size = read_int(n, "windowlisticonsize", 36);
    config_font_activewindow = read_font(n, "font:place=activewindow");
    config_font_inactivewindow = read_font(n, "font:place=inactivewindow");
    config_font_menuitem = read_font(n, "font:place=menuitem");
    config_font_menutitle = read_font(n, "font:place=menuheader");
    config_font_activeosd = read_font(n, "font:place=activeonscreendisplay");
    config_font_inactiveosd =
        read_font(n, "font:place=inactiveonscreendisplay");

    n = obt_xml_tree_get_node("desktops");
    config_desktops_num = MAX(1, read_int(n, "number", 4));
    config_screen_firstdesk = 1; /* XXX remove me */
    config_desktop_popup_time = MAX(0, read_int(n, "popuptime", 875));
    config_desktops_names = NULL; /* XXX remove me */

    n = obt_xml_tree_get_node("resize");
    config_resize_redraw = read_bool(n, "drawcontents", TRUE);
    config_resize_popup_show = read_enum(
        n, "popupshow",
        {
            {"never", 0},
            {"nonpixel", 1},
            {"always", 2},
            {0, 0}
        },
        "nonpixel");
    config_resize_popup_pos = read_enum(
        n, "popupposition",
        {
            {"top", OB_RESIZE_POS_TOP},
            {"center", OB_RESIZE_POS_CENTER},
            {"fixed", OB_RESIZE_POS_FIXED},
        },
        "center");
    if (config_resize_popup_pos == OB_RESIZE_POS_FIXED)
        config_resize_popup_fixed =
            read_gravity_point(n, "popupfixedposition",
                               0, 0, FALSE, FALSE, 0, 0, FALSE, FALSE);

    n = obt_xml_tree_get_node("dock");
    config_dock_pos = read_enum(
        n, "position",
        {
            {"topleft", OB_DIRECTION_NORTHWEST},
            {"top", OB_DIRECTION_NORTH},
            {"topright", OB_DIRECTION_NORTHEAST},
            {"right", OB_DIRECTION_EAST},
            {"bottomright", OB_DIRECTION_SOUTHEAST},
            {"bottom", OB_DIRECTION_SOUTH},
            {"bottomleft", OB_DIRECTION_SOUTHWEST},
            {"left", OB_DIRECTION_WEST},
            {"floating", (guint)-1},
            {0, 0}
        },
        "topright");
    config_dock_floating = (config_dock_pos == (guint)-1);
    config_dock_layer = read_enum(
        n, "stacking",
        {
            {"normal", OB_STACKING_LAYER_NORMAL},
            {"above", OB_STACKING_LAYER_ABOVE},
            {"below", OB_STACKING_LAYER_BELOW},
            {0, 0}
        },
        "above");
    config_dock_nostrut = read_bool(n, "nostrut", FALSE);
    config_dock_x = read_int(n, "floatingx", 0);
    config_dock_y = read_int(n, "floatingx", y);
    config_dock_orient = read_enum(
        n, "direction",
        {
            {"horizontal", OB_ORIENTATION_HORZ},
            {"vertical", OB_ORIENTATION_VERT},
            {0, 0}
        },
        "vertical");
    config_dock_hide = read_bool(n, "autohide", FALSE);
    config_dock_hide_delay = read_int(n, "hidedelay", 300);
    config_dock_show_delay = read_int(n, "showdelay", 300);
    read_button(n, "movebutton", "2",
                &config_dock_app_move_button,
                &config_dock_app_move_modifiers);

    n = obt_xml_tree_get_node("keyboard");
    read_key(n, "chainquitkey", "C-g",
             &config_keyboard_reset_keycode,
             &config_keyboard_reset_state);

    n = obt_xml_tree_get_node("mouse");
    config_mouse_threshold = read_int(n, "dragthreshold", 8);
    config_mouse_dclicktime = read_int(n, "doubleclicktime", 200);
    config_mouse_screenedgetime = read_int(n, "screenedgewarptime", 400);
    /* minimum value of 25 for this property, when it is 1 and you hit the
       edge it basically never stops */
    if (config_mouse_screenedgetime && config_mouse_screenedgetime < 25)
        config_mouse_screenedgetime = 25;
    config_mouse_screenedgewarp = read_bool(n, "screenedgewarpmouse", FALSE);

    n = obt_xml_tree_get_node("resistance");
    config_resist_win = read_int(n, "strength", 10);
    config_resist_edge = read_int(n, "screen_edge_strength", 20);

    n = obt_xml_tree_get_node("menu");
    config_menu_hide_delay = read_int(n, "hidedelay", 250);
    config_menu_middle = read_bool(n, "middle", FALSE);
    config_submenu_show_delay = read_int(n, "submenushowdelay", 100);
    config_submenu_hide_delay = read_int(n, "submenuhidedelay", 400);
    config_menu_manage_desktops = read_bool(n, "managedesktops", TRUE);
    config_menu_show_icons = read_bool(n, "showicons", TRUE);
#ifndef USE_IMLIB2
    if (config_menu_show_icons)
        g_message(_("Openbox was compiled without Imlib2 image loading support. Icons in menus will not be loaded."));
#endif

    config_menu_files = NULL; /* XXX remove me */
}



RrFont *read_font(xmlNodePtr n, const gchar *path)
{
    RrFont *font;
    gchar *name;
    gint size;
    RrFontWeight weight;
    RrFontSlant slant;

    n = obt_xml_tree_get_node(n, path);

    name = read_string(n, "name", RrDefaultFontFamily);
    size = read_int(n, "size", RrDefaultFontSize);
    weight = read_enum(n, "weight",
                       {
                           {"normal", RR_FONTWEIGHT_NORMAL},
                           {"bold", RR_FONTWEIGHT_BOLD},
                           {0, 0}
                       },
                       "normal");
    slant = read_enum(n, "slant",
                      {
                          {"normal", RR_FONTSLANT_NORMAL},
                          {"italic", RR_FONTSLANT_ITALIC},
                          {"oblique", RR_FONTSLANT_OBLIQUE},
                      },
                      "normal");

    font = RrFontOpen(ob_rr_inst, name, size, weight, slant);
    g_free(name);
    return font;
}
#endif
