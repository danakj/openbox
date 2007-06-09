/* -*- indent-tabs-mode: nil; tab-width: 4; c-basic-offset: 4; -*-

   gradient.c for the Openbox window manager
   Copyright (c) 2006        Mikael Magnusson
   Copyright (c) 2003-2007   Dana Jansens
   Copyright (c) 2003        Derek Foreman

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

#include "render.h"
#include "gradient.h"
#include "color.h"
#include <glib.h>

static void highlight(RrPixel32 *x, RrPixel32 *y, gboolean raised);
static void gradient_parentrelative(RrAppearance *a, gint w, gint h);
static void gradient_solid(RrAppearance *l, gint w, gint h);
static void gradient_splitvertical(RrAppearance *a, gint w, gint h);
static void gradient_vertical(RrSurface *sf, gint w, gint h);
static void gradient_horizontal(RrSurface *sf, gint w, gint h);
static void gradient_mirrorhorizontal(RrSurface *sf, gint w, gint h);
static void gradient_diagonal(RrSurface *sf, gint w, gint h);
static void gradient_crossdiagonal(RrSurface *sf, gint w, gint h);
static void gradient_pyramid(RrSurface *sf, gint inw, gint inh);

void RrRender(RrAppearance *a, gint w, gint h)
{
    RrPixel32 *data = a->surface.pixel_data;
    RrPixel32 current;
    guint r,g,b;
    gint off, x;

    switch (a->surface.grad) {
    case RR_SURFACE_PARENTREL:
        gradient_parentrelative(a, w, h);
        break;
    case RR_SURFACE_SOLID:
        gradient_solid(a, w, h);
        break;
    case RR_SURFACE_SPLIT_VERTICAL:
        gradient_splitvertical(a, w, h);
        break;
    case RR_SURFACE_VERTICAL:
        gradient_vertical(&a->surface, w, h);
        break;
    case RR_SURFACE_HORIZONTAL:
        gradient_horizontal(&a->surface, w, h);
        break;
    case RR_SURFACE_MIRROR_HORIZONTAL:
        gradient_mirrorhorizontal(&a->surface, w, h);
        break;
    case RR_SURFACE_DIAGONAL:
        gradient_diagonal(&a->surface, w, h);
        break;
    case RR_SURFACE_CROSS_DIAGONAL:
        gradient_crossdiagonal(&a->surface, w, h);
        break;
    case RR_SURFACE_PYRAMID:
        gradient_pyramid(&a->surface, w, h);
        break;
    default:
        g_assert_not_reached(); /* unhandled gradient */
        return;
    }
  
    if (a->surface.interlaced) {
        gint i;
        RrPixel32 *p;

        r = a->surface.interlace_color->r;
        g = a->surface.interlace_color->g;
        b = a->surface.interlace_color->b;
        current = (r << RrDefaultRedOffset)
            + (g << RrDefaultGreenOffset)
            + (b << RrDefaultBlueOffset);
        p = data;
        for (i = 0; i < h; i += 2, p += w)
            for (x = 0; x < w; ++x, ++p)
                *p = current;
    }

    if (a->surface.relief == RR_RELIEF_FLAT && a->surface.border) {
        r = a->surface.border_color->r;
        g = a->surface.border_color->g;
        b = a->surface.border_color->b;
        current = (r << RrDefaultRedOffset)
            + (g << RrDefaultGreenOffset)
            + (b << RrDefaultBlueOffset);
        for (off = 0, x = 0; x < w; ++x, off++) {
            *(data + off) = current;
            *(data + off + ((h-1) * w)) = current;
        }
        for (off = 0, x = 0; x < h; ++x, off++) {
            *(data + (off * w)) = current;
            *(data + (off * w) + w - 1) = current;
        }
    }

    if (a->surface.relief != RR_RELIEF_FLAT) {
        if (a->surface.bevel == RR_BEVEL_1) {
            for (off = 1, x = 1; x < w - 1; ++x, off++)
                highlight(data + off,
                          data + off + (h-1) * w,
                          a->surface.relief==RR_RELIEF_RAISED);
            for (off = 0, x = 0; x < h; ++x, off++)
                highlight(data + off * w,
                          data + off * w + w - 1,
                          a->surface.relief==RR_RELIEF_RAISED);
        }

        if (a->surface.bevel == RR_BEVEL_2) {
            for (off = 2, x = 2; x < w - 2; ++x, off++)
                highlight(data + off + w,
                          data + off + (h-2) * w,
                          a->surface.relief==RR_RELIEF_RAISED);
            for (off = 1, x = 1; x < h-1; ++x, off++)
                highlight(data + off * w + 1,
                          data + off * w + w - 2,
                          a->surface.relief==RR_RELIEF_RAISED);
        }
    }
}

