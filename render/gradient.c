#include "render.h"
#include "gradient.h"
#include "color.h"
#include <glib.h>

static void highlight(RrPixel32 *x, RrPixel32 *y, gboolean raised);
static void gradient_solid(RrAppearance *l, int w, int h);
static void gradient_vertical(RrSurface *sf, int w, int h);
static void gradient_horizontal(RrSurface *sf, int w, int h);
static void gradient_diagonal(RrSurface *sf, int w, int h);
static void gradient_crossdiagonal(RrSurface *sf, int w, int h);
static void gradient_pyramid(RrSurface *sf, int inw, int inh);

void RrRender(RrAppearance *a, int w, int h)
{
    RrPixel32 *data = a->surface.pixel_data;
    RrPixel32 current;
    unsigned int r,g,b;
    int off, x;

    switch (a->surface.grad) {
    case RR_SURFACE_SOLID:
        gradient_solid(a, w, h);
        break;
    case RR_SURFACE_VERTICAL:
        gradient_vertical(&a->surface, w, h);
        break;
    case RR_SURFACE_HORIZONTAL:
        gradient_horizontal(&a->surface, w, h);
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
    int r, g, b;

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
    int r, g, b;

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

static void gradient_solid(RrAppearance *l, int w, int h) 
{
    RrPixel32 pix;
    int i, a, b;
    RrSurface *sp = &l->surface;
    int left = 0, top = 0, right = w - 1, bottom = h - 1;

    pix = (sp->primary->r << RrDefaultRedOffset)
        + (sp->primary->g << RrDefaultGreenOffset)
        + (sp->primary->b << RrDefaultBlueOffset);

    for (a = 0; a < w; a++)
        for (b = 0; b < h; b++)
            sp->pixel_data[a + b * w] = pix;

    XFillRectangle(RrDisplay(l->inst), l->pixmap, RrColorGC(sp->primary),
                   0, 0, w, h);

    if (sp->interlaced) {
        for (i = 0; i < h; i += 2)
            XDrawLine(RrDisplay(l->inst), l->pixmap, RrColorGC(sp->secondary),
                      0, i, w, i);
    }

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
                      left + 1, bottom - 2, right - 2, bottom - 2);
            XDrawLine(RrDisplay(l->inst), l->pixmap, RrColorGC(sp->bevel_dark),
                      right - 2, bottom - 2, right - 2, top + 1);

            XDrawLine(RrDisplay(l->inst), l->pixmap,RrColorGC(sp->bevel_light),
                      left + 1, top + 1, right - 2, top + 1);
            XDrawLine(RrDisplay(l->inst), l->pixmap,RrColorGC(sp->bevel_light),
                      left + 1, bottom - 2, left + 1, top + 1);
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
                      left + 1, bottom - 2, right - 2, bottom - 2);
            XDrawLine(RrDisplay(l->inst), l->pixmap,RrColorGC(sp->bevel_light),
                      right - 2, bottom - 2, right - 2, top + 1);
      
            XDrawLine(RrDisplay(l->inst), l->pixmap, RrColorGC(sp->bevel_dark),
                      left + 1, top + 1, right - 2, top + 1);
            XDrawLine(RrDisplay(l->inst), l->pixmap, RrColorGC(sp->bevel_dark),
                      left + 1, bottom - 2, left + 1, top + 1);

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
    unsigned int color##x[3];                                       \
    int len##x, cdelta##x[3], error##x[3] = { 0, 0, 0 }, inc##x[3]; \
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
    int i;                                                \
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

static void gradient_horizontal(RrSurface *sf, int w, int h)
{
    int x, y;
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

static void gradient_vertical(RrSurface *sf, int w, int h)
{
    int x, y;
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


static void gradient_diagonal(RrSurface *sf, int w, int h)
{
    int x, y;
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

static void gradient_crossdiagonal(RrSurface *sf, int w, int h)
{
    int x, y;
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

static void gradient_pyramid(RrSurface *sf, int inw, int inh)
{
    int x, y, w = (inw >> 1) + 1, h = (inh >> 1) + 1;
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

