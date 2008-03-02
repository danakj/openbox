#include "openbox/actions.h"
#include "openbox/openbox.h"
#include "openbox/prompt.h"
#include "gettext.h"

typedef struct {
    gboolean prompt;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_exit_startup(void)
{
    actions_register("Exit", setup_func, NULL, run_func, NULL, NULL);
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

static void prompt_cb(ObPrompt *p, gint result, gpointer data)
{
    if (result)
        ob_exit(0);
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

        p = prompt_new(_("Are you sure you want to exit Openbox?"),
                       answers, 2, 0, 0, prompt_cb, NULL);
        prompt_show(p, NULL, FALSE);
    }
    else
        ob_exit(0);

    return FALSE;
}
