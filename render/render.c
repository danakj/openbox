#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "render.h"
#include "gradient.h"
#include "font.h"
#include "mask.h"
#include "color.h"
#include "image.h"
#include "theme.h"

#include <glib.h>

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

static void pixel_data_to_pixmap(RrAppearance *l,
                                gint x, gint y, gint w, gint h);

void RrPaint(RrAppearance *l, Window win, gint w, gint h)
{
    int i, transferred = 0, sw;
    RrPixel32 *source, *dest;
    Pixmap oldp;
    RrRect tarea; /* area in which to draw textures */
    gboolean resized;

    if (w <= 0 || h <= 0) return;

    resized = (l->w != w || l->h != h);

    oldp = l->pixmap; /* save to free after changing the visible pixmap */
    l->pixmap = XCreatePixmap(RrDisplay(l->inst),
                              RrRootWindow(l->inst),
                              w, h, RrDepth(l->inst));

    g_assert(l->pixmap != None);
    l->w = w;
    l->h = h;

    if (l->xftdraw != NULL)
        XftDrawDestroy(l->xftdraw);
    l->xftdraw = XftDrawCreate(RrDisplay(l->inst), l->pixmap,
                               RrVisual(l->inst), RrColormap(l->inst));
    g_assert(l->xftdraw != NULL);

    g_free(l->surface.pixel_data);
    l->surface.pixel_data = g_new(RrPixel32, w * h);

    if (l->surface.grad == RR_SURFACE_PARENTREL) {
        g_assert (l->surface.parent);
        g_assert (l->surface.parent->w);

        sw = l->surface.parent->w;
        source = (l->surface.parent->surface.pixel_data +
                  l->surface.parentx + sw * l->surface.parenty);
        dest = l->surface.pixel_data;
        for (i = 0; i < h; i++, source += sw, dest += w) {
            memcpy(dest, source, w * sizeof(RrPixel32));
        }
    } else
        RrRender(l, w, h);

    RECT_SET(tarea, 0, 0, w, h);
    if (l->surface.grad != RR_SURFACE_PARENTREL) {
        if (l->surface.relief != RR_RELIEF_FLAT) {
            switch (l->surface.bevel) {
            case RR_BEVEL_1:
                tarea.x += 1; tarea.y += 1;
                tarea.width -= 2; tarea.height -= 2;
                break;
            case RR_BEVEL_2:
                tarea.x += 2; tarea.y += 2;
                tarea.width -= 4; tarea.height -= 4;
                break;
            }
        } else if (l->surface.border) {
            tarea.x += 1; tarea.y += 1;
            tarea.width -= 2; tarea.height -= 2;
        }
    }

    for (i = 0; i < l->textures; i++) {
        switch (l->texture[i].type) {
        case RR_TEXTURE_NONE:
            break;
        case RR_TEXTURE_TEXT:
            if (!transferred) {
                transferred = 1;
                if (l->surface.grad != RR_SURFACE_SOLID)
                    pixel_data_to_pixmap(l, 0, 0, w, h);
            }
            if (l->xftdraw == NULL) {
                l->xftdraw = XftDrawCreate(RrDisplay(l->inst), l->pixmap, 
                                           RrVisual(l->inst),
                                           RrColormap(l->inst));
            }
            RrFontDraw(l->xftdraw, &l->texture[i].data.text, &tarea);
        break;
        case RR_TEXTURE_MASK:
            if (!transferred) {
                transferred = 1;
                if (l->surface.grad != RR_SURFACE_SOLID)
                    pixel_data_to_pixmap(l, 0, 0, w, h);
            }
            if (l->texture[i].data.mask.color->gc == None)
                RrColorAllocateGC(l->texture[i].data.mask.color);
            RrPixmapMaskDraw(l->pixmap, &l->texture[i].data.mask, &tarea);
        break;
        case RR_TEXTURE_RGBA:
            RrImageDraw(l->surface.pixel_data,
                        &l->texture[i].data.rgba, &tarea);
        break;
        }
    }

    if (!transferred) {
        transferred = 1;
        if (l->surface.grad != RR_SURFACE_SOLID)
            pixel_data_to_pixmap(l, 0, 0, w, h);
    }


    XSetWindowBackgroundPixmap(RrDisplay(l->inst), win, l->pixmap);
    XClearWindow(RrDisplay(l->inst), win);
    if (oldp) XFreePixmap(RrDisplay(l->inst), oldp);
}

