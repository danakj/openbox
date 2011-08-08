#include "openbox/action.h"
#include "openbox/action_list.h"
#include "openbox/action_list_run.h"
#include "openbox/action_parser.h"
#include "openbox/config_value.h"
#include "openbox/client_set.h"
#include "openbox/event.h"
#include "openbox/stacking.h"
#include "openbox/window.h"
#include "openbox/focus_cycle.h"
#include "openbox/openbox.h"
#include "openbox/misc.h"
#include "gettext.h"
#include "obt/keyboard.h"

typedef struct {
    gboolean interactive;
    gboolean dialog;
    gboolean dock_windows;
    gboolean desktop_windows;
    ObDirection direction;
    gboolean bar;
    gboolean raise;
    ObActionList *actions;
} Options;

static gboolean cycling = FALSE;

static gpointer setup_func(GHashTable *config);
static gpointer setup_cycle_func(GHashTable *config,
                                 ObActionIPreFunc *pre,
                                 ObActionIInputFunc *input,
                                 ObActionICancelFunc *cancel,
                                 ObActionIPostFunc *post);
static gpointer setup_target_func(GHashTable *config);
static void     free_func(gpointer options);
static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options);
static gboolean i_input_func(guint initial_state,
                             XEvent *e,
                             ObtIC *ic,
                             gpointer options,
                             gboolean *used);
static void     i_cancel_func(gpointer options);

static void     end_cycle(gboolean cancel, guint state, Options *o);

void action_directionalwindows_startup(void)
{
    action_register_i("DirectionalCycleWindows", OB_ACTION_DEFAULT_FILTER_ALL,
                      setup_cycle_func, free_func, run_func);
    action_register("DirectionalTargetWindow", OB_ACTION_DEFAULT_FILTER_ALL,
                    setup_target_func, free_func, run_func);
}

static gpointer setup_func(GHashTable *config)
{
    ObConfigValue *v;
    Options *o;

    o = g_slice_new0(Options);
    o->dialog = TRUE;
    o->bar = TRUE;

    v = g_hash_table_lookup(config, "dialog");
    if (v && config_value_is_string(v))
        o->dialog = config_value_bool(v);
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
    v = g_hash_table_lookup(config, "direction");
    if (v && config_value_is_string(v)) {
        const gchar *s = config_value_string(v);
        if (!g_ascii_strcasecmp(s, "north") ||
            !g_ascii_strcasecmp(s, "up"))
            o->direction = OB_DIRECTION_NORTH;
        else if (!g_ascii_strcasecmp(s, "northwest"))
            o->direction = OB_DIRECTION_NORTHWEST;
        else if (!g_ascii_strcasecmp(s, "northeast"))
            o->direction = OB_DIRECTION_NORTHEAST;
        else if (!g_ascii_strcasecmp(s, "west") ||
                 !g_ascii_strcasecmp(s, "left"))
            o->direction = OB_DIRECTION_WEST;
        else if (!g_ascii_strcasecmp(s, "east") ||
                 !g_ascii_strcasecmp(s, "right"))
            o->direction = OB_DIRECTION_EAST;
        else if (!g_ascii_strcasecmp(s, "south") ||
                 !g_ascii_strcasecmp(s, "down"))
            o->direction = OB_DIRECTION_SOUTH;
        else if (!g_ascii_strcasecmp(s, "southwest"))
            o->direction = OB_DIRECTION_SOUTHWEST;
        else if (!g_ascii_strcasecmp(s, "southeast"))
            o->direction = OB_DIRECTION_SOUTHEAST;
    }

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

    return o;
}

static gpointer setup_cycle_func(GHashTable *config,
                                 ObActionIPreFunc *pre,
                                 ObActionIInputFunc *input,
                                 ObActionICancelFunc *cancel,
                                 ObActionIPostFunc *post)
{
    Options *o = setup_func(config);
    o->interactive = TRUE;
    *input = i_input_func;
    *cancel = i_cancel_func;
    return o;
}

static gpointer setup_target_func(GHashTable *config)
{
    Options *o = setup_func(config);
    o->interactive = FALSE;
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

    if (client_set_is_empty(set)) return FALSE;

    if (!o->interactive)
        end_cycle(FALSE, data->mod_state, o);
    else {
        struct _ObClient *ft;

        ft = focus_directional_cycle(o->direction,
                                     set,
                                     o->dock_windows,
                                     o->desktop_windows,
                                     TRUE,
                                     o->bar,
                                     o->dialog,
                                     FALSE, FALSE);
        cycling = TRUE;

        stacking_restore();
        if (o->raise && ft) stacking_temp_raise(CLIENT_AS_WINDOW(ft));
    }

    return o->interactive;
}

static gboolean i_input_func(guint initial_state,
                             XEvent *e,
                             ObtIC *ic,
                             gpointer options,
                             gboolean *used)
{
    guint mods, initial_mods;

    initial_mods = obt_keyboard_only_modmasks(initial_state);
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
            end_cycle(TRUE, e->xkey.state, options);
            return FALSE;
        }

        /* There were no modifiers and they pressed enter */
        else if ((sym == XK_Return || sym == XK_KP_Enter) && !initial_mods) {
            end_cycle(FALSE, e->xkey.state, options);
            return FALSE;
        }
    }
    /* They released the modifiers */
    else if (e->type == KeyRelease && initial_mods && !(mods & initial_mods)) {
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
                                 NULL,
                                 o->dock_windows,
                                 o->desktop_windows,
                                 o->interactive,
                                 o->bar,
                                 o->dialog,
                                 TRUE, cancel);
    cycling = FALSE;

    if (ft)
        action_list_run(o->actions, OB_USER_ACTION_KEYBOARD_KEY,
                        state, -1, -1, 0, OB_FRAME_CONTEXT_NONE, ft);

    stacking_restore();
}
