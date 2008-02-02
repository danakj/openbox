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

static gpointer setup_go_last_func(ObParseInst *i, xmlDocPtr doc,
                                   xmlNodePtr node);
static gpointer setup_send_last_func(ObParseInst *i, xmlDocPtr doc,
                                     xmlNodePtr node);
static gpointer setup_go_abs_func(ObParseInst *i, xmlDocPtr doc,
                                   xmlNodePtr node);
static gpointer setup_send_abs_func(ObParseInst *i, xmlDocPtr doc,
                                     xmlNodePtr node);
static gpointer setup_go_next_func(ObParseInst *i, xmlDocPtr doc,
                                   xmlNodePtr node);
static gpointer setup_send_next_func(ObParseInst *i, xmlDocPtr doc,
                                     xmlNodePtr node);
static gpointer setup_go_prev_func(ObParseInst *i, xmlDocPtr doc,
                                   xmlNodePtr node);
static gpointer setup_send_prev_func(ObParseInst *i, xmlDocPtr doc,
                                     xmlNodePtr node);
static gpointer setup_go_left_func(ObParseInst *i, xmlDocPtr doc,
                                   xmlNodePtr node);
static gpointer setup_send_left_func(ObParseInst *i, xmlDocPtr doc,
                                     xmlNodePtr node);
static gpointer setup_go_right_func(ObParseInst *i, xmlDocPtr doc,
                                   xmlNodePtr node);
static gpointer setup_send_right_func(ObParseInst *i, xmlDocPtr doc,
                                     xmlNodePtr node);
static gpointer setup_go_up_func(ObParseInst *i, xmlDocPtr doc,
                                   xmlNodePtr node);
static gpointer setup_send_up_func(ObParseInst *i, xmlDocPtr doc,
                                     xmlNodePtr node);
static gpointer setup_go_down_func(ObParseInst *i, xmlDocPtr doc,
                                   xmlNodePtr node);
static gpointer setup_send_down_func(ObParseInst *i, xmlDocPtr doc,
                                     xmlNodePtr node);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_desktop_startup(void)
{
    actions_register("DesktopLast", setup_go_last_func, g_free,
                     run_func, NULL, NULL);
    actions_register("SendToDesktopLast", setup_send_last_func, g_free,
                     run_func, NULL, NULL);
    actions_register("Desktop", setup_go_abs_func, g_free,
                     run_func, NULL, NULL);
    actions_register("SendToDesktop", setup_send_abs_func, g_free,
                     run_func, NULL, NULL);
    actions_register("DesktopNext", setup_go_next_func, g_free,
                     run_func, NULL, NULL);
    actions_register("SendToDesktopNext", setup_send_next_func, g_free,
                     run_func, NULL, NULL);
    actions_register("DesktopPrevious", setup_go_prev_func, g_free,
                     run_func, NULL, NULL);
    actions_register("SendToDesktopPrevious", setup_send_prev_func, g_free,
                     run_func, NULL, NULL);
    actions_register("DesktopLeft", setup_go_left_func, g_free,
                     run_func, NULL, NULL);
    actions_register("SendToDesktopLeft", setup_send_left_func, g_free,
                     run_func, NULL, NULL);
    actions_register("DesktopRight", setup_go_right_func, g_free,
                     run_func, NULL, NULL);
    actions_register("SendToDesktopRight", setup_send_right_func, g_free,
                     run_func, NULL, NULL);
    actions_register("DesktopUp", setup_go_up_func, g_free,
                     run_func, NULL, NULL);
    actions_register("SendToDesktopUp", setup_send_up_func, g_free,
                     run_func, NULL, NULL);
    actions_register("DesktopDown", setup_go_down_func, g_free,
                     run_func, NULL, NULL);
    actions_register("SendToDesktopDown", setup_send_down_func, g_free,
                     run_func, NULL, NULL);
}

static gpointer setup_follow(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o = g_new0(Options, 1);
    o->send = TRUE;
    o->follow = TRUE;
    if ((n = parse_find_node("follow", node)))
        o->follow = parse_bool(doc, n);
    return o;
}

static gpointer setup_go_last_func(ObParseInst *i, xmlDocPtr doc,
                                   xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    o->type = LAST;
    return o;
}

static gpointer setup_send_last_func(ObParseInst *i, xmlDocPtr doc,
                                     xmlNodePtr node)
{
    Options *o = setup_follow(i, doc, node);
    o->type = LAST;
    return o;
}

