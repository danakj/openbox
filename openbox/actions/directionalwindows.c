#include "openbox/actions.h"
#include "openbox/actions_list.h"
#include "openbox/actions_parser.h"
#include "openbox/actions_value.h"
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
    ObActionsList *actions;
} Options;

static gboolean cycling = FALSE;

static gpointer setup_func(GHashTable *config);
static gpointer setup_cycle_func(GHashTable *config,
                                 ObActionsIPreFunc *pre,
                                 ObActionsIInputFunc *input,
                                 ObActionsICancelFunc *cancel,
                                 ObActionsIPostFunc *post);
static gpointer setup_target_func(GHashTable *config);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);
static gboolean i_input_func(guint initial_state,
                             XEvent *e,
                             ObtIC *ic,
                             gpointer options,
                             gboolean *used);
static void     i_cancel_func(gpointer options);

static void     end_cycle(gboolean cancel, guint state, Options *o);

/* 3.4-compatibility */
static gpointer setup_north_cycle_func(GHashTable *config,
                                       ObActionsIPreFunc *pre,
                                       ObActionsIInputFunc *in,
                                       ObActionsICancelFunc *c,
                                       ObActionsIPostFunc *post);
static gpointer setup_south_cycle_func(GHashTable *config,
                                       ObActionsIPreFunc *pre,
                                       ObActionsIInputFunc *in,
                                       ObActionsICancelFunc *c,
                                       ObActionsIPostFunc *post);
static gpointer setup_east_cycle_func(GHashTable *config,
                                      ObActionsIPreFunc *pre,
                                      ObActionsIInputFunc *in,
                                      ObActionsICancelFunc *c,
                                      ObActionsIPostFunc *post);
static gpointer setup_west_cycle_func(GHashTable *config,
                                      ObActionsIPreFunc *pre,
                                      ObActionsIInputFunc *in,
                                      ObActionsICancelFunc *c,
                                      ObActionsIPostFunc *post);
static gpointer setup_northwest_cycle_func(GHashTable *config,
                                           ObActionsIPreFunc *pre,
                                           ObActionsIInputFunc *in,
                                           ObActionsICancelFunc *c,
                                           ObActionsIPostFunc *post);
static gpointer setup_northeast_cycle_func(GHashTable *config,
                                           ObActionsIPreFunc *pre,
                                           ObActionsIInputFunc *in,
                                           ObActionsICancelFunc *c,
                                           ObActionsIPostFunc *post);
static gpointer setup_southwest_cycle_func(GHashTable *config,
                                           ObActionsIPreFunc *pre,
                                           ObActionsIInputFunc *in,
                                           ObActionsICancelFunc *c,
                                           ObActionsIPostFunc *post);
static gpointer setup_southeast_cycle_func(GHashTable *config,
                                           ObActionsIPreFunc *pre,
                                           ObActionsIInputFunc *in,
                                           ObActionsICancelFunc *c,
                                           ObActionsIPostFunc *post);
static gpointer setup_north_target_func(GHashTable *config);
static gpointer setup_south_target_func(GHashTable *config);
static gpointer setup_east_target_func(GHashTable *config);
static gpointer setup_west_target_func(GHashTable *config);
static gpointer setup_northwest_target_func(GHashTable *config);
static gpointer setup_northeast_target_func(GHashTable *config);
static gpointer setup_southwest_target_func(GHashTable *config);
static gpointer setup_southeast_target_func(GHashTable *config);

