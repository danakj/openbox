#include <glib.h>
#include "../kernel/geom.h"
#include "image.h"

void image_draw(pixel32 *target, TextureRGBA *rgba, Rect *position)
{
  unsigned long *draw = rgba->data;
  int c, sfw, sfh;
  unsigned int i, e, bgi;
  sfw = position->width;
  sfh = position->height;

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
    for (i = 0, c = 0, e = sfw*sfh; i < e; ++i, ++bgi) {
      unsigned char alpha = draw[i] >> 24;
      unsigned char r = draw[i] >> 16;
      unsigned char g = draw[i] >> 8;
      unsigned char b = draw[i];

      /* background color */
      unsigned char bgr = target[i] >> default_red_shift;
      unsigned char bgg = target[i] >> default_green_shift;
      unsigned char bgb = target[i] >> default_blue_shift;

      r = bgr + (((r - bgr) * alpha) >> 8);
      g = bgg + (((g - bgg) * alpha) >> 8);
      b = bgb + (((b - bgb) * alpha) >> 8);

      target[i] = (r << default_red_shift) | (g << default_green_shift) |
        (b << default_blue_shift);
    }
  }
}
