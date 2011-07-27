#include "openbox/actions.h"
#include "openbox/actions_value.h"
#include "openbox/event.h"
#include "openbox/client.h"
#include "openbox/focus.h"
#include "openbox/screen.h"

typedef struct {
    gboolean here;
    gboolean stop_int;
} Options;

static gpointer setup_func(GHashTable *config);
static void free_func(gpointer o);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_focus_startup(void)
{
    actions_register("Focus", setup_func, free_func, run_func);
}

static gpointer setup_func(GHashTable *config)
{
    ObActionsValue *v;
    Options *o;

    o = g_slice_new0(Options);
    o->stop_int = TRUE;

    v = g_hash_table_lookup(config, "here");
    if (v && actions_value_is_string(v))
        o->here = actions_value_bool(v);
    v = g_hash_table_lookup(config, "stopInteractive");
    if (v && actions_value_is_string(v))
        o->stop_int = actions_value_bool(v);
    return o;
}

static void free_func(gpointer o)
{
    g_slice_free(Options, o);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;

    if (data->client) {
/*
        ob_debug("button %d focusable %d context %d %d %d\n",
                 data->button, client_mouse_focusable(data->client),
                 data->context,
                 OB_FRAME_CONTEXT_CLIENT, OB_FRAME_CONTEXT_FRAME);
*/
        if (data->button == 0 || client_mouse_focusable(data->client) ||
            (data->context != OB_FRAME_CONTEXT_CLIENT &&
             data->context != OB_FRAME_CONTEXT_FRAME))
        {
            if (o->stop_int)
                actions_interactive_cancel_act();

            actions_client_move(data, TRUE);
            client_activate(data->client, TRUE, o->here, FALSE, FALSE, TRUE);
            actions_client_move(data, FALSE);
        }
    } else if (data->context == OB_FRAME_CONTEXT_DESKTOP) {
        if (o->stop_int)
            actions_interactive_cancel_act();

        /* focus action on the root window. make keybindings work for this
           openbox instance, but don't focus any specific client */
        focus_nothing();
    }

    return FALSE;
}
