#include "config.h"
#include "keyboard.h"
#include "mouse.h"
#include "prop.h"
#include "translate.h"
#include "parser/parse.h"
#include "openbox.h"

gboolean config_focus_new;
gboolean config_focus_follow;
gboolean config_focus_last;
gboolean config_focus_last_on_desktop;
guint    config_focus_delay;

char *config_theme;

gchar *config_title_layout;

int     config_desktops_num;
GSList *config_desktops_names;

gboolean config_redraw_resize;

ObStackingLayer config_dock_layer;
gboolean        config_dock_floating;
ObDirection     config_dock_pos;
gint            config_dock_x;
gint            config_dock_y;
ObOrientation   config_dock_orient;
gboolean        config_dock_hide;
guint           config_dock_hide_timeout;

guint config_keyboard_reset_keycode;
guint config_keyboard_reset_state;

gint config_mouse_threshold;
gint config_mouse_dclicktime;

GSList *config_menu_files;

gint config_resist_win;
gint config_resist_edge;

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
    char *key;
    ObAction *action;
    xmlNodePtr n, nact;
    GList *it;

    if ((n = parse_find_node("chainQuitKey", node))) {
        key = parse_string(doc, n);
        translate_key(key, &config_keyboard_reset_state,
                      &config_keyboard_reset_keycode);
        g_free(key);
    }

    n = parse_find_node("keybind", node);
    while (n) {
        if (parse_attr_string("key", n, &key)) {
            keylist = g_list_append(keylist, key);

            parse_key(i, doc, n->children, keylist);

            it = g_list_last(keylist);
            g_free(it->data);
            keylist = g_list_delete_link(keylist, it);
        }
        n = parse_find_node("keybind", n->next);
    }
    if (keylist) {
        nact = parse_find_node("action", node);
        while (nact) {
            if ((action = action_parse(i, doc, nact,
                                       OB_USER_ACTION_KEYBOARD_KEY)))
                keyboard_bind(keylist, action);
            nact = parse_find_node("action", nact->next);
        }
    }
}

static void parse_keyboard(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                           void *d)
{
    parse_key(i, doc, node->children, NULL);
}

/*

<context name="Titlebar"> 
  <mousebind button="Left" action="Press">
    <action name="Raise"></action>
  </mousebind>
</context>

*/

