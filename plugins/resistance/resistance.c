#include "kernel/dispatch.h"
#include "kernel/client.h"
#include "kernel/frame.h"
#include "kernel/stacking.h"
#include "kernel/screen.h"
#include "parser/parse.h"
#include "resistance.h"
#include <glib.h>

static int win_resistance;
static int edge_resistance;

static void parse_xml(xmlDocPtr doc, xmlNodePtr node, void *d)
{
    xmlNodePtr n;

    node = node->xmlChildrenNode;
    if ((n = parse_find_node("strength", node)))
        win_resistance = parse_int(doc, n);
    if ((n = parse_find_node("screen_edge_strength", node)))
        edge_resistance = parse_int(doc, n);
}

void plugin_setup_config()
{
    win_resistance = edge_resistance = DEFAULT_RESISTANCE;

    parse_register("resistance", parse_xml, NULL);
}

static void resist_move(ObClient *c, int *x, int *y)
{
    GList *it;
    Rect *area;
    guint i;
    int l, t, r, b; /* requested edges */
    int al, at, ar, ab; /* screen area edges */
    int cl, ct, cr, cb; /* current edges */
    int w, h; /* current size */
    ObClient *snapx = NULL, *snapy = NULL;

    w = c->frame->area.width;
    h = c->frame->area.height;

    l = *x;
    t = *y;
    r = l + w - 1;
    b = t + h - 1;

    cl = c->frame->area.x;
    ct = c->frame->area.y;
    cr = cl + c->frame->area.width - 1;
    cb = ct + c->frame->area.height - 1;
    
    /* snap to other clients */
    if (win_resistance)
        for (it = stacking_list; it != NULL; it = it->next) {
            ObClient *target;
            int tl, tt, tr, tb; /* 1 past the target's edges on each side */

            if (!WINDOW_IS_CLIENT(it->data))
                continue;
            target = it->data;
            /* don't snap to self or non-visibles */
            if (!target->frame->visible || target == c) continue; 

            tl = target->frame->area.x - 1;
            tt = target->frame->area.y - 1;
            tr = tl + target->frame->area.width + 1;
            tb = tt + target->frame->area.height + 1;

            /* snapx and snapy ensure that the window snaps to the top-most
               window edge available, without going all the way from
               bottom-to-top in the stacking list
            */
            if (snapx == NULL) {
                if (ct < tb && cb > tt) {
                    if (cl >= tr && l < tr && l >= tr - win_resistance)
                        *x = tr, snapx = target;
                    else if (cr <= tl && r > tl && r <= tl + win_resistance)
                        *x = tl - w + 1, snapx = target;
                    if (snapx != NULL) {
                        /* try to corner snap to the window */
                        if (ct > tt && t <= tt && t > tt - win_resistance)
                            *y = tt + 1, snapy = target;
                        else if (cb < tb && b >= tb && b < tb + win_resistance)
                            *y = tb - h, snapy = target;
                    }
                }
            }
            if (snapy == NULL) {
                if (cl < tr && cr > tl) {
                    if (ct >= tb && t < tb && t >= tb - win_resistance)
                        *y = tb, snapy = target;
                    else if (cb <= tt && b > tt && b <= tt + win_resistance)
                        *y = tt - h + 1, snapy = target;
                    if (snapy != NULL) {
                        /* try to corner snap to the window */
                        if (cl > tl && l <= tl && l > tl - win_resistance)
                            *x = tl + 1, snapx = target;
                        else if (cr < tr && r >= tr && r < tr + win_resistance)
                            *x = tr - w, snapx = target;
                    }
                }
            }

            if (snapx && snapy) break;
        }

    /* get the screen boundaries */
    if (edge_resistance) {
        for (i = 0; i < screen_num_monitors; ++i) {
            area = screen_area_monitor(c->desktop, i);

            if (!RECT_INTERSECTS_RECT(*area, c->frame->area))
                continue;

            al = area->x;
            at = area->y;
            ar = al + area->width - 1;
            ab = at + area->height - 1;

            /* snap to screen edges */
            if (cl >= al && l < al && l >= al - edge_resistance)
                *x = al;
            else if (cr <= ar && r > ar && r <= ar + edge_resistance)
                *x = ar - w + 1;
            if (ct >= at && t < at && t >= at - edge_resistance)
                *y = at;
            else if (cb <= ab && b > ab && b < ab + edge_resistance)
                *y = ab - h + 1;
        }
    }
}

