#include "mask.h"
#include "../kernel/openbox.h"

pixmap_mask *pixmap_mask_new(int w, int h, char *data)
{
    pixmap_mask *m = g_new(pixmap_mask, 1);
    m->w = w;
    m->h = h;
    /* round up to nearest byte */
    m->data = g_memdup(data, (w * h + 7) / 8);
    m->mask = XCreateBitmapFromData(ob_display, ob_root, data, w, h);
    return m;
}

void pixmap_mask_free(pixmap_mask *m)
{
    XFreePixmap(ob_display, m->mask);
    g_free(m->data);
    g_free(m);
}

void mask_draw(Pixmap p, TextureMask *m, Rect *position)
{
    int x, y;
    if (m->mask == None) return; /* no mask given */

    /* set the clip region */
    x = position->x + (position->width - m->mask->w) / 2;
    y = position->y + (position->height - m->mask->h) / 2;

    if (x < 0) x = 0;
    if (y < 0) y = 0;

    XSetClipMask(ob_display, m->color->gc, m->mask->mask);
    XSetClipOrigin(ob_display, m->color->gc, x, y);

    /* fill in the clipped region */
    XFillRectangle(ob_display, p, m->color->gc, x, y,
                   x + m->mask->w, y + m->mask->h);

    /* unset the clip region */
    XSetClipMask(ob_display, m->color->gc, None);
    XSetClipOrigin(ob_display, m->color->gc, 0, 0);
}

pixmap_mask *pixmap_mask_copy(pixmap_mask *src)
{
    pixmap_mask *m = g_new(pixmap_mask, 1);
    m->w = src->w;
    m->h = src->h;
    /* round up to nearest byte */
    m->data = g_memdup(src->data, (src->w * src->h + 7) / 8);
    m->mask = XCreateBitmapFromData(ob_display, ob_root, m->data, m->w, m->h);
    return m;
}
