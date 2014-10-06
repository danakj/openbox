#include "openbox/actions.h"
#include "openbox/menu.h"
#include "openbox/place.h"
#include "openbox/geom.h"
#include "openbox/screen.h"
#include <glib.h>

typedef struct {
    gchar         *name;
    GravityCoord   x, y;
    ObPlaceMonitor monitor_type;
    gint           monitor;
    gboolean       use_position;
} Options;

static gpointer setup_func(xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_showmenu_startup(void)
{
    actions_register("ShowMenu", setup_func, free_func, run_func);
}

static gpointer setup_func(xmlNodePtr node)
{
    xmlNodePtr n, c;
    Options *o;
    gboolean x_pos_given = FALSE;

    o = g_slice_new0(Options);
    o->monitor = -1;

    if ((n = obt_xml_find_node(node, "menu")))
        o->name = obt_xml_node_string(n);

    if ((n = obt_xml_find_node(node, "position"))) {
        if ((c = obt_xml_find_node(n->children, "x"))) {
            if (!obt_xml_node_contains(c, "default")) {
                config_parse_gravity_coord(c, &o->x);
                x_pos_given = TRUE;
            }
        }

        if (x_pos_given && (c = obt_xml_find_node(n->children, "y"))) {
            if (!obt_xml_node_contains(c, "default")) {
                config_parse_gravity_coord(c, &o->y);
                o->use_position = TRUE;
            }
        }

        /* unlike client placement, x/y is needed to specify a monitor,
         * either it's under the mouse or it's in an exact actual position */
        if (o->use_position && (c = obt_xml_find_node(n->children, "monitor"))) {
            if (!obt_xml_node_contains(c, "default")) {
                gchar *s = obt_xml_node_string(c);
                if (!g_ascii_strcasecmp(s, "mouse"))
                    o->monitor_type = OB_PLACE_MONITOR_MOUSE;
                else if (!g_ascii_strcasecmp(s, "active"))
                    o->monitor_type = OB_PLACE_MONITOR_ACTIVE;
                else if (!g_ascii_strcasecmp(s, "primary"))
                    o->monitor_type = OB_PLACE_MONITOR_PRIMARY;
                else if (!g_ascii_strcasecmp(s, "all"))
                    o->monitor_type = OB_PLACE_MONITOR_ALL;
                else
                    o->monitor = obt_xml_node_int(c) - 1;
                g_free(s);
            }
        }
    }
    return o;
}

static void free_func(gpointer options)
{
    Options *o = options;
    g_free(o->name);
    g_slice_free(Options, o);
}

static void calc_position(Options *o, gint *x, gint *y)
{
    gint monitor = -1;
    const Rect *area;
    if (o->monitor >= 0)
        monitor = o->monitor;
    else switch (o->monitor_type) {
        case OB_PLACE_MONITOR_ANY:
        case OB_PLACE_MONITOR_PRIMARY:
            monitor = screen_monitor_primary(FALSE);
            break;
        case OB_PLACE_MONITOR_MOUSE:
            monitor = screen_monitor_pointer();
            break;
        case OB_PLACE_MONITOR_ACTIVE:
            monitor = screen_monitor_active();
            break;
        case OB_PLACE_MONITOR_ALL:
            monitor = screen_num_monitors;
            break;
        default:
            g_assert_not_reached();
    }
    area = screen_physical_area_monitor(monitor);

    if (o->x.center)
        *x = area->width / 2; /* - client->area.width / 2; */
    else {
        *x = o->x.pos;
        if (o->x.denom)
            *x = (*x * area->width) / o->x.denom;
        if (o->x.opposite)
            *x = area->width /* - frame_size.width */ - *x;
    }

    if (o->y.center)
        *y = area->height / 2; /* - client->area.height / 2; */
    else {
        *y = o->y.pos;
        if (o->y.denom)
            *y = (*y * area->height) / o->y.denom;
        if (o->y.opposite)
            *y = area->height /* - frame_size.height */ - *y;
    }

    *x += area->x;
    *y += area->y;
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;
    gint x, y;

    if (o->use_position) {
        calc_position(o, &x, &y);
    } else {
        x = data->x;
        y = data->y;
    }

    /* you cannot call ShowMenu from inside a menu */
    if (data->uact != OB_USER_ACTION_MENU_SELECTION && o->name)
        menu_show(o->name, x, y, data->button != 0, data->client);

    return FALSE;
}
