/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   image.c for the Openbox window manager
   Copyright (c) 2003        Ben Jansens

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   See the COPYING file for a copy of the GNU General Public License.
*/

#include "geom.h"
#include "image.h"
#include "color.h"

#include <glib.h>

#define FRACTION        12
#define FLOOR(i)        ((i) & (~0UL << FRACTION))
#define AVERAGE(a, b)   (((((a) ^ (b)) & 0xfefefefeL) >> 1) + ((a) & (b)))

static RrPixel32* scale_half(RrPixel32 *source, gint w, gint h)
{
    RrPixel32 *out, *dest, *sourceline, *sourceline2;
    gint dw, dh, x, y;

    sourceline = source;
    sourceline2 = source + w;

    dw = w >> 1;
    dh = h >> 1;

    out = dest = g_new(RrPixel32, dw * dh);

    for (y = 0; y < dh; ++y) {
        RrPixel32 *s, *s2;

        s = sourceline;
        s2 = sourceline2;

        for (x = 0; x < dw; ++x) {
            *dest++ = AVERAGE(AVERAGE(*s, *(s+1)),
                              AVERAGE(*s2, *(s2+1)));
            s += 2;
            s2 += 2;
        }
        sourceline += w << 1;
        sourceline2 += w << 1;
    }
    return out;
}

static void ImageCopyResampled(RrPixel32 *dst, RrPixel32 *src,
                               gulong dstW, gulong dstH,
                               gulong srcW, gulong srcH)
{
    gulong dstX, dstY, srcX, srcY;
    gulong srcX1, srcX2, srcY1, srcY2;
    gulong ratioX, ratioY;
    
    ratioX = (srcW << FRACTION) / dstW;
    ratioY = (srcH << FRACTION) / dstH;
    
    srcY2 = 0;
    for (dstY = 0; dstY < dstH; dstY++) {
        srcY1 = srcY2;
        srcY2 += ratioY;
        
        srcX2 = 0;
        for (dstX = 0; dstX < dstW; dstX++) {
            gulong red = 0, green = 0, blue = 0, alpha = 0;
            gulong portionX, portionY, portionXY, sumXY = 0;
            RrPixel32 pixel;
            
            srcX1 = srcX2;
            srcX2 += ratioX;
            
            for (srcY = srcY1; srcY < srcY2; srcY += (1UL << FRACTION)) {
                if (srcY == srcY1) {
                    srcY = FLOOR(srcY);
                    portionY = (1UL << FRACTION) - (srcY1 - srcY);
                    if (portionY > srcY2 - srcY1)
                        portionY = srcY2 - srcY1;
                }
                else if (srcY == FLOOR(srcY2))
                    portionY = srcY2 - srcY;
                else
                    portionY = (1UL << FRACTION);
                
                for (srcX = srcX1; srcX < srcX2; srcX += (1UL << FRACTION)) {
                    if (srcX == srcX1) {
                        srcX = FLOOR(srcX);
                        portionX = (1UL << FRACTION) - (srcX1 - srcX);
                        if (portionX > srcX2 - srcX1)
                            portionX = srcX2 - srcX1;
                    }
                    else if (srcX == FLOOR(srcX2))
                        portionX = srcX2 - srcX;
                    else
                        portionX = (1UL << FRACTION);
                    
                    portionXY = (portionX * portionY) >> FRACTION;
                    sumXY += portionXY;
                    
                    pixel = *(src + (srcY >> FRACTION) * srcW + (srcX >> FRACTION));
                    red   += ((pixel >> RrDefaultRedOffset)   & 0xFF) * portionXY;
                    green += ((pixel >> RrDefaultGreenOffset) & 0xFF) * portionXY;
                    blue  += ((pixel >> RrDefaultBlueOffset)  & 0xFF) * portionXY;
                    alpha += ((pixel >> RrDefaultAlphaOffset) & 0xFF) * portionXY;
                }
            }
            
            g_assert(sumXY != 0);
            red   /= sumXY;
            green /= sumXY;
            blue  /= sumXY;
            alpha /= sumXY;
            
            *dst++ = (red   << RrDefaultRedOffset)   |
                     (green << RrDefaultGreenOffset) |
                     (blue  << RrDefaultBlueOffset)  |
                     (alpha << RrDefaultAlphaOffset);
        }
    }
}

void RrImageDraw(RrPixel32 *target, RrTextureRGBA *rgba,
                 gint target_w, gint target_h,
                 RrRect *area)
{
    RrPixel32 *dest;
    RrPixel32 *source;
    gint sw, sh, dw, dh;
    gint col, num_pixels;

    sw = rgba->width;
    sh = rgba->height;

    /* keep the ratio */
    dw = area->width;
    dh = (gint)(dw * ((gdouble)sh / sw));
    if (dh > area->height) {
        dh = area->height;
        dw = (gint)(dh * ((gdouble)sw / sh));
    }

    if (!(dw && dh))
        return; /* XXX sanity check */

    if (sw != dw || sh != dh) {
        /*if (!(rgba->cache && dw == rgba->cwidth && dh == rgba->cheight))*/ {
            g_free(rgba->cache);
            rgba->cache = g_new(RrPixel32, dw * dh);
            ImageCopyResampled(rgba->cache, rgba->data, dw, dh, sw, sh);
            rgba->cwidth = dw;
            rgba->cheight = dh;
        }
        source = rgba->cache;
    } else {
        source = rgba->data;
    }

    /* copy source -> dest, and apply the alpha channel */
    col = 0;
    num_pixels = dw * dh;
    dest = target + area->x + target_w * area->y;
    while (num_pixels-- > 0) {
        guchar alpha, r, g, b, bgr, bgg, bgb;

        alpha = *source >> RrDefaultAlphaOffset;
        r = *source >> RrDefaultRedOffset;
        g = *source >> RrDefaultGreenOffset;
        b = *source >> RrDefaultBlueOffset;

        /* background color */
        bgr = *dest >> RrDefaultRedOffset;
        bgg = *dest >> RrDefaultGreenOffset;
        bgb = *dest >> RrDefaultBlueOffset;

        r = bgr + (((r - bgr) * alpha) >> 8);
        g = bgg + (((g - bgg) * alpha) >> 8);
        b = bgb + (((b - bgb) * alpha) >> 8);

        *dest = ((r << RrDefaultRedOffset) |
                 (g << RrDefaultGreenOffset) |
                 (b << RrDefaultBlueOffset));

        dest++;
        source++;

        if (++col >= dw) {
            col = 0;
            dest += target_w - dw;
        }
    }
}
