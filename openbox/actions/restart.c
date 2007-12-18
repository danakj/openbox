#include "openbox/actions.h"
#include "openbox/openbox.h"

typedef struct {
    gchar   *cmd;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_restart_startup(void)
{
    actions_register("Restart",
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

    if ((n = parse_find_node("command", node)) ||
        (n = parse_find_node("execute", node)))
    {
        gchar *s = parse_string(doc, n);
        o->cmd = parse_expand_tilde(s);
        g_free(s);
    }
    return o;
}

static void free_func(gpointer options)
{
    Options *o = options;

    if (o) {
        g_free(o->cmd);
        g_free(o);
    }
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;

    ob_restart_other(o->cmd);

    return FALSE;
}
