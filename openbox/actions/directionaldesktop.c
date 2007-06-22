#include "openbox/actions.h"
#include "openbox/screen.h"
#include <glib.h>

typedef struct {
    gboolean linear;
    gboolean wrap;
    ObDirection dir;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_directionaldesktop_startup()
{
    actions_register("DirectionalDesktop",
                     setup_func,
                     free_func,
                     run_func,
                     NULL, NULL);
}

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_new0(Options, 1);
    o->wrap = TRUE;
    o->dir = OB_DIRECTION_EAST;

    if ((n = parse_find_node("wrap", node)))
        o->wrap = parse_bool(doc, n);
    if ((n = parse_find_node("direction", node))) {
        gchar *s = parse_string(doc, n);
        if (!g_ascii_strcasecmp(s, "next")) {
            o->linear = TRUE;
            o->dir = OB_DIRECTION_EAST;
        }
        else if (!g_ascii_strcasecmp(s, "previous")) {
            o->linear = TRUE;
            o->dir = OB_DIRECTION_WEST;
        }
        else if (!g_ascii_strcasecmp(s, "north") ||
                 !g_ascii_strcasecmp(s, "up"))
            o->dir = OB_DIRECTION_NORTH;
        else if (!g_ascii_strcasecmp(s, "south") ||
                 !g_ascii_strcasecmp(s, "down"))
            o->dir = OB_DIRECTION_SOUTH;
        else if (!g_ascii_strcasecmp(s, "west") ||
                 !g_ascii_strcasecmp(s, "left"))
            o->dir = OB_DIRECTION_WEST;
        else if (!g_ascii_strcasecmp(s, "east") ||
                 !g_ascii_strcasecmp(s, "right"))
            o->dir = OB_DIRECTION_EAST;
        g_free(s);
    }

    return o;
}

static void free_func(gpointer options)
{
    Options *o = options;

    g_free(o);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;
    guint d;

    d = screen_cycle_desktop(o->dir,
                             o->wrap,
                             o->linear,
                             FALSE, TRUE, FALSE);
    if (d != screen_desktop)
        screen_set_desktop(d, TRUE);

    return FALSE;
}
