/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   config.c for the Openbox window manager
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

#include "config.h"
#include "keyboard.h"
#include "mouse.h"
#include "actions.h"
#include "prop.h"
#include "translate.h"
#include "client.h"
#include "screen.h"
#include "parser/parse.h"
#include "openbox.h"
#include "gettext.h"

gboolean config_focus_new;
gboolean config_focus_follow;
guint    config_focus_delay;
gboolean config_focus_raise;
gboolean config_focus_last;
gboolean config_focus_under_mouse;

ObPlacePolicy  config_place_policy;
gboolean       config_place_center;
ObPlaceMonitor config_place_monitor;

StrutPartial config_margins;

gchar   *config_theme;
gboolean config_theme_keepborder;

gchar   *config_title_layout;

gboolean config_animate_iconify;

RrFont *config_font_activewindow;
RrFont *config_font_inactivewindow;
RrFont *config_font_menuitem;
RrFont *config_font_menutitle;
RrFont *config_font_osd;

guint   config_desktops_num;
GSList *config_desktops_names;
guint   config_screen_firstdesk;
guint   config_desktop_popup_time;

gboolean         config_resize_redraw;
gint             config_resize_popup_show;
ObResizePopupPos config_resize_popup_pos;
GravityPoint     config_resize_popup_fixed;

ObStackingLayer config_dock_layer;
gboolean        config_dock_floating;
gboolean        config_dock_nostrut;
ObDirection     config_dock_pos;
gint            config_dock_x;
gint            config_dock_y;
ObOrientation   config_dock_orient;
gboolean        config_dock_hide;
guint           config_dock_hide_delay;
guint           config_dock_show_delay;
guint           config_dock_app_move_button;
guint           config_dock_app_move_modifiers;

guint config_keyboard_reset_keycode;
guint config_keyboard_reset_state;

gint config_mouse_threshold;
gint config_mouse_dclicktime;
gint config_mouse_screenedgetime;

guint    config_menu_hide_delay;
gboolean config_menu_middle;
guint    config_submenu_show_delay;
gboolean config_menu_client_list_icons;

GSList *config_menu_files;

gint     config_resist_win;
gint     config_resist_edge;

GSList *config_per_app_settings;

ObAppSettings* config_create_app_settings(void)
{
    ObAppSettings *settings = g_new0(ObAppSettings, 1);
    settings->decor = -1;
    settings->shade = -1;
    settings->monitor = -1;
    settings->focus = -1;
    settings->desktop = 0;
    settings->layer = -2;
    settings->iconic = -1;
    settings->skip_pager = -1;
    settings->skip_taskbar = -1;
    settings->fullscreen = -1;
    settings->max_horz = -1;
    settings->max_vert = -1;
    return settings;
}

#define copy_if(setting, default) \
  if (src->setting != default) dst->setting = src->setting
void config_app_settings_copy_non_defaults(const ObAppSettings *src,
                                           ObAppSettings *dst)
{
    g_assert(src != NULL);
    g_assert(dst != NULL);

    copy_if(decor, -1);
    copy_if(shade, -1);
    copy_if(focus, -1);
    copy_if(desktop, 0);
    copy_if(layer, -2);
    copy_if(iconic, -1);
    copy_if(skip_pager, -1);
    copy_if(skip_taskbar, -1);
    copy_if(fullscreen, -1);
    copy_if(max_horz, -1);
    copy_if(max_vert, -1);

    if (src->pos_given) {
        dst->pos_given = TRUE;
        dst->position = src->position;
        dst->monitor = src->monitor;
    }
}

static void config_parse_gravity_coord(xmlDocPtr doc, xmlNodePtr node,
                                       GravityCoord *c)
{
    gchar *s = parse_string(doc, node);
    if (!g_ascii_strcasecmp(s, "center"))
        c->center = TRUE;
    else {
        if (s[0] == '-')
            c->opposite = TRUE;
        if (s[0] == '-' || s[0] == '+')
            c->pos = atoi(s+1);
        else
            c->pos = atoi(s);
    }
    g_free(s);
}

/*
  <applications>
    <application name="aterm">
      <decor>false</decor>
    </application>
    <application name="Rhythmbox">
      <layer>above</layer>
      <position>
        <x>700</x>
        <y>0</y>
        <monitor>1</monitor>
      </position>
      .. there is a lot more settings available
    </application>
  </applications>
*/

