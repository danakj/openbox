#include "render.h"
#include "surface.h"
#include "instance.h"
#include "debug.h"
#include <assert.h>
#include <GL/glx.h>

/*! Paints the surface, and all its children */
void RrPaint(struct RrSurface *sur)
{
    RrPaintArea(sur, 0, 0, RrSurfaceWidth(sur), RrSurfaceHeight(sur));
}

/*! Paints the surface, and all its children, but only in the given area. */
void RrPaintArea(struct RrSurface *sur, int x, int y, int w, int h)
{
    struct RrInstance *inst;
    int ok;

    inst = RrSurfaceInstance(sur);

    /* can't paint a prototype! */
    assert(inst);
    if (!inst) return;

    /* bounds checking */
    assert(x >= 0 && y >= 0);
    if (!(x >= 0 && y >= 0)) return;
    assert(x + w <= RrSurfaceWidth(sur) && y + h <= RrSurfaceHeight(sur));
    if (!(x + w <= RrSurfaceWidth(sur) && y + h <= RrSurfaceHeight(sur)))
        return;

    RrDebug("making %p, %p, %p current\n",
            RrDisplay(inst), RrSurfaceWindow(sur), RrContext(inst));

    ok = glXMakeCurrent(RrDisplay(inst), RrSurfaceWindow(sur),RrContext(inst));
    assert(ok);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, RrScreenWidth(inst), RrScreenHeight(inst), 0, 0, 10);
    glViewport(0, 0, RrScreenWidth(inst), RrScreenHeight(inst));
    glMatrixMode(GL_MODELVIEW);
    glTranslatef(-RrSurfaceX(sur),
                 RrScreenHeight(inst)-RrSurfaceHeight(sur)-RrSurfaceY(sur), 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    switch (RrSurfaceType(sur)) {
    case RR_SURFACE_PLANAR:
        RrPlanarPaint(sur, RrSurfaceX(sur) + x, RrSurfaceY(sur) + y, w, h);
        break;
    case RR_SURFACE_NONPLANAR:
        assert(0);
        break;
    }

    glXSwapBuffers(RrDisplay(inst), RrSurfaceWindow(sur));
}
