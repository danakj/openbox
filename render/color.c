#include <X11/Xlib.h>
#include <X11/Xutil.h>
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
                    p32[x] = (r << render_red_offset)
                           + (g << render_green_offset)
                           + (b << render_blue_offset);
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