/* Manages settings for individual applications.
   Some notes: monitor is the screen number in a multi monitor
   (Xinerama) setup (starting from 0) or mouse, meaning the
   monitor the pointer is on. Default: mouse.
   Layer can be three values, above (Always on top), below
   (Always on bottom) and everything else (normal behaviour).
   Positions can be an integer value or center, which will
   center the window in the specified axis. Position is within
   the monitor, so <position><x>center</x></position><monitor>2</monitor>
   will center the window on the second monitor.
*/
static void parse_per_app_settings(ObParseInst *inst, xmlDocPtr doc,
                                   xmlNodePtr node, gpointer data)
{
    xmlNodePtr app = parse_find_node("application", node->children);
    gchar *name = NULL, *class = NULL, *role = NULL;
    gboolean name_set, class_set;
    gboolean x_pos_given;

    while (app) {
        name_set = class_set = x_pos_given = FALSE;

        class_set = parse_attr_string("class", app, &class);
        name_set = parse_attr_string("name", app, &name);
        if (class_set || name_set) {
            xmlNodePtr n, c;
            ObAppSettings *settings = config_create_app_settings();;

            if (name_set)
                settings->name = g_pattern_spec_new(name);

            if (class_set)
                settings->class = g_pattern_spec_new(class);

            if (parse_attr_string("role", app, &role))
                settings->role = g_pattern_spec_new(role);

            if ((n = parse_find_node("decor", app->children)))
                if (!parse_contains("default", doc, n))
                    settings->decor = parse_bool(doc, n);

            if ((n = parse_find_node("shade", app->children)))
                if (!parse_contains("default", doc, n))
                    settings->shade = parse_bool(doc, n);

            if ((n = parse_find_node("position", app->children))) {
                if ((c = parse_find_node("x", n->children)))
                    if (!parse_contains("default", doc, c)) {
                        config_parse_gravity_coord(doc, c,
                                                   &settings->position.x);
                        x_pos_given = TRUE;
                    }

                if (x_pos_given && (c = parse_find_node("y", n->children)))
                    if (!parse_contains("default", doc, c)) {
                        config_parse_gravity_coord(doc, c,
                                                   &settings->position.y);
                        settings->pos_given = TRUE;
                    }

                if (settings->pos_given &&
                    (c = parse_find_node("monitor", n->children)))
                    if (!parse_contains("default", doc, c)) {
                        gchar *s = parse_string(doc, c);
                        if (!g_ascii_strcasecmp(s, "mouse"))
                            settings->monitor = 0;
                        else
                            settings->monitor = parse_int(doc, c) + 1;
                        g_free(s);
                    }
            }

            if ((n = parse_find_node("focus", app->children)))
                if (!parse_contains("default", doc, n))
                    settings->focus = parse_bool(doc, n);

            if ((n = parse_find_node("desktop", app->children))) {
                if (!parse_contains("default", doc, n)) {
                    gchar *s = parse_string(doc, n);
                    if (!g_ascii_strcasecmp(s, "all"))
                        settings->desktop = DESKTOP_ALL;
                    else {
                        gint i = parse_int(doc, n);
                        if (i > 0)
                            settings->desktop = i;
                    }
                    g_free(s);
                }
            }

            if ((n = parse_find_node("layer", app->children)))
                if (!parse_contains("default", doc, n)) {
                    gchar *s = parse_string(doc, n);
                    if (!g_ascii_strcasecmp(s, "above"))
                        settings->layer = 1;
                    else if (!g_ascii_strcasecmp(s, "below"))
                        settings->layer = -1;
                    else
                        settings->layer = 0;
                    g_free(s);
                }

            if ((n = parse_find_node("iconic", app->children)))
                if (!parse_contains("default", doc, n))
                    settings->iconic = parse_bool(doc, n);

            if ((n = parse_find_node("skip_pager", app->children)))
                if (!parse_contains("default", doc, n))
                    settings->skip_pager = parse_bool(doc, n);

            if ((n = parse_find_node("skip_taskbar", app->children)))
                if (!parse_contains("default", doc, n))
                    settings->skip_taskbar = parse_bool(doc, n);

            if ((n = parse_find_node("fullscreen", app->children)))
                if (!parse_contains("default", doc, n))
                    settings->fullscreen = parse_bool(doc, n);

            if ((n = parse_find_node("maximized", app->children)))
                if (!parse_contains("default", doc, n)) {
                    gchar *s = parse_string(doc, n);
                    if (!g_ascii_strcasecmp(s, "horizontal")) {
                        settings->max_horz = TRUE;
                        settings->max_vert = FALSE;
                    } else if (!g_ascii_strcasecmp(s, "vertical")) {
                        settings->max_horz = FALSE;
                        settings->max_vert = TRUE;
                    } else
                        settings->max_horz = settings->max_vert =
                            parse_bool(doc, n);
                    g_free(s);
                }

            config_per_app_settings = g_slist_append(config_per_app_settings,
                                              (gpointer) settings);
            g_free(name);
            g_free(class);
            g_free(role);
            name = class = role = NULL;
        }

        app = parse_find_node("application", app->next);
    }
}

