#include "render.h"
#include "surface.h"
#include "instance.h"
#include "debug.h"
#include <assert.h>
#include <stdlib.h>
#include <GL/glx.h>

struct ExposeArea {
    struct RrSurface *sur;
    int x;
    int y;
    int w;
    int h;
};

#define MERGE_AREA(a, x, y, w, h) \
            a->w = MAX(a->x + a->w - 1, x + w - 1) - MIN(a->x, x), \
            a->h = MAX(a->y + a->h - 1, y + h - 1) - MIN(a->y, y), \
            a->x = MIN(a->x, x), \
            a->y = MIN(a->y, y)


void RrExpose(struct RrInstance *inst, XEvent *e)
{
    XEvent e2;
    GSList *tops = NULL, *it, *n;
    struct RrSurface *sur;

    e2 = *e;
    if ((sur = RrInstaceLookupSurface(inst, e2.xexpose.window))) {
        while (XCheckTypedWindowEvent(RrDisplay(inst), Expose,
                                      e->xexpose.window, &e2));
        while (sur->parent &&
               RrSurfaceType(sur->parent) != RR_SURFACE_NONE) {
            sur = sur->parent;
        }
        RrPaint(sur);
    } else {
        RrDebug("Unable to find surface for window 0x%lx\n",
                e2.xexpose.window);
    }
}

/*! Paints the surface, and all its children */
void RrPaint(struct RrSurface *sur)
{
    RrPaintArea(sur, 0, 0, RrSurfaceWidth(sur), RrSurfaceHeight(sur));
}

/*! Paints the surface, and all its children, but only in the given area. */
void RrPaintArea(struct RrSurface *sur, int x, int y, int w, int h)
{
    struct RrInstance *inst;
    struct RrSurface *p;
    int ok;
    int surx, sury;
    GSList *it;

    inst = RrSurfaceInstance(sur);

    /* can't paint a prototype! */
    assert(inst);
    if (!inst) return;

    if (!RrSurfaceVisible(sur)) return;

    /* recurse and paint children */
    for (it = RrSurfaceChildren(sur); it; it = g_slist_next(it))
        RrPaint(it->data);

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

    p = sur;
    surx = sury = 0;
    while (p) {
        surx += RrSurfaceX(p);
        sury += RrSurfaceY(p);
        p = p->parent;
    }

    switch (RrSurfaceType(sur)) {
    case RR_SURFACE_PLANAR:
        RrPlanarPaint(sur, surx + x, sury + y, w, h);
        break;
    case RR_SURFACE_NONPLANAR:
        assert(0);
        break;
    case RR_SURFACE_NONE:
        break;
    }

    glXSwapBuffers(RrDisplay(inst), RrSurfaceWindow(sur));
}
