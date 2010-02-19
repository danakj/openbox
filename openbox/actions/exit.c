#include "openbox/actions.h"
#include "openbox/openbox.h"
#include "openbox/prompt.h"
#include "openbox/session.h"
#include "gettext.h"

typedef struct {
    gboolean prompt;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static gboolean run_func(ObActionsData *data, gpointer options);
static void free_func(gpointer options);

void action_exit_startup(void)
{
    actions_register("Exit", setup_func, free_func, run_func, NULL, NULL);
    actions_register("SessionLogout", setup_func, free_func, run_func, NULL, NULL);
}

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_new0(Options, 1);
    o->prompt = TRUE;

    if ((n = parse_find_node("prompt", node)))
        o->prompt = parse_bool(doc, n);

    return o;
}

static void free_func(gpointer options)
{
    if (options)
        g_free(options);
}

static void do_exit(void)
{
    if (session_connected())
        session_request_logout(FALSE);
    else
        ob_exit(0);
}

static gboolean prompt_cb(ObPrompt *p, gint result, gpointer data)
{
    if (result)
        do_exit();
    return TRUE; /* call the cleanup func */
}

static void prompt_cleanup(ObPrompt *p, gpointer data)
{
    prompt_unref(p);
}

/* Always return FALSE because its not interactive */
static gboolean run_func(ObActionsData *data, gpointer options)
{
    Options *o = options;

    if (o->prompt) {
        ObPrompt *p;
        ObPromptAnswer answers[] = {
            { _("Cancel"), 0 },
            { _("Exit"), 1 }
        };

        if (session_connected())
            p = prompt_new(_("Are you sure you want to log out?"),
                           _("Log Out"),
                           answers, 2, 0, 0, prompt_cb, prompt_cleanup, NULL);
        else
            p = prompt_new(_("Are you sure you want to exit Openbox?"),
                           _("Exit Openbox"),
                           answers, 2, 0, 0, prompt_cb, prompt_cleanup, NULL);

        prompt_show(p, NULL, FALSE);
    }
    else
        do_exit();

    return FALSE;
}
