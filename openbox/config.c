#include "config.h"
#include "parser/parse.h"

gboolean config_focus_new;
gboolean config_focus_follow;
gboolean config_focus_last;
gboolean config_focus_last_on_desktop;
gboolean config_focus_popup;

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

static void parse_focus(xmlDocPtr doc, xmlNodePtr node, void *d)
{
    xmlNodePtr n;

    node = node->xmlChildrenNode;
    
    if ((n = parse_find_node("focusNew", node)))
        config_focus_new = parse_bool(doc, n);
    if ((n = parse_find_node("followMouse", node)))
        config_focus_follow = parse_bool(doc, n);
    if ((n = parse_find_node("focusLast", node)))
        config_focus_last = parse_bool(doc, n);
    if ((n = parse_find_node("focusLastOnDesktop", node)))
        config_focus_last_on_desktop = parse_bool(doc, n);
    if ((n = parse_find_node("cyclingDialog", node)))
        config_focus_popup = parse_bool(doc, n);
}

static void parse_theme(xmlDocPtr doc, xmlNodePtr node, void *d)
{
    xmlNodePtr n;

    node = node->xmlChildrenNode;

    if ((n = parse_find_node("theme", node))) {
        g_free(config_theme);
        config_theme = parse_string(doc, n);
    }
    if ((n = parse_find_node("titlelayout", node))) {
        g_free(config_title_layout);
        config_title_layout = parse_string(doc, n);
    }
}

static void parse_desktops(xmlDocPtr doc, xmlNodePtr node, void *d)
{
    xmlNodePtr n;

    node = node->xmlChildrenNode;
    
    if ((n = parse_find_node("number", node)))
        config_desktops_num = parse_int(doc, n);
    if ((n = parse_find_node("names", node))) {
        GSList *it;
        xmlNodePtr nname;

        for (it = config_desktops_names; it; it = it->next)
            g_free(it->data);
        g_slist_free(config_desktops_names);
        config_desktops_names = NULL;

        nname = parse_find_node("name", n->xmlChildrenNode);
        while (nname) {
            config_desktops_names = g_slist_append(config_desktops_names,
                                                   parse_string(doc, nname));
            nname = parse_find_node("name", nname->next);
        }
    }
}

static void parse_resize(xmlDocPtr doc, xmlNodePtr node, void *d)
{
    xmlNodePtr n;

    node = node->xmlChildrenNode;
    
    if ((n = parse_find_node("drawContents", node)))
        config_redraw_resize = parse_bool(doc, n);
}

static void parse_dock(xmlDocPtr doc, xmlNodePtr node, void *d)
{
    xmlNodePtr n;

    node = node->xmlChildrenNode;

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
        config_dock_hide_timeout = parse_int(doc, n);
}

void config_startup()
{
    config_focus_new = TRUE;
    config_focus_follow = FALSE;
    config_focus_last = TRUE;
    config_focus_last_on_desktop = TRUE;
    config_focus_popup = TRUE;

    parse_register("focus", parse_focus, NULL);

    config_theme = NULL;

    config_title_layout = g_strdup("NLIMC");

    parse_register("theme", parse_theme, NULL);

    config_desktops_num = 4;
    config_desktops_names = NULL;

    parse_register("desktops", parse_desktops, NULL);

    config_redraw_resize = TRUE;

    parse_register("resize", parse_resize, NULL);

    config_dock_layer = OB_STACKING_LAYER_TOP;
    config_dock_pos = OB_DIRECTION_NORTHEAST;
    config_dock_floating = FALSE;
    config_dock_x = 0;
    config_dock_y = 0;
    config_dock_orient = OB_ORIENTATION_VERT;
    config_dock_hide = FALSE;
    config_dock_hide_timeout = 3000;

    parse_register("dock", parse_dock, NULL);
}

void config_shutdown()
{
    GSList *it;

    g_free(config_theme);

    for (it = config_desktops_names; it; it = it->next)
        g_free(it->data);
    g_slist_free(config_desktops_names);
}