/*

<keybind key="C-x">
  <action name="ChangeDesktop">
    <desktop>3</desktop>
  </action>
</keybind>

*/

static void parse_key(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                      GList *keylist)
{
    gchar *key;
    xmlNodePtr n;
    gboolean is_chroot = FALSE;

    if (!parse_attr_string("key", node, &key))
        return;

    parse_attr_bool("chroot", node, &is_chroot);

    keylist = g_list_append(keylist, key);

    if ((n = parse_find_node("keybind", node->children))) {
        while (n) {
            parse_key(i, doc, n, keylist);
            n = parse_find_node("keybind", n->next);
        }
    }
    else if ((n = parse_find_node("action", node->children))) {
        while (n) {
            ObActionsAct *action;

            action = actions_parse(i, doc, n);
            if (action)
                keyboard_bind(keylist, action);
            n = parse_find_node("action", n->next);
        }
    }

    if (is_chroot)
        keyboard_chroot(keylist);

    g_free(key);
    keylist = g_list_delete_link(keylist, g_list_last(keylist));
}

static void parse_keyboard(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                           gpointer data)
{
    xmlNodePtr n;
    gchar *key;

    keyboard_unbind_all();

    if ((n = parse_find_node("chainQuitKey", node->children))) {
        key = parse_string(doc, n);
        translate_key(key, &config_keyboard_reset_state,
                      &config_keyboard_reset_keycode);
        g_free(key);
    }

    if ((n = parse_find_node("keybind", node->children)))
        while (n) {
            parse_key(i, doc, n, NULL);
            n = parse_find_node("keybind", n->next);
        }
}

/*

<context name="Titlebar">
  <mousebind button="Left" action="Press">
    <action name="Raise"></action>
  </mousebind>
</context>

*/

static void parse_mouse(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                        gpointer data)
{
    xmlNodePtr n, nbut, nact;
    gchar *buttonstr;
    gchar *contextstr;
    ObMouseAction mact;

    mouse_unbind_all();

    node = node->children;

    if ((n = parse_find_node("dragThreshold", node)))
        config_mouse_threshold = parse_int(doc, n);
    if ((n = parse_find_node("doubleClickTime", node)))
        config_mouse_dclicktime = parse_int(doc, n);
    if ((n = parse_find_node("screenEdgeWarpTime", node)))
        config_mouse_screenedgetime = parse_int(doc, n);

    n = parse_find_node("context", node);
    while (n) {
        if (!parse_attr_string("name", n, &contextstr))
            goto next_n;
        nbut = parse_find_node("mousebind", n->children);
        while (nbut) {
            if (!parse_attr_string("button", nbut, &buttonstr))
                goto next_nbut;
            if (parse_attr_contains("press", nbut, "action")) {
                mact = OB_MOUSE_ACTION_PRESS;
            } else if (parse_attr_contains("release", nbut, "action")) {
                mact = OB_MOUSE_ACTION_RELEASE;
            } else if (parse_attr_contains("click", nbut, "action")) {
                mact = OB_MOUSE_ACTION_CLICK;
            } else if (parse_attr_contains("doubleclick", nbut,"action")) {
                mact = OB_MOUSE_ACTION_DOUBLE_CLICK;
            } else if (parse_attr_contains("drag", nbut, "action")) {
                mact = OB_MOUSE_ACTION_MOTION;
            } else
                goto next_nbut;
            nact = parse_find_node("action", nbut->children);
            while (nact) {
                ObActionsAct *action;

                if ((action = actions_parse(i, doc, nact)))
                    mouse_bind(buttonstr, contextstr, mact, action);
                nact = parse_find_node("action", nact->next);
            }
            g_free(buttonstr);
        next_nbut:
            nbut = parse_find_node("mousebind", nbut->next);
        }
        g_free(contextstr);
    next_n:
        n = parse_find_node("context", n->next);
    }
}

