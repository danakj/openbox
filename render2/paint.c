#include "render.h"
#include "surface.h"
#include "instance.h"
#include "debug.h"
#include <assert.h>
#include <GL/glx.h>

void RrExpose(struct RrInstance *inst, XEvent *e)
{
    XEvent e2;
    GSList *tops = NULL, *it;
    struct RrSurface *sur;

    XPutBackEvent(RrDisplay(inst), e);
    while (XCheckTypedEvent(RrDisplay(inst), Expose, &e2)) {
        if ((sur = RrInstaceLookupSurface(inst, e2.xexpose.window))) {
            while (sur->parent) sur = sur->parent;
            if (!g_slist_find(tops, sur))
                tops = g_slist_append(tops, sur);
        }
    }

    for (it = tops; it; it = g_slist_next(it))
        RrPaint(it->data);

/*
    XEvent e2;
    struct RrSurface *sur;

    assert(e->type == Expose);
    if (!(e->type == Expose)) return;

    if ((sur = RrInstaceLookupSurface(inst, e->xexpose.window))) {
        int l, r, t, b;

        l = e->xexpose.x;
        t = e->xexpose.y;
        r = e->xexpose.width + e->xexpose.x - 1;
        b = e->xexpose.height + e->xexpose.y - 1;

        while (XCheckTypedWindowEvent(RrDisplay(inst), e->xexpose.window,
                                      Expose, &e2)) {
            l = MIN(l, e2.xexpose.x);
            t = MIN(t, e2.xexpose.y);
            r = MAX(r, e2.xexpose.width + e2.xexpose.x - 1);
            b = MAX(b, e2.xexpose.height + e2.xexpose.y - 1);
        }
        RrPaintArea(sur, l, t,
                    MIN(r - l + 1, RrSurfaceWidth(sur) - l),
                    MIN(b - t + 1, RrSurfaceHeight(sur) - t));
    } else {
        RrDebug("Unable to find surface for window 0x%lx\n",e->xexpose.window);
    }
*/
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

    /* bounds checking */
    assert(x >= 0 && y >= 0);
    if (!(x >= 0 && y >= 0)) return;
    assert(x + w <= RrSurfaceWidth(sur) && y + h <= RrSurfaceHeight(sur));
    if (!(x + w <= RrSurfaceWidth(sur) && y + h <= RrSurfaceHeight(sur)))
        return;

    /* recurse and paint children */
    for (it = RrSurfaceChildren(sur); it; it = g_slist_next(it)) {
        struct RrSurface *child = it->data;
        /* in the area to repaint? */
        if (RrSurfaceX(child) < x+w &&
            RrSurfaceX(child) + RrSurfaceWidth(child) > x &&
            RrSurfaceY(child) < y+h &&
            RrSurfaceY(child) + RrSurfaceHeight(child) > y)
            RrPaintArea(child,
                        MAX(0, x-RrSurfaceX(child)),
                        MAX(0, y-RrSurfaceY(child)),
                        MIN(RrSurfaceWidth(child),
                            w - MAX(0, (RrSurfaceX(child)-x))),
                        MIN(RrSurfaceHeight(child),
                            h - MAX(0, (RrSurfaceY(child)-y))));
    }

    if (!RrSurfaceVisible(sur)) return;

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