void action_directionalwindows_startup(void)
{
    actions_register_i("DirectionalCycleWindows", setup_cycle_func, free_func,
                       run_func);
    actions_register("DirectionalTargetWindow", setup_target_func, free_func,
                     run_func);
    /* 3.4-compatibility */
    actions_register_i("DirectionalFocusNorth", setup_north_cycle_func,
                       free_func, run_func);
    actions_register_i("DirectionalFocusSouth", setup_south_cycle_func,
                       free_func, run_func);
    actions_register_i("DirectionalFocusWest", setup_west_cycle_func,
                       free_func, run_func);
    actions_register_i("DirectionalFocusEast", setup_east_cycle_func,
                       free_func, run_func);
    actions_register_i("DirectionalFocusNorthWest", setup_northwest_cycle_func,
                       free_func, run_func);
    actions_register_i("DirectionalFocusNorthEast", setup_northeast_cycle_func,
                       free_func, run_func);
    actions_register_i("DirectionalFocusSouthWest", setup_southwest_cycle_func,
                       free_func, run_func);
    actions_register_i("DirectionalFocusSouthEast", setup_southeast_cycle_func,
                       free_func, run_func);
    actions_register("DirectionalTargetNorth", setup_north_target_func,
                     free_func, run_func);
    actions_register("DirectionalTargetSouth", setup_south_target_func,
                     free_func, run_func);
    actions_register("DirectionalTargetWest", setup_west_target_func,
                     free_func, run_func);
    actions_register("DirectionalTargetEast", setup_east_target_func,
                     free_func, run_func);
    actions_register("DirectionalTargetNorthWest", setup_northwest_target_func,
                     free_func, run_func);
    actions_register("DirectionalTargetNorthEast", setup_northeast_target_func,
                     free_func, run_func);
    actions_register("DirectionalTargetSouthWest", setup_southwest_target_func,
                     free_func, run_func);
    actions_register("DirectionalTargetSouthEast", setup_southeast_target_func,
                     free_func, run_func);
}

static gpointer setup_func(GHashTable *config)
{
    ObActionsValue *v;
    Options *o;

    o = g_slice_new0(Options);
    o->dialog = TRUE;
    o->bar = TRUE;

    v = g_hash_table_lookup(config, "dialog");
    if (v && actions_value_is_string(v))
        o->dialog = actions_value_bool(v);
    v = g_hash_table_lookup(config, "bar");
    if (v && actions_value_is_string(v))
        o->bar = actions_value_bool(v);
    v = g_hash_table_lookup(config, "raise");
    if (v && actions_value_is_string(v))
        o->raise = actions_value_bool(v);
    v = g_hash_table_lookup(config, "panels");
    if (v && actions_value_is_string(v))
        o->dock_windows = actions_value_bool(v);
    v = g_hash_table_lookup(config, "desktop");
    if (v && actions_value_is_string(v))
        o->desktop_windows = actions_value_bool(v);
    v = g_hash_table_lookup(config, "direction");
    if (v && actions_value_is_string(v)) {
        const gchar *s = actions_value_string(v);
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
    if (v && actions_value_is_actions_list(v)) {
        o->actions = actions_value_actions_list(v);
        actions_list_ref(o->actions);
    }
    else {
        ObActionsParser *p = actions_parser_new();
        o->actions = actions_parser_read_string(p,
                                                "focus\n"
                                                "raise\n"
                                                "unshade\n");
        actions_parser_unref(p);
    }

    return o;
}

static gpointer setup_cycle_func(GHashTable *config,
                                 ObActionsIPreFunc *pre,
                                 ObActionsIInputFunc *input,
                                 ObActionsICancelFunc *cancel,
                                 ObActionsIPostFunc *post)
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

    actions_list_unref(o->actions);
    g_slice_free(Options, o);
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
            end_cycle(TRUE, e->xkey.state, options);
            return FALSE;
        }

        /* There were no modifiers and they pressed enter */
        else if ((sym == XK_Return || sym == XK_KP_Enter) && !initial_state) {
            end_cycle(FALSE, e->xkey.state, options);
            return FALSE;
        }
    }
    /* They released the modifiers */
    else if (e->type == KeyRelease && initial_state && !(mods & initial_state))
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

/* 3.4-compatibility */
static gpointer setup_north_cycle_func(GHashTable *config,
                                       ObActionsIPreFunc *pre,
                                       ObActionsIInputFunc *input,
                                       ObActionsICancelFunc *cancel,
                                       ObActionsIPostFunc *post)
{
    Options *o = setup_cycle_func(config, pre, input, cancel, post);
    o->direction = OB_DIRECTION_NORTH;
    return o;
}

