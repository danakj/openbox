#ifndef __render_surface_h
#define __render_surface_h

#include "render.h"

union RrTextureData {
    int foo;
};

struct RrTexture {
    enum RrTextureType type;
    union RrTextureData data;
};

struct RrPlanarSurface {
    enum RrSurfaceColorType colortype;

    struct RrColor primary;
    struct RrColor secondary;
};

struct RrNonPlanarSurface {
    int foo;
};

union RrSurfaceData {
    struct RrPlanarSurface planar;
    struct RrNonPlanarSurface nonplanar;
};

struct RrSurface {
    struct RrInstance *inst;

    enum RrSurfaceType type;
    union RrSurfaceData data;

    Window win; /* this can optionally be None if parent != NULL ... */

    int ntextures;
    struct RrTexture *texture;

    struct RrSurface *parent;
    int parentx;
    int parenty;
};

#endif
