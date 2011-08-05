#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/config_value.h"
#include "openbox/screen.h"
#include "openbox/client.h"
#include "openbox/client_set.h"
#include "openbox/openbox.h"
#include "obt/keyboard.h"

typedef enum {
    LAST,
    CURRENT,
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

    /* for the foreach function */
    guint d;
    GSList *moved;
} Options;

static gpointer setup_go_func(GHashTable *config,
                              ObActionIPreFunc *pre,
                              ObActionIInputFunc *input,
                              ObActionICancelFunc *cancel,
                              ObActionIPostFunc *post);
static gpointer setup_send_func(GHashTable *config,
                                ObActionIPreFunc *pre,
                                ObActionIInputFunc *input,
                                ObActionICancelFunc *cancel,
                                ObActionIPostFunc *post);
static void free_func(gpointer o);
static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options);

static gboolean i_pre_func(guint state, gpointer options);
static gboolean i_input_func(guint initial_state,
                             XEvent *e,
                             ObtIC *ic,
                             gpointer options,
                             gboolean *used);
static void i_post_func(gpointer options);


void action_desktop_startup(void)
{
    action_register_i("GoToDesktop", OB_ACTION_DEFAULT_FILTER_EMPTY,
                      setup_go_func, free_func, run_func);
    action_register_i("SendToDesktop", OB_ACTION_DEFAULT_FILTER_SINGLE,
                      setup_send_func, free_func, run_func);
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
    /* don't go anywhere if there are no options given */
    o->type = ABSOLUTE;
    o->u.abs.desktop = screen_desktop;
    /* wrap by default - it's handy! */
    o->u.rel.wrap = TRUE;

    v = g_hash_table_lookup(config, "to");
    if (v && config_value_is_string(v)) {
        const gchar *s = config_value_string(v);
        if (!g_ascii_strcasecmp(s, "last"))
            o->type = LAST;
        else if (!g_ascii_strcasecmp(s, "current"))
            o->type = CURRENT;
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
    }

    v = g_hash_table_lookup(config, "wrap");
    if (v && config_value_is_string(v))
        o->u.rel.wrap = config_value_bool(v);

    return o;
}


static gpointer setup_go_func(GHashTable *config,
                              ObActionIPreFunc *pre,
                              ObActionIInputFunc *input,
                              ObActionICancelFunc *cancel,
                              ObActionIPostFunc *post)
{
    Options *o;

    o = setup_func(config, pre, input, cancel, post);
    if (o->type == RELATIVE) {
        o->interactive = TRUE;
        *pre = i_pre_func;
        *input = i_input_func;
        *post = i_post_func;
    }

    return o;
}

static gpointer setup_send_func(GHashTable *config,
                                ObActionIPreFunc *pre,
                                ObActionIInputFunc *input,
                                ObActionICancelFunc *cancel,
                                ObActionIPostFunc *post)
{
    ObConfigValue *v;
    Options *o;

    o = setup_func(config, pre, input, cancel, post);
    o->send = TRUE;
    o->follow = TRUE;

    v = g_hash_table_lookup(config, "follow");
    if (v && config_value_is_string(v))
        o->follow = config_value_bool(v);

    if (o->type == RELATIVE && o->follow) {
        o->interactive = TRUE;
        *pre = i_pre_func;
        *input = i_input_func;
        *post = i_post_func;
    }

    return o;
}

static void free_func(gpointer o)
{
    g_slice_free(Options, o);
}

static gboolean each_send(ObClient *c, const ObActionListRun *data,
                          gpointer options)
{
    Options *o = options;
    if (client_normal(c)) {
        client_set_desktop(c, o->d, o->follow, FALSE);
        o->moved = g_slist_prepend(o->moved, c);
    }
    return TRUE;
}

static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options)
{
    Options *o = options;
    guint d;

    switch (o->type) {
    case LAST:
        d = screen_last_desktop;
        break;
    case CURRENT:
        d = screen_desktop;
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

    if (d < screen_num_desktops) {        
        gboolean go;

        go = TRUE;
        o->d = d;
        o->moved = NULL;

        action_client_move(data, TRUE);
        if (o->send) {
            client_set_run(set, data, each_send, o);
            go = o->follow;
        }

        if (go) {
            GSList *it;

            if (d != screen_desktop)
                screen_set_desktop(d, TRUE);
            for (it = o->moved; it; it = g_slist_next(it))
                client_bring_helper_windows(it->data);
        }
        g_slist_free(o->moved);

        action_client_move(data, FALSE);
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
        if (sym == XK_Escape)
            return FALSE;

        /* There were no modifiers and they pressed enter */
        else if ((sym == XK_Return || sym == XK_KP_Enter) && !initial_state)
            return FALSE;
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
