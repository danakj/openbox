#include "openbox/actions.h"
#include "openbox/screen.h"
#include <glib.h>

typedef struct {
    gboolean current;
    gboolean add;
} Options;

static gpointer setup_func(xmlNodePtr node);
static gpointer setup_add_func(xmlNodePtr node);
static gpointer setup_remove_func(xmlNodePtr node);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_addremovedesktop_startup(void)
{
    actions_register("AddDesktop", setup_add_func, g_free, run_func,
                     NULL, NULL);
    actions_register("RemoveDesktop", setup_remove_func, g_free, run_func,
                     NULL, NULL);
}

static gpointer setup_func(xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_new0(Options, 1);

    if ((n = obt_parse_find_node(node, "where"))) {
        gchar *s = obt_parse_node_string(n);
        if (!g_ascii_strcasecmp(s, "last"))
            o->current = FALSE;
        else if (!g_ascii_strcasecmp(s, "current"))
            o->current = TRUE;
        g_free(s);
    }

    return o;
}

static gpointer setup_add_func(xmlNodePtr node)
{
    Options *o = setup_func(node);
    o->add = TRUE;
    return o;
}

static gpointer setup_remove_func(xmlNodePtr node)
{
    Options *o = setup_func(node);
    o->add = FALSE;
    return o;
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;

    actions_client_move(data, TRUE);

    if (o->add)
        screen_add_desktop(o->current);
    else
        screen_remove_desktop(o->current);

    actions_client_move(data, FALSE);

    return FALSE;
}
