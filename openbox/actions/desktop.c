#include "openbox/actions.h"
#include "openbox/screen.h"
#include <glib.h>

typedef struct {
    gboolean last;
    guint desktop;
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

    if ((n = parse_find_node("desktop", node))) {
        gchar *s = parse_string(doc, n);
        if (!g_ascii_strcasecmp(s, "last"))
            o->last = TRUE;
        else
            o->desktop = parse_int(doc, n) - 1;
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
    guint d;

    if (o->last)
        d = screen_last_desktop;
    else
        d = o->desktop;

    if (d < screen_num_desktops)
        screen_set_desktop(d, TRUE);

    return FALSE;
}