static void parse_focus(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                        gpointer data)
{
    xmlNodePtr n;

    node = node->children;

    if ((n = parse_find_node("focusNew", node)))
        config_focus_new = parse_bool(doc, n);
    if ((n = parse_find_node("followMouse", node)))
        config_focus_follow = parse_bool(doc, n);
    if ((n = parse_find_node("focusDelay", node)))
        config_focus_delay = parse_int(doc, n);
    if ((n = parse_find_node("raiseOnFocus", node)))
        config_focus_raise = parse_bool(doc, n);
    if ((n = parse_find_node("focusLast", node)))
        config_focus_last = parse_bool(doc, n);
    if ((n = parse_find_node("underMouse", node)))
        config_focus_under_mouse = parse_bool(doc, n);
}

static void parse_placement(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                            gpointer data)
{
    xmlNodePtr n;

    node = node->children;

    if ((n = parse_find_node("policy", node)))
        if (parse_contains("UnderMouse", doc, n))
            config_place_policy = OB_PLACE_POLICY_MOUSE;
    if ((n = parse_find_node("center", node)))
        config_place_center = parse_bool(doc, n);
    if ((n = parse_find_node("placeOn", node))) {
        if (parse_contains("active", doc, n))
            config_place_monitor = OB_PLACE_MONITOR_ACTIVE;
        else if (parse_contains("mouse", doc, n))
            config_place_monitor = OB_PLACE_MONITOR_MOUSE;
    }
}

static void parse_margins(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                          gpointer data)
{
    xmlNodePtr n;

    node = node->children;

    if ((n = parse_find_node("top", node)))
        config_margins.top = MAX(0, parse_int(doc, n));
    if ((n = parse_find_node("left", node)))
        config_margins.left = MAX(0, parse_int(doc, n));
    if ((n = parse_find_node("right", node)))
        config_margins.right = MAX(0, parse_int(doc, n));
    if ((n = parse_find_node("bottom", node)))
        config_margins.bottom = MAX(0, parse_int(doc, n));
}

static void parse_theme(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                        gpointer data)
{
    xmlNodePtr n;

    node = node->children;

    if ((n = parse_find_node("name", node))) {
        gchar *c;

        g_free(config_theme);
        c = parse_string(doc, n);
        config_theme = parse_expand_tilde(c);
        g_free(c);
    }
    if ((n = parse_find_node("titleLayout", node))) {
        gchar *c, *d;

        g_free(config_title_layout);
        config_title_layout = parse_string(doc, n);

        /* replace duplicates with spaces */
        for (c = config_title_layout; *c != '\0'; ++c)
            for (d = c+1; *d != '\0'; ++d)
                if (*c == *d) *d = ' ';
    }
    if ((n = parse_find_node("keepBorder", node)))
        config_theme_keepborder = parse_bool(doc, n);
    if ((n = parse_find_node("animateIconify", node)))
        config_animate_iconify = parse_bool(doc, n);

    n = parse_find_node("font", node);
    while (n) {
        xmlNodePtr   fnode;
        RrFont     **font;
        gchar       *name = g_strdup(RrDefaultFontFamily);
        gint         size = RrDefaultFontSize;
        RrFontWeight weight = RrDefaultFontWeight;
        RrFontSlant  slant = RrDefaultFontSlant;

        if (parse_attr_contains("ActiveWindow", n, "place"))
            font = &config_font_activewindow;
        else if (parse_attr_contains("InactiveWindow", n, "place"))
            font = &config_font_inactivewindow;
        else if (parse_attr_contains("MenuHeader", n, "place"))
            font = &config_font_menutitle;
        else if (parse_attr_contains("MenuItem", n, "place"))
            font = &config_font_menuitem;
        else if (parse_attr_contains("OnScreenDisplay", n, "place"))
            font = &config_font_osd;
        else
            goto next_font;

        if ((fnode = parse_find_node("name", n->children))) {
            g_free(name);
            name = parse_string(doc, fnode);
        }
        if ((fnode = parse_find_node("size", n->children))) {
            int s = parse_int(doc, fnode);
            if (s > 0) size = s;
        }
        if ((fnode = parse_find_node("weight", n->children))) {
            gchar *w = parse_string(doc, fnode);
            if (!g_ascii_strcasecmp(w, "Bold"))
                weight = RR_FONTWEIGHT_BOLD;
            g_free(w);
        }
        if ((fnode = parse_find_node("slant", n->children))) {
            gchar *s = parse_string(doc, fnode);
            if (!g_ascii_strcasecmp(s, "Italic"))
                slant = RR_FONTSLANT_ITALIC;
            if (!g_ascii_strcasecmp(s, "Oblique"))
                slant = RR_FONTSLANT_OBLIQUE;
            g_free(s);
        }

        *font = RrFontOpen(ob_rr_inst, name, size, weight, slant);
        g_free(name);
    next_font:
        n = parse_find_node("font", n->next);
    }
}

