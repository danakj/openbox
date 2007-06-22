#include "openbox/actions.h"
#include "openbox/event.h"
#include "openbox/client.h"
#include "openbox/focus.h"

typedef struct {
    gboolean here;
    gboolean raise;
    gboolean unshade;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_activate_startup()
{
    actions_register("Activate",
                     setup_func,
                     free_func,
                     run_func,
                     NULL, NULL);
}

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_new0(Options, 1);
    o->raise = TRUE;
    o->unshade = TRUE;

    if ((n = parse_find_node("here", node)))
        o->here = parse_bool(doc, n);
    if ((n = parse_find_node("raise", node)))
        o->raise = parse_bool(doc, n);
    if ((n = parse_find_node("unshade", node)))
        o->unshade = parse_bool(doc, n);
    return o;
}

static void free_func(gpointer options)
{
    Options *o = options;

    g_free(o);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;

    if (data->client) {
        if (data->button == 0 || client_mouse_focusable(data->client) ||
            data->context != OB_FRAME_CONTEXT_CLIENT ||
            data->context != OB_FRAME_CONTEXT_FRAME)
        {
            client_activate(data->client, o->here, o->raise, o->unshade, TRUE);
        }
    } else {
        /* focus action on something other than a client, make keybindings
           work for this openbox instance, but don't focus any specific client
        */
        focus_nothing();
    }

    return FALSE;
}
