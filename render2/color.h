#ifndef __render_color_h
#define __render_color_h

#include "render.h"

/*! Sets up color masks for grabbing pixmaps from the server*/
void RrColorInspect(struct RrInstance *i);

/*! Returns if an RrColor is non-opaque */
#define RrColorHasAlpha(c) ((c).a < 0.999999999)

#define RrColor3f(c) glColor3f((c)->r, (c)->g, (c)->b)
#define RrColor4f(c) glColor4f((c)->r, (c)->g, (c)->b, (c)->a)

#define RrColorAvg(avg, c1, c2) \
    RrColorSet((avg), \
               ((c1)->r + (c2)->r) / 2.0, \
               ((c1)->g + (c2)->g) / 2.0, \
               ((c1)->b + (c2)->b) / 2.0, \
               ((c1)->a + (c2)->a) / 2.0)

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

#endif
