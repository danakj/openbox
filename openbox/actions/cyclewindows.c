#include "openbox/action.h"
#include "openbox/action_list.h"
#include "openbox/action_list_run.h"
#include "openbox/action_parser.h"
#include "openbox/config_value.h"
#include "openbox/client_set.h"
#include "openbox/stacking.h"
#include "openbox/window.h"
#include "openbox/event.h"
#include "openbox/focus_cycle.h"
#include "openbox/openbox.h"
#include "gettext.h"
#include "obt/keyboard.h"

typedef struct {
    gboolean linear;
    gboolean dock_windows;
    gboolean desktop_windows;
    gboolean forward;
    gboolean bar;
    gboolean raise;
    ObFocusCyclePopupMode dialog_mode;
    ObActionList *actions;


    /* options for after we're done */
    gboolean cancel; /* did the user cancel or not */
    guint state;     /* keyboard state when finished */
} Options;

static gpointer setup_func(GHashTable *config,
                           ObActionIPreFunc *pre,
                           ObActionIInputFunc *in,
                           ObActionICancelFunc *c,
                           ObActionIPostFunc *post);
static gpointer setup_forward_func(GHashTable *config,
                                   ObActionIPreFunc *pre,
                                   ObActionIInputFunc *in,
                                   ObActionICancelFunc *c,
                                   ObActionIPostFunc *post);
static gpointer setup_backward_func(GHashTable *config,
                                    ObActionIPreFunc *pre,
                                    ObActionIInputFunc *in,
                                    ObActionICancelFunc *c,
                                    ObActionIPostFunc *post);
static void     free_func(gpointer options);
static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options);
static gboolean i_input_func(guint initial_state,
                             XEvent *e,
                             ObtIC *ic,
                             gpointer options,
                             gboolean *used);
static void     i_cancel_func(gpointer options);
static void     i_post_func(gpointer options);

void action_cyclewindows_startup(void)
{
    action_register_i("NextWindow", OB_ACTION_DEFAULT_FILTER_ALL,
                      setup_forward_func, free_func, run_func);
    action_register_i("PreviousWindow", OB_ACTION_DEFAULT_FILTER_ALL,
                      setup_backward_func, free_func, run_func);
}

static gpointer setup_func(GHashTable *config,
                           ObActionIPreFunc *pre,
                           ObActionIInputFunc *input,
                           ObActionICancelFunc *cancel,
                           ObActionIPostFunc *post)
{
    ObConfigValue *v;
    Options *o;

    o = g_slice_new0(Options);
    o->bar = TRUE;
    o->dialog_mode = OB_FOCUS_CYCLE_POPUP_MODE_LIST;

    v = g_hash_table_lookup(config, "linear");
    if (v && config_value_is_string(v))
        o->linear = config_value_bool(v);
    v = g_hash_table_lookup(config, "dialog");
    if (v && config_value_is_string(v)) {
        const gchar *s = config_value_string(v);
        if (g_strcasecmp(s, "none") == 0)
            o->dialog_mode = OB_FOCUS_CYCLE_POPUP_MODE_NONE;
        else if (g_strcasecmp(s, "icons") == 0)
            o->dialog_mode = OB_FOCUS_CYCLE_POPUP_MODE_ICONS;
    }
    v = g_hash_table_lookup(config, "bar");
    if (v && config_value_is_string(v))
        o->bar = config_value_bool(v);
    v = g_hash_table_lookup(config, "raise");
    if (v && config_value_is_string(v))
        o->raise = config_value_bool(v);
    v = g_hash_table_lookup(config, "panels");
    if (v && config_value_is_string(v))
        o->dock_windows = config_value_bool(v);
    v = g_hash_table_lookup(config, "desktop");
    if (v && config_value_is_string(v))
        o->desktop_windows = config_value_bool(v);

    v = g_hash_table_lookup(config, "finalactions");
    if (v && config_value_is_action_list(v)) {
        o->actions = config_value_action_list(v);
        action_list_ref(o->actions);
    }
    else {
        ObActionParser *p = action_parser_new();
        o->actions = action_parser_read_string(p,
                                                "focus\n"
                                                "raise\n"
                                                "shade set:off\n");
        action_parser_unref(p);
    }

    *input = i_input_func;
    *cancel = i_cancel_func;
    *post = i_post_func;
    return o;
}

static gpointer setup_forward_func(GHashTable *config,
                                   ObActionIPreFunc *pre,
                                   ObActionIInputFunc *input,
                                   ObActionICancelFunc *cancel,
                                   ObActionIPostFunc *post)
{
    Options *o = setup_func(config, pre, input, cancel, post);
    o->forward = TRUE;
    return o;
}

static gpointer setup_backward_func(GHashTable *config,
                                    ObActionIPreFunc *pre,
                                    ObActionIInputFunc *input,
                                    ObActionICancelFunc *cancel,
                                    ObActionIPostFunc *post)
{
    Options *o = setup_func(config, pre, input, cancel, post);
    o->forward = FALSE;
    return o;
}

static void free_func(gpointer options)
{
    Options *o = options;

    action_list_unref(o->actions);
    g_slice_free(Options, o);
}

static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options)
{
    Options *o = options;
    struct _ObClient *ft;

    if (client_set_is_empty(set)) return FALSE;

    ft = focus_cycle(set,
                     o->forward,
                     o->dock_windows,
                     o->desktop_windows,
                     o->linear,
                     TRUE,
                     o->bar,
                     o->dialog_mode,
                     FALSE, FALSE);

    stacking_restore();
    if (o->raise && ft) stacking_temp_raise(CLIENT_AS_WINDOW(ft));

    return TRUE;
}

static gboolean i_input_func(guint initial_state,
                             XEvent *e,
                             ObtIC *ic,
                             gpointer options,
                             gboolean *used)
{
    Options *o = options;
    guint mods;

    mods = obt_keyboard_only_modmasks(e->xkey.state);
    if (e->type == KeyRelease) {
        /* remove from the state the mask of the modifier key being
           released, if it is a modifier key being released that is */
        mods &= ~obt_keyboard_keyevent_to_modmask(e);
    }

    if (e->type == KeyPress) {
        KeySym sym = obt_keyboard_keypress_to_keysym(e);

        /* Escape cancels no matter what */
        if (sym == XK_Escape) {
            o->cancel = TRUE;
            o->state = e->xkey.state;
            return FALSE;
        }

        /* There were no modifiers and they pressed enter */
        else if ((sym == XK_Return || sym == XK_KP_Enter) && !initial_state) {
            o->cancel = FALSE;
            o->state = e->xkey.state;
            return FALSE;
        }
    }
    /* They released the modifiers */
    else if (e->type == KeyRelease && initial_state && !(mods & initial_state))
    {
        o->cancel = FALSE;
        o->state = e->xkey.state;
        return FALSE;
    }

    return TRUE;
}

static void i_cancel_func(gpointer options)
{
    Options *o = options;
    o->cancel = TRUE;
    o->state = 0;
}

static void i_post_func(gpointer options)
{
    Options *o = options;
    struct _ObClient *ft;

    ft = focus_cycle(NULL,
                     o->forward,
                     o->dock_windows,
                     o->desktop_windows,
                     o->linear,
                     TRUE,
                     o->bar,
                     o->dialog_mode,
                     TRUE, o->cancel);

    if (ft)
        action_list_run(o->actions, OB_USER_ACTION_KEYBOARD_KEY,
                        o->state, -1, -1, 0, OB_FRAME_CONTEXT_NONE, ft);

    stacking_restore();
}
