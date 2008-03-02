#include "openbox/actions.h"
#include "openbox/prompt.h"
#include "openbox/session.h"
#include "gettext.h"

typedef struct {
    gboolean prompt;
    gboolean silent;
} Options;

static gpointer setup_func(xmlNodePtr node);
static gboolean logout_func(ObActionsData *data, gpointer options);

void action_session_startup(void)
{
    actions_register("SessionLogout", setup_func, NULL, logout_func,
                     NULL, NULL);
}

static gpointer setup_func(xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_new0(Options, 1);
    o->prompt = TRUE;

    if ((n = obt_parse_find_node(node, "prompt")))
        o->prompt = obt_parse_node_bool(n);

    return o;
}

static void prompt_cb(ObPrompt *p, gint result, gpointer data)
{
    Options *o = data;
    if (result) {
#ifndef USE_SM
        session_request_logout(o->silent);
#else
        g_message(_("The SessionLogout actions is not available since Openbox was built without session management support"));
#endif
    }
    g_free(o);
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
            { _("Log out"), 1 }
        };

        o2 = g_memdup(o, sizeof(Options));
        p = prompt_new(_("Are you sure you want to log out?"),
                       answers, 2, 0, 0, prompt_cb, o2);
        prompt_show(p, NULL, FALSE);
    }
    else
        prompt_cb(NULL, 1, NULL);

    return FALSE;
}
