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

    gint key;
    gint refcount;
};

void RrColorAllocateGC(RrColor *in);
XColor *RrPickColor(const RrInstance *inst, gint r, gint g, gint b);
void RrReduceDepth(const RrInstance *inst, RrPixel32 *data, XImage *im);
void RrIncreaseDepth(const RrInstance *inst, RrPixel32 *data, XImage *im);

#endif /* __color_h */
