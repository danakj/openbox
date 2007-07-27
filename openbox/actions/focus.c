#include "openbox/actions.h"
#include "openbox/event.h"
#include "openbox/client.h"
#include "openbox/focus.h"

typedef struct {
    gboolean here;
} Options;

static gpointer setup_func(xmlNodePtr node);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_focus_startup(void)
{
    actions_register("Focus", setup_func, g_free, run_func, NULL, NULL);
}

static gpointer setup_func(xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_new0(Options, 1);

    if ((n = obt_parse_find_node(node, "here")))
        o->here = obt_parse_node_bool(n);
    return o;
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
            client_activate(data->client, o->here, FALSE, FALSE, TRUE);
        }
    } else if (data->context == OB_FRAME_CONTEXT_DESKTOP) {
        /* focus action on the root window. make keybindings work for this
           openbox instance, but don't focus any specific client */
        focus_nothing();
    }

    return FALSE;
}
