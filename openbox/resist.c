/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   resist.c for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
   Copyright (c) 2003-2007   Dana Jansens

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   See the COPYING file for a copy of the GNU General Public License.
*/

#include "client.h"
#include "frame.h"
#include "stacking.h"
#include "screen.h"
#include "config.h"
#include "parser/parse.h"

#include <glib.h>

void resist_move_windows(ObClient *c, gint resist, gint *x, gint *y)
{
    GList *it;
    gint l, t, r, b; /* requested edges */
    gint cl, ct, cr, cb; /* current edges */
    gint w, h; /* current size */
    ObClient *snapx = NULL, *snapy = NULL;

    if (!resist) return;

    frame_client_gravity(c->frame, x, y, c->area.width, c->area.height);

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
    
    for (it = stacking_list; it; it = g_list_next(it)) {
        ObClient *target;
        gint tl, tt, tr, tb; /* 1 past the target's edges on each side */

        if (!WINDOW_IS_CLIENT(it->data))
            continue;
        target = it->data;

        /* don't snap to self or non-visibles */
        if (!target->frame->visible || target == c) continue; 
        /* don't snap to windows set to below and skip_taskbar (desklets) */
        if (target->below && target->skip_taskbar) continue;

        tl = RECT_LEFT(target->frame->area) - 1;
        tt = RECT_TOP(target->frame->area) - 1;
        tr = RECT_RIGHT(target->frame->area) + 1;
        tb = RECT_BOTTOM(target->frame->area) + 1;

        /* snapx and snapy ensure that the window snaps to the top-most
           window edge available, without going all the way from
           bottom-to-top in the stacking list
        */
        if (snapx == NULL) {
            if (ct < tb && cb > tt) {
                if (cl >= tr && l < tr && l >= tr - resist)
                    *x = tr, snapx = target;
                else if (cr <= tl && r > tl &&
                         r <= tl + resist)
                    *x = tl - w + 1, snapx = target;
                if (snapx != NULL) {
                    /* try to corner snap to the window */
                    if (ct > tt && t <= tt &&
                        t > tt - resist)
                        *y = tt + 1, snapy = target;
                    else if (cb < tb && b >= tb &&
                             b < tb + resist)
                        *y = tb - h, snapy = target;
                }
            }
        }
        if (snapy == NULL) {
            if (cl < tr && cr > tl) {
                if (ct >= tb && t < tb && t >= tb - resist)
                    *y = tb, snapy = target;
                else if (cb <= tt && b > tt &&
                         b <= tt + resist)
                    *y = tt - h + 1, snapy = target;
                if (snapy != NULL) {
                    /* try to corner snap to the window */
                    if (cl > tl && l <= tl &&
                        l > tl - resist)
                        *x = tl + 1, snapx = target;
                    else if (cr < tr && r >= tr &&
                             r < tr + resist)
                        *x = tr - w, snapx = target;
                }
            }
        }

        if (snapx && snapy) break;
    }

    frame_frame_gravity(c->frame, x, y, c->area.width, c->area.height);
}

void resist_move_monitors(ObClient *c, gint resist, gint *x, gint *y)
{
    Rect *area, *parea;
    guint i;
    gint l, t, r, b; /* requested edges */
    gint al, at, ar, ab; /* screen area edges */
    gint pl, pt, pr, pb; /* physical screen area edges */
    gint cl, ct, cr, cb; /* current edges */
    gint w, h; /* current size */
    Rect desired_area;

    if (!resist) return;

    frame_client_gravity(c->frame, x, y, c->area.width, c->area.height);

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

    RECT_SET(desired_area, *x, *y, c->area.width, c->area.height);
    
    for (i = 0; i < screen_num_monitors; ++i) {
        parea = screen_physical_area_monitor(i);

        if (!RECT_INTERSECTS_RECT(*parea, c->frame->area)) {
            g_free(parea);
            continue;
        }

        area = screen_area(c->desktop, SCREEN_AREA_ALL_MONITORS,
                           &desired_area);

        al = RECT_LEFT(*area);
        at = RECT_TOP(*area);
        ar = RECT_RIGHT(*area);
        ab = RECT_BOTTOM(*area);
        pl = RECT_LEFT(*parea);
        pt = RECT_TOP(*parea);
        pr = RECT_RIGHT(*parea);
        pb = RECT_BOTTOM(*parea);

        if (cl >= al && l < al && l >= al - resist)
            *x = al;
        else if (cr <= ar && r > ar && r <= ar + resist)
            *x = ar - w + 1;
        else if (cl >= pl && l < pl && l >= pl - resist)
            *x = pl;
        else if (cr <= pr && r > pr && r <= pr + resist)
            *x = pr - w + 1;

        if (ct >= at && t < at && t >= at - resist)
            *y = at;
        else if (cb <= ab && b > ab && b < ab + resist)
            *y = ab - h + 1;
        else if (ct >= pt && t < pt && t >= pt - resist)
            *y = pt;
        else if (cb <= pb && b > pb && b < pb + resist)
            *y = pb - h + 1;

        g_free(area);
        g_free(parea);
    }

    frame_frame_gravity(c->frame, x, y, c->area.width, c->area.height);
}

