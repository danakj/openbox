#ifndef __color_h
#define __color_h

#include "render.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <glib.h>

struct _RrColor {
    const RrInstance *inst;

    int r;
    int g;
    int b;
    unsigned long pixel;
    GC gc;
};

void color_allocate_gc(RrColor *in);
XColor *pickColor(const RrInstance *inst, gint r, gint g, gint b);
void reduce_depth(const RrInstance *inst, RrPixel32 *data, XImage *im);
void increase_depth(const RrInstance *inst, RrPixel32 *data, XImage *im);

#endif /* __color_h */
