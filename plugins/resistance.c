#include "../kernel/dispatch.h"
#include "../kernel/client.h"
#include "../kernel/frame.h"
#include "../kernel/stacking.h"
#include "../kernel/screen.h"
#include "../kernel/config.h"
#include <glib.h>

#define DEFAULT_RESISTANCE 10

void plugin_setup_config()
{
    ConfigValue val;

    config_def_set(config_def_new("resistance", Config_Integer,
                                  "Edge Resistance",
                                  "The amount of resistance to provide when "
                                  "moving windows past edges."
                                  "positioned."));
    config_def_set(config_def_new("resistance.windows", Config_Bool,
                                  "Edge Resistance On Windows",
                                  "Whether to provide edge resistance when "
                                  "moving windows past the edge of another "
                                  "window."));
    val.bool = TRUE;
    config_set("resistance.windows", Config_Bool, val);
}

static void resist_move(Client *c, int *x, int *y)
{
    GList *it;
    Rect *area;
    int l, t, r, b; /* requested edges */
    int al, at, ar, ab; /* screen area edges */
    int cl, ct, cr, cb; /* current edges */
    int w, h; /* current size */
    Client *snapx = NULL, *snapy = NULL;
    ConfigValue resist, window_resist;

    if (!config_get("resistance", Config_Integer, &resist) ||
        resist.integer < 0) {
        resist.integer = DEFAULT_RESISTANCE;
        config_set("resistance", Config_Integer, resist);
    }
    if (!config_get("resistance.windows", Config_Bool, &window_resist))
        g_assert_not_reached();

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
    if (window_resist.bool)
        for (it = stacking_list; it != NULL; it = it->next) {
            Client *target;
            int tl, tt, tr, tb; /* 1 past the target's edges on each side */

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
                if (cl >= tr && l < tr && l >= tr - resist.integer)
                    *x = tr, snapx = target;
                else if (cr <= tl && r > tl && r <= tl + resist.integer)
                    *x = tl - w + 1, snapx = target;
                if (snapx != NULL) {
                    /* try to corner snap to the window */
                    if (ct > tt && t <= tt && t > tt - resist.integer)
                        *y = tt + 1, snapy = target;
                    else if (cb < tb && b >= tb && b < tb + resist.integer)
                        *y = tb - h, snapy = target;
                }
            }
            if (snapy == NULL) {
                if (ct >= tb && t < tb && t >= tb - resist.integer)
                    *y = tb, snapy = target;
                else if (!cb <= tt && b > tt && b <= tt + resist.integer)
                    *y = tt - h + 1, snapy = target;
                if (snapy != NULL) {
                    /* try to corner snap to the window */
                    if (cl > tl && l <= tl && l > tl - resist.integer)
                        *x = tl + 1, snapx = target;
                    else if (cr < tr && r >= tr && r < tr + resist.integer)
                        *x = tr - w, snapx = target;
                }
            }

            if (snapx && snapy) break;
        }

    /* get the screen boundaries */
    area = screen_area(c->desktop);
    al = area->x;
    at = area->y;
    ar = al + area->width - 1;
    ab = at + area->height - 1;

    /* snap to screen edges */
    if (cl >= al && l < al && l >= al - resist.integer)
        *x = al;
    else if (cr <= ar && r > ar && r <= ar + resist.integer)
            *x = ar - w + 1;
    if (ct >= at && t < at && t >= at - resist.integer)
        *y = at;
    else if (cb <= ab && b > ab && b < ab + resist.integer)
        *y = ab - h + 1;
}

