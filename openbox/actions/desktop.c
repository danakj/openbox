#include "openbox/actions.h"
#include "openbox/screen.h"
#include "openbox/client.h"
#include "openbox/openbox.h"
#include "obt/keyboard.h"

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
    } u;
    gboolean send;
    gboolean follow;
    gboolean interactive;
} Options;

static gpointer setup_go_func(xmlNodePtr node,
                              ObActionsIPreFunc *pre,
                              ObActionsIInputFunc *input,
                              ObActionsICancelFunc *cancel,
                              ObActionsIPostFunc *post);
static gpointer setup_send_func(xmlNodePtr node,
                                ObActionsIPreFunc *pre,
                                ObActionsIInputFunc *input,
                                ObActionsICancelFunc *cancel,
                                ObActionsIPostFunc *post);
static gboolean run_func(ObActionsData *data, gpointer options);

static gboolean i_pre_func(guint state, gpointer options);
static gboolean i_input_func(guint initial_state,
                             XEvent *e,
                             ObtIC *ic,
                             gpointer options,
                             gboolean *used);
static void i_post_func(gpointer options);

/* 3.4-compatibility */
static gpointer setup_go_last_func(xmlNodePtr node);
static gpointer setup_send_last_func(xmlNodePtr node);
static gpointer setup_go_abs_func(xmlNodePtr node);
static gpointer setup_send_abs_func(xmlNodePtr node);
static gpointer setup_go_next_func(xmlNodePtr node,
                                   ObActionsIPreFunc *pre,
                                   ObActionsIInputFunc *input,
                                   ObActionsICancelFunc *cancel,
                                   ObActionsIPostFunc *post);
static gpointer setup_send_next_func(xmlNodePtr node,
                                     ObActionsIPreFunc *pre,
                                     ObActionsIInputFunc *input,
                                     ObActionsICancelFunc *cancel,
                                     ObActionsIPostFunc *post);
static gpointer setup_go_prev_func(xmlNodePtr node,
                                   ObActionsIPreFunc *pre,
                                   ObActionsIInputFunc *input,
                                   ObActionsICancelFunc *cancel,
                                   ObActionsIPostFunc *post);
static gpointer setup_send_prev_func(xmlNodePtr node,
                                     ObActionsIPreFunc *pre,
                                     ObActionsIInputFunc *input,
                                     ObActionsICancelFunc *cancel,
                                     ObActionsIPostFunc *post);
static gpointer setup_go_left_func(xmlNodePtr node,
                                   ObActionsIPreFunc *pre,
                                   ObActionsIInputFunc *input,
                                   ObActionsICancelFunc *cancel,
                                   ObActionsIPostFunc *post);
static gpointer setup_send_left_func(xmlNodePtr node,
                                     ObActionsIPreFunc *pre,
                                     ObActionsIInputFunc *input,
                                     ObActionsICancelFunc *cancel,
                                     ObActionsIPostFunc *post);
static gpointer setup_go_right_func(xmlNodePtr node,
                                    ObActionsIPreFunc *pre,
                                    ObActionsIInputFunc *input,
                                    ObActionsICancelFunc *cancel,
                                    ObActionsIPostFunc *post);
static gpointer setup_send_right_func(xmlNodePtr node,
                                      ObActionsIPreFunc *pre,
                                      ObActionsIInputFunc *input,
                                      ObActionsICancelFunc *cancel,
                                      ObActionsIPostFunc *post);
static gpointer setup_go_up_func(xmlNodePtr node,
                                 ObActionsIPreFunc *pre,
                                 ObActionsIInputFunc *input,
                                 ObActionsICancelFunc *cancel,
                                 ObActionsIPostFunc *post);
static gpointer setup_send_up_func(xmlNodePtr node,
                                   ObActionsIPreFunc *pre,
                                   ObActionsIInputFunc *input,
                                   ObActionsICancelFunc *cancel,
                                   ObActionsIPostFunc *post);
static gpointer setup_go_down_func(xmlNodePtr node,
                                   ObActionsIPreFunc *pre,
                                   ObActionsIInputFunc *input,
                                   ObActionsICancelFunc *cancel,
                                   ObActionsIPostFunc *post);
static gpointer setup_send_down_func(xmlNodePtr node,
                                     ObActionsIPreFunc *pre,
                                     ObActionsIInputFunc *input,
                                     ObActionsICancelFunc *cancel,
                                     ObActionsIPostFunc *post);
 
