#include "client.h"
#include "frame.h"
#include "stacking.h"
#include "screen.h"
#include "config.h"
#include "parser/parse.h"

#include <glib.h>

void resist_move_windows(ObClient *c, gint *x, gint *y)
{
    GList *it;
    gint l, t, r, b; /* requested edges */
    gint cl, ct, cr, cb; /* current edges */
    gint w, h; /* current size */
    ObClient *snapx = NULL, *snapy = NULL;

    w = c->frame->area.width;
    h = c->frame->area.height;

    l = *x;
    t = *y;
    r = l + w - 1;
    b = t + h - 1;

    cl = RECT_LEFT(c->frame->area);
    ct = RECT_TOP(c->frame->area);
    cr = RECT_RIGHT(c->frame->area);
    cb = RECT_BOTTOM(c->frame->area);
    
    if (config_resist_win)
        for (it = stacking_list; it != NULL; it = it->next) {
            ObClient *target;
            int tl, tt, tr, tb; /* 1 past the target's edges on each side */

            if (!WINDOW_IS_CLIENT(it->data))
                continue;
            target = it->data;
            /* don't snap to self or non-visibles */
            if (!target->frame->visible || target == c) continue; 

            tl = RECT_LEFT(target->frame->area) - 1;
            tt = RECT_TOP(target->frame->area) - 1;
            tr = RECT_RIGHT(target->frame->area)+ 2;
            tb = RECT_BOTTOM(target->frame->area) + 2;

            /* snapx and snapy ensure that the window snaps to the top-most
               window edge available, without going all the way from
               bottom-to-top in the stacking list
            */
            if (snapx == NULL) {
                if (ct < tb && cb > tt) {
                    if (cl >= tr && l < tr && l >= tr - config_resist_win)
                        *x = tr, snapx = target;
                    else if (cr <= tl && r > tl &&
                             r <= tl + config_resist_win)
                        *x = tl - w + 1, snapx = target;
                    if (snapx != NULL) {
                        /* try to corner snap to the window */
                        if (ct > tt && t <= tt &&
                            t > tt - config_resist_win)
                            *y = tt + 1, snapy = target;
                        else if (cb < tb && b >= tb &&
                                 b < tb + config_resist_win)
                            *y = tb - h, snapy = target;
                    }
                }
            }
            if (snapy == NULL) {
                if (cl < tr && cr > tl) {
                    if (ct >= tb && t < tb && t >= tb - config_resist_win)
                        *y = tb, snapy = target;
                    else if (cb <= tt && b > tt &&
                             b <= tt + config_resist_win)
                        *y = tt - h + 1, snapy = target;
                    if (snapy != NULL) {
                        /* try to corner snap to the window */
                        if (cl > tl && l <= tl &&
                            l > tl - config_resist_win)
                            *x = tl + 1, snapx = target;
                        else if (cr < tr && r >= tr &&
                                 r < tr + config_resist_win)
                            *x = tr - w, snapx = target;
                    }
                }
            }

            if (snapx && snapy) break;
        }
}

void resist_move_monitors(ObClient *c, gint *x, gint *y)
{
    Rect *area;
    guint i;
    gint l, t, r, b; /* requested edges */
    gint al, at, ar, ab; /* screen area edges */
    gint cl, ct, cr, cb; /* current edges */
    gint w, h; /* current size */

    w = c->frame->area.width;
    h = c->frame->area.height;

    l = *x;
    t = *y;
    r = l + w - 1;
    b = t + h - 1;

    cl = RECT_LEFT(c->frame->area);
    ct = RECT_TOP(c->frame->area);
    cr = RECT_RIGHT(c->frame->area);
    cb = RECT_BOTTOM(c->frame->area);
    
    if (config_resist_edge) {
        for (i = 0; i < screen_num_monitors; ++i) {
            area = screen_area_monitor(c->desktop, i);

            if (!RECT_INTERSECTS_RECT(*area, c->frame->area))
                continue;

            al = RECT_LEFT(*area);
            at = RECT_TOP(*area);
            ar = RECT_RIGHT(*area);
            ab = RECT_BOTTOM(*area);

            if (cl >= al && l < al && l >= al - config_resist_edge)
                *x = al;
            else if (cr <= ar && r > ar && r <= ar + config_resist_edge)
                *x = ar - w + 1;
            if (ct >= at && t < at && t >= at - config_resist_edge)
                *y = at;
            else if (cb <= ab && b > ab && b < ab + config_resist_edge)
                *y = ab - h + 1;
        }
    }
}

