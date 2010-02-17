#include "openbox/actions.h"
#include "openbox/client.h"
#include "openbox/event.h"
#include "openbox/startupnotify.h"
#include "openbox/prompt.h"
#include "openbox/screen.h"
#include "gettext.h"

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

typedef struct {
    gchar   *cmd;
    gboolean sn;
    gchar   *sn_name;
    gchar   *sn_icon;
    gchar   *sn_wmclass;
    gchar   *prompt;
    ObActionsData *data;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);
static void shutdown_func(void);
static void client_dest(ObClient *client, gpointer data);

static GSList *prompt_opts = NULL;

void action_execute_startup(void)
{
    actions_register("Execute",
                     setup_func,
                     free_func,
                     run_func,
                     NULL, NULL);
    actions_set_shutdown("Execute", shutdown_func);

    client_add_destroy_notify(client_dest, NULL);
}

static void client_dest(ObClient *client, gpointer data)
{
    GSList *it;

    for (it = prompt_opts; it; it = g_slist_next(it)) {
        Options *o = it->data;
        if (o->data->client == client)
            o->data->client = NULL;
    }
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

    if ((n = parse_find_node("prompt", node)))
        o->prompt = parse_string(doc, n);

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

static void shutdown_func(void)
{
    client_remove_destroy_notify(client_dest);
}

static void free_func(gpointer options)
{
    Options *o = options;

    if (o) {
        prompt_opts = g_slist_remove(prompt_opts, o);

        g_free(o->cmd);
        g_free(o->sn_name);
        g_free(o->sn_icon);
        g_free(o->sn_wmclass);
        g_free(o->prompt);
        if (o->data) g_free(o->data);
        g_free(o);
    }
}

static Options* dup_options(Options *in, ObActionsData *data)
{
    Options *o = g_new(Options, 1);
    o->cmd = g_strdup(in->cmd);
    o->sn = in->sn;
    o->sn_name = g_strdup(in->sn_name);
    o->sn_icon = g_strdup(in->sn_icon);
    o->sn_wmclass = g_strdup(in->sn_wmclass);
    o->prompt = NULL;
    o->data = g_memdup(data, sizeof(ObActionsData));
    return o;
}

static gboolean prompt_cb(ObPrompt *p, gint result, gpointer options)
{
    Options *o = options;
    if (result)
        run_func(o->data, o);
    return TRUE; /* call the cleanup func */
}

static void prompt_cleanup(ObPrompt *p, gpointer options)
{
    prompt_unref(p);
    free_func(options);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    GError *e = NULL;
    gchar **argv = NULL;
    gchar *cmd;
    Options *o = options;

    if (!o->cmd) return FALSE;

    if (o->prompt) {
        ObPrompt *p;
        Options *ocp;
        ObPromptAnswer answers[] = {
            { _("No"), 0 },
            { _("Yes"), 1 }
        };

        ocp = dup_options(options, data);
        p = prompt_new(o->prompt, _("Execute"), answers, 2, 0, 0,
                       prompt_cb, prompt_cleanup, ocp);
        prompt_show(p, NULL, FALSE);

        return FALSE;
    }

    cmd = g_filename_from_utf8(o->cmd, -1, NULL, NULL, NULL);
    if (!cmd) {
        g_message(_("Failed to convert the path \"%s\" from utf8"), o->cmd);
        return FALSE;
    }

    /* If there is a keyboard grab going on then we need to cancel
       it so the application can grab things */
    if (data->uact != OB_USER_ACTION_MENU_SELECTION)
        event_cancel_all_key_grabs();

    if (!g_shell_parse_argv(cmd, NULL, &argv, &e)) {
        g_message("%s", e->message);
        g_error_free(e);
    }
    else {
        gchar *program = NULL;

        if (o->sn) {
            program = g_path_get_basename(argv[0]);
            /* sets up the environment */
            sn_setup_spawn_environment(program, o->sn_name, o->sn_icon,
                                       o->sn_wmclass,
                                       /* launch it on the current desktop */
                                       screen_desktop);
        }

        if (!g_spawn_async(NULL, argv, NULL,
                           G_SPAWN_SEARCH_PATH | G_SPAWN_DO_NOT_REAP_CHILD,
                           NULL, NULL, NULL, &e))
        {
            g_message("%s", e->message);
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
