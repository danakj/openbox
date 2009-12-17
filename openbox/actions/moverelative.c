#include "openbox/actions.h"
#include "openbox/client.h"
#include "openbox/screen.h"
#include "openbox/frame.h"
#include <stdlib.h> /* for atoi */

typedef struct {
    gint x;
    gint y;
} Options;

static gpointer setup_func(xmlNodePtr node);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_moverelative_startup(void)
{
    actions_register("MoveRelative", setup_func, g_free, run_func);
}

static gpointer setup_func(xmlNodePtr node)
{
    xmlNodePtr n;
    Options *o;

    o = g_new0(Options, 1);

    if ((n = obt_parse_find_node(node, "x")))
        o->x = obt_parse_node_int(n);
    if ((n = obt_parse_find_node(node, "y")))
        o->y = obt_parse_node_int(n);

    return o;
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
