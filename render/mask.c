#include "mask.h"
#include "../kernel/openbox.h"

pixmap_mask *pixmap_mask_new(int w, int h, char *data)
{
    pixmap_mask *m = g_new(pixmap_mask, 1);
    m->w = w;
    m->h = h;
    m->mask = XCreateBitmapFromData(ob_display, ob_root, data, w, h);
    return m;
}

void pixmap_mask_free(pixmap_mask *m)
{
    XFreePixmap(ob_display, m->mask);
    g_free(m);
}

void mask_draw(pixmap_mask *p, TextureMask *m)
{
}
