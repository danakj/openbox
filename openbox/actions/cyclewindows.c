#include "openbox/actions.h"
#include "openbox/stacking.h"
#include "openbox/window.h"
#include "openbox/event.h"
#include "openbox/focus_cycle.h"
#include "openbox/openbox.h"
#include "gettext.h"

typedef struct {
    gboolean linear;
    gboolean dialog;
    gboolean dock_windows;
    gboolean desktop_windows;
    gboolean all_desktops;
    gboolean forward;
    gboolean bar;
    gboolean raise;
    GSList *actions;
} Options;

static gboolean cycling = FALSE;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_forward_func(ObParseInst *i, xmlDocPtr doc,
                                   xmlNodePtr node);
static gpointer setup_backward_func(ObParseInst *i, xmlDocPtr doc,
                                    xmlNodePtr node);
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

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_new0(Options, 1);
    o->dialog = TRUE;
    o->bar = TRUE;

    if ((n = parse_find_node("linear", node)))
        o->linear = parse_bool(doc, n);
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
    if ((n = parse_find_node("allDesktops", node)))
        o->all_desktops = parse_bool(doc, n);

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

static gpointer setup_forward_func(ObParseInst *i, xmlDocPtr doc,
                                   xmlNodePtr node)
{
    Options *o = setup_func(i, doc, node);
    o->forward = TRUE;
    return o;
}

static gpointer setup_backward_func(ObParseInst *i, xmlDocPtr doc,
                                    xmlNodePtr node)
{
    Options *o = setup_func(i, doc, node);
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
                     o->dialog,
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

    ft = focus_cycle(o->forward,
                     o->all_desktops,
                     o->dock_windows,
                     o->desktop_windows,
                     o->linear,
                     TRUE,
                     o->bar,
                     o->dialog,
                     TRUE, cancel);
    cycling = FALSE;

    if (ft)
        actions_run_acts(o->actions, OB_USER_ACTION_KEYBOARD_KEY,
                         state, -1, -1, 0, OB_FRAME_CONTEXT_NONE, ft);

    stacking_restore();
}