static void highlight(RrPixel32 *x, RrPixel32 *y, gboolean raised)
{
    gint r, g, b;

    RrPixel32 *up, *down;
    if (raised) {
        up = x;
        down = y;
    } else {
        up = y;
        down = x;
    }
    r = (*up >> RrDefaultRedOffset) & 0xFF;
    r += r >> 1;
    g = (*up >> RrDefaultGreenOffset) & 0xFF;
    g += g >> 1;
    b = (*up >> RrDefaultBlueOffset) & 0xFF;
    b += b >> 1;
    if (r > 0xFF) r = 0xFF;
    if (g > 0xFF) g = 0xFF;
    if (b > 0xFF) b = 0xFF;
    *up = (r << RrDefaultRedOffset) + (g << RrDefaultGreenOffset)
        + (b << RrDefaultBlueOffset);
  
    r = (*down >> RrDefaultRedOffset) & 0xFF;
    r = (r >> 1) + (r >> 2);
    g = (*down >> RrDefaultGreenOffset) & 0xFF;
    g = (g >> 1) + (g >> 2);
    b = (*down >> RrDefaultBlueOffset) & 0xFF;
    b = (b >> 1) + (b >> 2);
    *down = (r << RrDefaultRedOffset) + (g << RrDefaultGreenOffset)
        + (b << RrDefaultBlueOffset);
}

static void create_bevel_colors(RrAppearance *l)
{
    gint r, g, b;

    /* light color */
    r = l->surface.primary->r;
    r += r >> 1;
    g = l->surface.primary->g;
    g += g >> 1;
    b = l->surface.primary->b;
    b += b >> 1;
    if (r > 0xFF) r = 0xFF;
    if (g > 0xFF) g = 0xFF;
    if (b > 0xFF) b = 0xFF;
    g_assert(!l->surface.bevel_light);
    l->surface.bevel_light = RrColorNew(l->inst, r, g, b);

    /* dark color */
    r = l->surface.primary->r;
    r = (r >> 1) + (r >> 2);
    g = l->surface.primary->g;
    g = (g >> 1) + (g >> 2);
    b = l->surface.primary->b;
    b = (b >> 1) + (b >> 2);
    g_assert(!l->surface.bevel_dark);
    l->surface.bevel_dark = RrColorNew(l->inst, r, g, b);
}

static void gradient_parentrelative(RrAppearance *a, gint w, gint h)
{
    RrPixel32 *source, *dest;
    gint sw, sh, partial_w, partial_h, i;

    g_assert (a->surface.parent);
    g_assert (a->surface.parent->w);

    sw = a->surface.parent->w;
    sh = a->surface.parent->h;

    /* This is a little hack. When a texture is parentrelative, and the same
       area as the parent, and has a bevel, it will draw its bevel on top
       of the parent's, amplifying it. So instead, rerender the child with
       the parent's settings, but the child's bevel and interlace */
    if (a->surface.relief != RR_RELIEF_FLAT &&
        (a->surface.parent->surface.relief != RR_RELIEF_FLAT ||
         a->surface.parent->surface.border) &&
        !a->surface.parentx && !a->surface.parenty &&
        sw == w && sh == h)
    {
        RrSurface old = a->surface;
        a->surface = a->surface.parent->surface;

        /* turn these off for the parent */
        a->surface.relief = RR_RELIEF_FLAT;
        a->surface.border = FALSE;

        a->surface.pixel_data = old.pixel_data;

        RrRender(a, w, h);
        a->surface = old;
    } else {
        source = (a->surface.parent->surface.pixel_data +
                  a->surface.parentx + sw * a->surface.parenty);
        dest = a->surface.pixel_data;

        if (a->surface.parentx + w > sw) {
            partial_w = sw - a->surface.parentx;
        } else partial_w = w;

        if (a->surface.parenty + h > sh) {
            partial_h = sh - a->surface.parenty;
        } else partial_h = h;

        for (i = 0; i < partial_h; i++, source += sw, dest += w) {
            memcpy(dest, source, partial_w * sizeof(RrPixel32));
        }
    }
}

