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
static void gradient_rectangle(RrSurface *sf, int inw, int inh);
static void gradient_pipecross(RrSurface *sf, int inw, int inh);

void RrRender(RrAppearance *a, int w, int h)
{
    RrPixel32 *data = a->surface.RrPixel_data;
    RrPixel32 current;
    unsigned int r,g,b;
    int off, x;

    switch (a->surface.grad) {
    case RR_SURFACE_SOLID:
        gradient_solid(a, w, h);
        return;
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
    case RR_SURFACE_PIPECROSS:
        gradient_pipecross(&a->surface, w, h);
        break;
    case RR_SURFACE_RECTANGLE:
        gradient_rectangle(&a->surface, w, h);
        break;
    default:
        g_message("unhandled gradient");
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



static void gradient_vertical(RrSurface *sf, int w, int h)
{
    RrPixel32 *data = sf->RrPixel_data;
    RrPixel32 current;
    float dr, dg, db;
    unsigned int r,g,b;
    int x, y;

    dr = (float)(sf->secondary->r - sf->primary->r);
    dr/= (float)h;

    dg = (float)(sf->secondary->g - sf->primary->g);
    dg/= (float)h;

    db = (float)(sf->secondary->b - sf->primary->b);
    db/= (float)h;

    for (y = 0; y < h; ++y) {
        r = sf->primary->r + (int)(dr * y);
        g = sf->primary->g + (int)(dg * y);
        b = sf->primary->b + (int)(db * y);
        current = (r << RrDefaultRedOffset)
            + (g << RrDefaultGreenOffset)
            + (b << RrDefaultBlueOffset);
        for (x = 0; x < w; ++x, ++data)
            *data = current;
    }
}

static void gradient_horizontal(RrSurface *sf, int w, int h)
{
    RrPixel32 *data = sf->RrPixel_data;
    RrPixel32 current;
    float dr, dg, db;
    unsigned int r,g,b;
    int x, y;

    dr = (float)(sf->secondary->r - sf->primary->r);
    dr/= (float)w;

    dg = (float)(sf->secondary->g - sf->primary->g);
    dg/= (float)w;

    db = (float)(sf->secondary->b - sf->primary->b);
    db/= (float)w;

    for (x = 0; x < w; ++x, ++data) {
        r = sf->primary->r + (int)(dr * x);
        g = sf->primary->g + (int)(dg * x);
        b = sf->primary->b + (int)(db * x);
        current = (r << RrDefaultRedOffset)
            + (g << RrDefaultGreenOffset)
            + (b << RrDefaultBlueOffset);
        for (y = 0; y < h; ++y)
            *(data + y*w) = current;
    }
}

static void gradient_diagonal(RrSurface *sf, int w, int h)
{
    RrPixel32 *data = sf->RrPixel_data;
    RrPixel32 current;
    float drx, dgx, dbx, dry, dgy, dby;
    unsigned int r,g,b;
    int x, y;

    for (y = 0; y < h; ++y) {
        drx = (float)(sf->secondary->r -
                      sf->primary->r);
        dry = drx/(float)h;
        drx/= (float)w;

        dgx = (float)(sf->secondary->g -
                      sf->primary->g);
        dgy = dgx/(float)h;
        dgx/= (float)w;

        dbx = (float)(sf->secondary->b -
                      sf->primary->b);
        dby = dbx/(float)h;
        dbx/= (float)w;
        for (x = 0; x < w; ++x, ++data) {
            r = sf->primary->r +
                ((int)(drx * x) + (int)(dry * y))/2;
            g = sf->primary->g +
                ((int)(dgx * x) + (int)(dgy * y))/2;
            b = sf->primary->b +
                ((int)(dbx * x) + (int)(dby * y))/2;
            current = (r << RrDefaultRedOffset)
                + (g << RrDefaultGreenOffset)
                + (b << RrDefaultBlueOffset);
            *data = current;
        }
    }
}

static void gradient_crossdiagonal(RrSurface *sf, int w, int h)
{
    RrPixel32 *data = sf->RrPixel_data;
    RrPixel32 current;
    float drx, dgx, dbx, dry, dgy, dby;
    unsigned int r,g,b;
    int x, y;

    for (y = 0; y < h; ++y) {
        drx = (float)(sf->secondary->r -
                      sf->primary->r);
        dry = drx/(float)h;
        drx/= (float)w;

        dgx = (float)(sf->secondary->g -
                      sf->primary->g);
        dgy = dgx/(float)h;
        dgx/= (float)w;

        dbx = (float)(sf->secondary->b -
                      sf->primary->b);
        dby = dbx/(float)h;
        dbx/= (float)w;
        for (x = w; x > 0; --x, ++data) {
            r = sf->primary->r +
                ((int)(drx * (x-1)) + (int)(dry * y))/2;
            g = sf->primary->g +
                ((int)(dgx * (x-1)) + (int)(dgy * y))/2;
            b = sf->primary->b +
                ((int)(dbx * (x-1)) + (int)(dby * y))/2;
            current = (r << RrDefaultRedOffset)
                + (g << RrDefaultGreenOffset)
                + (b << RrDefaultBlueOffset);
            *data = current;
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
    RrColorAllocateGC(l->surface.bevel_light);

    /* dark color */
    r = l->surface.primary->r;
    r = (r >> 1) + (r >> 2);
    g = l->surface.primary->g;
    g = (g >> 1) + (g >> 2);
    b = l->surface.primary->b;
    b = (b >> 1) + (b >> 2);
    g_assert(!l->surface.bevel_dark);
    l->surface.bevel_dark = RrColorNew(l->inst, r, g, b);
    RrColorAllocateGC(l->surface.bevel_dark);
}

static void gradient_solid(RrAppearance *l, int w, int h) 
{
    RrPixel32 pix;
    int i, a, b;
    RrSurface *sp = &l->surface;
    int left = 0, top = 0, right = w - 1, bottom = h - 1;

    if (sp->primary->gc == None)
        RrColorAllocateGC(sp->primary);
    pix = (sp->primary->r << RrDefaultRedOffset)
        + (sp->primary->g << RrDefaultGreenOffset)
        + (sp->primary->b << RrDefaultBlueOffset);

    for (a = 0; a < w; a++)
        for (b = 0; b < h; b++)
            sp->RrPixel_data[a + b * w] = pix;

    XFillRectangle(RrDisplay(l->inst), l->pixmap, sp->primary->gc,
                   0, 0, w, h);

    if (sp->interlaced) {
        if (sp->secondary->gc == None)
            RrColorAllocateGC(sp->secondary);
        for (i = 0; i < h; i += 2)
            XDrawLine(RrDisplay(l->inst), l->pixmap, sp->secondary->gc,
                      0, i, w, i);
    }

    switch (sp->relief) {
    case RR_RELIEF_RAISED:
        if (!sp->bevel_dark)
            create_bevel_colors(l);

        switch (sp->bevel) {
        case RR_BEVEL_1:
            XDrawLine(RrDisplay(l->inst), l->pixmap, sp->bevel_dark->gc,
                      left, bottom, right, bottom);
            XDrawLine(RrDisplay(l->inst), l->pixmap, sp->bevel_dark->gc,
                      right, bottom, right, top);
                
            XDrawLine(RrDisplay(l->inst), l->pixmap, sp->bevel_light->gc,
                      left, top, right, top);
            XDrawLine(RrDisplay(l->inst), l->pixmap, sp->bevel_light->gc,
                      left, bottom, left, top);
            break;
        case RR_BEVEL_2:
            XDrawLine(RrDisplay(l->inst), l->pixmap,
                      sp->bevel_dark->gc,
                      left + 1, bottom - 2, right - 2, bottom - 2);
            XDrawLine(RrDisplay(l->inst), l->pixmap,
                      sp->bevel_dark->gc,
                      right - 2, bottom - 2, right - 2, top + 1);

            XDrawLine(RrDisplay(l->inst), l->pixmap,
                      sp->bevel_light->gc,
                      left + 1, top + 1, right - 2, top + 1);
            XDrawLine(RrDisplay(l->inst), l->pixmap,
                      sp->bevel_light->gc,
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
            XDrawLine(RrDisplay(l->inst), l->pixmap, sp->bevel_light->gc,
                      left, bottom, right, bottom);
            XDrawLine(RrDisplay(l->inst), l->pixmap, sp->bevel_light->gc,
                      right, bottom, right, top);
      
            XDrawLine(RrDisplay(l->inst), l->pixmap, sp->bevel_dark->gc,
                      left, top, right, top);
            XDrawLine(RrDisplay(l->inst), l->pixmap, sp->bevel_dark->gc,
                      left, bottom, left, top);
            break;
        case RR_BEVEL_2:
            XDrawLine(RrDisplay(l->inst), l->pixmap, sp->bevel_light->gc,
                      left + 1, bottom - 2, right - 2, bottom - 2);
            XDrawLine(RrDisplay(l->inst), l->pixmap, sp->bevel_light->gc,
                      right - 2, bottom - 2, right - 2, top + 1);
      
            XDrawLine(RrDisplay(l->inst), l->pixmap, sp->bevel_dark->gc,
                      left + 1, top + 1, right - 2, top + 1);
            XDrawLine(RrDisplay(l->inst), l->pixmap, sp->bevel_dark->gc,
                      left + 1, bottom - 2, left + 1, top + 1);

            break;
        default:
            g_assert_not_reached(); /* unhandled BevelType */
        }
        break;
    case RR_RELIEF_FLAT:
        if (sp->border) {
            if (sp->border_color->gc == None)
                RrColorAllocateGC(sp->border_color);
            XDrawRectangle(RrDisplay(l->inst), l->pixmap, sp->border_color->gc,
                           left, top, right, bottom);
        }
        break;
    default:  
        g_assert_not_reached(); /* unhandled ReliefType */
    }
}

static void gradient_pyramid(RrSurface *sf, int inw, int inh)
{
    RrPixel32 *data = sf->RrPixel_data;
    RrPixel32 *end = data + inw*inh - 1;
    RrPixel32 current;
    float drx, dgx, dbx, dry, dgy, dby;
    unsigned int r,g,b;
    int x, y, h=(inh/2) + 1, w=(inw/2) + 1;

    drx = (float)(sf->secondary->r -
                  sf->primary->r);
    dry = drx/(float)h;
    drx/= (float)w;

    dgx = (float)(sf->secondary->g -
                  sf->primary->g);
    dgy = dgx/(float)h;
    dgx/= (float)w;

    dbx = (float)(sf->secondary->b -
                  sf->primary->b);
    dby = dbx/(float)h;
    dbx/= (float)w;

    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x, data) {
            r = sf->primary->r +
                ((int)(drx * x) + (int)(dry * y))/2;
            g = sf->primary->g +
                ((int)(dgx * x) + (int)(dgy * y))/2;
            b = sf->primary->b +
                ((int)(dbx * x) + (int)(dby * y))/2;
            current = (r << RrDefaultRedOffset)
                + (g << RrDefaultGreenOffset)
                + (b << RrDefaultBlueOffset);
            *(data+x) = current;
            *(data+inw-x) = current;
            *(end-x) = current;
            *(end-(inw-x)) = current;
        }
        data+=inw;
        end-=inw;
    }
}

static void gradient_rectangle(RrSurface *sf, int inw, int inh)
{
    RrPixel32 *data = sf->RrPixel_data;
    RrPixel32 *end = data + inw*inh - 1;
    RrPixel32 current;
    float drx, dgx, dbx, dry, dgy, dby;
    unsigned int r,g,b;
    int x, y, h=(inh/2) + 1, w=(inw/2) + 1;

    drx = (float)(sf->primary->r -
                  sf->secondary->r);
    dry = drx/(float)h;
    drx/= (float)w;

    dgx = (float)(sf->primary->g -
                  sf->secondary->g);
    dgy = dgx/(float)h;
    dgx/= (float)w;

    dbx = (float)(sf->primary->b -
                  sf->secondary->b);
    dby = dbx/(float)h;
    dbx/= (float)w;

    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x, data) {
            if ((float)x/(float)w < (float)y/(float)h) {
                r = sf->primary->r + (drx * x);
                g = sf->primary->g + (dgx * x);
                b = sf->primary->b + (dbx * x);
            } else {
                r = sf->primary->r + (dry * x);
                g = sf->primary->g + (dgy * x);
                b = sf->primary->b + (dby * x);
            }
            current = (r << RrDefaultRedOffset)
                + (g << RrDefaultGreenOffset)
                + (b << RrDefaultBlueOffset);
            *(data+x) = current;
            *(data+inw-x) = current;
            *(end-x) = current;
            *(end-(inw-x)) = current;
        }
        data+=inw;
        end-=inw;
    }
}

static void gradient_pipecross(RrSurface *sf, int inw, int inh)
{
    RrPixel32 *data = sf->RrPixel_data;
    RrPixel32 *end = data + inw*inh - 1;
    RrPixel32 current;
    float drx, dgx, dbx, dry, dgy, dby;
    unsigned int r,g,b;
    int x, y, h=(inh/2) + 1, w=(inw/2) + 1;

    drx = (float)(sf->secondary->r -
                  sf->primary->r);
    dry = drx/(float)h;
    drx/= (float)w;

    dgx = (float)(sf->secondary->g -
                  sf->primary->g);
    dgy = dgx/(float)h;
    dgx/= (float)w;

    dbx = (float)(sf->secondary->b -
                  sf->primary->b);
    dby = dbx/(float)h;
    dbx/= (float)w;

    for (y = 0; y < h; ++y) {
        for (x = 0; x < w; ++x, data) {
            if ((float)x/(float)w > (float)y/(float)h) {
                r = sf->primary->r + (drx * x);
                g = sf->primary->g + (dgx * x);
                b = sf->primary->b + (dbx * x);
            } else {
                r = sf->primary->r + (dry * x);
                g = sf->primary->g + (dgy * x);
                b = sf->primary->b + (dby * x);
            }
            current = (r << RrDefaultRedOffset)
                + (g << RrDefaultGreenOffset)
                + (b << RrDefaultBlueOffset);
            *(data+x) = current;
            *(data+inw-x) = current;
            *(end-x) = current;
            *(end-(inw-x)) = current;
        }
        data+=inw;
        end-=inw;
    }
}