RrAppearance *RrAppearanceNew(const RrInstance *inst, gint numtex)
{
  RrAppearance *out;

  out = g_new0(RrAppearance, 1);
  out->inst = inst;
  out->textures = numtex;
  if (numtex) out->texture = g_new0(RrTexture, numtex);

  return out;
}

RrAppearance *RrAppearanceCopy(RrAppearance *orig)
{
    RrSurface *spo, *spc;
    RrAppearance *copy = g_new(RrAppearance, 1);

    copy->inst = orig->inst;

    spo = &(orig->surface);
    spc = &(copy->surface);
    spc->grad = spo->grad;
    spc->relief = spo->relief;
    spc->bevel = spo->bevel;
    if (spo->primary != NULL)
        spc->primary = RrColorNew(copy->inst,
                                  spo->primary->r,
                                  spo->primary->g, 
                                  spo->primary->b);
    else spc->primary = NULL;

    if (spo->secondary != NULL)
        spc->secondary = RrColorNew(copy->inst,
                                    spo->secondary->r,
                                    spo->secondary->g,
                                    spo->secondary->b);
    else spc->secondary = NULL;

    if (spo->border_color != NULL)
        spc->border_color = RrColorNew(copy->inst,
                                       spo->border_color->r,
                                       spo->border_color->g,
                                       spo->border_color->b);
    else spc->border_color = NULL;

    if (spo->bevel_dark != NULL)
        spc->bevel_dark = RrColorNew(copy->inst,
                                     spo->bevel_dark->r,
                                     spo->bevel_dark->g,
                                     spo->bevel_dark->b);
    else spc->bevel_dark = NULL;

    if (spo->bevel_light != NULL)
        spc->bevel_light = RrColorNew(copy->inst,
                                      spo->bevel_light->r,
                                      spo->bevel_light->g,
                                      spo->bevel_light->b);
    else spc->bevel_light = NULL;

    spc->interlaced = spo->interlaced;
    spc->border = spo->border;
    spc->parent = NULL;
    spc->parentx = spc->parenty = 0;
    spc->pixel_data = NULL;

    copy->textures = orig->textures;
    copy->texture = g_memdup(orig->texture,
                             orig->textures * sizeof(RrTexture));
    copy->pixmap = None;
    copy->xftdraw = NULL;
    copy->w = copy->h = 0;
    return copy;
}

void RrAppearanceFree(RrAppearance *a)
{
    if (a) {
        RrSurface *p;
        if (a->pixmap != None) XFreePixmap(RrDisplay(a->inst), a->pixmap);
        if (a->xftdraw != NULL) XftDrawDestroy(a->xftdraw);
        if (a->textures)
            g_free(a->texture);
        p = &a->surface;
        RrColorFree(p->primary);
        RrColorFree(p->secondary);
        RrColorFree(p->border_color);
        RrColorFree(p->bevel_dark);
        RrColorFree(p->bevel_light);
        g_free(p->pixel_data);

        g_free(a);
    }
}


