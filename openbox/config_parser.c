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
    gboolean cb;
    union _ObConfigParserEntityUnion {
        struct _ObConfigParserEntityCallback {
            ObConfigParserFunc func;
            gpointer data;
        } cb;
        struct _ObConfigParserEntityValue {
            ObConfigValue *def;
            ObConfigValue *value;
            ObConfigValueDataType type;
            ObConfigValueDataPtr data;
            const ObConfigValueEnum *enum_choices;
            gchar* (*string_modify_func)(const gchar *s);
        } v;
    } u;
} ObConfigParserEntity;

struct _ObConfigParser {
    guint ref;
    GHashTable *entities;
};

static void entity_free(ObConfigParserEntity *e)
{
    g_free(e->name);
    if (!e->cb) {
        config_value_unref(e->u.v.def);
        config_value_unref(e->u.v.value);
        /* if it was holding a pointer, then NULL it */
        e->u.v.data.pointer = NULL;
    }
    g_slice_free(ObConfigParserEntity, e);
}

ObConfigParser *config_parser_new(void)
{
    ObConfigParser *p = g_slice_new(ObConfigParser);
    p->ref = 1;
    p->entities = g_hash_table_new_full(
        g_str_hash, g_str_equal, NULL, (GDestroyNotify)entity_free);
    return p;
}

void config_parser_ref(ObConfigParser *p)
{
    ++p->ref;
}

void config_parser_unref(ObConfigParser *p)
{
    if (p && --p->ref < 1) {
        g_hash_table_unref(p->entities);
        g_slice_free(ObConfigParser, p);
    }
}

static void copy_value(ObConfigParserEntity *e)
{
    gboolean b;

    g_return_if_fail(e->cb == FALSE);

    if (e->u.v.string_modify_func &&
        config_value_is_string(e->u.v.value))
    {
        gchar *s = e->u.v.string_modify_func(
            config_value_string(e->u.v.value));
        if (s) {
            config_value_unref(e->u.v.value);
            e->u.v.value = config_value_new_string_steal(s);
        }
    }

    switch (e->u.v.type) {
    case OB_CONFIG_VALUE_ENUM:
        b = config_value_copy_ptr(e->u.v.value, e->u.v.type,
                                  e->u.v.data, e->u.v.enum_choices);
        break;
    default:
        b = config_value_copy_ptr(e->u.v.value, e->u.v.type,
                                  e->u.v.data, NULL);
        break;
    }

    /* replace the bad value with the default (if it's not the default) */
    if (!b && e->u.v.value != e->u.v.def) {
        config_value_unref(e->u.v.value);
        e->u.v.value = e->u.v.def;
        config_value_ref(e->u.v.value);
        copy_value(e);
    }

    if (b && e->notify) e->notify(e->u.v.data, e->notify_data);
}

static ObConfigParserEntity* add(ObConfigParser *p,
                                 const gchar *name, ObConfigValue *def,
                                 ObConfigValueDataType type,
                                 ObConfigValueDataPtr v)
{
    ObConfigParserEntity *e;

    g_return_val_if_fail(def != NULL, NULL);
    g_return_val_if_fail(g_hash_table_lookup(p->entities, name) == NULL, NULL);

    e = g_slice_new0(ObConfigParserEntity);
    e->name = g_strdup(name);
    e->cb = FALSE;
    e->u.v.value = def;
    e->u.v.def = def;
    config_value_ref(def);
    config_value_ref(def);
    e->u.v.type = type;
    e->u.v.data = v;
    g_hash_table_replace(p->entities, e->name, e);

    copy_value(e);
    return e;
}

void config_parser_bool(ObConfigParser *p,
                        const gchar *name, const gchar *def, gboolean *v)
{
    ObConfigValue *cv = config_value_new_string(def);
    add(p, name, cv, OB_CONFIG_VALUE_BOOLEAN, (ObConfigValueDataPtr)v);
    config_value_unref(cv);
}