void resist_size_windows(ObClient *c, gint resist, gint *w, gint *h,
                         ObCorner corn)
{
    GList *it;
    ObClient *target; /* target */
    gint l, t, r, b; /* my left, top, right and bottom sides */
    gint dlt, drb; /* my destination left/top and right/bottom sides */
    gint tl, tt, tr, tb; /* target's left, top, right and bottom bottom sides*/
    gint incw, inch;
    ObClient *snapx = NULL, *snapy = NULL;

    if (!resist) return;

    incw = c->size_inc.width;
    inch = c->size_inc.height;

    l = RECT_LEFT(c->frame->area);
    r = RECT_RIGHT(c->frame->area);
    t = RECT_TOP(c->frame->area);
    b = RECT_BOTTOM(c->frame->area);

    for (it = stacking_list; it; it = g_list_next(it)) {
        if (!WINDOW_IS_CLIENT(it->data))
            continue;
        target = it->data;

        /* don't snap to invisibles or ourself */
        if (!target->frame->visible || target == c) continue; 
        /* don't snap to windows set to below and skip_taskbar (desklets) */
        if (target->below && target->skip_taskbar) continue;

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
                        drb < tl + resist)
                        *w = tl - l, snapx = target;
                    break;
                case OB_CORNER_TOPRIGHT:
                case OB_CORNER_BOTTOMRIGHT:
                    dlt = l - *w + c->frame->area.width;
                    drb = r;
                    if (l > tr && dlt <= tr &&
                        dlt > tr - resist)
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
                        drb < tt + resist)
                        *h = tt - t, snapy = target;
                    break;
                case OB_CORNER_BOTTOMLEFT:
                case OB_CORNER_BOTTOMRIGHT:
                    dlt = t - *h + c->frame->area.height;
                    drb = b;
                    if (t > tb && dlt <= tb &&
                        dlt > tb - resist)
                        *h = b - tb, snapy = target;
                    break;
                }
            }
        }

        /* snapped both ways */
        if (snapx && snapy) break;
    }
}

void resist_size_monitors(ObClient *c, gint resist, gint *w, gint *h,
                          ObCorner corn)
{
    gint l, t, r, b; /* my left, top, right and bottom sides */
    gint dlt, drb; /* my destination left/top and right/bottom sides */
    Rect *area, *parea;
    gint al, at, ar, ab; /* screen boundaries */ 
    gint pl, pt, pr, pb; /* physical screen boundaries */
    gint incw, inch;
    guint i;
    Rect desired_area;

    if (!resist) return;

    l = RECT_LEFT(c->frame->area);
    r = RECT_RIGHT(c->frame->area);
    t = RECT_TOP(c->frame->area);
    b = RECT_BOTTOM(c->frame->area);

    incw = c->size_inc.width;
    inch = c->size_inc.height;

    RECT_SET(desired_area, c->area.x, c->area.y, *w, *h);

    for (i = 0; i < screen_num_monitors; ++i) {
        parea = screen_physical_area_monitor(i);

        if (!RECT_INTERSECTS_RECT(*parea, c->frame->area)) {
            g_free(parea);
            continue;
        }

        area = screen_area(c->desktop, SCREEN_AREA_ALL_MONITORS,
                           &desired_area);

        /* get the screen boundaries */
        al = RECT_LEFT(*area);
        at = RECT_TOP(*area);
        ar = RECT_RIGHT(*area);
        ab = RECT_BOTTOM(*area);
        pl = RECT_LEFT(*parea);
        pt = RECT_TOP(*parea);
        pr = RECT_RIGHT(*parea);
        pb = RECT_BOTTOM(*parea);

        /* horizontal snapping */
        switch (corn) {
        case OB_CORNER_TOPLEFT:
        case OB_CORNER_BOTTOMLEFT:
            dlt = l;
            drb = r + *w - c->frame->area.width;
            if (r <= ar && drb > ar && drb <= ar + resist)
                *w = ar - l + 1;
            else if (r <= pr && drb > pr && drb <= pr + resist)
                *w = pr - l + 1;
            break;
        case OB_CORNER_TOPRIGHT:
        case OB_CORNER_BOTTOMRIGHT:
            dlt = l - *w + c->frame->area.width;
            drb = r;
            if (l >= al && dlt < al && dlt >= al - resist)
                *w = r - al + 1;
            else if (l >= pl && dlt < pl && dlt >= pl - resist)
                *w = r - pl + 1;
            break;
        }

        /* vertical snapping */
        switch (corn) {
        case OB_CORNER_TOPLEFT:
        case OB_CORNER_TOPRIGHT:
            dlt = t;
            drb = b + *h - c->frame->area.height;
            if (b <= ab && drb > ab && drb <= ab + resist)
                *h = ab - t + 1;
            else if (b <= pb && drb > pb && drb <= pb + resist)
                *h = pb - t + 1;
            break;
        case OB_CORNER_BOTTOMLEFT:
        case OB_CORNER_BOTTOMRIGHT:
            dlt = t - *h + c->frame->area.height;
            drb = b;
            if (t >= at && dlt < at && dlt >= at - resist)
                *h = b - at + 1;
            else if (t >= pt && dlt < pt && dlt >= pt - resist)
                *h = b - pt + 1;
            break;
        }

        g_free(area);
        g_free(parea);
    }
}
