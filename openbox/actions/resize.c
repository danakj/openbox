#include "openbox/actions.h"
#include "openbox/prop.h"
#include "openbox/moveresize.h"
#include "openbox/client.h"
#include "openbox/frame.h"

typedef struct {
    gboolean corner_specified;
    guint32 corner;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

static guint32 pick_corner(gint x, gint y, gint cx, gint cy, gint cw, gint ch,
                           gboolean shaded);

void action_resize_startup(void)
{
    actions_register("Resize",
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

    if ((n = parse_find_node("edge", node))) {
        gchar *s = parse_string(doc, n);

        o->corner_specified = TRUE;
        if (!g_ascii_strcasecmp(s, "top"))
            o->corner = prop_atoms.net_wm_moveresize_size_top;
        else if (!g_ascii_strcasecmp(s, "bottom"))
            o->corner = prop_atoms.net_wm_moveresize_size_bottom;
        else if (!g_ascii_strcasecmp(s, "left"))
            o->corner = prop_atoms.net_wm_moveresize_size_left;
        else if (!g_ascii_strcasecmp(s, "right"))
            o->corner = prop_atoms.net_wm_moveresize_size_right;
        else if (!g_ascii_strcasecmp(s, "topleft"))
            o->corner = prop_atoms.net_wm_moveresize_size_topleft;
        else if (!g_ascii_strcasecmp(s, "topright"))
            o->corner = prop_atoms.net_wm_moveresize_size_topright;
        else if (!g_ascii_strcasecmp(s, "bottomleft"))
            o->corner = prop_atoms.net_wm_moveresize_size_bottomleft;
        else if (!g_ascii_strcasecmp(s, "bottomright"))
            o->corner = prop_atoms.net_wm_moveresize_size_bottomright;
        else
            o->corner_specified = FALSE;

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

    if (data->client) {
        ObClient *c = data->client;
        guint32 corner;

        if (!data->button)
            corner = prop_atoms.net_wm_moveresize_size_keyboard;
        else if (o->corner_specified)
            corner = o->corner; /* it was specified in the binding */
        else
            corner = pick_corner(data->x, data->y,
                                 c->frame->area.x, c->frame->area.y,
                                 /* use the client size because the frame
                                    can be differently sized (shaded
                                    windows) and we want this based on the
                                    clients size */
                                 c->area.width + c->frame->size.left +
                                 c->frame->size.right,
                                 c->area.height + c->frame->size.top +
                                 c->frame->size.bottom, c->shaded);

        moveresize_start(c, data->x, data->y, data->button, corner);
    }

    return FALSE;
}

static guint32 pick_corner(gint x, gint y, gint cx, gint cy, gint cw, gint ch,
                           gboolean shaded)
{
    /* let's make x and y client relative instead of screen relative */
    x = x - cx;
    y = ch - (y - cy); /* y is inverted, 0 is at the bottom of the window */

#define X x*ch/cw
#define A -4*X + 7*ch/3
#define B  4*X -15*ch/9
#define C -X/4 + 2*ch/3
#define D  X/4 + 5*ch/12
#define E  X/4 +   ch/3
#define F -X/4 + 7*ch/12
#define G  4*X - 4*ch/3
#define H -4*X + 8*ch/3
#define a (y > 5*ch/9)
#define b (x < 4*cw/9)
#define c (x > 5*cw/9)
#define d (y < 4*ch/9)

    /*
      Each of these defines (except X which is just there for fun), represents
      the equation of a line. The lines they represent are shown in the diagram
      below. Checking y against these lines, we are able to choose a region
      of the window as shown.

      +---------------------A-------|-------|-------B---------------------+
      |                     |A                     B|                     |
      |                     |A      |       |      B|                     |
      |                     | A                   B |                     |
      |                     | A     |       |     B |                     |
      |                     |  A                 B  |                     |
      |                     |  A    |       |    B  |                     |
      |        northwest    |   A     north     B   |   northeast         |
      |                     |   A   |       |   B   |                     |
      |                     |    A             B    |                     |
      C---------------------+----A--+-------+--B----+---------------------D
      |CCCCCCC              |     A           B     |              DDDDDDD|
      |       CCCCCCCC      |     A |       | B     |      DDDDDDDD       |
      |               CCCCCCC      A         B      DDDDDDD               |
      - - - - - - - - - - - +CCCCCCC+aaaaaaa+DDDDDDD+ - - - - - - - - - - - -
      |                     |       b       c       |                     | sh
      |             west    |       b  move c       |   east              | ad
      |                     |       b       c       |                     | ed
      - - - - - - - - - - - +EEEEEEE+ddddddd+FFFFFFF+- - - - - - - - - - -  -
      |               EEEEEEE      G         H      FFFFFFF               |
      |       EEEEEEEE      |     G |       | H     |      FFFFFFFF       |
      |EEEEEEE              |     G           H     |              FFFFFFF|
      E---------------------+----G--+-------+--H----+---------------------F
      |                     |    G             H    |                     |
      |                     |   G   |       |   H   |                     |
      |        southwest    |   G     south     H   |   southeast         |
      |                     |  G    |       |    H  |                     |
      |                     |  G                 H  |                     |
      |                     | G     |       |     H |                     |
      |                     | G                   H |                     |
      |                     |G      |       |      H|                     |
      |                     |G                     H|                     |
      +---------------------G-------|-------|-------H---------------------+
    */

    if (shaded) {
        /* for shaded windows, you can only resize west/east and move */
        if (b)
            return prop_atoms.net_wm_moveresize_size_left;
        if (c)
            return prop_atoms.net_wm_moveresize_size_right;
        return prop_atoms.net_wm_moveresize_move;
    }

    if (y < A && y >= C)
        return prop_atoms.net_wm_moveresize_size_topleft;
    else if (y >= A && y >= B && a)
        return prop_atoms.net_wm_moveresize_size_top;
    else if (y < B && y >= D)
        return prop_atoms.net_wm_moveresize_size_topright;
    else if (y < C && y >= E && b)
        return prop_atoms.net_wm_moveresize_size_left;
    else if (y < D && y >= F && c)
        return prop_atoms.net_wm_moveresize_size_right;
    else if (y < E && y >= G)
        return prop_atoms.net_wm_moveresize_size_bottomleft;
    else if (y < G && y < H && d)
        return prop_atoms.net_wm_moveresize_size_bottom;
    else if (y >= H && y < F)
        return prop_atoms.net_wm_moveresize_size_bottomright;
    else
        return prop_atoms.net_wm_moveresize_move;

#undef X
#undef A
#undef B
#undef C
#undef D
#undef E
#undef F
#undef G
#undef H
#undef a
#undef b
#undef c
#undef d
}
