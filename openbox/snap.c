#include "client.h"
#include "frame.h"
#include "stacking.h"
#include "screen.h"

static int resistance = 10;

void snap_move(Client *c, int *x, int *y, int w, int h)
{
    GList *it;
    Rect *area;
    int l, t, r, b; /* requested edges */
    int al, at, ar, ab; /* screen area edges */
    int cl, ct, cr, cb; /* current edges */

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
    for (it = stacking_list; it != NULL; it = it->next) {
        /* XXX foo */
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
