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
                if (ct < tb && cb > tt) {
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
            }
            if (snapy == NULL) {
                if (cl < tr && cr > tl) {
                    if (ct >= tb && t < tb && t >= tb - resist.integer)
                        *y = tb, snapy = target;
                    else if (cb <= tt && b > tt && b <= tt + resist.integer)
                        *y = tt - h + 1, snapy = target;
                    if (snapy != NULL) {
                        /* try to corner snap to the window */
                        if (cl > tl && l <= tl && l > tl - resist.integer)
                            *x = tl + 1, snapx = target;
                        else if (cr < tr && r >= tr && r < tr + resist.integer)
                            *x = tr - w, snapx = target;
                    }
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
    Client *target; /* target */
    int l, t, r, b; /* my left, top, right and bottom sides */
    int dlt, drb; /* my destination left/top and right/bottom sides */
    int tl, tt, tr, tb; /* target's left, top, right and bottom bottom sides */
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
    if (window_resist.bool) {
        for (it = stacking_list; it != NULL; it = it->next) {
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
                    case Corner_TopLeft:
                    case Corner_BottomLeft:
                        dlt = l;
                        drb = r + *w - c->frame->area.width;
                        if (r < tl && drb >= tl && drb < tl + resist.integer)
                            *w = tl - l, snapx = target;
                        break;
                    case Corner_TopRight:
                    case Corner_BottomRight:
                        dlt = l - *w + c->frame->area.width;
                        drb = r;
                        if (l > tr && dlt <= tr && dlt > tr - resist.integer)
                            *w = r - tr, snapx = target;
                        break;
                    }
                }
            }

            if (snapy == NULL) {
                /* vertical snapping */
                if (l < tr && r > tl) {
                    switch (corn) {
                    case Corner_TopLeft:
                    case Corner_TopRight:
                        dlt = t;
                        drb = b + *h - c->frame->area.height;
                        if (b < tt && drb >= tt && drb < tt + resist.integer)
                            *h = tt - t, snapy = target;
                        break;
                    case Corner_BottomLeft:
                    case Corner_BottomRight:
                        dlt = t - *h + c->frame->area.height;
                        drb = b;
                        if (t > tb && dlt <= tb && dlt > tb - resist.integer)
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
    case Corner_TopLeft:
    case Corner_BottomLeft:
        dlt = l;
        drb = r + *w - c->frame->area.width;
        if (r <= ar && drb > ar && drb <= ar + resist.integer)
            *w = ar - l + 1;
        break;
    case Corner_TopRight:
    case Corner_BottomRight:
        dlt = l - *w + c->frame->area.width;
        drb = r;
        if (l >= al && dlt < al && dlt >= al - resist.integer)
            *w = r - al + 1;
        break;
    }

    /* vertical snapping */
    switch (corn) {
    case Corner_TopLeft:
    case Corner_TopRight:
        dlt = t;
        drb = b + *h - c->frame->area.height;
        if (b <= ab && drb > ab && drb <= ab + resist.integer)
            *h = ab - t + 1;
        break;
    case Corner_BottomLeft:
    case Corner_BottomRight:
        dlt = t - *h + c->frame->area.height;
        drb = b;
        if (t >= at && dlt < at && dlt >= at - resist.integer)
            *h = b - at + 1;
        break;
    }

    /* round down for size increments */
    *w -= c->frame->area.width + c->frame->size.left + c->frame->size.right;
    *w = *w / c->size_inc.width * c->size_inc.width;
    *w += c->frame->area.width + c->frame->size.left + c->frame->size.right;

    *h -= c->frame->area.height + c->frame->size.top + c->frame->size.bottom;
    *h = *h / c->size_inc.height * c->size_inc.height;
    *h += c->frame->area.height + c->frame->size.top + c->frame->size.bottom;
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