static void parse_desktops(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                           gpointer data)
{
    xmlNodePtr n;

    node = node->children;

    if ((n = parse_find_node("number", node))) {
        gint d = parse_int(doc, n);
        if (d > 0)
            config_desktops_num = (unsigned) d;
    }
    if ((n = parse_find_node("firstdesk", node))) {
        gint d = parse_int(doc, n);
        if (d > 0)
            config_screen_firstdesk = (unsigned) d;
    }
    if ((n = parse_find_node("names", node))) {
        GSList *it;
        xmlNodePtr nname;

        for (it = config_desktops_names; it; it = it->next)
            g_free(it->data);
        g_slist_free(config_desktops_names);
        config_desktops_names = NULL;

        nname = parse_find_node("name", n->children);
        while (nname) {
            config_desktops_names = g_slist_append(config_desktops_names,
                                                   parse_string(doc, nname));
            nname = parse_find_node("name", nname->next);
        }
    }
    if ((n = parse_find_node("popupTime", node)))
        config_desktop_popup_time = parse_int(doc, n);
}

static void parse_resize(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                         gpointer data)
{
    xmlNodePtr n;

    node = node->children;

    if ((n = parse_find_node("drawContents", node)))
        config_resize_redraw = parse_bool(doc, n);
    if ((n = parse_find_node("popupShow", node))) {
        config_resize_popup_show = parse_int(doc, n);
        if (parse_contains("Always", doc, n))
            config_resize_popup_show = 2;
        else if (parse_contains("Never", doc, n))
            config_resize_popup_show = 0;
        else if (parse_contains("Nonpixel", doc, n))
            config_resize_popup_show = 1;
    }
    if ((n = parse_find_node("popupPosition", node))) {
        if (parse_contains("Top", doc, n))
            config_resize_popup_pos = OB_RESIZE_POS_TOP;
        else if (parse_contains("Center", doc, n))
            config_resize_popup_pos = OB_RESIZE_POS_CENTER;
        else if (parse_contains("Fixed", doc, n)) {
            config_resize_popup_pos = OB_RESIZE_POS_FIXED;

            if ((n = parse_find_node("popupFixedPosition", node))) {
                xmlNodePtr n2;

                if ((n2 = parse_find_node("x", n->children)))
                    config_parse_gravity_coord(doc, n2,
                                               &config_resize_popup_fixed.x);
                if ((n2 = parse_find_node("y", n->children)))
                    config_parse_gravity_coord(doc, n2,
                                               &config_resize_popup_fixed.y);
            }
        }
    }
}