void clamp_int(ObConfigValue *v

void config_parser_int(ObConfigParser *p,
                       const gchar *name, const gchar *def, gint *v,
                       gint min, gint max)
{
    ObConfigValue *cv = config_value_new_string(def);
    ObConfigParserEntity *e;
    e = add(p, name, cv, OB_CONFIG_VALUE_INTEGER, (ObConfigValueDataPtr)v);
    config_value_unref(cv);

    
}

void config_parser_string(ObConfigParser *p,
                          const gchar *name, const gchar *def, const gchar **v)
{
    ObConfigValue *cv = config_value_new_string(def);
    add(p, name, cv, OB_CONFIG_VALUE_STRING, (ObConfigValueDataPtr)v);
    config_value_unref(cv);
}

void config_parser_enum(ObConfigParser *p,
                        const gchar *name, const gchar *def, guint *v,
                        const ObConfigValueEnum choices[])
{
    ObConfigValue *cv;
    ObConfigParserEntity *e;

    cv = config_value_new_string(def);
    e = add(p, name, cv, OB_CONFIG_VALUE_ENUM, (ObConfigValueDataPtr)v);
    config_value_unref(cv);
    e->u.v.enum_choices = choices;
}

void config_parser_string_list(ObConfigParser *p,
                               const gchar *name, gchar **def,
                               const gchar *const**v)
{
    ObConfigValue *cv = config_value_new_string_list(def);
    add(p, name, cv, OB_CONFIG_VALUE_STRING_LIST, (ObConfigValueDataPtr)v);
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

void config_parser_string_uniq(ObConfigParser *p,
                               const gchar *name, const gchar *def,
                               const gchar **v)
{
    ObConfigValue *cv;
    ObConfigParserEntity *e;

    cv = config_value_new_string(def);
    e = add(p, name, cv, OB_CONFIG_VALUE_STRING, (ObConfigValueDataPtr)v);
    config_value_unref(cv);
    e->u.v.string_modify_func = modify_uniq;
}

gchar *modify_path(const gchar *s)
{
    return obt_paths_expand_tilde(s);
}

void config_parser_string_path(ObConfigParser *p,
                               const gchar *name, const gchar *def,
                               const gchar **v)
{
    ObConfigValue *cv;
    ObConfigParserEntity *e;

    cv = config_value_new_string(def);
    e = add(p, name, cv, OB_CONFIG_VALUE_STRING, (ObConfigValueDataPtr)v);
    config_value_unref(cv);
    e->u.v.string_modify_func = modify_path;
}
void config_parser_key(ObConfigParser *p,
                       const gchar *name, const gchar *def,
                       const gchar **v)
{
    ObConfigValue *cv;
    ObConfigParserEntity *e;

    cv = config_value_new_string(def);
    e = add(p, name, cv, OB_CONFIG_VALUE_KEY, (ObConfigValueDataPtr)v);
    config_value_unref(cv);
}

void config_parser_callback(ObConfigParser *p, const gchar *name,
                            ObConfigParserFunc cb, gpointer data)
{
    ObConfigParserEntity *e;

    g_return_if_fail(g_hash_table_lookup(p->entities, name) == NULL);

    e = g_slice_new0(ObConfigParserEntity);
    e->name = g_strdup(name);
    e->cb = TRUE;
    e->u.cb.func = cb;
    e->u.cb.data = data;
    g_hash_table_replace(p->entities, e->name, e);
}


static void foreach_read(gpointer k, gpointer v, gpointer u)
{
    ObtXmlInst *i = u;
    ObConfigParserEntity *e = v;
    xmlNodePtr root, n;

    root = obt_xml_root(i);

    if (e->cb) {
        n = obt_xml_path_get_node(root, e->name, NULL);
        e->u.cb.func(n, e->u.cb.data);
        return;
    }

    g_return_if_fail(e->cb == FALSE);

    if (config_value_is_string(e->u.v.def)) {
        n = obt_xml_path_get_node(root, e->name,
                                  config_value_string(e->u.v.def));

        config_value_unref(e->u.v.value);
        e->u.v.value = config_value_new_string_steal(obt_xml_node_string(n));
        copy_value(e);
    }
    else if (config_value_is_string_list(e->u.v.def)) {
        GList *list;

        root = obt_xml_root(i);
        list = obt_xml_path_get_list(root, e->name);

        if (list) {
            GList *it;
            gchar **out, **c;

            c = out = g_new(gchar*, g_list_length(list) + 1);
            for (it = list; it; it = g_list_next(it)) {
                n = it->data;
                *c = obt_xml_node_string(v);
                ++c;
            }
            *c = NULL;

            config_value_unref(e->u.v.value);
            e->u.v.value = config_value_new_string_list_steal(out);
        }
    }
    else if (config_value_is_action_list(e->u.v.def))
        g_error("Unable to read action lists from config file");
    else
        g_assert_not_reached();
}

void config_parser_read(ObConfigParser *p, ObtXmlInst *i)
{
    g_hash_table_foreach(p->entities, foreach_read, i);
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
