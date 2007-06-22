#include "openbox/actions.h"
#include "openbox/client.h"
#include "openbox/screen.h"

typedef struct {
    gboolean toggle;
    gboolean on;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_omnipresent_startup()
{
    actions_register("omnipresent",
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
    o->toggle = TRUE;

    if ((n = parse_find_node("omnipresent", node))) {
        gchar *s = parse_string(doc, n);
        if (g_ascii_strcasecmp(s, "toggle")) {
            o->toggle = FALSE;
            o->on = parse_bool(doc, n);
        }
        g_free(s);
    }

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

    if (data->client)
        if (o->toggle || (o->on != (data->client->desktop == DESKTOP_ALL)))
            client_set_desktop(data->client,
                               data->client->desktop == DESKTOP_ALL ?
                               screen_desktop : DESKTOP_ALL, FALSE, TRUE);

    return FALSE;
}