void action_desktop_startup(void)
{
    actions_register_i("GoToDesktop", setup_go_func, g_free, run_func);
    actions_register_i("SendToDesktop", setup_send_func, g_free, run_func);
    /* 3.4-compatibility */
    actions_register("DesktopLast", setup_go_last_func, g_free, run_func);
    actions_register("SendToDesktopLast", setup_send_last_func,
                     g_free, run_func);
    actions_register("Desktop", setup_go_abs_func, g_free, run_func);
    actions_register("SendToDesktop", setup_send_abs_func, g_free, run_func);
    actions_register_i("DesktopNext", setup_go_next_func, g_free, run_func);
    actions_register_i("SendToDesktopNext", setup_send_next_func,
                       g_free, run_func);
    actions_register_i("DesktopPrevious", setup_go_prev_func,
                       g_free, run_func);
    actions_register_i("SendToDesktopPrevious", setup_send_prev_func,
                       g_free, run_func);
    actions_register_i("DesktopLeft", setup_go_left_func, g_free, run_func);
    actions_register_i("SendToDesktopLeft", setup_send_left_func,
                       g_free, run_func);
    actions_register_i("DesktopRight", setup_go_right_func, g_free, run_func);
    actions_register_i("SendToDesktopRight", setup_send_right_func,
                       g_free, run_func);
    actions_register_i("DesktopUp", setup_go_up_func, g_free, run_func);
    actions_register_i("SendToDesktopUp", setup_send_up_func,
                       g_free, run_func);
    actions_register_i("DesktopDown", setup_go_down_func, g_free, run_func);
    actions_register_i("SendToDesktopDown", setup_send_down_func,
                       g_free, run_func);
}

static gpointer setup_func(xmlNodePtr node,
                           ObActionsIPreFunc *pre,
                           ObActionsIInputFunc *input,
                           ObActionsICancelFunc *cancel,
                           ObActionsIPostFunc *post)
{
    xmlNodePtr n;
    Options *o;

    o = g_new0(Options, 1);
    /* don't go anywhere if there are no options given */
    o->type = ABSOLUTE;
    o->u.abs.desktop = screen_desktop;
    /* wrap by default - it's handy! */
    o->u.rel.wrap = TRUE;

    if ((n = obt_xml_find_node(node, "to"))) {
        gchar *s = obt_xml_node_string(n);
        if (!g_ascii_strcasecmp(s, "last"))
            o->type = LAST;
        else if (!g_ascii_strcasecmp(s, "next")) {
            o->type = RELATIVE;
            o->u.rel.linear = TRUE;
            o->u.rel.dir = OB_DIRECTION_EAST;
        }
        else if (!g_ascii_strcasecmp(s, "previous")) {
            o->type = RELATIVE;
            o->u.rel.linear = TRUE;
            o->u.rel.dir = OB_DIRECTION_WEST;
        }
        else if (!g_ascii_strcasecmp(s, "north") ||
                 !g_ascii_strcasecmp(s, "up")) {
            o->type = RELATIVE;
            o->u.rel.dir = OB_DIRECTION_NORTH;
        }
        else if (!g_ascii_strcasecmp(s, "south") ||
                 !g_ascii_strcasecmp(s, "down")) {
            o->type = RELATIVE;
            o->u.rel.dir = OB_DIRECTION_SOUTH;
        }
        else if (!g_ascii_strcasecmp(s, "west") ||
                 !g_ascii_strcasecmp(s, "left")) {
            o->type = RELATIVE;
            o->u.rel.dir = OB_DIRECTION_WEST;
        }
        else if (!g_ascii_strcasecmp(s, "east") ||
                 !g_ascii_strcasecmp(s, "right")) {
            o->type = RELATIVE;
            o->u.rel.dir = OB_DIRECTION_EAST;
        }
        else {
            o->type = ABSOLUTE;
            o->u.abs.desktop = atoi(s) - 1;
        }
        g_free(s);
    }

    if ((n = obt_xml_find_node(node, "wrap")))
        o->u.rel.wrap = obt_xml_node_bool(n);

    return o;
}


static gpointer setup_go_func(xmlNodePtr node,
                              ObActionsIPreFunc *pre,
                              ObActionsIInputFunc *input,
                              ObActionsICancelFunc *cancel,
                              ObActionsIPostFunc *post)
{
    Options *o;

    o = setup_func(node, pre, input, cancel, post);
    if (o->type == RELATIVE) {
        o->interactive = TRUE;
        *pre = i_pre_func;
        *input = i_input_func;
        *post = i_post_func;
    }

    return o;
}

