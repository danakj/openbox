#include "openbox/actions.h"
#include "openbox/screen.h"
#include <glib.h>

typedef struct {
    guint desktop;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_desktop_startup()
{
    actions_register("desktop",
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

    if ((n = parse_find_node("desktop", node)))
        o->desktop = parse_int(doc, n) - 1;
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

    if (o->desktop < screen_num_desktops)
        screen_set_desktop(o->desktop, TRUE);

    return FALSE;
}
