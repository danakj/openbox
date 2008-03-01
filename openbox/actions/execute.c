#include "openbox/actions.h"
#include "openbox/event.h"
#include "openbox/startupnotify.h"
#include "openbox/client.h"
#include "openbox/prompt.h"
#include "openbox/screen.h"
#include "obt/paths.h"
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
} Options;

static gpointer setup_func(xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);
/*
static gboolean i_input_func(guint initial_state,
                             XEvent *e,
                             gpointer options,
                             gboolean *used);
static void     i_cancel_func(gpointer options);
*/

void action_execute_startup(void)
{
    actions_register("Execute", setup_func, free_func, run_func, NULL, NULL);
}

static gpointer setup_func(xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_new0(Options, 1);

    if ((n = obt_parse_find_node(node, "command")) ||
        (n = obt_parse_find_node(node, "execute")))
    {
        gchar *s = obt_parse_node_string(n);
        o->cmd = obt_paths_expand_tilde(s);
        g_free(s);
    }

    if ((n = obt_parse_find_node(node, "prompt")))
        o->prompt = obt_parse_node_string(n);

    if ((n = obt_parse_find_node(node, "startupnotify"))) {
        xmlNodePtr m;
        if ((m = obt_parse_find_node(n->children, "enabled")))
            o->sn = obt_parse_node_bool(m);
        if ((m = obt_parse_find_node(n->children, "name")))
            o->sn_name = obt_parse_node_string(m);
        if ((m = obt_parse_find_node(n->children, "icon")))
            o->sn_icon = obt_parse_node_string(m);
        if ((m = obt_parse_find_node(n->children, "wmclass")))
            o->sn_wmclass = obt_parse_node_string(m);
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
        g_free(o->prompt);
        g_free(o);
    }
}

static Options* dup_options(Options *in)
{
    Options *o = g_new(Options, 1);
    o->cmd = g_strdup(in->cmd);
    o->sn = in->sn;
    o->sn_name = g_strdup(in->sn_name);
    o->sn_icon = g_strdup(in->sn_icon);
    o->sn_wmclass = g_strdup(in->sn_wmclass);
    o->prompt = NULL;
    return o;
}

static gboolean run_func(ObActionsData *data, gpointer options);

static void prompt_cb(ObPrompt *p, gint result, gpointer options)
{
    if (result)
        run_func(NULL, options);

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

        ocp = dup_options(options);
        p = prompt_new(o->prompt, answers, 2, 0, 0, prompt_cb, ocp);
        prompt_show(p, NULL, FALSE);

        return FALSE;
    }

    cmd = g_filename_from_utf8(o->cmd, -1, NULL, NULL, NULL);
    if (!cmd) {
        g_message(_("Failed to convert the path \"%s\" from utf8"), o->cmd);
        return FALSE;
    }

    if (data->client) {
        gchar *c, *before, *expand;

        /* replace occurances of $pid and $window */

        expand = NULL;
        before = cmd;

        while ((c = strchr(before, '$'))) {
            if ((c[1] == 'p' || c[1] == 'P') &&
                (c[2] == 'i' || c[2] == 'I') &&
                (c[3] == 'd' || c[3] == 'D') &&
                !g_ascii_isalnum(c[4]))
            {
                /* found $pid */
                gchar *tmp;

                *c = '\0';
                tmp = expand;
                expand = g_strdup_printf("%s%s%u",
                                         (expand ? expand : ""),
                                         before,
                                         data->client->pid);
                g_free(tmp);

                before = c + 4; /* 4 = strlen("$pid") */
            }

            if ((c[1] == 'w' || c[1] == 'W') &&
                (c[2] == 'i' || c[2] == 'I') &&
                (c[3] == 'n' || c[3] == 'N') &&
                (c[4] == 'd' || c[4] == 'D') &&
                (c[5] == 'o' || c[5] == 'O') &&
                (c[6] == 'w' || c[6] == 'W') &&
                !g_ascii_isalnum(c[7]))
            {
                /* found $window */
                gchar *tmp;

                *c = '\0';
                tmp = expand;
                expand = g_strdup_printf("%s%s%lu",
                                         (expand ? expand : ""),
                                         before,
                                         data->client->window);
                g_free(tmp);

                before = c + 7; /* 4 = strlen("$window") */
            }
        }

        if (expand) {
            gchar *tmp;

            /* add on the end of the string after the last replacement */
            tmp = expand;
            expand = g_strconcat(expand, before, NULL);
            g_free(tmp);

            /* replace the command with the expanded one */
            g_free(cmd);
            cmd = expand;
        }
    }

    /* If there is a keyboard grab going on then we need to cancel
       it so the application can grab things */
    event_cancel_all_key_grabs();

    if (!g_shell_parse_argv(cmd, NULL, &argv, &e)) {
        g_message(_("Failed to execute \"%s\": %s"), o->cmd, e->message);
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
            g_message(_("Failed to execute \"%s\": %s"), o->cmd, e->message);
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
