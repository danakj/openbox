#include <glib.h>
#include "../kernel/geom.h"
#include "image.h"

void image_draw(pixel32 *target, TextureRGBA *rgba, Rect *position,
                Rect *surarea)
{
    gulong *draw = rgba->data;
    guint c, i, e, t, sfw, sfh;
    sfw = position->width;
    sfh = position->height;

    /* it would be nice if this worked, but this function is well broken in
       these circumstances. */
    g_assert(position->width == surarea->width &&
             position->height == surarea->height);

    g_assert(rgba->data != NULL);

    if ((rgba->width != sfw || rgba->height != sfh) &&
        (rgba->width != rgba->cwidth || rgba->height != rgba->cheight)) {
        double dx = rgba->width / (double)sfw;
        double dy = rgba->height / (double)sfh;
        double px = 0.0;
        double py = 0.0;
        int iy = 0;

        /* scale it and cache it */
        if (rgba->cache != NULL)
            g_free(rgba->cache);
        rgba->cache = g_new(unsigned long, sfw * sfh);
        rgba->cwidth = sfw;
        rgba->cheight = sfh;
        for (i = 0, c = 0, e = sfw*sfh; i < e; ++i) {
            rgba->cache[i] = rgba->data[(int)px + iy];
            if (++c >= sfw) {
                c = 0;
                px = 0;
                py += dy;
                iy = (int)py * rgba->width;
            } else
                px += dx;
        }

        /* do we use the cache we may have just created, or the original? */
        if (rgba->width != sfw || rgba->height != sfh)
            draw = rgba->cache;

        /* apply the alpha channel */
        for (i = 0, c = 0, t = position->x, e = sfw*sfh; i < e; ++i, ++t) {
            guchar alpha, r, g, b, bgr, bgg, bgb;

            alpha = draw[i] >> 24;
            r = draw[i] >> 16;
            g = draw[i] >> 8;
            b = draw[i];

            if (c >= sfw) {
                c = 0;
                t += surarea->width - sfw;
            }

            /* background color */
            bgr = target[t] >> default_red_shift;
            bgg = target[t] >> default_green_shift;
            bgb = target[t] >> default_blue_shift;

            r = bgr + (((r - bgr) * alpha) >> 8);
            g = bgg + (((g - bgg) * alpha) >> 8);
            b = bgb + (((b - bgb) * alpha) >> 8);

            target[t] = (r << default_red_shift) | (g << default_green_shift) |
                (b << default_blue_shift);
        }
    }
}