static gpointer setup_go_abs_func(ObParseInst *i, xmlDocPtr doc,
                                   xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o = g_new0(Options, 1);
    o->type = ABSOLUTE;
    if ((n = parse_find_node("desktop", node)))
        o->abs.desktop = parse_int(doc, n) - 1;
    else
        o->abs.desktop = screen_desktop;
    return o;
}

static gpointer setup_send_abs_func(ObParseInst *i, xmlDocPtr doc,
                                     xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o = setup_follow(i, doc, node);
    o->type = ABSOLUTE;
    if ((n = parse_find_node("desktop", node)))
        o->abs.desktop = parse_int(doc, n) - 1;
    else
        o->abs.desktop = screen_desktop;
    return o;
}

static void setup_rel(Options *o, ObParseInst *i, xmlDocPtr doc,
                      xmlNodePtr node, gboolean lin, ObDirection dir)
{
    xmlNodePtr n;

    o->type = RELATIVE;
    o->rel.linear = lin;
    o->rel.dir = dir;
    o->rel.wrap = TRUE;

    if ((n = parse_find_node("wrap", node)))
        o->rel.wrap = parse_bool(doc, n);
}

static gpointer setup_go_next_func(ObParseInst *i, xmlDocPtr doc,
                                   xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    setup_rel(o, i, doc, node, TRUE, OB_DIRECTION_EAST);
    return o;
}

static gpointer setup_send_next_func(ObParseInst *i, xmlDocPtr doc,
                                     xmlNodePtr node)
{
    Options *o = setup_follow(i, doc, node);
    setup_rel(o, i, doc, node, TRUE, OB_DIRECTION_EAST);
    return o;
}

static gpointer setup_go_prev_func(ObParseInst *i, xmlDocPtr doc,
                                   xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    setup_rel(o, i, doc, node, TRUE, OB_DIRECTION_WEST);
    return o;
}

static gpointer setup_send_prev_func(ObParseInst *i, xmlDocPtr doc,
                                     xmlNodePtr node)
{
    Options *o = setup_follow(i, doc, node);
    setup_rel(o, i, doc, node, TRUE, OB_DIRECTION_WEST);
    return o;
}

static gpointer setup_go_left_func(ObParseInst *i, xmlDocPtr doc,
                                   xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    setup_rel(o, i, doc, node, FALSE, OB_DIRECTION_WEST);
    return o;
}

static gpointer setup_send_left_func(ObParseInst *i, xmlDocPtr doc,
                                     xmlNodePtr node)
{
    Options *o = setup_follow(i, doc, node);
    setup_rel(o, i, doc, node, FALSE, OB_DIRECTION_WEST);
    return o;
}

static gpointer setup_go_right_func(ObParseInst *i, xmlDocPtr doc,
                                    xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    setup_rel(o, i, doc, node, FALSE, OB_DIRECTION_EAST);
    return o;
}

static gpointer setup_send_right_func(ObParseInst *i, xmlDocPtr doc,
                                      xmlNodePtr node)
{
    Options *o = setup_follow(i, doc, node);
    setup_rel(o, i, doc, node, FALSE, OB_DIRECTION_EAST);
    return o;
}

static gpointer setup_go_up_func(ObParseInst *i, xmlDocPtr doc,
                                 xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    setup_rel(o, i, doc, node, FALSE, OB_DIRECTION_NORTH);
    return o;
}

static gpointer setup_send_up_func(ObParseInst *i, xmlDocPtr doc,
                                   xmlNodePtr node)
{
    Options *o = setup_follow(i, doc, node);
    setup_rel(o, i, doc, node, FALSE, OB_DIRECTION_NORTH);
    return o;
}

static gpointer setup_go_down_func(ObParseInst *i, xmlDocPtr doc,
                                   xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    setup_rel(o, i, doc, node, FALSE, OB_DIRECTION_SOUTH);
    return o;
}

static gpointer setup_send_down_func(ObParseInst *i, xmlDocPtr doc,
                                     xmlNodePtr node)
{
    Options *o = setup_follow(i, doc, node);
    setup_rel(o, i, doc, node, FALSE, OB_DIRECTION_SOUTH);
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

        actions_client_move(data, TRUE);
        if (o->send && data->client && client_normal(data->client)) {
            client_set_desktop(data->client, d, o->follow, FALSE);
            go = o->follow;
        }

        if (go) screen_set_desktop(d, TRUE);
        actions_client_move(data, FALSE);
    }
    return FALSE;
}
