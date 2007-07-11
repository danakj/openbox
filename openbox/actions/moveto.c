#include "openbox/actions.h"
#include "openbox/client.h"
#include "openbox/screen.h"
#include "openbox/frame.h"
#include <stdlib.h> /* for atoi */

typedef struct {
    gboolean xcenter;
    gboolean ycenter;
    gboolean xopposite;
    gboolean yopposite;
    gint x;
    gint y;
    gint monitor;
} Options;

static gpointer setup_func(ObParseInst *i, xmlDocPtr doc, xmlNodePtr node);
static void     free_func(gpointer options);
static gboolean run_func(ObActionsData *data, gpointer options);

void action_moveto_startup()
{
    actions_register("MoveTo",
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
    o->x = G_MININT;
    o->y = G_MININT;
    o->monitor = -1;

    if ((n = parse_find_node("x", node))) {
        gchar *s = parse_string(doc, n);
        if (!g_ascii_strcasecmp(s, "center"))
            o->xcenter = TRUE;
        else {
            if (s[0] == '-')
                o->xopposite = TRUE;
            if (s[0] == '-' || s[0] == '+')
                o->x = atoi(s+1);
            else
                o->x = atoi(s);
        }
        g_free(s);
    }

    if ((n = parse_find_node("y", node))) {
        gchar *s = parse_string(doc, n);
        if (!g_ascii_strcasecmp(s, "center"))
            o->ycenter = TRUE;
        else {
            if (s[0] == '-')
                o->yopposite = TRUE;
            if (s[0] == '-' || s[0] == '+')
                o->y = atoi(s+1);
            else
                o->y = atoi(s);
        }
        g_free(s);
    }

    if ((n = parse_find_node("monitor", node)))
        o->monitor = parse_int(doc, n) - 1;

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
        Rect *area, *carea;
        ObClient *c;
        gint mon, cmon;
        gint x, y, lw, lh, w, h;

        c = data->client;
        mon = o->monitor;
        cmon = client_monitor(c);
        if (mon < 0) mon = cmon;
        area = screen_area(c->desktop, mon, NULL);
        carea = screen_area(c->desktop, cmon, NULL);

        x = o->x;
        if (o->xcenter) x = (area->width - c->frame->area.width) / 2;
        else if (x == G_MININT) x = c->frame->area.x - carea->x;
        else if (o->xopposite) x = area->width - c->frame->area.width;
        x += area->x;

        y = o->y;
        if (o->ycenter) y = (area->height - c->frame->area.height) / 2;
        else if (y == G_MININT) y = c->frame->area.y - carea->y;
        else if (o->yopposite) y = area->height - c->frame->area.height;
        y += area->y;

        w = c->area.width;
        h = c->area.height;

        frame_frame_gravity(c->frame, &x, &y); /* get the client coords */
        client_try_configure(c, &x, &y, &w, &h, &lw, &lh, TRUE);
        /* force it on screen if its moving to another monitor */
        client_find_onscreen(c, &x, &y, w, h, mon != cmon);

        actions_client_move(data, TRUE);
        client_configure(c, x, y, w, h, TRUE, TRUE, FALSE);
        actions_client_move(data, FALSE);

        g_free(area);
        g_free(carea);
    }

    return FALSE;
}