static void gradient_solid(RrAppearance *l, gint w, gint h) 
{
    gint i;
    RrPixel32 pix;
    RrPixel32 *data = l->surface.pixel_data;
    RrSurface *sp = &l->surface;
    gint left = 0, top = 0, right = w - 1, bottom = h - 1;

    pix = (sp->primary->r << RrDefaultRedOffset)
        + (sp->primary->g << RrDefaultGreenOffset)
        + (sp->primary->b << RrDefaultBlueOffset);

    for (i = 0; i < w * h; i++)
        *data++ = pix;

    if (sp->interlaced)
        return;

    XFillRectangle(RrDisplay(l->inst), l->pixmap, RrColorGC(sp->primary),
                   0, 0, w, h);

    switch (sp->relief) {
    case RR_RELIEF_RAISED:
        if (!sp->bevel_dark)
            create_bevel_colors(l);

        switch (sp->bevel) {
        case RR_BEVEL_1:
            XDrawLine(RrDisplay(l->inst), l->pixmap, RrColorGC(sp->bevel_dark),
                      left, bottom, right, bottom);
            XDrawLine(RrDisplay(l->inst), l->pixmap, RrColorGC(sp->bevel_dark),
                      right, bottom, right, top);
                
            XDrawLine(RrDisplay(l->inst), l->pixmap,RrColorGC(sp->bevel_light),
                      left, top, right, top);
            XDrawLine(RrDisplay(l->inst), l->pixmap,RrColorGC(sp->bevel_light),
                      left, bottom, left, top);
            break;
        case RR_BEVEL_2:
            XDrawLine(RrDisplay(l->inst), l->pixmap, RrColorGC(sp->bevel_dark),
                      left + 2, bottom - 1, right - 2, bottom - 1);
            XDrawLine(RrDisplay(l->inst), l->pixmap, RrColorGC(sp->bevel_dark),
                      right - 1, bottom - 1, right - 1, top + 1);

            XDrawLine(RrDisplay(l->inst), l->pixmap,RrColorGC(sp->bevel_light),
                      left + 2, top + 1, right - 2, top + 1);
            XDrawLine(RrDisplay(l->inst), l->pixmap,RrColorGC(sp->bevel_light),
                      left + 1, bottom - 1, left + 1, top + 1);
            break;
        default:
            g_assert_not_reached(); /* unhandled BevelType */
        }
        break;
    case RR_RELIEF_SUNKEN:
        if (!sp->bevel_dark)
            create_bevel_colors(l);

        switch (sp->bevel) {
        case RR_BEVEL_1:
            XDrawLine(RrDisplay(l->inst), l->pixmap,RrColorGC(sp->bevel_light),
                      left, bottom, right, bottom);
            XDrawLine(RrDisplay(l->inst), l->pixmap,RrColorGC(sp->bevel_light),
                      right, bottom, right, top);
      
            XDrawLine(RrDisplay(l->inst), l->pixmap, RrColorGC(sp->bevel_dark),
                      left, top, right, top);
            XDrawLine(RrDisplay(l->inst), l->pixmap, RrColorGC(sp->bevel_dark),
                      left, bottom, left, top);
            break;
        case RR_BEVEL_2:
            XDrawLine(RrDisplay(l->inst), l->pixmap,RrColorGC(sp->bevel_light),
                      left + 2, bottom - 1, right - 2, bottom - 1);
            XDrawLine(RrDisplay(l->inst), l->pixmap,RrColorGC(sp->bevel_light),
                      right - 1, bottom - 1, right - 1, top + 1);

            XDrawLine(RrDisplay(l->inst), l->pixmap, RrColorGC(sp->bevel_dark),
                      left + 2, top + 1, right - 2, top + 1);
            XDrawLine(RrDisplay(l->inst), l->pixmap, RrColorGC(sp->bevel_dark),
                      left + 1, bottom - 1, left + 1, top + 1);
            break;
        default:
            g_assert_not_reached(); /* unhandled BevelType */
        }
        break;
    case RR_RELIEF_FLAT:
        if (sp->border) {
            XDrawRectangle(RrDisplay(l->inst), l->pixmap,
                           RrColorGC(sp->border_color),
                           left, top, right, bottom);
        }
        break;
    default:  
        g_assert_not_reached(); /* unhandled ReliefType */
    }
}

