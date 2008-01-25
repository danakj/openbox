#include "openbox/actions.h"
#include "openbox/event.h"
#include "openbox/stacking.h"
#include "openbox/window.h"
#include "openbox/focus_cycle.h"
#include "openbox/openbox.h"
#include "openbox/client.h"
#include "openbox/misc.h"
#include "gettext.h"

typedef struct {
    gboolean interactive;
    gboolean dialog;
    gboolean dock_windows;
    gboolean desktop_windows;
    ObDirection direction;
    gboolean bar;
    gboolean raise;
    GSList *actions;
} Options;

static gboolean cycling = FALSE;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_north_cycle_func(ObParseInst *i,
                                       xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_south_cycle_func(ObParseInst *i,
                                       xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_east_cycle_func(ObParseInst *i,
                                      xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_west_cycle_func(ObParseInst *i,
                                      xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_northwest_cycle_func(ObParseInst *i,
                                           xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_northeast_cycle_func(ObParseInst *i,
                                           xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_southwest_cycle_func(ObParseInst *i,
                                           xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_southeast_cycle_func(ObParseInst *i,
                                           xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_north_target_func(ObParseInst *i,
                                        xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_south_target_func(ObParseInst *i,
                                        xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_east_target_func(ObParseInst *i,
                                       xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_west_target_func(ObParseInst *i,
                                       xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_northwest_target_func(ObParseInst *i,
                                            xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_northeast_target_func(ObParseInst *i,
                                            xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_southwest_target_func(ObParseInst *i,
                                            xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_southeast_target_func(ObParseInst *i,
                                            xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);
static gboolean i_input_func(guint initial_state,
                             XEvent *e,
                             gpointer options,
                             gboolean *used);
static void     i_cancel_func(gpointer options);

static void     end_cycle(gboolean cancel, guint state, Options *o);

void action_directionalwindows_startup(void)
{
    actions_register("DirectionalFocusNorth", setup_north_cycle_func,
                     free_func, run_func, i_input_func, i_cancel_func);
    actions_register("DirectionalFocusSouth", setup_south_cycle_func,
                     free_func, run_func, i_input_func, i_cancel_func);
    actions_register("DirectionalFocusWest", setup_west_cycle_func,
                     free_func, run_func, i_input_func, i_cancel_func);
    actions_register("DirectionalFocusEast", setup_east_cycle_func,
                     free_func, run_func, i_input_func, i_cancel_func);
    actions_register("DirectionalFocusNorthWest", setup_northwest_cycle_func,
                     free_func, run_func, i_input_func, i_cancel_func);
    actions_register("DirectionalFocusNorthEast", setup_northeast_cycle_func,
                     free_func, run_func, i_input_func, i_cancel_func);
    actions_register("DirectionalFocusSouthWest", setup_southwest_cycle_func,
                     free_func, run_func, i_input_func, i_cancel_func);
    actions_register("DirectionalFocusSouthEast", setup_southeast_cycle_func,
                     free_func, run_func, i_input_func, i_cancel_func);
    actions_register("DirectionalTargetNorth", setup_north_target_func,
                     free_func, run_func, i_input_func, i_cancel_func);
    actions_register("DirectionalTargetSouth", setup_south_target_func,
                     free_func, run_func, i_input_func, i_cancel_func);
    actions_register("DirectionalTargetWest", setup_west_target_func,
                     free_func, run_func, i_input_func, i_cancel_func);
    actions_register("DirectionalTargetEast", setup_east_target_func,
                     free_func, run_func, i_input_func, i_cancel_func);
    actions_register("DirectionalTargetNorthWest", setup_northwest_target_func,
                     free_func, run_func, i_input_func, i_cancel_func);
    actions_register("DirectionalTargetNorthEast", setup_northeast_target_func,
                     free_func, run_func, i_input_func, i_cancel_func);
    actions_register("DirectionalTargetSouthWest", setup_southwest_target_func,
                     free_func, run_func, i_input_func, i_cancel_func);
    actions_register("DirectionalTargetSouthEast", setup_southeast_target_func,
                     free_func, run_func, i_input_func, i_cancel_func);
}

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_new0(Options, 1);
    o->dialog = TRUE;
    o->bar = TRUE;

    if ((n = parse_find_node("dialog", node)))
        o->dialog = parse_bool(doc, n);
    if ((n = parse_find_node("bar", node)))
        o->bar = parse_bool(doc, n);
    if ((n = parse_find_node("raise", node)))
        o->raise = parse_bool(doc, n);
    if ((n = parse_find_node("panels", node)))
        o->dock_windows = parse_bool(doc, n);
    if ((n = parse_find_node("desktop", node)))
        o->desktop_windows = parse_bool(doc, n);

    if ((n = parse_find_node("finalactions", node))) {
        xmlNodePtr m;

        m = parse_find_node("action", n->xmlChildrenNode);
        while (m) {
            ObActionsAct *action = actions_parse(i, doc, m);
            if (action) o->actions = g_slist_prepend(o->actions, action);
            m = parse_find_node("action", m->next);
        }
    }
    else {
        o->actions = g_slist_prepend(o->actions,
                                     actions_parse_string("Focus"));
        o->actions = g_slist_prepend(o->actions,
                                     actions_parse_string("Raise"));
        o->actions = g_slist_prepend(o->actions,
                                     actions_parse_string("Unshade"));
    }

    return o;
}

static gpointer setup_north_cycle_func(ObParseInst *i,
                                       xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = setup_func(i, doc, node);
    o->interactive = TRUE;
    o->direction = OB_DIRECTION_NORTH;
    return o;
}

static gpointer setup_south_cycle_func(ObParseInst *i,
                                       xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = setup_func(i, doc, node);
    o->interactive = TRUE;
    o->direction = OB_DIRECTION_SOUTH;
    return o;
}

static gpointer setup_east_cycle_func(ObParseInst *i,
                                      xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = setup_func(i, doc, node);
    o->interactive = TRUE;
    o->direction = OB_DIRECTION_EAST;
    return o;
}

static gpointer setup_west_cycle_func(ObParseInst *i,
                                      xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = setup_func(i, doc, node);
    o->interactive = TRUE;
    o->direction = OB_DIRECTION_WEST;
    return o;
}

static gpointer setup_northwest_cycle_func(ObParseInst *i,
                                           xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = setup_func(i, doc, node);
    o->interactive = TRUE;
    o->direction = OB_DIRECTION_NORTHWEST;
    return o;
}

static gpointer setup_northeast_cycle_func(ObParseInst *i,
                                           xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = setup_func(i, doc, node);
    o->interactive = TRUE;
    o->direction = OB_DIRECTION_EAST;
    return o;
}

static gpointer setup_southwest_cycle_func(ObParseInst *i,
                                           xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = setup_func(i, doc, node);
    o->interactive = TRUE;
    o->direction = OB_DIRECTION_SOUTHWEST;
    return o;
}

static gpointer setup_southeast_cycle_func(ObParseInst *i,
                                           xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = setup_func(i, doc, node);
    o->interactive = TRUE;
    o->direction = OB_DIRECTION_SOUTHEAST;
    return o;
}

static gpointer setup_north_target_func(ObParseInst *i,
                                        xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = setup_func(i, doc, node);
    o->interactive = FALSE;
    o->direction = OB_DIRECTION_NORTH;
    return o;
}

static gpointer setup_south_target_func(ObParseInst *i,
                                        xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = setup_func(i, doc, node);
    o->interactive = FALSE;
    o->direction = OB_DIRECTION_SOUTH;
    return o;
}

static gpointer setup_east_target_func(ObParseInst *i,
                                       xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = setup_func(i, doc, node);
    o->interactive = FALSE;
    o->direction = OB_DIRECTION_EAST;
    return o;
}

static gpointer setup_west_target_func(ObParseInst *i,
                                       xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = setup_func(i, doc, node);
    o->interactive = FALSE;
    o->direction = OB_DIRECTION_WEST;
    return o;
}

static gpointer setup_northwest_target_func(ObParseInst *i,
                                            xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = setup_func(i, doc, node);
    o->interactive = FALSE;
    o->direction = OB_DIRECTION_NORTHWEST;
    return o;
}

static gpointer setup_northeast_target_func(ObParseInst *i,
                                            xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = setup_func(i, doc, node);
    o->interactive = FALSE;
    o->direction = OB_DIRECTION_NORTHEAST;
    return o;
}

static gpointer setup_southwest_target_func(ObParseInst *i,
                                            xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = setup_func(i, doc, node);
    o->interactive = FALSE;
    o->direction = OB_DIRECTION_SOUTHWEST;
    return o;
}

static gpointer setup_southeast_target_func(ObParseInst *i,
                                            xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = setup_func(i, doc, node);
    o->interactive = FALSE;
    o->direction = OB_DIRECTION_SOUTHEAST;
    return o;
}

static void free_func(gpointer options)
{
    Options *o = options;

    while (o->actions) {
        actions_act_unref(o->actions->data);
        o->actions = g_slist_delete_link(o->actions, o->actions);
    }

    g_free(o);
}

static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;

    if (!o->interactive)
        end_cycle(FALSE, data->state, o);
    else {
        struct _ObClient *ft;

        ft = focus_directional_cycle(o->direction,
                                     o->dock_windows,
                                     o->desktop_windows,
                                     TRUE,
                                     o->bar,
                                     o->dialog,
                                     FALSE, FALSE);
        cycling = TRUE;

        stacking_restore();
        if (o->raise) stacking_temp_raise(CLIENT_AS_WINDOW(ft));
    }

    return o->interactive;
}

static gboolean i_input_func(guint initial_state,
                             XEvent *e,
                             gpointer options,
                             gboolean *used)
{
    if (e->type == KeyPress) {
        /* Escape cancels no matter what */
        if (e->xkey.keycode == ob_keycode(OB_KEY_ESCAPE)) {
            end_cycle(TRUE, e->xkey.state, options);
            return FALSE;
        }

        /* There were no modifiers and they pressed enter */
        else if (e->xkey.keycode == ob_keycode(OB_KEY_RETURN) &&
                 !initial_state)
        {
            end_cycle(FALSE, e->xkey.state, options);
            return FALSE;
        }
    }
    /* They released the modifiers */
    else if (e->type == KeyRelease && initial_state &&
             (e->xkey.state & initial_state) == 0)
    {
        end_cycle(FALSE, e->xkey.state, options);
        return FALSE;
    }

    return TRUE;
}

static void i_cancel_func(gpointer options)
{
    /* we get cancelled when we move focus, but we're not cycling anymore, so
       just ignore that */
    if (cycling)
        end_cycle(TRUE, 0, options);
}

static void end_cycle(gboolean cancel, guint state, Options *o)
{
    struct _ObClient *ft;

    ft = focus_directional_cycle(o->direction,
                                 o->dock_windows,
                                 o->desktop_windows,
                                 o->interactive,
                                 o->bar,
                                 o->dialog,
                                 TRUE, cancel);
    cycling = FALSE;

    if (ft)
        actions_run_acts(o->actions, OB_USER_ACTION_KEYBOARD_KEY,
                         state, -1, -1, 0, OB_FRAME_CONTEXT_NONE, ft);

    stacking_restore();
}
