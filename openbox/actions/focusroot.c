#include "openbox/actions.h"
#include "openbox/client.h"
#include "openbox/screen.h"
#include "openbox/focus.h"
#include "openbox/openbox.h"

enum {
    CURRENT_MONITOR = -1,
    ALL_MONITORS = -2,
    NEXT_MONITOR = -3,
    PREV_MONITOR = -4
};

typedef struct {
    gint monitor;
    gboolean pointer;
    gint x;
    gint y;
} Options;

static gpointer setup_func(xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_focusroot_startup(void)
{
    actions_register("FocusRoot", setup_func, free_func, run_func);
}

static gpointer setup_func(xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_slice_new0(Options);
    o->monitor = CURRENT_MONITOR;
    o->pointer = FALSE;
    o->x = 0;
    o->y = 0;

    if ((n = obt_xml_find_node(node, "monitor"))) {
        gchar *s = obt_xml_node_string(n);
        if (g_ascii_strcasecmp(s, "current") != 0) {
            if (!g_ascii_strcasecmp(s, "all"))
                o->monitor = ALL_MONITORS;
            else if(!g_ascii_strcasecmp(s, "next"))
                o->monitor = NEXT_MONITOR;
            else if(!g_ascii_strcasecmp(s, "prev"))
                o->monitor = PREV_MONITOR;
            else
                o->monitor = obt_xml_node_int(n) - 1;
        }
        g_free(s);
    }
    if ((n = obt_xml_find_node(node, "pointer"))) {
        xmlNodePtr m;

        o->pointer = TRUE;
        if ((m = obt_xml_find_node(n->children, "x")))
            o->x = obt_xml_node_int(m);
        if ((m = obt_xml_find_node(n->children, "y")))
            o->y = obt_xml_node_int(m);
    }

    return o;
}

static void free_func(gpointer options)
{
    Options *o = options;

    g_slice_free(Options, o);
}

static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;
    GSList *desk;
    guint mon, cmon;

    mon = o->monitor;
    cmon = g_slist_index(screen_visible_desktops, screen_desktop);

    /* CURRENT_MONITOR and ALL_MONITORS are meaningless in this context */
    switch (mon) {
        case CURRENT_MONITOR:
            return FALSE; break;
        case ALL_MONITORS:
            return FALSE; break;
        case NEXT_MONITOR:
            mon = (cmon + 1 > screen_num_monitors - 1) ? 0 : (cmon + 1); break;
        case PREV_MONITOR:
            mon = (cmon == 0) ? (screen_num_monitors - 1) : (cmon - 1); break;
        default:
            ;
    }

    if((desk = g_slist_nth(screen_visible_desktops, mon)) != NULL) {
        /* If the client focuses, it will switch the desktop for us */
        if(!client_focus(focus_order_find_first(desk->data))) {
            focus_nothing();
            screen_store_desktop(desk->data);
        }
    }
    else
        return FALSE;

    /* Move the pointer too? */
    if (o->pointer) {
        Rect *ma;
        gint x, y;

        ma = screen_monitor_area(mon);
        x = o->x >= 0 ? ma->x + o->x : ma->width + ma->x + o->x;
        y = o->y >= 0 ? ma->y + o->y : ma->height + ma->y + o->y;

        XWarpPointer(obt_display, 0, obt_root(ob_screen), 0, 0, 0, 0, x, y);

        g_slice_free(Rect, ma);
    }

    return FALSE;
}

