#include <glib.h>
#include "render.h"
#include "instance.h"
#include "debug.h"
#include "color.h"
#include <X11/Xlib.h>

void RrColorInspect(struct RrInstance *i)
{
    unsigned long red_mask, green_mask, blue_mask;
    XImage *timage = NULL;
    timage = XCreateImage(i->display, i->visual, i->depth,
                          ZPixmap, 0, NULL, 1, 1, 32, 0);
    g_assert(timage != NULL);
    /* find the offsets for each color in the visual's masks */
    i->red_mask = red_mask = timage->red_mask;
    i->green_mask = green_mask = timage->green_mask;
    i->blue_mask = blue_mask = timage->blue_mask;
    i->red_offset = 0;
    i->green_offset = 0;
    i->blue_offset = 0;
    while (! (red_mask & 1))   { i->red_offset++;   red_mask   >>= 1; }
    while (! (green_mask & 1)) { i->green_offset++; green_mask >>= 1; }
    while (! (blue_mask & 1))  { i->blue_offset++;  blue_mask  >>= 1; }
    i->red_shift = i->green_shift = i->blue_shift = 8;
    while (red_mask)   { red_mask   >>= 1; i->red_shift--;   }
    while (green_mask) { green_mask >>= 1; i->green_shift--; }
    while (blue_mask)  { blue_mask  >>= 1; i->blue_shift--;  }
    XFree(timage);
}

int RrColorParse(struct RrInstance *inst, const char *colorname,
                 struct RrColor *ret)
{
    XColor xcol;

    if (!XParseColor(RrDisplay(inst), RrColormap(inst), colorname, &xcol)) {
        RrDebug("unable to parse color '%s'", colorname);
        ret->r = 0.0;
        ret->g = 0.0;
        ret->b = 0.0;
        ret->a = 0.0;
        return 0;
    }
    ret->r = (xcol.red >> 8) / 255.0;
    ret->g = (xcol.green >> 8) / 255.0;
    ret->b = (xcol.blue >> 8) / 255.0;
    ret->a = 0.0;
    return 1;
}

static void swap_byte_order(XImage *im)
{
    int x, y, di;

    g_message("SWAPPING BYTE ORDER");

    di = 0;
    for (y = 0; y < im->height; ++y) {
        for (x = 0; x < im->height; ++x) {
            char *c = &im->data[di + x * im->bits_per_pixel / 8];
            char t;

            switch (im->bits_per_pixel) {
            case 32:
                t = c[2];
                c[2] = c[3];
                c[3] = t;
            case 16:
                t = c[0];
                c[0] = c[1];
                c[1] = t;
            case 8:
                break;
            default:
                g_message("your bit depth is currently unhandled\n");
            }
        }
        di += im->bytes_per_line;
    }

    if (im->byte_order == LSBFirst)
        im->byte_order = MSBFirst;
    else
        im->byte_order = LSBFirst;
}

void RrIncreaseDepth(struct RrInstance *i, RrData32 *data, XImage *im)
{
    int r, g, b;
    int x,y;
    RrData32 *p32 = (RrData32 *) im->data;
    guint16 *p16 = (guint16 *) im->data;
    unsigned char *p8 = (unsigned char *)im->data;

    if (im->byte_order != render_endian)
        swap_byte_order(im);

    switch (im->bits_per_pixel) {
    case 32:
        for (y = 0; y < im->height; y++) {
            for (x = 0; x < im->width; x++) {
                r = (p32[x] >> i->red_offset) & 0xff;
                g = (p32[x] >> i->green_offset) & 0xff;
                b = (p32[x] >> i->blue_offset) & 0xff;
                data[x] = (r << default_red_offset)
                    + (g << default_green_offset)
                    + (b << default_blue_offset)
                    + (0xff << default_alpha_offset);
            }
            data += im->width;
            p32 += im->bytes_per_line/4;
        }
        break;
    case 16:
        for (y = 0; y < im->height; y++) {
            for (x = 0; x < im->width; x++) {
                r = (p16[x] & i->red_mask) >> i->red_offset <<
                    i->red_shift;
                g = (p16[x] & i->green_mask) >> i->green_offset <<
                    i->green_shift;
                b = (p16[x] & i->blue_mask) >> i->blue_offset <<
                    i->blue_shift;
                data[x] = (r << default_red_offset)
                    + (g << default_green_offset)
                    + (b << default_blue_offset)
                    + (0xff << default_alpha_offset);
            }
            data += im->width;
            p16 += im->bytes_per_line/2;
        }
        break;
    case 8:
        g_message("this image bit depth is currently unhandled\n");
        break;
    case 1:
        for (y = 0; y < im->height; y++) {
            for (x = 0; x < im->width; x++) {
                if (!(((p8[x / 8]) >> (x % 8)) & 0x1))
                    data[x] = 0xff << default_alpha_offset; /* black */
                else
                    data[x] = 0xffffffff; /* white */
            }
            data += im->width;
            p8 += im->bytes_per_line;
        }
        break;
    default:
        g_message("this image bit depth is currently unhandled\n");
    }
}


gboolean RrPixmapToRGBA(struct RrInstance *inst, Pixmap pmap, 
                               Pixmap mask, int *w, int *h, RrData32 **data)
{
    Window xr;
    int xx, xy;
    guint pw, ph, mw, mh, xb, xd, i, x, y, di;
    XImage *xi, *xm = NULL;
    if (!XGetGeometry(inst->display, pmap, &xr, &xx, &xy, &pw, &ph, &xb, &xd))
        return FALSE;
    if (mask) {
        if (!XGetGeometry(inst->display, mask,
                          &xr, &xx, &xy, &mw, &mh, &xb, &xd))
            return FALSE;
        if (pw != mw || ph != mh || xd != 1)
            return FALSE;
    }
    xi = XGetImage(inst->display, pmap, 0, 0, pw, ph, 0xffffffff, ZPixmap);
    if (!xi)
        return FALSE;
    if (mask) {
        xm = XGetImage(inst->display, mask, 0, 0, mw, mh, 0xffffffff, ZPixmap);
        if (!xm)
            return FALSE;
    }
    *data = g_new(RrData32, pw * ph);
    RrIncreaseDepth(inst, *data, xi);
    if (mask) {
        /* apply transparency from the mask */
        di = 0;
        for (i = 0, y = 0; y < ph; ++y) {
            for (x = 0; x < pw; ++x, ++i) {
                if (!((((unsigned)xm->data[di + x / 8]) >> (x % 8)) & 0x1))
                    (*data)[i] &= ~(0xff << default_alpha_offset);
            }
            di += xm->bytes_per_line;
        }
    }
    *w = pw;
    *h = ph;
    return TRUE;
}
