#include "openbox/action.h"
#include "openbox/action_list_run.h"
#include "openbox/action_value.h"
#include "openbox/event.h"
#include "openbox/client.h"
#include "openbox/client_set.h"
#include "openbox/focus.h"
#include "openbox/screen.h"

typedef struct {
    gboolean here;
    gboolean stop_int;
} Options;

static gpointer setup_func(GHashTable *config);
static void free_func(gpointer o);
static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options);

void action_focus_startup(void)
{
    action_register("Focus", OB_ACTION_DEFAULT_FILTER_SINGLE,
                    setup_func, free_func, run_func);
}

static gpointer setup_func(GHashTable *config)
{
    ObActionValue *v;
    Options *o;

    o = g_slice_new0(Options);
    o->stop_int = TRUE;

    v = g_hash_table_lookup(config, "here");
    if (v && action_value_is_string(v))
        o->here = action_value_bool(v);
    v = g_hash_table_lookup(config, "stopInteractive");
    if (v && action_value_is_string(v))
        o->stop_int = action_value_bool(v);
    return o;
}

static void free_func(gpointer o)
{
    g_slice_free(Options, o);
}

static gboolean each_focus(ObClient *c, const ObActionListRun *data,
                           gpointer options)
{
    Options *o = options;

/*
    ob_debug("button %d focusable %d context %d %d %d\n",
             data->button, client_mouse_focusable(data->client),
             data->context,
             OB_FRAME_CONTEXT_CLIENT, OB_FRAME_CONTEXT_FRAME);
*/
    if (data->pointer_button == 0 ||
        client_mouse_focusable(c) ||
        (data->pointer_context != OB_FRAME_CONTEXT_CLIENT &&
         data->pointer_context != OB_FRAME_CONTEXT_FRAME))
    {
        if (o->stop_int)
            action_interactive_cancel_act();

        client_activate(c, TRUE, o->here, FALSE, FALSE, TRUE);
    }

    return FALSE; /* only do this to one client */
}

/* Always return FALSE because its not interactive */
static gboolean run_func(const ObClientSet *set,
                         const ObActionListRun *data, gpointer options)
{
    Options *o = options;

    if (client_set_is_empty(set)) {
        if (data->pointer_context == OB_FRAME_CONTEXT_DESKTOP)
            if (o->stop_int)
                action_interactive_cancel_act();

        /* focus action on the root window. make keybindings work for this
           openbox instance, but don't focus any specific client */
        focus_nothing();
    }
    else {
        action_client_move(data, TRUE);
        client_set_run(set, data, each_focus, options);
        action_client_move(data, FALSE);
    }
    return FALSE;
}
