#ifndef __color_h
#define __color_h

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef HAVE_STDINT_H 
#  include <stdint.h>
#else
#  ifdef HAVE_SYS_TYPES_H
#    include <sys/types.h>
#  endif   
#endif


#ifdef HAVE_STDINT_H
typedef uint32_t pixel32;
typedef uint16_t pixel16;
#else
typedef u_int32_t pixel32;  
typedef u_int16_t pixel16;   
#endif /* HAVE_STDINT_H */  

#if (G_BYTE_ORDER == G_BIG_ENDIAN)
#define default_red_shift 0
#define default_green_shift 8
#define default_blue_shift 16
#define endian MSBFirst  
#else
#define default_red_shift 16
#define default_green_shift 8
#define default_blue_shift 0
#define endian LSBFirst
#endif /* G_BYTE_ORDER == G_BIG_ENDIAN */


typedef struct color_rgb {
    int r;
    int g;
    int b;
    unsigned long pixel;
    GC gc;
} color_rgb;

void color_allocate_gc(color_rgb *in);
XColor *pickColor(int r, int g, int b);
color_rgb *color_parse(char *colorname);
color_rgb *color_new(int r, int g, int b);
void color_free(color_rgb *in);
void reduce_depth(pixel32 *data, XImage *im);

extern int render_red_offset;
extern int render_green_offset;
extern int render_blue_offset;

extern int render_red_shift;
extern int render_green_shift;
extern int render_blue_shift;

extern int pseudo_bpc;
extern XColor *pseudo_colors;
#endif /* __color_h */
