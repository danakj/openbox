#include "client.h"
#include "frame.h"
#include "stacking.h"
#include "screen.h"

static int resistance = 10;
static gboolean edge_resistance = TRUE; /* window-to-edge */
static gboolean window_resistance = TRUE; /* window-to-window */

void snap_move(Client *c, int *x, int *y, int w, int h)
{
    GList *it;
    Rect *area;
    int l, t, r, b; /* requested edges */
    int al, at, ar, ab; /* screen area edges */
    int cl, ct, cr, cb; /* current edges */
    gboolean snapx = FALSE, snapy = FALSE;

    if (!edge_resistance) return;

    /* add the frame to the dimensions */
    l = *x;
    t = *y;
    r = l + w - 1;
    b = t + h - 1;

    cl = c->frame->area.x;
    ct = c->frame->area.y;
    cr = cl + c->frame->area.width - 1;
    cb = ct + c->frame->area.height - 1;
    
    /* snap to other clients */
    if (window_resistance)
        for (it = stacking_list; it != NULL; it = it->next) {
            Client *target;
            int tl, tt, tr, tb; /* 1 past the target's edges on each side */

            target = it->data;

            tl = target->frame->area.x - 1;
            tt = target->frame->area.y - 1;
            tr = tl + target->frame->area.width + 1;
            tb = tt + target->frame->area.height + 1;

            /* snapx and snapy ensure that the window snaps to the top-most
               window edge available, without going all the way from
               bottom-to-top in the stacking list
            */
            if (!snapx && cl >= tr && l < tr && l >= tr - resistance)
                *x = tr, snapx = TRUE;
            else if (!snapx && cr <= tl && r > tl && r <= tl + resistance)
                *x = tl - w + 1, snapx = TRUE;
            else if (!snapy && ct >= tb && t < tb && t >= tb - resistance)
                *y = tb, snapy = TRUE;
            else if (!snapy && cb <= tt && b > tt && b <= tt + resistance)
                *y = tt - h + 1, snapy = TRUE;

            if (snapx && snapy) break;
        }

    /* get the screen boundaries */
    area = screen_area(c->desktop);
    al = area->x;
    at = area->y;
    ar = al + area->width - 1;
    ab = at + area->height - 1;

    /* snap to screen edges */
    if (cl >= al && l < al && l >= al - resistance)
        *x = al;
    else if (cr <= ar && r > ar && r <= ar + resistance)
            *x = ar - w + 1;
    if (ct >= at && t < at && t >= at - resistance)
        *y = at;
    else if (cb <= ab && b > ab && b < ab + resistance)
        *y = ab - h + 1;
}
