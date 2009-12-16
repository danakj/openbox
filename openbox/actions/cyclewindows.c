#include "openbox/actions.h"
#include "openbox/stacking.h"
#include "openbox/window.h"
#include "openbox/event.h"
#include "openbox/focus_cycle.h"
#include "openbox/openbox.h"
#include "gettext.h"

typedef struct {
    gboolean linear;
    gboolean dock_windows;
    gboolean desktop_windows;
    gboolean all_desktops;
    gboolean forward;
    gboolean bar;
    gboolean raise;
    ObFocusCyclePopupMode dialog_mode;
    GSList *actions;
} Options;

static gboolean cycling = FALSE;

static gpointer setup_func(xmlNodePtr node);
static gpointer setup_forward_func(xmlNodePtr node);
static gpointer setup_backward_func(xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);
static gboolean i_input_func(guint initial_state,
                             XEvent *e,
                             gpointer options,
                             gboolean *used);
static void     i_cancel_func(gpointer options);

static void     end_cycle(gboolean cancel, guint state, Options *o);

void action_cyclewindows_startup(void)
{
    actions_register("NextWindow", setup_forward_func, free_func,
                     run_func, i_input_func, i_cancel_func);
    actions_register("PreviousWindow", setup_backward_func, free_func,
                     run_func, i_input_func, i_cancel_func);
}

static gpointer setup_func(xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_new0(Options, 1);
    o->bar = TRUE;
    o->dialog_mode = OB_FOCUS_CYCLE_POPUP_MODE_LIST;

    if ((n = obt_parse_find_node(node, "linear")))
        o->linear = obt_parse_node_bool(n);
    if ((n = obt_parse_find_node(node, "dialog"))) {
        if (obt_parse_node_contains(n, "none"))
            o->dialog_mode = OB_FOCUS_CYCLE_POPUP_MODE_NONE;
        else if (obt_parse_node_contains(n, "icons"))
            o->dialog_mode = OB_FOCUS_CYCLE_POPUP_MODE_ICONS;
    }
    if ((n = obt_parse_find_node(node, "bar")))
        o->bar = obt_parse_node_bool(n);
    if ((n = obt_parse_find_node(node, "raise")))
        o->raise = obt_parse_node_bool(n);
    if ((n = obt_parse_find_node(node, "panels")))
        o->dock_windows = obt_parse_node_bool(n);
    if ((n = obt_parse_find_node(node, "desktop")))
        o->desktop_windows = obt_parse_node_bool(n);
    if ((n = obt_parse_find_node(node, "allDesktops")))
        o->all_desktops = obt_parse_node_bool(n);

    if ((n = obt_parse_find_node(node, "finalactions"))) {
        xmlNodePtr m;

        m = obt_parse_find_node(n->children, "action");
        while (m) {
            ObActionsAct *action = actions_parse(m);
            if (action) o->actions = g_slist_append(o->actions, action);
            m = obt_parse_find_node(m->next, "action");
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

static gpointer setup_forward_func(xmlNodePtr node)
{
    Options *o = setup_func(node);
    o->forward = TRUE;
    return o;
}

static gpointer setup_backward_func(xmlNodePtr node)
{
    Options *o = setup_func(node);
    o->forward = FALSE;
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
    struct _ObClient *ft;

    ft = focus_cycle(o->forward,
                     o->all_desktops,
                     o->dock_windows,
                     o->desktop_windows,
                     o->linear,
                     TRUE,
                     o->bar,
                     o->dialog_mode,
                     FALSE, FALSE);
    cycling = TRUE;

    stacking_restore();
    if (o->raise && ft) stacking_temp_raise(CLIENT_AS_WINDOW(ft));

    return TRUE;
}

static gboolean i_input_func(guint initial_state,
                             XEvent *e,
                             gpointer options,
                             gboolean *used)
{
    if (e->type == KeyPress) {
        /* Escape cancels no matter what */
        if (ob_keycode_match(e->xkey.keycode, OB_KEY_ESCAPE)) {
            end_cycle(TRUE, e->xkey.state, options);
            return FALSE;
        }

        /* There were no modifiers and they pressed enter */
        else if (ob_keycode_match(e->xkey.keycode, OB_KEY_RETURN) &&
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

    ft = focus_cycle(o->forward,
                     o->all_desktops,
                     o->dock_windows,
                     o->desktop_windows,
                     o->linear,
                     TRUE,
                     o->bar,
                     o->dialog_mode,
                     TRUE, cancel);
    cycling = FALSE;

    if (ft)
        actions_run_acts(o->actions, OB_USER_ACTION_KEYBOARD_KEY,
                         state, -1, -1, 0, OB_FRAME_CONTEXT_NONE, ft);

    stacking_restore();
}