void resist_size_windows(ObClient *c, gint *w, gint *h, ObCorner corn)
{
    GList *it;
    ObClient *target; /* target */
    gint l, t, r, b; /* my left, top, right and bottom sides */
    gint dlt, drb; /* my destination left/top and right/bottom sides */
    gint tl, tt, tr, tb; /* target's left, top, right and bottom bottom sides*/
    gint incw, inch;
    ObClient *snapx = NULL, *snapy = NULL;

    incw = c->size_inc.width;
    inch = c->size_inc.height;

    l = RECT_LEFT(c->frame->area);
    r = RECT_RIGHT(c->frame->area);
    t = RECT_TOP(c->frame->area);
    b = RECT_BOTTOM(c->frame->area);

    if (config_resist_win) {
        for (it = stacking_list; it != NULL; it = it->next) {
            if (!WINDOW_IS_CLIENT(it->data))
                continue;
            target = it->data;

            /* don't snap to invisibles or ourself */
            if (!target->frame->visible || target == c) continue;

            tl = RECT_LEFT(target->frame->area);
            tr = RECT_RIGHT(target->frame->area);
            tt = RECT_TOP(target->frame->area);
            tb = RECT_BOTTOM(target->frame->area);

            if (snapx == NULL) {
                /* horizontal snapping */
                if (t < tb && b > tt) {
                    switch (corn) {
                    case OB_CORNER_TOPLEFT:
                    case OB_CORNER_BOTTOMLEFT:
                        dlt = l;
                        drb = r + *w - c->frame->area.width;
                        if (r < tl && drb >= tl &&
                            drb < tl + config_resist_win)
                            *w = tl - l, snapx = target;
                        break;
                    case OB_CORNER_TOPRIGHT:
                    case OB_CORNER_BOTTOMRIGHT:
                        dlt = l - *w + c->frame->area.width;
                        drb = r;
                        if (l > tr && dlt <= tr &&
                            dlt > tr - config_resist_win)
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
                        if (b < tt && drb >= tt &&
                            drb < tt + config_resist_win)
                            *h = tt - t, snapy = target;
                        break;
                    case OB_CORNER_BOTTOMLEFT:
                    case OB_CORNER_BOTTOMRIGHT:
                        dlt = t - *h + c->frame->area.height;
                        drb = b;
                        if (t > tb && dlt <= tb &&
                            dlt > tb - config_resist_win)
                            *h = b - tb, snapy = target;
                        break;
                    }
                }
            }

            /* snapped both ways */
            if (snapx && snapy) break;
        }
    }
}

void resist_size_monitors(ObClient *c, gint *w, gint *h, ObCorner corn)
{
    gint l, t, r, b; /* my left, top, right and bottom sides */
    gint dlt, drb; /* my destination left/top and right/bottom sides */
    Rect *area;
    gint al, at, ar, ab; /* screen boundaries */
    gint incw, inch;

    l = RECT_LEFT(c->frame->area);
    r = RECT_RIGHT(c->frame->area);
    t = RECT_TOP(c->frame->area);
    b = RECT_BOTTOM(c->frame->area);

    incw = c->size_inc.width;
    inch = c->size_inc.height;

    /* get the screen boundaries */
    area = screen_area(c->desktop);
    al = RECT_LEFT(*area);
    at = RECT_TOP(*area);
    ar = RECT_RIGHT(*area);
    ab = RECT_BOTTOM(*area);

    if (config_resist_edge) {
        /* horizontal snapping */
        switch (corn) {
        case OB_CORNER_TOPLEFT:
        case OB_CORNER_BOTTOMLEFT:
            dlt = l;
            drb = r + *w - c->frame->area.width;
            if (r <= ar && drb > ar && drb <= ar + config_resist_edge)
                *w = ar - l + 1;
            break;
        case OB_CORNER_TOPRIGHT:
        case OB_CORNER_BOTTOMRIGHT:
            dlt = l - *w + c->frame->area.width;
            drb = r;
            if (l >= al && dlt < al && dlt >= al - config_resist_edge)
                *w = r - al + 1;
            break;
        }

        /* vertical snapping */
        switch (corn) {
        case OB_CORNER_TOPLEFT:
        case OB_CORNER_TOPRIGHT:
            dlt = t;
            drb = b + *h - c->frame->area.height;
            if (b <= ab && drb > ab && drb <= ab + config_resist_edge)
                *h = ab - t + 1;
            break;
        case OB_CORNER_BOTTOMLEFT:
        case OB_CORNER_BOTTOMRIGHT:
            dlt = t - *h + c->frame->area.height;
            drb = b;
            if (t >= at && dlt < at && dlt >= at - config_resist_edge)
                *h = b - at + 1;
            break;
        }
    }
}
