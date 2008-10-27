#include "openbox/actions.h"
#include "openbox/prompt.h"
#include "openbox/session.h"
#include "gettext.h"

typedef struct {
    gboolean prompt;
    gboolean silent;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static gboolean logout_func(ObActionsData *data, gpointer options);

void action_session_startup(void)
{
    actions_register("SessionLogout", setup_func, NULL, logout_func,
                     NULL, NULL);
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

static gboolean prompt_cb(ObPrompt *p, gint result, gpointer data)
{
    Options *o = data;
    if (result) {
#ifdef USE_SM
        session_request_logout(o->silent);
#else
        /* TRANSLATORS: Don't translate the word "SessionLogout" as it's the
           name of the action you write in rc.xml */
        g_message(_("The SessionLogout action is not available since Openbox was built without session management support"));
#endif
    }
    return TRUE; /* call cleanup func */
}

static void prompt_cleanup(ObPrompt *p, gpointer data)
{
    g_free(data);
    prompt_unref(p);
}

/* Always return FALSE because its not interactive */
static gboolean logout_func(ObActionsData *data, gpointer options)
{
    Options *o = options;

    if (o->prompt) {
        Options *o2;
        ObPrompt *p;
        ObPromptAnswer answers[] = {
            { _("Cancel"), 0 },
            { _("Log Out"), 1 }
        };

        o2 = g_memdup(o, sizeof(Options));
        p = prompt_new(_("Are you sure you want to log out?"),
                       _("Log Out"),
                       answers, 2, 0, 0, prompt_cb, prompt_cleanup, o2);
        prompt_show(p, NULL, FALSE);
    }
    else
        prompt_cb(NULL, 1, NULL);

    return FALSE;
}
