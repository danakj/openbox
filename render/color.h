#ifndef __color_h
#define __color_h

#include <X11/Xlib.h>

typedef struct color_rgb {
    int r;
    int g;
    int b;
    unsigned long pixel;
    GC gc;
} color_rgb;

void color_allocate_gc(color_rgb *in);
color_rgb *color_parse(char *colorname);
color_rgb *color_new(int r, int g, int b);
void color_free(color_rgb *in);

#endif /* __color_h */
