#ifndef __render_planar_h
#define __render_planar_h

#include "render.h"

struct RrPlanarSurface {
    enum RrSurfaceColorType colortype;

    struct RrColor primary;
    struct RrColor secondary;
};

#define RrPlanarColorType(sur) ((sur)->data.planar.colortype)
#define RrPlanarPrimaryColor(sur) ((sur)->data.planar.primary)
#define RrPlanarSecondaryColor(sur) ((sur)->data.planar.secondary)

void RrPlanarPaint(struct RrSurface *sur, int x, int y, int w, int h);

void RrPlanarMinSize(struct RrSurface *sur, int *w, int *h);

#endif