/* * * * * * * * * * * * * * GRADIENT MAGIC WOOT * * * * * * * * * * * * * * */

#define VARS(x)                                                     \
    guint color##x[3];                                       \
    gint len##x, cdelta##x[3], error##x[3] = { 0, 0, 0 }, inc##x[3]; \
    gboolean bigslope##x[3] /* color slope > 1 */

#define SETUP(x, from, to, w)         \
    len##x = w;                       \
                                      \
    color##x[0] = from->r;            \
    color##x[1] = from->g;            \
    color##x[2] = from->b;            \
                                      \
    cdelta##x[0] = to->r - from->r;   \
    cdelta##x[1] = to->g - from->g;   \
    cdelta##x[2] = to->b - from->b;   \
                                      \
    if (cdelta##x[0] < 0) {           \
        cdelta##x[0] = -cdelta##x[0]; \
        inc##x[0] = -1;               \
    } else                            \
        inc##x[0] = 1;                \
    if (cdelta##x[1] < 0) {           \
        cdelta##x[1] = -cdelta##x[1]; \
        inc##x[1] = -1;               \
    } else                            \
        inc##x[1] = 1;                \
    if (cdelta##x[2] < 0) {           \
        cdelta##x[2] = -cdelta##x[2]; \
        inc##x[2] = -1;               \
    } else                            \
        inc##x[2] = 1;                \
    bigslope##x[0] = cdelta##x[0] > w;\
    bigslope##x[1] = cdelta##x[1] > w;\
    bigslope##x[2] = cdelta##x[2] > w

#define COLOR_RR(x, c)                       \
    c->r = color##x[0];                      \
    c->g = color##x[1];                      \
    c->b = color##x[2]