static gpointer setup_south_cycle_func(GHashTable *config,
                                       ObActionsIPreFunc *pre,
                                       ObActionsIInputFunc *input,
                                       ObActionsICancelFunc *cancel,
                                       ObActionsIPostFunc *post)
{
    Options *o = setup_cycle_func(config, pre, input, cancel, post);
    o->direction = OB_DIRECTION_SOUTH;
    return o;
}

static gpointer setup_east_cycle_func(GHashTable *config,
                                      ObActionsIPreFunc *pre,
                                      ObActionsIInputFunc *input,
                                      ObActionsICancelFunc *cancel,
                                      ObActionsIPostFunc *post)
{
    Options *o = setup_cycle_func(config, pre, input, cancel, post);
    o->direction = OB_DIRECTION_EAST;
    return o;
}

static gpointer setup_west_cycle_func(GHashTable *config,
                                      ObActionsIPreFunc *pre,
                                      ObActionsIInputFunc *input,
                                      ObActionsICancelFunc *cancel,
                                      ObActionsIPostFunc *post)
{
    Options *o = setup_cycle_func(config, pre, input, cancel, post);
    o->direction = OB_DIRECTION_WEST;
    return o;
}

static gpointer setup_northwest_cycle_func(GHashTable *config,
                                           ObActionsIPreFunc *pre,
                                           ObActionsIInputFunc *input,
                                           ObActionsICancelFunc *cancel,
                                           ObActionsIPostFunc *post)
{
    Options *o = setup_cycle_func(config, pre, input, cancel, post);
    o->direction = OB_DIRECTION_NORTHWEST;
    return o;
}

static gpointer setup_northeast_cycle_func(GHashTable *config,
                                           ObActionsIPreFunc *pre,
                                           ObActionsIInputFunc *input,
                                           ObActionsICancelFunc *cancel,
                                           ObActionsIPostFunc *post)
{
    Options *o = setup_cycle_func(config, pre, input, cancel, post);
    o->direction = OB_DIRECTION_EAST;
    return o;
}

static gpointer setup_southwest_cycle_func(GHashTable *config,
                                           ObActionsIPreFunc *pre,
                                           ObActionsIInputFunc *input,
                                           ObActionsICancelFunc *cancel,
                                           ObActionsIPostFunc *post)
{
    Options *o = setup_cycle_func(config, pre, input, cancel, post);
    o->direction = OB_DIRECTION_SOUTHWEST;
    return o;
}

static gpointer setup_southeast_cycle_func(GHashTable *config,
                                           ObActionsIPreFunc *pre,
                                           ObActionsIInputFunc *input,
                                           ObActionsICancelFunc *cancel,
                                           ObActionsIPostFunc *post)
{
    Options *o = setup_cycle_func(config, pre, input, cancel, post);
    o->direction = OB_DIRECTION_SOUTHEAST;
    return o;
}

static gpointer setup_north_target_func(GHashTable *config)
{
    Options *o = setup_target_func(config);
    o->direction = OB_DIRECTION_NORTH;
    return o;
}

static gpointer setup_south_target_func(GHashTable *config)
{
    Options *o = setup_target_func(config);
    o->direction = OB_DIRECTION_SOUTH;
    return o;
}

static gpointer setup_east_target_func(GHashTable *config)
{
    Options *o = setup_target_func(config);
    o->direction = OB_DIRECTION_EAST;
    return o;
}

static gpointer setup_west_target_func(GHashTable *config)
{
    Options *o = setup_target_func(config);
    o->direction = OB_DIRECTION_WEST;
    return o;
}

static gpointer setup_northwest_target_func(GHashTable *config)
{
    Options *o = setup_target_func(config);
    o->direction = OB_DIRECTION_NORTHWEST;
    return o;
}

static gpointer setup_northeast_target_func(GHashTable *config)
{
    Options *o = setup_target_func(config);
    o->direction = OB_DIRECTION_NORTHEAST;
    return o;
}

static gpointer setup_southwest_target_func(GHashTable *config)
{
    Options *o = setup_target_func(config);
    o->direction = OB_DIRECTION_SOUTHWEST;
    return o;
}

static gpointer setup_southeast_target_func(GHashTable *config)
{
    Options *o = setup_target_func(config);
    o->direction = OB_DIRECTION_SOUTHEAST;
    return o;
}