static void parse_dock(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                       gpointer data)
{
    xmlNodePtr n;

    node = node->children;

    if ((n = parse_find_node("position", node))) {
        if (parse_contains("TopLeft", doc, n))
            config_dock_floating = FALSE,
            config_dock_pos = OB_DIRECTION_NORTHWEST;
        else if (parse_contains("Top", doc, n))
            config_dock_floating = FALSE,
            config_dock_pos = OB_DIRECTION_NORTH;
        else if (parse_contains("TopRight", doc, n))
            config_dock_floating = FALSE,
            config_dock_pos = OB_DIRECTION_NORTHEAST;
        else if (parse_contains("Right", doc, n))
            config_dock_floating = FALSE,
            config_dock_pos = OB_DIRECTION_EAST;
        else if (parse_contains("BottomRight", doc, n))
            config_dock_floating = FALSE,
            config_dock_pos = OB_DIRECTION_SOUTHEAST;
        else if (parse_contains("Bottom", doc, n))
            config_dock_floating = FALSE,
            config_dock_pos = OB_DIRECTION_SOUTH;
        else if (parse_contains("BottomLeft", doc, n))
            config_dock_floating = FALSE,
            config_dock_pos = OB_DIRECTION_SOUTHWEST;
        else if (parse_contains("Left", doc, n))
            config_dock_floating = FALSE,
            config_dock_pos = OB_DIRECTION_WEST;
        else if (parse_contains("Floating", doc, n))
            config_dock_floating = TRUE;
    }
    if (config_dock_floating) {
        if ((n = parse_find_node("floatingX", node)))
            config_dock_x = parse_int(doc, n);
        if ((n = parse_find_node("floatingY", node)))
            config_dock_y = parse_int(doc, n);
    } else {
        if ((n = parse_find_node("noStrut", node)))
            config_dock_nostrut = parse_bool(doc, n);
    }
    if ((n = parse_find_node("stacking", node))) {
        if (parse_contains("above", doc, n))
            config_dock_layer = OB_STACKING_LAYER_ABOVE;
        else if (parse_contains("normal", doc, n))
            config_dock_layer = OB_STACKING_LAYER_NORMAL;
        else if (parse_contains("below", doc, n))
            config_dock_layer = OB_STACKING_LAYER_BELOW;
    }
    if ((n = parse_find_node("direction", node))) {
        if (parse_contains("horizontal", doc, n))
            config_dock_orient = OB_ORIENTATION_HORZ;
        else if (parse_contains("vertical", doc, n))
            config_dock_orient = OB_ORIENTATION_VERT;
    }
    if ((n = parse_find_node("autoHide", node)))
        config_dock_hide = parse_bool(doc, n);
    if ((n = parse_find_node("hideDelay", node)))
        config_dock_hide_delay = parse_int(doc, n);
    if ((n = parse_find_node("showDelay", node)))
        config_dock_show_delay = parse_int(doc, n);
    if ((n = parse_find_node("moveButton", node))) {
        gchar *str = parse_string(doc, n);
        guint b, s;
        if (translate_button(str, &s, &b)) {
            config_dock_app_move_button = b;
            config_dock_app_move_modifiers = s;
        } else {
            g_message(_("Invalid button '%s' specified in config file"), str);
        }
        g_free(str);
    }
}

static void parse_menu(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                       gpointer data)
{
    xmlNodePtr n;
    for (node = node->children; node; node = node->next) {
        if (!xmlStrcasecmp(node->name, (const xmlChar*) "file")) {
            gchar *c;

            c = parse_string(doc, node);
            config_menu_files = g_slist_append(config_menu_files,
                                               parse_expand_tilde(c));
            g_free(c);
        }
        if ((n = parse_find_node("hideDelay", node)))
            config_menu_hide_delay = parse_int(doc, n);
        if ((n = parse_find_node("middle", node)))
            config_menu_middle = parse_bool(doc, n);
        if ((n = parse_find_node("submenuShowDelay", node)))
            config_submenu_show_delay = parse_int(doc, n);
        if ((n = parse_find_node("applicationIcons", node)))
            config_menu_client_list_icons = parse_bool(doc, n);
    }
}

static void parse_resistance(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                             gpointer data)
{
    xmlNodePtr n;

    node = node->children;
    if ((n = parse_find_node("strength", node)))
        config_resist_win = parse_int(doc, n);
    if ((n = parse_find_node("screen_edge_strength", node)))
        config_resist_edge = parse_int(doc, n);
}

typedef struct
{
    const gchar *key;
    const gchar *actname;
} ObDefKeyBind;

static void bind_default_keyboard(void)
{
    ObDefKeyBind *it;
    ObDefKeyBind binds[] = {
        { "A-Tab", "NextWindow" },
        { "S-A-Tab", "PreviousWindow" },
        { "A-F4", "Close" },
        { NULL, NULL }
    };
    for (it = binds; it->key; ++it) {
        GList *l = g_list_append(NULL, g_strdup(it->key));
        keyboard_bind(l, actions_parse_string(it->actname));
    }
}

typedef struct
{
    const gchar *button;
    const gchar *context;
    const ObMouseAction mact;
    const gchar *actname;
} ObDefMouseBind;

