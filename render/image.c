#include "geom.h"
#include "image.h"
#include "color.h"

#include <glib.h>

void RrImageDraw(RrPixel32 *target, RrTextureRGBA *rgba, RrRect *area)
{
    RrPixel32 *draw = rgba->data;
    gint c, i, e, t, sfw, sfh;
    sfw = area->width;
    sfh = area->height;

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
        rgba->cache = g_new(RrPixel32, sfw * sfh);
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
        for (i = 0, c = 0, t = area->x, e = sfw*sfh; i < e; ++i, ++t) {
            guchar alpha, r, g, b, bgr, bgg, bgb;

            alpha = draw[i] >> RrDefaultAlphaOffset;
            r = draw[i] >> RrDefaultRedOffset;
            g = draw[i] >> RrDefaultGreenOffset;
            b = draw[i] >> RrDefaultBlueOffset;

            if (c >= sfw) {
                c = 0;
                t += area->width - sfw;
            }

            /* background color */
            bgr = target[t] >> RrDefaultRedOffset;
            bgg = target[t] >> RrDefaultGreenOffset;
            bgb = target[t] >> RrDefaultBlueOffset;

            r = bgr + (((r - bgr) * alpha) >> 8);
            g = bgg + (((g - bgg) * alpha) >> 8);
            b = bgb + (((b - bgb) * alpha) >> 8);

            target[t] = (r << RrDefaultRedOffset)
                      | (g << RrDefaultGreenOffset)
                      | (b << RrDefaultBlueOffset);
        }
    }
}
