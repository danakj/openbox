#include "openbox/actions.h"
#include "openbox/client.h"
#include "openbox/screen.h"
#include "openbox/frame.h"
#include <stdlib.h> /* for atoi */

typedef struct {
    gint left;
    gint right;
    gint top;
    gint bottom;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_resizerelative_startup()
{
    actions_register("ResizeRelative",
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

    if ((n = parse_find_node("left", node)))
        o->left = parse_int(doc, n);
    if ((n = parse_find_node("right", node)))
        o->right = parse_int(doc, n);
    if ((n = parse_find_node("top", node)))
        o->top = parse_int(doc, n);
    if ((n = parse_find_node("bottom", node)))
        o->bottom = parse_int(doc, n);

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
        ObClient *c = data->client;
        gint x, y, ow, xoff, nw, oh, yoff, nh, lw, lh;

        x = c->area.x;
        y = c->area.y;
        ow = c->area.width;
        xoff = -o->left * c->size_inc.width;
        nw = ow + o->right * c->size_inc.width
            + o->left * c->size_inc.width;
        oh = c->area.height;
        yoff = -o->top * c->size_inc.height;
        nh = oh + o->bottom * c->size_inc.height
            + o->top * c->size_inc.height;

        client_try_configure(c, &x, &y, &nw, &nh, &lw, &lh, TRUE);
        xoff = xoff == 0 ? 0 :
            (xoff < 0 ? MAX(xoff, ow-nw) : MIN(xoff, ow-nw));
        yoff = yoff == 0 ? 0 :
            (yoff < 0 ? MAX(yoff, oh-nh) : MIN(yoff, oh-nh));

        actions_client_move(data, TRUE);
        client_move_resize(c, x + xoff, y + yoff, nw, nh);
        actions_client_move(data, FALSE);
    }

    return FALSE;
}
