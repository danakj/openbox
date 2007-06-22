#include "openbox/actions.h"
#include "openbox/menu.h"
#include <glib.h>

typedef struct {
    gchar   *name;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_menu_startup()
{
    actions_register("Menu",
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

    if ((n = parse_find_node("menu", node)))
        o->name = parse_string(doc, n);
    return o;
}

static void free_func(gpointer options)
{
    Options *o = options;

    if (o) {
        g_free(o->name);
        g_free(o);
    }
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;

    /* you cannot call ShowMenu from inside a menu */
    if (data->uact == OB_USER_ACTION_MENU_SELECTION) return FALSE;

    if (o->name) {
        menu_show(o->name, data->x, data->y, data->button != 0, data->client);
    }

    return FALSE;
}
