#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "render.h"
#include "color.h"
#include "../kernel/openbox.h"
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
    // get rgb values from colorname                                  

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

void color_free(color_rgb *c)
{
    if (c->gc != None)
        XFreeGC(ob_display, c->gc);
    free(c);
}