#define COLOR(x)                             \
    ((color##x[0] << RrDefaultRedOffset) +   \
     (color##x[1] << RrDefaultGreenOffset) + \
     (color##x[2] << RrDefaultBlueOffset))

#define INCREMENT(x, i) \
    (inc##x[i])

#define NEXT(x)                                           \
{                                                         \
    gint i;                                                \
    for (i = 2; i >= 0; --i) {                            \
        if (!cdelta##x[i]) continue;                      \
                                                          \
        if (!bigslope##x[i]) {                            \
            /* Y (color) is dependant on X */             \
            error##x[i] += cdelta##x[i];                  \
            if ((error##x[i] << 1) >= len##x) {           \
                color##x[i] += INCREMENT(x, i);           \
                error##x[i] -= len##x;                    \
            }                                             \
        } else {                                          \
            /* X is dependant on Y (color) */             \
            while (1) {                                   \
                color##x[i] += INCREMENT(x, i);           \
                error##x[i] += len##x;                    \
                if ((error##x[i] << 1) >= cdelta##x[i]) { \
                    error##x[i] -= cdelta##x[i];          \
                    break;                                \
                }                                         \
            }                                             \
        }                                                 \
    }                                                     \
}

static void gradient_splitvertical(RrAppearance *a, gint w, gint h)
{
    gint x, y1, y2, y3, r, g, b;
    RrSurface *sf = &a->surface;
    RrPixel32 *data = sf->pixel_data;
    RrPixel32 current;
    RrColor *primary_light, *secondary_light;
    gint y1sz, y2sz, y3sz;

    VARS(y1);
    VARS(y2);
    VARS(y3);

    r = sf->primary->r;
    r += r >> 2;
    g = sf->primary->g;
    g += g >> 2;
    b = sf->primary->b;
    b += b >> 2;
    if (r > 0xFF) r = 0xFF;
    if (g > 0xFF) g = 0xFF;
    if (b > 0xFF) b = 0xFF;
      primary_light = RrColorNew(a->inst, r, g, b);

    r = sf->secondary->r;
    r += r >> 4;
    g = sf->secondary->g;
    g += g >> 4;
    b = sf->secondary->b;
    b += b >> 4;
    if (r > 0xFF) r = 0xFF;
    if (g > 0xFF) g = 0xFF;
    if (b > 0xFF) b = 0xFF;
    secondary_light = RrColorNew(a->inst, r, g, b);

    y1sz = MAX(h/2 - 1, 1);
    /* setup to get the colors _in between_ these other 2 */
    y2sz = (h < 3 ? 0 : (h % 2 ? 3 : 2));
    y3sz = MAX(h/2 - 1, 0);

    SETUP(y1, primary_light, sf->primary, y1sz);
    SETUP(y2, sf->primary, sf->secondary, y2sz);
    SETUP(y3, sf->secondary, secondary_light,  y3sz);

    for (y1 = y1sz; y1 > 0; --y1) {
        current = COLOR(y1);
        for (x = w - 1; x >= 0; --x)
            *(data++) = current;

        NEXT(y1);
    }

    NEXT(y2); /* skip the first one, its the same as the last of y1 */
    for (y2 = y2sz; y2 > 0; --y2) {
        current = COLOR(y2);
        for (x = w - 1; x >= 0; --x)
            *(data++) = current;
        
        NEXT(y2);
    }
    
    for (y3 = y3sz; y3 > 0; --y3) {
        current = COLOR(y3);
        for (x = w - 1; x >= 0; --x)
            *(data++) = current;

        NEXT(y3);
    }

    RrColorFree(primary_light);
    RrColorFree(secondary_light);
}

static void gradient_horizontal(RrSurface *sf, gint w, gint h)
{
    gint x, y;
    RrPixel32 *data = sf->pixel_data, *datav;
    RrPixel32 current;

    VARS(x);
    SETUP(x, sf->primary, sf->secondary, w);

    for (x = w - 1; x > 0; --x) {  /* 0 -> w-1 */
        current = COLOR(x);
        datav = data;
        for (y = h - 1; y >= 0; --y) {  /* 0 -> h */
            *datav = current;
            datav += w;
        }
        ++data;

        NEXT(x);
    }
    current = COLOR(x);
    for (y = h - 1; y >= 0; --y)  /* 0 -> h */
        *(data + y * w) = current;
}

static void gradient_mirrorhorizontal(RrSurface *sf, gint w, gint h)
{
    gint x, y;
    RrPixel32 *data = sf->pixel_data, *datav;
    RrPixel32 current;

    VARS(x);
    SETUP(x, sf->primary, sf->secondary, w/2);

    if (w > 1) {
        for (x = w - 1; x > w/2-1; --x) {  /* 0 -> w-1 */
            current = COLOR(x);
            datav = data;
            for (y = h - 1; y >= 0; --y) {  /* 0 -> h */
                *datav = current;
                datav += w;
            }
            ++data;

            NEXT(x);
        }
        SETUP(x, sf->secondary, sf->primary, w/2);
        for (x = w/2 - 1; x > 0; --x) {  /* 0 -> w-1 */
            current = COLOR(x);
            datav = data;
            for (y = h - 1; y >= 0; --y) {  /* 0 -> h */
                *datav = current;
                datav += w;
            }
            ++data;

            NEXT(x);
        }
    }
    current = COLOR(x);
    for (y = h - 1; y >= 0; --y)  /* 0 -> h */
        *(data + y * w) = current;
}

static void gradient_vertical(RrSurface *sf, gint w, gint h)
{
    gint x, y;
    RrPixel32 *data = sf->pixel_data;
    RrPixel32 current;

    VARS(y);
    SETUP(y, sf->primary, sf->secondary, h);

    for (y = h - 1; y > 0; --y) {  /* 0 -> h-1 */
        current = COLOR(y);
        for (x = w - 1; x >= 0; --x)  /* 0 -> w */
            *(data++) = current;

        NEXT(y);
    }
    current = COLOR(y);
    for (x = w - 1; x >= 0; --x)  /* 0 -> w */
        *(data++) = current;
}


static void gradient_diagonal(RrSurface *sf, gint w, gint h)
{
    gint x, y;
    RrPixel32 *data = sf->pixel_data;
    RrColor left, right;
    RrColor extracorner;

    VARS(lefty);
    VARS(righty);
    VARS(x);

    extracorner.r = (sf->primary->r + sf->secondary->r) / 2;
    extracorner.g = (sf->primary->g + sf->secondary->g) / 2;
    extracorner.b = (sf->primary->b + sf->secondary->b) / 2;

    SETUP(lefty, sf->primary, (&extracorner), h);
    SETUP(righty, (&extracorner), sf->secondary, h);

    for (y = h - 1; y > 0; --y) {  /* 0 -> h-1 */
        COLOR_RR(lefty, (&left));
        COLOR_RR(righty, (&right));

        SETUP(x, (&left), (&right), w);

        for (x = w - 1; x > 0; --x) {  /* 0 -> w-1 */
            *(data++) = COLOR(x);

            NEXT(x);
        }
        *(data++) = COLOR(x);

        NEXT(lefty);
        NEXT(righty);
    }
    COLOR_RR(lefty, (&left));
    COLOR_RR(righty, (&right));

    SETUP(x, (&left), (&right), w);

    for (x = w - 1; x > 0; --x) {  /* 0 -> w-1 */
        *(data++) = COLOR(x);
        
        NEXT(x);
    }
    *data = COLOR(x);
}

static void gradient_crossdiagonal(RrSurface *sf, gint w, gint h)
{
    gint x, y;
    RrPixel32 *data = sf->pixel_data;
    RrColor left, right;
    RrColor extracorner;

    VARS(lefty);
    VARS(righty);
    VARS(x);

    extracorner.r = (sf->primary->r + sf->secondary->r) / 2;
    extracorner.g = (sf->primary->g + sf->secondary->g) / 2;
    extracorner.b = (sf->primary->b + sf->secondary->b) / 2;

    SETUP(lefty, (&extracorner), sf->secondary, h);
    SETUP(righty, sf->primary, (&extracorner), h);

    for (y = h - 1; y > 0; --y) {  /* 0 -> h-1 */
        COLOR_RR(lefty, (&left));
        COLOR_RR(righty, (&right));

        SETUP(x, (&left), (&right), w);

        for (x = w - 1; x > 0; --x) {  /* 0 -> w-1 */
            *(data++) = COLOR(x);

            NEXT(x);
        }
        *(data++) = COLOR(x);

        NEXT(lefty);
        NEXT(righty);
    }
    COLOR_RR(lefty, (&left));
    COLOR_RR(righty, (&right));

    SETUP(x, (&left), (&right), w);

    for (x = w - 1; x > 0; --x) {  /* 0 -> w-1 */
        *(data++) = COLOR(x);
        
        NEXT(x);
    }
    *data = COLOR(x);
}

static void gradient_pyramid(RrSurface *sf, gint inw, gint inh)
{
    gint x, y, w = (inw >> 1) + 1, h = (inh >> 1) + 1;
    RrPixel32 *data = sf->pixel_data;
    RrPixel32 *end = data + inw*inh - 1;
    RrPixel32 current;
    RrColor left, right;
    RrColor extracorner;

    VARS(lefty);
    VARS(righty);
    VARS(x);

    extracorner.r = (sf->primary->r + sf->secondary->r) / 2;
    extracorner.g = (sf->primary->g + sf->secondary->g) / 2;
    extracorner.b = (sf->primary->b + sf->secondary->b) / 2;

    SETUP(lefty, (&extracorner), sf->secondary, h);
    SETUP(righty, sf->primary, (&extracorner), h);

    for (y = h - 1; y > 0; --y) {  /* 0 -> h-1 */
        COLOR_RR(lefty, (&left));
        COLOR_RR(righty, (&right));

        SETUP(x, (&left), (&right), w);

        for (x = w - 1; x > 0; --x) {  /* 0 -> w-1 */
            current = COLOR(x);
            *(data+x) = current;
            *(data+inw-x) = current;
            *(end-x) = current;
            *(end-(inw-x)) = current;

            NEXT(x);
        }
        current = COLOR(x);
        *(data+x) = current;
        *(data+inw-x) = current;
        *(end-x) = current;
        *(end-(inw-x)) = current;

        data+=inw;
        end-=inw;

        NEXT(lefty);
        NEXT(righty);
    }
    COLOR_RR(lefty, (&left));
    COLOR_RR(righty, (&right));

    SETUP(x, (&left), (&right), w);

    for (x = w - 1; x > 0; --x) {  /* 0 -> w-1 */
        current = COLOR(x);
        *(data+x) = current;
        *(data+inw-x) = current;
        *(end-x) = current;
        *(end-(inw-x)) = current;
        
        NEXT(x);
    }
    current = COLOR(x);
    *(data+x) = current;
    *(data+inw-x) = current;
    *(end-x) = current;
    *(end-(inw-x)) = current;
}

