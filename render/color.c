#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>
#include "render.h"
#include "color.h"

XColor *pseudo_colors;
int pseudo_bpc;

void color_allocate_gc(RrColor *in)
{
    XGCValues gcv;

    gcv.foreground = in->pixel;
    gcv.cap_style = CapProjecting;
    in->gc = XCreateGC(RrDisplay(in->inst),
                       RrRootWindow(in->inst),
                       GCForeground | GCCapStyle, &gcv);
}

RrColor *RrColorParse(const RrInstance *inst, gchar *colorname)
{
    XColor xcol;

    g_assert(colorname != NULL);
    /* get rgb values from colorname */

    xcol.red = 0;
    xcol.green = 0;
    xcol.blue = 0;
    xcol.pixel = 0;
    if (!XParseColor(RrDisplay(inst), RrColormap(inst), colorname, &xcol)) {
        g_warning("unable to parse color '%s'", colorname);
	return NULL;
    }
    return RrColorNew(inst, xcol.red >> 8, xcol.green >> 8, xcol.blue >> 8);
}

RrColor *RrColorNew(const RrInstance *inst, gint r, gint g, gint b)
{
    /* this should be replaced with something far cooler */
    RrColor *out = NULL;
    XColor xcol;
    xcol.red = (r << 8) | r;
    xcol.green = (g << 8) | g;
    xcol.blue = (b << 8) | b;
    if (XAllocColor(RrDisplay(inst), RrColormap(inst), &xcol)) {
        out = g_new(RrColor, 1);
        out->inst = inst;
        out->r = xcol.red >> 8;
        out->g = xcol.green >> 8;
        out->b = xcol.blue >> 8;
        out->gc = None;
        out->pixel = xcol.pixel;
    }
    return out;
}

/*XXX same color could be pointed to twice, this might have to be a refcount*/

void RrColorFree(RrColor *c)
{
    if (c) {
        if (c->gc) XFreeGC(RrDisplay(c->inst), c->gc);
        g_free(c);
    }
}

void reduce_depth(const RrInstance *inst, RrPixel32 *data, XImage *im)
{
    int r, g, b;
    int x,y;
    RrPixel32 *p32 = (RrPixel32 *) im->data;
    RrPixel16 *p16 = (RrPixel16 *) im->data;
    unsigned char *p8 = (unsigned char *)im->data;
    switch (im->bits_per_pixel) {
    case 32:
        if ((RrRedOffset(inst) != default_red_offset) ||
            (RrBlueOffset(inst) != default_blue_offset) ||
            (RrGreenOffset(inst) != default_green_offset)) {
            for (y = 0; y < im->height; y++) {
                for (x = 0; x < im->width; x++) {
                    r = (data[x] >> default_red_offset) & 0xFF;
                    g = (data[x] >> default_green_offset) & 0xFF;
                    b = (data[x] >> default_blue_offset) & 0xFF;
                    p32[x] = (r << RrRedShift(inst))
                           + (g << RrGreenShift(inst))
                           + (b << RrBlueShift(inst));
                }
                data += im->width;
                p32 += im->width;
            } 
        } else im->data = (char*) data;
        break;
    case 16:
        for (y = 0; y < im->height; y++) {
            for (x = 0; x < im->width; x++) {
                r = (data[x] >> default_red_offset) & 0xFF;
                r = r >> RrRedShift(inst);
                g = (data[x] >> default_green_offset) & 0xFF;
                g = g >> RrGreenShift(inst);
                b = (data[x] >> default_blue_offset) & 0xFF;
                b = b >> RrBlueShift(inst);
                p16[x] = (r << RrRedOffset(inst))
                       + (g << RrGreenOffset(inst))
                       + (b << RrBlueOffset(inst));
            }
            data += im->width;
            p16 += im->bytes_per_line/2;
        }
    break;
    case 8:
        g_assert(RrVisual(inst)->class != TrueColor);
        for (y = 0; y < im->height; y++) {
            for (x = 0; x < im->width; x++) {
                p8[x] = pickColor(inst,
                                  data[x] >> default_red_offset,
                                  data[x] >> default_green_offset,
                                  data[x] >> default_blue_offset)->pixel;
        }
        data += im->width;
        p8 += im->bytes_per_line;
  }

    break;
    default:
        g_message("your bit depth is currently unhandled\n");
    }
}

XColor *pickColor(const RrInstance *inst, gint r, gint g, gint b) 
{
  r = (r & 0xff) >> (8-pseudo_bpc);
  g = (g & 0xff) >> (8-pseudo_bpc);
  b = (b & 0xff) >> (8-pseudo_bpc);
  return &RrPseudoColors(inst)[(r << (2*pseudo_bpc)) +
                               (g << (1*pseudo_bpc)) +
                               b];
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

void increase_depth(const RrInstance *inst, RrPixel32 *data, XImage *im)
{
    int r, g, b;
    int x,y;
    RrPixel32 *p32 = (RrPixel32 *) im->data;
    RrPixel16 *p16 = (RrPixel16 *) im->data;
    unsigned char *p8 = (unsigned char *)im->data;

    if (im->byte_order != render_endian)
        swap_byte_order(im);

    switch (im->bits_per_pixel) {
    case 32:
        for (y = 0; y < im->height; y++) {
            for (x = 0; x < im->width; x++) {
                r = (p32[x] >> RrRedOffset(inst)) & 0xff;
                g = (p32[x] >> RrGreenOffset(inst)) & 0xff;
                b = (p32[x] >> RrBlueOffset(inst)) & 0xff;
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
                r = (p16[x] & RrRedMask(inst)) >>
                    RrRedOffset(inst) <<
                    RrRedShift(inst);
                g = (p16[x] & RrGreenMask(inst)) >>
                    RrGreenOffset(inst) <<
                    RrGreenShift(inst);
                b = (p16[x] & RrBlueMask(inst)) >>
                    RrBlueOffset(inst) <<
                    RrBlueShift(inst);
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
