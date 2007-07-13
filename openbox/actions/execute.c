#include "openbox/actions.h"
#include "openbox/event.h"
#include "openbox/startupnotify.h"
#include "openbox/screen.h"
#include "gettext.h"

typedef struct {
    gchar   *cmd;
    gboolean sn;
    gchar   *sn_name;
    gchar   *sn_icon;
    gchar   *sn_wmclass;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);
/*
static gboolean i_input_func(guint initial_state,
                             XEvent *e,
                             gpointer options,
                             gboolean *used);
static void     i_cancel_func(gpointer options);
*/

void action_execute_startup()
{
    actions_register("Execute",
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

    if ((n = parse_find_node("startupnotify", node))) {
        xmlNodePtr m;
        if ((m = parse_find_node("enabled", n->xmlChildrenNode)))
            o->sn = parse_bool(doc, m);
        if ((m = parse_find_node("name", n->xmlChildrenNode)))
            o->sn_name = parse_string(doc, m);
        if ((m = parse_find_node("icon", n->xmlChildrenNode)))
            o->sn_icon = parse_string(doc, m);
        if ((m = parse_find_node("wmclass", n->xmlChildrenNode)))
            o->sn_wmclass = parse_string(doc, m);
    }
    return o;
}

static void free_func(gpointer options)
{
    Options *o = options;

    if (o) {
        g_free(o->cmd);
        g_free(o->sn_name);
        g_free(o->sn_icon);
        g_free(o->sn_wmclass);
        g_free(o);
    }
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    GError *e = NULL;
    gchar **argv = NULL;
    gchar *cmd;
    Options *o = options;

    if (!o->cmd) return FALSE;
    cmd = g_filename_from_utf8(o->cmd, -1, NULL, NULL, NULL);
    if (!cmd) {
        g_message(_("Failed to convert the path '%s' from utf8"), o->cmd);
        return FALSE;
    }

    /* If there is a keyboard grab going on then we need to cancel
       it so the application can grab things */
    event_cancel_all_key_grabs();

    if (!g_shell_parse_argv(cmd, NULL, &argv, &e)) {
        g_message(_("Failed to execute '%s': %s"), o->cmd, e->message);
        g_error_free(e);
    }
    else {
        gchar *program = NULL;

        if (o->sn) {
            program = g_path_get_basename(argv[0]);
            /* sets up the environment */
            sn_setup_spawn_environment(program, o->sn_name, o->sn_icon,
                                       /* launch it on the current desktop */
                                       screen_desktop);
        }

        if (!g_spawn_async(NULL, argv, NULL,
                           G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD,
                           NULL, NULL, NULL, &e))
        {
            g_message(_("Failed to execute '%s': %s"), o->cmd, e->message);
            g_error_free(e);

            if (o->sn)
                sn_spawn_cancel();
        }
        if (o->sn)
            unsetenv("DESKTOP_STARTUP_ID");

        g_free(program);
        g_strfreev(argv);
    }

    g_free(cmd);

    return FALSE;
}