static void parse_mouse(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                        void *d)
{
    xmlNodePtr n, nbut, nact;
    char *buttonstr;
    char *contextstr;
    ObUserAction uact;
    ObMouseAction mact;
    ObAction *action;

    node = node->children;
    
    if ((n = parse_find_node("dragThreshold", node)))
        config_mouse_threshold = parse_int(doc, n);
    if ((n = parse_find_node("doubleClickTime", node)))
        config_mouse_dclicktime = parse_int(doc, n);

    n = parse_find_node("context", node);
    while (n) {
        if (!parse_attr_string("name", n, &contextstr))
            goto next_n;
        nbut = parse_find_node("mousebind", n->children);
        while (nbut) {
            if (!parse_attr_string("button", nbut, &buttonstr))
                goto next_nbut;
            if (parse_attr_contains("press", nbut, "action")) {
                uact = OB_USER_ACTION_MOUSE_PRESS;
                mact = OB_MOUSE_ACTION_PRESS;
            } else if (parse_attr_contains("release", nbut, "action")) {
                uact = OB_USER_ACTION_MOUSE_RELEASE;
                mact = OB_MOUSE_ACTION_RELEASE;
            } else if (parse_attr_contains("click", nbut, "action")) {
                uact = OB_USER_ACTION_MOUSE_CLICK;
                mact = OB_MOUSE_ACTION_CLICK;
            } else if (parse_attr_contains("doubleclick", nbut,"action")) {
                uact = OB_USER_ACTION_MOUSE_DOUBLE_CLICK;
                mact = OB_MOUSE_ACTION_DOUBLE_CLICK;
            } else if (parse_attr_contains("drag", nbut, "action")) {
                uact = OB_USER_ACTION_MOUSE_MOTION;
                mact = OB_MOUSE_ACTION_MOTION;
            } else
                goto next_nbut;
            nact = parse_find_node("action", nbut->children);
            while (nact) {
                if ((action = action_parse(i, doc, nact, uact)))
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
                        void *d)
{
    xmlNodePtr n;

    node = node->children;
    
    if ((n = parse_find_node("focusNew", node)))
        config_focus_new = parse_bool(doc, n);
    if ((n = parse_find_node("followMouse", node)))
        config_focus_follow = parse_bool(doc, n);
    if ((n = parse_find_node("focusLast", node)))
        config_focus_last = parse_bool(doc, n);
    if ((n = parse_find_node("focusLastOnDesktop", node)))
        config_focus_last_on_desktop = parse_bool(doc, n);
    if ((n = parse_find_node("focusDelay", node)))
        config_focus_delay = parse_int(doc, n) * 1000;
}

static void parse_theme(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                        void *d)
{
    xmlNodePtr n;

    node = node->children;

    if ((n = parse_find_node("name", node))) {
        gchar *c;

        g_free(config_theme);
        c = parse_string(doc, n);
        config_theme = ob_expand_tilde(c);
        g_free(c);
    }
    if ((n = parse_find_node("titleLayout", node))) {
        g_free(config_title_layout);
        config_title_layout = parse_string(doc, n);
    }
}

static void parse_desktops(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                           void *d)
{
    xmlNodePtr n;

    node = node->children;
    
    if ((n = parse_find_node("number", node)))
        config_desktops_num = parse_int(doc, n);
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
}

static void parse_resize(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node,
                         void *d)
{
    xmlNodePtr n;

    node = node->children;
    
    if ((n = parse_find_node("drawContents", node)))
        config_redraw_resize = parse_bool(doc, n);
}

static void parse_dock(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node, void *d)
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
    }
    if ((n = parse_find_node("stacking", node))) {
        if (parse_contains("top", doc, n))
            config_dock_layer = OB_STACKING_LAYER_TOP;
        else if (parse_contains("normal", doc, n))
            config_dock_layer = OB_STACKING_LAYER_NORMAL;
        else if (parse_contains("bottom", doc, n))
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
    if ((n = parse_find_node("hideTimeout", node)))
        config_dock_hide_timeout = parse_int(doc, n) * 1000;
}

static void parse_menu(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node, void *d)
{
    for (node = node->children; node; node = node->next) {
        if (!xmlStrcasecmp(node->name, (const xmlChar*) "file")) {
            gchar *c;

            c = parse_string(doc, node);
            config_menu_files = g_slist_append(config_menu_files,
                                               ob_expand_tilde(c));
            g_free(c);
        }
    }
}
   
static void parse_resistance(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node, 
                             void *d)
{
    xmlNodePtr n;

    node = node->children;
    if ((n = parse_find_node("strength", node)))
        config_resist_win = parse_int(doc, n);
    if ((n = parse_find_node("screen_edge_strength", node)))
        config_resist_edge = parse_int(doc, n);
}

void config_startup(ObParseInst *i)
{
    config_focus_new = TRUE;
    config_focus_follow = FALSE;
    config_focus_last = TRUE;
    config_focus_last_on_desktop = TRUE;
    config_focus_delay = 0;

    parse_register(i, "focus", parse_focus, NULL);

    config_theme = NULL;

    config_title_layout = g_strdup("NLIMC");

    parse_register(i, "theme", parse_theme, NULL);

    config_desktops_num = 4;
    config_desktops_names = NULL;

    parse_register(i, "desktops", parse_desktops, NULL);

    config_redraw_resize = TRUE;

    parse_register(i, "resize", parse_resize, NULL);

    config_dock_layer = OB_STACKING_LAYER_TOP;
    config_dock_pos = OB_DIRECTION_NORTHEAST;
    config_dock_floating = FALSE;
    config_dock_x = 0;
    config_dock_y = 0;
    config_dock_orient = OB_ORIENTATION_VERT;
    config_dock_hide = FALSE;
    config_dock_hide_timeout = 300;

    parse_register(i, "dock", parse_dock, NULL);

    translate_key("C-g", &config_keyboard_reset_state,
                  &config_keyboard_reset_keycode);

    parse_register(i, "keyboard", parse_keyboard, NULL);

    config_mouse_threshold = 3;
    config_mouse_dclicktime = 200;

    parse_register(i, "mouse", parse_mouse, NULL);

    config_resist_win = 10;
    config_resist_edge = 20;

    parse_register(i, "resistance", parse_resistance, NULL);

    config_menu_files = NULL;

    parse_register(i, "menu", parse_menu, NULL);
}

void config_shutdown()
{
    GSList *it;

    g_free(config_theme);

    g_free(config_title_layout);

    for (it = config_desktops_names; it; it = g_slist_next(it))
        g_free(it->data);
    g_slist_free(config_desktops_names);

    for (it = config_menu_files; it; it = g_slist_next(it))
        g_free(it->data);
    g_slist_free(config_menu_files);
}