static void resist_size(Client *c, int *w, int *h, Corner corn)
{
    GList *it;
    Client *t; /* target */
    int lt, rb; /* my left/top and right/bottom sides */
    int dlt, drb; /* my destination left/top and right/bottom sides */
    int tlt, trb; /* target's left/top and right/bottom sides */
    Rect *area;
    int al, at, ar, ab; /* screen boundaries */
    Client *snapx = NULL, *snapy = NULL;
    ConfigValue resist, window_resist;

    if (!config_get("resistance", Config_Integer, &resist) ||
        resist.integer < 0) {
        resist.integer = DEFAULT_RESISTANCE;
        config_set("resistance", Config_Integer, resist);
    }
    if (!config_get("resistance.windows", Config_Bool, &window_resist))
        g_assert_not_reached();

    /* get the screen boundaries */
    area = screen_area(c->desktop);
    al = area->x;
    at = area->y;
    ar = al + area->width - 1;
    ab = at + area->height - 1;

    /* horizontal snapping */

    lt = c->frame->area.x;
    rb = lt + c->frame->area.width - 1;

    /* snap to other windows */
    if (window_resist.bool) {
        for (it = stacking_list; !snapx && it != NULL; it = it->next) {
            t = it->data;

            /* don't snap to invisibles or ourself */
            if (!t->frame->visible || t == c) continue;

            switch (corn) {
            case Corner_TopLeft:
            case Corner_BottomLeft:
                dlt = lt;
                drb = rb + *w - c->frame->area.width;
                tlt = t->frame->area.x;
                if (rb < tlt && drb >= tlt && drb < tlt + resist.integer)
                    *w = tlt - lt, snapx = t;
                break;
            case Corner_TopRight:
            case Corner_BottomRight:
                dlt = lt - *w + c->frame->area.width;
                drb = rb;
                trb = t->frame->area.x + t->frame->area.width - 1;
                if (lt > trb && dlt <= trb && dlt > trb - resist.integer)
                    *w = rb - trb, snapx = t;
                break;
            }
        }
    }

    /* snap to screen edges */
    switch (corn) {
    case Corner_TopLeft:
    case Corner_BottomLeft:
        dlt = lt;
        drb = rb + *w - c->frame->area.width;
        if (rb <= ar && drb > ar && drb <= ar + resist.integer)
            *w = ar - lt + 1;
        break;
    case Corner_TopRight:
    case Corner_BottomRight:
        dlt = lt - *w + c->frame->area.width;
        drb = rb;
        if (lt >= al && dlt < al && dlt >= al - resist.integer)
            *w = rb - al + 1;
        break;
    }

    /* vertical snapping */

    lt = c->frame->area.y;
    rb = lt + c->frame->area.height - 1;

    /* snap to other windows */
    if (window_resist.bool) {
        for (it = stacking_list; !snapy && it != NULL; it = it->next) {
            t = it->data;

            /* don't snap to invisibles or ourself */
            if (!t->frame->visible || t == c) continue;

            switch (corn) {
            case Corner_TopLeft:
            case Corner_TopRight:
                dlt = lt;
                drb = rb + *h - c->frame->area.height;
                tlt = t->frame->area.y;
                if (rb < tlt && drb >= tlt && drb < tlt + resist.integer)
                    *h = tlt - lt, snapy = t;
                break;
            case Corner_BottomLeft:
            case Corner_BottomRight:
                dlt = lt - *h + c->frame->area.height;
                drb = rb;
                trb = t->frame->area.y + t->frame->area.height - 1;
                if (lt > trb && dlt <= trb && dlt > trb - resist.integer)
                    *h = rb - trb, snapy = t;
                break;
            }
        }
    }

    /* snap to screen edges */
    switch (corn) {
    case Corner_TopLeft:
    case Corner_TopRight:
        dlt = lt;
        drb = rb + *h - c->frame->area.height;
        if (rb <= ab && drb > ab && drb <= ab + resist.integer)
            *h = ar - lt + 1;
        break;
    case Corner_BottomLeft:
    case Corner_BottomRight:
        dlt = lt - *h + c->frame->area.height;
        drb = rb;
        if (lt >= at && dlt < at && dlt >= at - resist.integer)
            *h = rb - al + 1;
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
