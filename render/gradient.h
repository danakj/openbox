#ifndef __gradient_h
#define __gradient_h

#include "render.h"

void gradient_render(Surface *sf, int w, int h);
void gradient_vertical(Surface *sf, int w, int h);
void gradient_horizontal(Surface *sf, int w, int h);
void gradient_diagonal(Surface *sf, int w, int h);
void gradient_crossdiagonal(Surface *sf, int w, int h);
void gradient_solid(Appearance *l, int w, int h); /* needs access to pixmap */
void highlight(pixel32 *x, pixel32 *y, gboolean raised);

#endif /* __gradient_h */