static void bind_default_mouse(void)
{
    ObDefMouseBind *it;
    ObDefMouseBind binds[] = {
        { "Left", "Client", OB_MOUSE_ACTION_PRESS, "Focus" },
        { "Middle", "Client", OB_MOUSE_ACTION_PRESS, "Focus" },
        { "Right", "Client", OB_MOUSE_ACTION_PRESS, "Focus" },
        { "Left", "Desktop", OB_MOUSE_ACTION_PRESS, "Focus" },
        { "Middle", "Desktop", OB_MOUSE_ACTION_PRESS, "Focus" },
        { "Right", "Desktop", OB_MOUSE_ACTION_PRESS, "Focus" },
        { "Left", "Titlebar", OB_MOUSE_ACTION_PRESS, "Focus" },
        { "Left", "Bottom", OB_MOUSE_ACTION_PRESS, "Focus" },
        { "Left", "BLCorner", OB_MOUSE_ACTION_PRESS, "Focus" },
        { "Left", "BRCorner", OB_MOUSE_ACTION_PRESS, "Focus" },
        { "Left", "TLCorner", OB_MOUSE_ACTION_PRESS, "Focus" },
        { "Left", "TRCorner", OB_MOUSE_ACTION_PRESS, "Focus" },
        { "Left", "Close", OB_MOUSE_ACTION_PRESS, "Focus" },
        { "Left", "Maximize", OB_MOUSE_ACTION_PRESS, "Focus" },
        { "Left", "Iconify", OB_MOUSE_ACTION_PRESS, "Focus" },
        { "Left", "Icon", OB_MOUSE_ACTION_PRESS, "Focus" },
        { "Left", "AllDesktops", OB_MOUSE_ACTION_PRESS, "Focus" },
        { "Left", "Shade", OB_MOUSE_ACTION_PRESS, "Focus" },
        { "Left", "Client", OB_MOUSE_ACTION_CLICK, "Raise" },
        { "Left", "Titlebar", OB_MOUSE_ACTION_CLICK, "Raise" },
        { "Middle", "Titlebar", OB_MOUSE_ACTION_CLICK, "Lower" },
        { "Left", "BLCorner", OB_MOUSE_ACTION_CLICK, "Raise" },
        { "Left", "BRCorner", OB_MOUSE_ACTION_CLICK, "Raise" },
        { "Left", "TLCorner", OB_MOUSE_ACTION_CLICK, "Raise" },
        { "Left", "TRCorner", OB_MOUSE_ACTION_CLICK, "Raise" },
        { "Left", "Close", OB_MOUSE_ACTION_CLICK, "Raise" },
        { "Left", "Maximize", OB_MOUSE_ACTION_CLICK, "Raise" },
        { "Left", "Iconify", OB_MOUSE_ACTION_CLICK, "Raise" },
        { "Left", "Icon", OB_MOUSE_ACTION_CLICK, "Raise" },
        { "Left", "AllDesktops", OB_MOUSE_ACTION_CLICK, "Raise" },
        { "Left", "Shade", OB_MOUSE_ACTION_CLICK, "Raise" },
        { "Left", "Close", OB_MOUSE_ACTION_CLICK, "Close" },
        { "Left", "Maximize", OB_MOUSE_ACTION_CLICK, "ToggleMaximize" },
        { "Left", "Iconify", OB_MOUSE_ACTION_CLICK, "Iconify" },
        { "Left", "AllDesktops", OB_MOUSE_ACTION_CLICK, "ToggleOmnipresent" },
        { "Left", "Shade", OB_MOUSE_ACTION_CLICK, "ToggleShade" },
        { "Left", "TLCorner", OB_MOUSE_ACTION_MOTION, "Resize" },
        { "Left", "TRCorner", OB_MOUSE_ACTION_MOTION, "Resize" },
        { "Left", "BLCorner", OB_MOUSE_ACTION_MOTION, "Resize" },
        { "Left", "BRCorner", OB_MOUSE_ACTION_MOTION, "Resize" },
        { "Left", "Top", OB_MOUSE_ACTION_MOTION, "Resize" },
        { "Left", "Bottom", OB_MOUSE_ACTION_MOTION, "Resize" },
        { "Left", "Left", OB_MOUSE_ACTION_MOTION, "Resize" },
        { "Left", "Right", OB_MOUSE_ACTION_MOTION, "Resize" },
        { "Left", "Titlebar", OB_MOUSE_ACTION_MOTION, "Move" },
        { "A-Left", "Frame", OB_MOUSE_ACTION_MOTION, "Move" },
        { "A-Middle", "Frame", OB_MOUSE_ACTION_MOTION, "Resize" },
        { NULL, NULL, 0, NULL }
    };

    for (it = binds; it->button; ++it)
        mouse_bind(it->button, it->context, it->mact,
                   actions_parse_string(it->actname));
}

