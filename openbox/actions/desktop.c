#include "openbox/actions.h"
#include "openbox/screen.h"
#include "openbox/client.h"
#include <glib.h>

typedef enum {
    LAST,
    RELATIVE,
    ABSOLUTE
} SwitchType;

typedef struct {
    SwitchType type;
    union {
        struct {
            guint desktop;
        } abs;

        struct {
            gboolean linear;
            gboolean wrap;
            ObDirection dir;
        } rel;
    };
    gboolean send;
    gboolean follow;
} Options;

static gpointer setup_go_func(ObParseInst *i, xmlDocPtr doc,
                                  xmlNodePtr node);
static gpointer setup_send_func(ObParseInst *i, xmlDocPtr doc,
                                xmlNodePtr node);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_desktop_startup(void)
{
    actions_register("GoToDesktop", setup_go_func, g_free, run_func,
                     NULL, NULL);
    actions_register("SendToDesktop", setup_send_func, g_free, run_func,
                     NULL, NULL);
}

static gpointer setup_go_func(ObParseInst *i, xmlDocPtr doc,
                                  xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_new0(Options, 1);
    /* don't go anywhere if theres no options given */
    o->type = ABSOLUTE;
    o->abs.desktop = screen_desktop;
    /* wrap by default - it's handy! */
    o->rel.wrap = TRUE;

    if ((n = parse_find_node("to", node))) {
        gchar *s = parse_string(doc, n);
        if (!g_ascii_strcasecmp(s, "last"))
            o->type = LAST;
        else if (!g_ascii_strcasecmp(s, "next")) {
            o->type = RELATIVE;
            o->rel.linear = TRUE;
            o->rel.dir = OB_DIRECTION_EAST;
        }
        else if (!g_ascii_strcasecmp(s, "previous")) {
            o->type = RELATIVE;
            o->rel.linear = TRUE;
            o->rel.dir = OB_DIRECTION_WEST;
        }
        else if (!g_ascii_strcasecmp(s, "north") ||
                 !g_ascii_strcasecmp(s, "up")) {
            o->type = RELATIVE;
            o->rel.dir = OB_DIRECTION_NORTH;
        }
        else if (!g_ascii_strcasecmp(s, "south") ||
                 !g_ascii_strcasecmp(s, "down")) {
            o->type = RELATIVE;
            o->rel.dir = OB_DIRECTION_SOUTH;
        }
        else if (!g_ascii_strcasecmp(s, "west") ||
                 !g_ascii_strcasecmp(s, "left")) {
            o->type = RELATIVE;
            o->rel.dir = OB_DIRECTION_WEST;
        }
        else if (!g_ascii_strcasecmp(s, "east") ||
                 !g_ascii_strcasecmp(s, "right")) {
            o->type = RELATIVE;
            o->rel.dir = OB_DIRECTION_EAST;
        }
        else {
            o->type = ABSOLUTE;
            o->abs.desktop = parse_int(doc, n) - 1;
        }
        g_free(s);
    }

    if ((n = parse_find_node("wrap", node)))
        o->rel.wrap = parse_bool(doc, n);

    return o;
}

static gpointer setup_send_func(ObParseInst *i, xmlDocPtr doc,
                                xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = setup_go_func(i, doc, node);
    o->send = TRUE;
    o->follow = TRUE;

    if ((n = parse_find_node("follow", node)))
        o->follow = parse_bool(doc, n);

    return o;
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;
    guint d;



    switch (o->type) {
    case LAST:
        d = screen_last_desktop;
        break;
    case ABSOLUTE:
        d = o->abs.desktop;
        break;
    case RELATIVE:
        d = screen_find_desktop(screen_desktop,
                                o->rel.dir, o->rel.wrap, o->rel.linear);
        break;
    }

    if (d < screen_num_desktops && d != screen_desktop) {
        gboolean go = TRUE;

        if (o->send && data->client && client_normal(data->client)) {
            client_set_desktop(data->client, d, o->follow, FALSE);
            go = o->follow;
        }

        if (go) screen_set_desktop(d, TRUE);
    }
    return FALSE;
}
