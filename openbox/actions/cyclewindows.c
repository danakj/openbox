#include "openbox/actions.h"
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
    GSList *actions;
} Options;

static gboolean cycling = FALSE;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);
static gboolean i_input_func(guint initial_state,
                             XEvent *e,
                             gpointer options,
                             gboolean *used);
static void     i_cancel_func(gpointer options);

static void     end_cycle(gboolean cancel, guint state, Options *o);

void action_cyclewindows_startup()
{
    actions_register("CycleWindows",
                     setup_func,
                     free_func,
                     run_func,
                     i_input_func,
                     i_cancel_func);
}

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_new0(Options, 1);
    o->dialog = TRUE;

    if ((n = parse_find_node("forward", node)))
        o->forward = parse_bool(doc, n);
    if ((n = parse_find_node("linear", node)))
        o->linear = parse_bool(doc, n);
    if ((n = parse_find_node("dialog", node)))
        o->dialog = parse_bool(doc, n);
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
    return o;
}

static void free_func(gpointer options)
{
    Options *o = options;

    g_free(o);
}

static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;

    /* if using focus_delay, stop the timer now so that focus doesn't go moving
       on us */
    event_halt_focus_delay();
    
    focus_cycle(o->forward,
                o->all_desktops,
                o->dock_windows,
                o->desktop_windows,
                o->linear,
                TRUE,
                o->dialog,
                FALSE, FALSE);
    cycling = TRUE;

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
                     o->dialog,
                     TRUE, cancel);

    if (ft) {
        actions_run_acts(o->actions, OB_USER_ACTION_KEYBOARD_KEY,
                         state, -1, -1, 0, OB_FRAME_CONTEXT_NONE, ft);
    }
    cycling = FALSE;
}
