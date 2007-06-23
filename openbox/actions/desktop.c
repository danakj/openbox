#include "openbox/actions.h"
#include "openbox/screen.h"
#include "openbox/client.h"
#include <glib.h>

typedef struct {
    gboolean last;
    guint desktop;
    gboolean send;
    gboolean follow;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_desktop_startup()
{
    actions_register("Desktop",
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
    o->follow = TRUE;

    if ((n = parse_find_node("desktop", node))) {
        gchar *s = parse_string(doc, n);
        if (!g_ascii_strcasecmp(s, "last"))
            o->last = TRUE;
        else
            o->desktop = parse_int(doc, n) - 1;
        g_free(s);
    }
    if ((n = parse_find_node("send", node)))
        o->send = parse_bool(doc, n);
    if ((n = parse_find_node("follow", node)))
        o->follow = parse_bool(doc, n);

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
    guint d;

    if (o->last)
        d = screen_last_desktop;
    else
        d = o->desktop;

    if (d < screen_num_desktops && d != screen_desktop) {
        gboolean go = !o->send;
        if (o->send) {
            if (data->client && client_normal(data->client)) {
                client_set_desktop(data->client, d, o->follow, FALSE);
                go = TRUE;
            }
        }
        if (go)
            screen_set_desktop(d, TRUE);
    }
    return FALSE;
}
