#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>
#include "render.h"
#include "color.h"
#include "../kernel/openbox.h"

XColor *pseudo_colors;
int pseudo_bpc;

void color_allocate_gc(color_rgb *in)
{
    XGCValues gcv;

    gcv.foreground = in->pixel;
    gcv.cap_style = CapProjecting;
    in->gc = XCreateGC(ob_display, ob_root, GCForeground | GCCapStyle, &gcv);
}

color_rgb *color_parse(char *colorname)
{
    XColor xcol;

    g_assert(colorname != NULL);
    /* get rgb values from colorname */

    xcol.red = 0;
    xcol.green = 0;
    xcol.blue = 0;
    xcol.pixel = 0;
    if (!XParseColor(ob_display, render_colormap, colorname, &xcol)) {
        g_warning("unable to parse color '%s'", colorname);
	return NULL;
    }
    return color_new(xcol.red >> 8, xcol.green >> 8, xcol.blue >> 8);
}

color_rgb *color_new(int r, int g, int b)
{
/* this should be replaced with something far cooler */
    color_rgb *out;
    XColor xcol;
    xcol.red = (r << 8) | r;
    xcol.green = (g << 8) | g;
    xcol.blue = (b << 8) | b;
    if (XAllocColor(ob_display, render_colormap, &xcol)) {
        out = g_new(color_rgb, 1);
        out->r = xcol.red >> 8;
        out->g = xcol.green >> 8;
        out->b = xcol.blue >> 8;
        out->gc = None;
        out->pixel = xcol.pixel;
        return out;
    }
    return NULL;
}

/*XXX same color could be pointed to twice, this might have to be a refcount*/

void color_free(color_rgb *c)
{
    if (c != NULL) {
        if (c->gc != None)
            XFreeGC(ob_display, c->gc);
        g_free(c);
    }
}

void reduce_depth(pixel32 *data, XImage *im)
{
    int r, g, b;
    int x,y;
    pixel32 *p32 = (pixel32 *) im->data;
    pixel16 *p16 = (pixel16 *) im->data;
    unsigned char *p8 = (unsigned char *)im->data;
    switch (im->bits_per_pixel) {
    case 32:
        if ((render_red_offset != default_red_offset) ||
            (render_blue_offset != default_blue_offset) ||
            (render_green_offset != default_green_offset)) {
            for (y = 0; y < im->height; y++) {
                for (x = 0; x < im->width; x++) {
                    r = (data[x] >> default_red_offset) & 0xFF;
                    g = (data[x] >> default_green_offset) & 0xFF;
                    b = (data[x] >> default_blue_offset) & 0xFF;
                    p32[x] = (r << render_red_shift)
                           + (g << render_green_shift)
                           + (b << render_blue_shift);
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
                r = r >> render_red_shift;
                g = (data[x] >> default_green_offset) & 0xFF;
                g = g >> render_green_shift;
                b = (data[x] >> default_blue_offset) & 0xFF;
                b = b >> render_blue_shift;
                p16[x] = (r << render_red_offset)
                    + (g << render_green_offset)
                    + (b << render_blue_offset);
            }
            data += im->width;
            p16 += im->bytes_per_line/2;
        }
    break;
    case 8:
        g_assert(render_visual->class != TrueColor);
        for (y = 0; y < im->height; y++) {
            for (x = 0; x < im->width; x++) {
                p8[x] = pickColor(data[x] >> default_red_offset,
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

XColor *pickColor(int r, int g, int b) 
{
  r = (r & 0xff) >> (8-pseudo_bpc);
  g = (g & 0xff) >> (8-pseudo_bpc);
  b = (b & 0xff) >> (8-pseudo_bpc);
  return &pseudo_colors[(r << (2*pseudo_bpc)) + (g << (1*pseudo_bpc)) + b];
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

void increase_depth(pixel32 *data, XImage *im)
{
    int r, g, b;
    int x,y;
    pixel32 *p32 = (pixel32 *) im->data;
    pixel16 *p16 = (pixel16 *) im->data;
    unsigned char *p8 = (unsigned char *)im->data;

    if (im->byte_order != render_endian)
        swap_byte_order(im);

    switch (im->bits_per_pixel) {
    case 32:
        for (y = 0; y < im->height; y++) {
            for (x = 0; x < im->width; x++) {
                r = (p32[x] >> render_red_offset) & 0xff;
                g = (p32[x] >> render_green_offset) & 0xff;
                b = (p32[x] >> render_blue_offset) & 0xff;
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
                r = (p16[x] & render_red_mask) >> render_red_offset <<
                    render_red_shift;
                g = (p16[x] & render_green_mask) >> render_green_offset <<
                    render_green_shift;
                b = (p16[x] & render_blue_mask) >> render_blue_offset <<
                    render_blue_shift;
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