static gpointer setup_send_func(xmlNodePtr node,
                                ObActionsIPreFunc *pre,
                                ObActionsIInputFunc *input,
                                ObActionsICancelFunc *cancel,
                                ObActionsIPostFunc *post)
{
    xmlNodePtr n;
    Options *o;

    o = setup_func(node, pre, input, cancel, post);
    o->send = TRUE;
    o->follow = TRUE;

    if ((n = obt_xml_find_node(node, "follow")))
        o->follow = obt_xml_node_bool(n);

    if (o->type == RELATIVE && o->follow) {
        o->interactive = TRUE;
        *pre = i_pre_func;
        *input = i_input_func;
        *post = i_post_func;
    }

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
        d = o->u.abs.desktop;
        break;
    case RELATIVE:
        d = screen_find_desktop(screen_desktop,
                                o->u.rel.dir, o->u.rel.wrap, o->u.rel.linear);
        break;
    default:
        g_assert_not_reached();
    }

    if (d < screen_num_desktops && d != screen_desktop) {
        gboolean go = TRUE;

        actions_client_move(data, TRUE);
        if (o->send && data->client && client_normal(data->client)) {
            client_set_desktop(data->client, d, o->follow, FALSE);
            go = o->follow;
        }

        if (go) {
            screen_set_desktop(d, TRUE);
            if (data->client)
                client_bring_helper_windows(data->client);
        }

        actions_client_move(data, FALSE);
    }

    return o->interactive;
}

static gboolean i_input_func(guint initial_state,
                             XEvent *e,
                             ObtIC *ic,
                             gpointer options,
                             gboolean *used)
{
    guint mods;

    mods = obt_keyboard_only_modmasks(e->xkey.state);
    if (e->type == KeyRelease) {
        /* remove from the state the mask of the modifier key being
           released, if it is a modifier key being released that is */
        mods &= ~obt_keyboard_keycode_to_modmask(e->xkey.keycode);
    }

    if (e->type == KeyPress) {
        /* Escape cancels no matter what */
        if (ob_keycode_match(e->xkey.keycode, OB_KEY_ESCAPE)) {
            return FALSE;
        }

        /* There were no modifiers and they pressed enter */
        else if (ob_keycode_match(e->xkey.keycode, OB_KEY_RETURN) &&
                 !initial_state)
        {
            return FALSE;
        }
    }
    /* They released the modifiers */
    else if (e->type == KeyRelease && initial_state && !(mods & initial_state))
    {
        return FALSE;
    }

    return TRUE;
}

static gboolean i_pre_func(guint initial_state, gpointer options)
{
    if (!initial_state) {
        Options *o = options;
        o->interactive = FALSE;
        return FALSE;
    }
    else {
        screen_show_desktop_popup(screen_desktop, TRUE);
        return TRUE;
    }
}

static void i_post_func(gpointer options)
{
    screen_hide_desktop_popup();
}

/* 3.4-compatilibity */
static gpointer setup_follow(xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o = g_new0(Options, 1);
    o->send = TRUE;
    o->follow = TRUE;
    if ((n = obt_xml_find_node(node, "follow")))
        o->follow = obt_xml_node_bool(n);
    return o;
}

static gpointer setup_go_last_func(xmlNodePtr node)
{
    Options *o = g_new0(Options, 1);
    o->type = LAST;
    return o;
}

static gpointer setup_send_last_func(xmlNodePtr node)
{
    Options *o = setup_follow(node);
    o->type = LAST;
    return o;
}

static gpointer setup_go_abs_func(xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o = g_new0(Options, 1);
    o->type = ABSOLUTE;
    if ((n = obt_xml_find_node(node, "desktop")))
        o->u.abs.desktop = obt_xml_node_int(n) - 1;
    else
        o->u.abs.desktop = screen_desktop;
    return o;
}

static gpointer setup_send_abs_func(xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o = setup_follow(node);
    o->type = ABSOLUTE;
    if ((n = obt_xml_find_node(node, "desktop")))
        o->u.abs.desktop = obt_xml_node_int(n) - 1;
    else
        o->u.abs.desktop = screen_desktop;
    return o;
}

static void setup_rel(Options *o, xmlNodePtr node, gboolean lin,
                      ObDirection dir,
                      ObActionsIPreFunc *pre,
                      ObActionsIInputFunc *input,
                      ObActionsIPostFunc *post)
{
    xmlNodePtr n;

    o->type = RELATIVE;
    o->u.rel.linear = lin;
    o->u.rel.dir = dir;
    o->u.rel.wrap = TRUE;

    if ((n = obt_xml_find_node(node, "wrap")))
        o->u.rel.wrap = obt_xml_node_bool(n);

    if (input) {
        o->interactive = TRUE;
        *pre = i_pre_func;
        *input = i_input_func;
        *post = i_post_func;
    }
}

static gpointer setup_go_next_func(xmlNodePtr node,
                                   ObActionsIPreFunc *pre,
                                   ObActionsIInputFunc *input,
                                   ObActionsICancelFunc *cancel,
                                   ObActionsIPostFunc *post)
{
    Options *o = g_new0(Options, 1);
    setup_rel(o, node, TRUE, OB_DIRECTION_EAST, pre, input, post);
    return o;
}

