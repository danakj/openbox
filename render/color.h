#ifndef __color_h
#define __color_h

#include "render.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <glib.h>

#if (G_BYTE_ORDER == G_BIG_ENDIAN)
#define default_red_offset 0
#define default_green_offset 8
#define default_blue_offset 16
#define default_alpha_offset 24
#define render_endian MSBFirst  
#else
#define default_alpha_offset 24
#define default_red_offset 16
#define default_green_offset 8
#define default_blue_offset 0
#define render_endian LSBFirst
#endif /* G_BYTE_ORDER == G_BIG_ENDIAN */

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
void reduce_depth(const RrInstance *inst, pixel32 *data, XImage *im);
void increase_depth(const RrInstance *inst, pixel32 *data, XImage *im);

#endif /* __color_h */
