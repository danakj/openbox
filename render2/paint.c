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
            a->h = MAX(a->y + a->h - 1, y + h - 1) - MIN(a->x, x), \
            a->x = MIN(a->x, x), \
            a->y = MIN(a->y, y)


void RrExpose(struct RrInstance *inst, XEvent *e)
{
    XEvent e2;
    GSList *tops = NULL, *it, *n;
    struct RrSurface *sur;
    int x, y, w, h;

    XPutBackEvent(RrDisplay(inst), e);
    while (XCheckTypedEvent(RrDisplay(inst), Expose, &e2)) {
        if ((sur = RrInstaceLookupSurface(inst, e2.xexpose.window))) {
            x = e->xexpose.x;
            y = e->xexpose.y;
            w = e->xexpose.width;
            h = e->xexpose.height;
            x = 0;
            y = 0;
            w = RrSurfaceWidth(sur);
            h = RrSurfaceHeight(sur);

            while (sur->parent) {
                x += RrSurfaceX(sur);
                y += RrSurfaceY(sur);
                sur = sur->parent;
            }
            for (it = tops; it; it = g_slist_next(it)) {
                struct ExposeArea *a = it->data;
                if (a->sur == sur) {
                    MERGE_AREA(a, x, y, w, h);
                    break;
                }
            }
            if (!it) {
                struct ExposeArea *a = malloc(sizeof(struct ExposeArea));
                a->sur = sur;
                a->x = x;
                a->y = y;
                a->w = w;
                a->h = h;
                tops = g_slist_append(tops, a);
            }
        } else {
            RrDebug("Unable to find surface for window 0x%lx\n",
                    e2.xexpose.window);
        }
    }
    for (it = tops; it; it = n) {
        struct ExposeArea *a = it->data;
        n = g_slist_next(it);
        RrPaintArea(sur, a->x, a->y, a->w, a->h);
        tops = g_slist_delete_link(tops, it);
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