static gpointer setup_send_next_func(xmlNodePtr node,
                                     ObActionsIPreFunc *pre,
                                     ObActionsIInputFunc *input,
                                     ObActionsICancelFunc *cancel,
                                     ObActionsIPostFunc *post)
{
    Options *o = setup_follow(node);
    setup_rel(o, node, TRUE, OB_DIRECTION_EAST,
              pre, (o->follow ? input : NULL), post);
    return o;
}

static gpointer setup_go_prev_func(xmlNodePtr node,
                                   ObActionsIPreFunc *pre,
                                   ObActionsIInputFunc *input,
                                   ObActionsICancelFunc *cancel,
                                   ObActionsIPostFunc *post)
{
    Options *o = g_new0(Options, 1);
    setup_rel(o, node, TRUE, OB_DIRECTION_WEST, pre, input, post);
    return o;
}

static gpointer setup_send_prev_func(xmlNodePtr node,
                                     ObActionsIPreFunc *pre,
                                     ObActionsIInputFunc *input,
                                     ObActionsICancelFunc *cancel,
                                     ObActionsIPostFunc *post)
{
    Options *o = setup_follow(node);
    setup_rel(o, node, TRUE, OB_DIRECTION_WEST,
              pre, (o->follow ? input : NULL), post);
    return o;
}

static gpointer setup_go_left_func(xmlNodePtr node,
                                   ObActionsIPreFunc *pre,
                                   ObActionsIInputFunc *input,
                                   ObActionsICancelFunc *cancel,
                                   ObActionsIPostFunc *post)
{
    Options *o = g_new0(Options, 1);
    setup_rel(o, node, FALSE, OB_DIRECTION_WEST, pre, input, post);
    return o;
}

static gpointer setup_send_left_func(xmlNodePtr node,
                                     ObActionsIPreFunc *pre,
                                     ObActionsIInputFunc *input,
                                     ObActionsICancelFunc *cancel,
                                     ObActionsIPostFunc *post)
{
    Options *o = setup_follow(node);
    setup_rel(o, node, FALSE, OB_DIRECTION_WEST,
              pre, (o->follow ? input : NULL), post);
    return o;
}

static gpointer setup_go_right_func(xmlNodePtr node,
                                    ObActionsIPreFunc *pre,
                                    ObActionsIInputFunc *input,
                                    ObActionsICancelFunc *cancel,
                                    ObActionsIPostFunc *post)
{
    Options *o = g_new0(Options, 1);
    setup_rel(o, node, FALSE, OB_DIRECTION_EAST, pre, input, post);
    return o;
}

static gpointer setup_send_right_func(xmlNodePtr node,
                                      ObActionsIPreFunc *pre,
                                      ObActionsIInputFunc *input,
                                      ObActionsICancelFunc *cancel,
                                      ObActionsIPostFunc *post)
{
    Options *o = setup_follow(node);
    setup_rel(o, node, FALSE, OB_DIRECTION_EAST,
              pre, (o->follow ? input : NULL), post);
    return o;
}

static gpointer setup_go_up_func(xmlNodePtr node,
                                 ObActionsIPreFunc *pre,
                                 ObActionsIInputFunc *input,
                                 ObActionsICancelFunc *cancel,
                                 ObActionsIPostFunc *post)
{
    Options *o = g_new0(Options, 1);
    setup_rel(o, node, FALSE, OB_DIRECTION_NORTH, pre, input, post);
    return o;
}

static gpointer setup_send_up_func(xmlNodePtr node,
                                   ObActionsIPreFunc *pre,
                                   ObActionsIInputFunc *input,
                                   ObActionsICancelFunc *cancel,
                                   ObActionsIPostFunc *post)
{
    Options *o = setup_follow(node);
    setup_rel(o, node, FALSE, OB_DIRECTION_NORTH,
              pre, (o->follow ? input : NULL), post);
    return o;
}

static gpointer setup_go_down_func(xmlNodePtr node,
                                   ObActionsIPreFunc *pre,
                                   ObActionsIInputFunc *input,
                                   ObActionsICancelFunc *cancel,
                                   ObActionsIPostFunc *post)
{
    Options *o = g_new0(Options, 1);
    setup_rel(o, node, FALSE, OB_DIRECTION_SOUTH, pre, input, post);
    return o;
}

static gpointer setup_send_down_func(xmlNodePtr node,
                                     ObActionsIPreFunc *pre,
                                     ObActionsIInputFunc *input,
                                     ObActionsICancelFunc *cancel,
                                     ObActionsIPostFunc *post)
{
    Options *o = setup_follow(node);
    setup_rel(o, node, FALSE, OB_DIRECTION_SOUTH,
              pre, (o->follow ? input : NULL), post);
    return o;
}