void config_startup(ObParseInst *i)
{
    config_focus_new = TRUE;
    config_focus_follow = FALSE;
    config_focus_delay = 0;
    config_focus_raise = FALSE;
    config_focus_last = TRUE;
    config_focus_under_mouse = FALSE;

    parse_register(i, "focus", parse_focus, NULL);

    config_place_policy = OB_PLACE_POLICY_SMART;
    config_place_center = TRUE;
    config_place_monitor = OB_PLACE_MONITOR_ANY;

    parse_register(i, "placement", parse_placement, NULL);

    STRUT_PARTIAL_SET(config_margins, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

    parse_register(i, "margins", parse_margins, NULL);

    config_theme = NULL;

    config_animate_iconify = TRUE;
    config_title_layout = g_strdup("NLIMC");
    config_theme_keepborder = TRUE;

    config_font_activewindow = NULL;
    config_font_inactivewindow = NULL;
    config_font_menuitem = NULL;
    config_font_menutitle = NULL;

    parse_register(i, "theme", parse_theme, NULL);

    config_desktops_num = 4;
    config_screen_firstdesk = 1;
    config_desktops_names = NULL;
    config_desktop_popup_time = 875;

    parse_register(i, "desktops", parse_desktops, NULL);

    config_resize_redraw = TRUE;
    config_resize_popup_show = 1; /* nonpixel increments */
    config_resize_popup_pos = OB_RESIZE_POS_CENTER;
    GRAVITY_COORD_SET(config_resize_popup_fixed.x, 0, FALSE, FALSE);
    GRAVITY_COORD_SET(config_resize_popup_fixed.y, 0, FALSE, FALSE);

    parse_register(i, "resize", parse_resize, NULL);

    config_dock_layer = OB_STACKING_LAYER_ABOVE;
    config_dock_pos = OB_DIRECTION_NORTHEAST;
    config_dock_floating = FALSE;
    config_dock_nostrut = FALSE;
    config_dock_x = 0;
    config_dock_y = 0;
    config_dock_orient = OB_ORIENTATION_VERT;
    config_dock_hide = FALSE;
    config_dock_hide_delay = 300;
    config_dock_show_delay = 300;
    config_dock_app_move_button = 2; /* middle */
    config_dock_app_move_modifiers = 0;

    parse_register(i, "dock", parse_dock, NULL);

    translate_key("C-g", &config_keyboard_reset_state,
                  &config_keyboard_reset_keycode);

    bind_default_keyboard();

    parse_register(i, "keyboard", parse_keyboard, NULL);

    config_mouse_threshold = 8;
    config_mouse_dclicktime = 200;
    config_mouse_screenedgetime = 400;

    bind_default_mouse();

    parse_register(i, "mouse", parse_mouse, NULL);

    config_resist_win = 10;
    config_resist_edge = 20;

    parse_register(i, "resistance", parse_resistance, NULL);

    config_menu_hide_delay = 250;
    config_menu_middle = FALSE;
    config_submenu_show_delay = 0;
    config_menu_client_list_icons = TRUE;
    config_menu_files = NULL;

    parse_register(i, "menu", parse_menu, NULL);

    config_per_app_settings = NULL;

    parse_register(i, "applications", parse_per_app_settings, NULL);
}

void config_shutdown(void)
{
    GSList *it;

    g_free(config_theme);

    g_free(config_title_layout);

    RrFontClose(config_font_activewindow);
    RrFontClose(config_font_inactivewindow);
    RrFontClose(config_font_menuitem);
    RrFontClose(config_font_menutitle);
    RrFontClose(config_font_osd);

    for (it = config_desktops_names; it; it = g_slist_next(it))
        g_free(it->data);
    g_slist_free(config_desktops_names);

    for (it = config_menu_files; it; it = g_slist_next(it))
        g_free(it->data);
    g_slist_free(config_menu_files);

    for (it = config_per_app_settings; it; it = g_slist_next(it)) {
        ObAppSettings *itd = (ObAppSettings *)it->data;
        if (itd->name)  g_pattern_spec_free(itd->name);
        if (itd->role)  g_pattern_spec_free(itd->role);
        if (itd->class) g_pattern_spec_free(itd->class);
        g_free(it->data);
    }
    g_slist_free(config_per_app_settings);
}
