#ifndef __render_color_h
#define __render_color_h

#include "render.h"

#define RrColor3f(c) glColor3f((c)->r, (c)->g, (c)->b)
#define RrColor4f(c) glColor4f((c)->r, (c)->g, (c)->b, (c)->a)

#define RrColorAvg(avg, c1, c2) \
    RrColorSet((avg), \
               ((c1)->r + (c2)->r) / 2.0, \
               ((c1)->g + (c2)->g) / 2.0, \
               ((c1)->b + (c2)->b) / 2.0, \
               ((c1)->a + (c2)->a) / 2.0)


#endif
