#ifndef __render_planar_h
#define __render_planar_h

#include "render.h"

struct RrPlanarSurface {
    enum RrSurfaceColorType colortype;
    enum RrBevelType bevel;

    struct RrColor primary;
    struct RrColor secondary;

    int borderwidth;
    struct RrColor border;
};

#define RrPlanarColorType(sur) ((sur)->data.planar.colortype)
#define RrPlanarPrimaryColor(sur) ((sur)->data.planar.primary)
#define RrPlanarSecondaryColor(sur) ((sur)->data.planar.secondary)
#define RrPlanarBevelType(sur) ((sur)->data.planar.bevel)

void RrPlanarPaint(struct RrSurface *sur, int absx, int absy);

void RrPlanarMinSize(struct RrSurface *sur, int *w, int *h);

#endif
