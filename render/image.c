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

#define AVERAGE(a, b)   ( ((((a) ^ (b)) & 0xfefefefeL) >> 1) + ((a) & (b)) )

static void scale_line(RrPixel32 *dest, RrPixel32 *source, gint w, gint dw)
{
    gint num_pixels = dw;
    gint int_part = w / dw;
    gint fract_part = w % dw;
    gint err = 0;

    while (num_pixels-- > 0) {
        *dest++ = *source;
        source += int_part;
        err += fract_part;
        if (err >= dw) {
            err -= dw;
            source++;
        }
    }
}

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

static RrPixel32* scale_rect(RrPixel32 *fullsource,
                             gint w, gint h, gint dw, gint dh)
{
    RrPixel32 *out, *dest;
    RrPixel32 *source = fullsource;
    RrPixel32 *oldsource = NULL;
    RrPixel32 *prev_source = NULL;
    gint num_pixels;
    gint int_part;
    gint fract_part;
    gint err = 0;

    while (dw <= (w >> 1) && dh <= (h >> 1)) {
        source = scale_half(source, w, h);
        w >>= 1; h >>= 1;
        g_free(oldsource);
        oldsource = source;
    }

    num_pixels = dh;
    int_part = (h / dh) * w;
    fract_part = h % dh;

    out = dest = g_new(RrPixel32, dw * dh);

    while (num_pixels-- > 0) {
        if (source == prev_source) {
            memcpy(dest, dest - dw, dw * sizeof(RrPixel32));
        } else {
            scale_line(dest, source, w, dw);
            prev_source = source;
        }
        dest += dw;
        source += int_part;
        err += fract_part;
        if (err >= dh) {
            err -= dh;
            source += w;
        }
    }

    g_free(oldsource);

    return out;
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

    if (sw != dw || sh != dh) {
        /*if (!(rgba->cache && dw == rgba->cwidth && dh == rgba->cheight))*/ {
            g_free(rgba->cache);
            rgba->cache = scale_rect(rgba->data, sw, sh, dw, dh);
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

        if (col++ >= dw) {
            col = 0;
            dest += target_w - dw;
        }
    }
}
