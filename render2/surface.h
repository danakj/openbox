#ifndef __render_surface_h
#define __render_surface_h

#include "render.h"
#include "texture.h"

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

    /* This member is created inside Render if parent != NULL, but is passed
       in if parent == NULL and should not be destroyed!

       Always check for this to be None before rendering it. Just skip by
       (and assert) if it is None.
    */
    Window win; /* XXX this can optionally be None if parent != NULL ... */

    int ntextures;
    struct RrTexture *texture;

    struct RrSurface *parent;
    int parentx;
    int parenty;
};

#endif
