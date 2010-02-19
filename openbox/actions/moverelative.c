#include "openbox/actions.h"
#include "openbox/client.h"
#include "openbox/screen.h"
#include "openbox/frame.h"
#include <stdlib.h> /* for atoi */

typedef struct {
    gint x;
    gint y;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_moverelative_startup(void)
{
    actions_register("MoveRelative",
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

    if ((n = parse_find_node("x", node)))
        o->x = parse_int(doc, n);
    if ((n = parse_find_node("y", node)))
        o->y = parse_int(doc, n);

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

    if (data->client) {
        ObClient *c;
        gint x, y, lw, lh, w, h;

        c = data->client;
        x = c->area.x + o->x;
        y = c->area.y + o->y;
        w = c->area.width;
        h = c->area.height;
        client_try_configure(c, &x, &y, &w, &h, &lw, &lh, TRUE);
        client_find_onscreen(c, &x, &y, w, h, FALSE);

        actions_client_move(data, TRUE);
        client_configure(c, x, y, w, h, TRUE, TRUE, FALSE);
        actions_client_move(data, FALSE);
    }

    return FALSE;
}