static void pixel_data_to_pixmap(RrAppearance *l,
                                 gint x, gint y, gint w, gint h)
{
    RrPixel32 *in, *scratch;
    Pixmap out;
    XImage *im = NULL;
    im = XCreateImage(RrDisplay(l->inst), RrVisual(l->inst), RrDepth(l->inst),
                      ZPixmap, 0, NULL, w, h, 32, 0);
    g_assert(im != NULL);

    in = l->surface.pixel_data;
    out = l->pixmap;

    im->byte_order = LSBFirst;
/* this malloc is a complete waste of time on normal 32bpp
   as reduce_depth just sets im->data = data and returns
*/
    scratch = g_new(RrPixel32, im->width * im->height);
    im->data = (char*) scratch;
    RrReduceDepth(l->inst, in, im);
    XPutImage(RrDisplay(l->inst), out,
              DefaultGC(RrDisplay(l->inst), RrScreen(l->inst)),
              im, 0, 0, x, y, w, h);
    im->data = NULL;
    XDestroyImage(im);
    g_free(scratch);
}

void RrMinsize(RrAppearance *l, gint *w, gint *h)
{
    gint i;
    gint m;
    *w = *h = 0;

    for (i = 0; i < l->textures; ++i) {
        switch (l->texture[i].type) {
        case RR_TEXTURE_NONE:
            break;
        case RR_TEXTURE_MASK:
            *w = MAX(*w, l->texture[i].data.mask.mask->width);
            *h = MAX(*h, l->texture[i].data.mask.mask->height);
            break;
        case RR_TEXTURE_TEXT:
            m = RrFontMeasureString(l->texture[i].data.text.font,
                                    l->texture[i].data.text.string);
            *w = MAX(*w, m);
            m = RrFontHeight(l->texture[i].data.text.font);
            *h += MAX(*h, m);
            break;
        case RR_TEXTURE_RGBA:
            *w += MAX(*w, l->texture[i].data.rgba.width);
            *h += MAX(*h, l->texture[i].data.rgba.height);
            break;
        }
    }

    if (l->surface.relief != RR_RELIEF_FLAT) {
        switch (l->surface.bevel) {
        case RR_BEVEL_1:
            *w += 2;
            *h += 2;
            break;
        case RR_BEVEL_2:
            *w += 4;
            *h += 4;
            break;
        }
    } else if (l->surface.border) {
        *w += 2;
        *h += 2;
    }

    if (*w < 1) *w = 1;
    if (*h < 1) *h = 1;
}

gboolean RrPixmapToRGBA(const RrInstance *inst,
                        Pixmap pmap, Pixmap mask,
                        gint *w, gint *h, RrPixel32 **data)
{
    Window xr;
    gint xx, xy;
    guint pw, ph, mw, mh, xb, xd, i, x, y, di;
    XImage *xi, *xm = NULL;

    if (!XGetGeometry(RrDisplay(inst),
                      pmap, &xr, &xx, &xy, &pw, &ph, &xb, &xd))
        return FALSE;
    if (mask) {
        if (!XGetGeometry(RrDisplay(inst), mask,
                          &xr, &xx, &xy, &mw, &mh, &xb, &xd))
            return FALSE;
        if (pw != mw || ph != mh || xd != 1)
            return FALSE;
    }

    xi = XGetImage(RrDisplay(inst), pmap,
                   0, 0, pw, ph, 0xffffffff, ZPixmap);
    if (!xi)
        return FALSE;

    if (mask) {
        xm = XGetImage(RrDisplay(inst), mask,
                       0, 0, mw, mh, 0xffffffff, ZPixmap);
        if (!xm)
            return FALSE;
    }

    *data = g_new(RrPixel32, pw * ph);
    RrIncreaseDepth(inst, *data, xi);

    if (mask) {
        /* apply transparency from the mask */
        di = 0;
        for (i = 0, y = 0; y < ph; ++y) {
            for (x = 0; x < pw; ++x, ++i) {
                if (!((((unsigned)xm->data[di + x / 8]) >> (x % 8)) & 0x1))
                    (*data)[i] &= ~(0xff << RrDefaultAlphaOffset);
            }
            di += xm->bytes_per_line;
        }
    }

    *w = pw;
    *h = ph;

    return TRUE;
}
