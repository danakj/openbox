#ifndef __gradient_h
#define __gradient_h

#include "render.h"

void gradient_render(RrSurface *sf, int w, int h);
void gradient_vertical(RrSurface *sf, int w, int h);
void gradient_horizontal(RrSurface *sf, int w, int h);
void gradient_diagonal(RrSurface *sf, int w, int h);
void gradient_crossdiagonal(RrSurface *sf, int w, int h);
void gradient_pyramid(RrSurface *sf, int w, int h);
void gradient_pipecross(RrSurface *sf, int w, int h);
void gradient_rectangle(RrSurface *sf, int w, int h);
void gradient_solid(RrAppearance *l, int x, int y, int w, int h);
void highlight(RrPixel32 *x, RrPixel32 *y, gboolean raised);

void render_gl_gradient(RrSurface *sf, int x, int y, int w, int h);

#endif /* __gradient_h */
