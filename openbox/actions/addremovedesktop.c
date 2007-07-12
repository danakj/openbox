#include "openbox/actions.h"
#include "openbox/screen.h"
#include <glib.h>

typedef struct {
    gboolean current;
    gboolean add;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_add_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static gpointer setup_remove_func(ObParseInst *i,
                                  xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_addremovedesktop_startup()
{
    actions_register("AddDesktop",
                     setup_add_func,
                     free_func,
                     run_func,
                     NULL, NULL);
    actions_register("RemoveDesktop",
                     setup_remove_func,
                     free_func,
                     run_func,
                     NULL, NULL);
}

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_new0(Options, 1);

    if ((n = parse_find_node("where", node))) {
        gchar *s = parse_string(doc, n);
        if (!g_ascii_strcasecmp(s, "last"))
            o->current = FALSE;
        else if (!g_ascii_strcasecmp(s, "current"))
            o->current = TRUE;
        g_free(s);
    }

    return o;
}

static gpointer setup_add_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = setup_func(i, doc, node);
    o->add = TRUE;
    return o;
}

static gpointer setup_remove_func(ObParseInst *i,
                                  xmlDocPtr doc, xmlNodePtr node)
{
    Options *o = setup_func(i, doc, node);
    o->add = FALSE;
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

    actions_client_move(data, TRUE);

    if (o->add)
        screen_add_desktop(o->current);
    else
        screen_remove_desktop(o->current);

    actions_client_move(data, FALSE);

    return FALSE;
}
