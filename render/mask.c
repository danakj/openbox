#include "render.h"
#include "color.h"
#include "mask.h"

RrPixmapMask *RrPixmapMaskNew(const RrInstance *inst,
                              gint w, gint h, const gchar *data)
{
    RrPixmapMask *m = g_new(RrPixmapMask, 1);
    m->inst = inst;
    m->width = w;
    m->height = h;
    /* round up to nearest byte */
    m->data = g_memdup(data, (w * h + 7) / 8);
    m->mask = XCreateBitmapFromData(RrDisplay(inst), RrRootWindow(inst),
                                    data, w, h);
    return m;
}

void RrPixmapMaskFree(RrPixmapMask *m)
{
    if (m) {
        XFreePixmap(RrDisplay(m->inst), m->mask);
        g_free(m->data);
        g_free(m);
    }
}

void RrPixmapMaskDraw(Pixmap p, const RrTextureMask *m, const RrRect *area)
{
    int x, y;
    if (m->mask == None) return; /* no mask given */

    /* set the clip region */
    x = area->x + (area->width - m->mask->width) / 2;
    y = area->y + (area->height - m->mask->height) / 2;

    if (x < 0) x = 0;
    if (y < 0) y = 0;

    XSetClipMask(RrDisplay(m->mask->inst), RrColorGC(m->color), m->mask->mask);
    XSetClipOrigin(RrDisplay(m->mask->inst), RrColorGC(m->color), x, y);

    /* fill in the clipped region */
    XFillRectangle(RrDisplay(m->mask->inst), p, RrColorGC(m->color), x, y,
                   x + m->mask->width, y + m->mask->height);

    /* unset the clip region */
    XSetClipMask(RrDisplay(m->mask->inst), RrColorGC(m->color), None);
    XSetClipOrigin(RrDisplay(m->mask->inst), RrColorGC(m->color), 0, 0);
}

RrPixmapMask *RrPixmapMaskCopy(const RrPixmapMask *src)
{
    RrPixmapMask *m = g_new(RrPixmapMask, 1);
    m->inst = src->inst;
    m->width = src->width;
    m->height = src->height;
    /* round up to nearest byte */
    m->data = g_memdup(src->data, (src->width * src->height + 7) / 8);
    m->mask = XCreateBitmapFromData(RrDisplay(m->inst), RrRootWindow(m->inst),
                                    m->data, m->width, m->height);
    return m;
}