static void resist_size(ObClient *c, int *w, int *h, ObCorner corn)
{
    GList *it;
    ObClient *target; /* target */
    int l, t, r, b; /* my left, top, right and bottom sides */
    int dlt, drb; /* my destination left/top and right/bottom sides */
    int tl, tt, tr, tb; /* target's left, top, right and bottom bottom sides */
    Rect *area;
    int al, at, ar, ab; /* screen boundaries */
    ObClient *snapx = NULL, *snapy = NULL;

    /* don't snap windows with size increments */
    if (c->size_inc.width > 1 || c->size_inc.height > 1)
        return;

    l = c->frame->area.x;
    r = l + c->frame->area.width - 1;
    t = c->frame->area.y;
    b = t + c->frame->area.height - 1;

    /* get the screen boundaries */
    area = screen_area(c->desktop);
    al = area->x;
    at = area->y;
    ar = al + area->width - 1;
    ab = at + area->height - 1;

    /* snap to other windows */
    if (resist_windows) {
        for (it = stacking_list; it != NULL; it = it->next) {
            if (!WINDOW_IS_CLIENT(it->data))
                continue;
            target = it->data;

            /* don't snap to invisibles or ourself */
            if (!target->frame->visible || target == c) continue;

            tl = target->frame->area.x;
            tr = target->frame->area.x + target->frame->area.width - 1;
            tt = target->frame->area.y;
            tb = target->frame->area.y + target->frame->area.height - 1;

            if (snapx == NULL) {
                /* horizontal snapping */
                if (t < tb && b > tt) {
                    switch (corn) {
                    case OB_CORNER_TOPLEFT:
                    case OB_CORNER_BOTTOMLEFT:
                        dlt = l;
                        drb = r + *w - c->frame->area.width;
                        if (r < tl && drb >= tl && drb < tl + win_resistance)
                            *w = tl - l, snapx = target;
                        break;
                    case OB_CORNER_TOPRIGHT:
                    case OB_CORNER_BOTTOMRIGHT:
                        dlt = l - *w + c->frame->area.width;
                        drb = r;
                        if (l > tr && dlt <= tr && dlt > tr - win_resistance)
                            *w = r - tr, snapx = target;
                        break;
                    }
                }
            }

            if (snapy == NULL) {
                /* vertical snapping */
                if (l < tr && r > tl) {
                    switch (corn) {
                    case OB_CORNER_TOPLEFT:
                    case OB_CORNER_TOPRIGHT:
                        dlt = t;
                        drb = b + *h - c->frame->area.height;
                        if (b < tt && drb >= tt && drb < tt + win_resistance)
                            *h = tt - t, snapy = target;
                        break;
                    case OB_CORNER_BOTTOMLEFT:
                    case OB_CORNER_BOTTOMRIGHT:
                        dlt = t - *h + c->frame->area.height;
                        drb = b;
                        if (t > tb && dlt <= tb && dlt > tb - win_resistance)
                            *h = b - tb, snapy = target;
                        break;
                    }
                }
            }

            /* snapped both ways */
            if (snapx && snapy) break;
        }
    }

    /* snap to screen edges */
    
    /* horizontal snapping */
    switch (corn) {
    case OB_CORNER_TOPLEFT:
    case OB_CORNER_BOTTOMLEFT:
        dlt = l;
        drb = r + *w - c->frame->area.width;
        if (r <= ar && drb > ar && drb <= ar + edge_resistance)
            *w = ar - l + 1;
        break;
    case OB_CORNER_TOPRIGHT:
    case OB_CORNER_BOTTOMRIGHT:
        dlt = l - *w + c->frame->area.width;
        drb = r;
        if (l >= al && dlt < al && dlt >= al - edge_resistance)
            *w = r - al + 1;
        break;
    }

    /* vertical snapping */
    switch (corn) {
    case OB_CORNER_TOPLEFT:
    case OB_CORNER_TOPRIGHT:
        dlt = t;
        drb = b + *h - c->frame->area.height;
        if (b <= ab && drb > ab && drb <= ab + edge_resistance)
            *h = ab - t + 1;
        break;
    case OB_CORNER_BOTTOMLEFT:
    case OB_CORNER_BOTTOMRIGHT:
        dlt = t - *h + c->frame->area.height;
        drb = b;
        if (t >= at && dlt < at && dlt >= at - edge_resistance)
            *h = b - at + 1;
        break;
    }
}

static void event(ObEvent *e, void *foo)
{
    if (e->type == Event_Client_Moving)
        resist_move(e->data.c.client, &e->data.c.num[0], &e->data.c.num[1]);
    else if (e->type == Event_Client_Resizing)
        resist_size(e->data.c.client, &e->data.c.num[0], &e->data.c.num[1],
                    e->data.c.num[2]);
}

void plugin_startup()
{
    dispatch_register(Event_Client_Moving | Event_Client_Resizing,
                      (EventHandler)event, NULL);
}

void plugin_shutdown()
{
    dispatch_register(0, (EventHandler)event, NULL);
}
