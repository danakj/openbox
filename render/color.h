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


struct RrRGB {
    float r,g,b;
};

#define rr_color_set(col, x, y, z) (col).r = (x), (col).g = (y), (col).b = (z)

gboolean color_parse(char *colorname, struct RrRGB *ret);
void reduce_depth(pixel32 *data, XImage *im);
void increase_depth(pixel32 *data, XImage *im);

extern int render_red_offset;
extern int render_green_offset;
extern int render_blue_offset;

extern int render_red_shift;
extern int render_green_shift;
extern int render_blue_shift;

extern int render_red_mask;
extern int render_green_mask;
extern int render_blue_mask;

#endif /* __color_h */
