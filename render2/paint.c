#include "render.h"
#include "surface.h"
#include "instance.h"
#include "debug.h"
#include <assert.h>
#include <stdlib.h>
#include <GL/glx.h>

void steal_children_exposes(struct RrInstance *inst, struct RrSurface *sur)
{
    GSList *it;
    XEvent e2;

    for (it = sur->children; it; it = g_slist_next(it)) {
        switch (RrSurfaceType(((struct RrSurface*)it->data))) {
        case RR_SURFACE_NONE:
            break;
        case RR_SURFACE_PLANAR:
            if (RrPlanarHasAlpha(((struct RrSurface*)it->data))) {
                while (XCheckTypedWindowEvent
                       (RrDisplay(inst), Expose,
                        ((struct RrSurface*)it->data)->win, &e2));
                steal_children_exposes(inst, it->data);
            }
            break;
        case RR_SURFACE_NONPLANAR:
            assert(0);
        }
    }
}

void RrExpose(struct RrInstance *inst, XExposeEvent *e)
{
    XEvent e2;
    struct RrSurface *sur;

    if ((sur = RrInstaceLookupSurface(inst, e->window))) {
        while (1) {
            struct RrSurface *p = NULL;

            /* steal events along the way */
            while (XCheckTypedWindowEvent(RrDisplay(inst), Expose,
                                          sur->win, &e2));

            switch (RrSurfaceType(sur)) {
            case RR_SURFACE_NONE:
                break;
            case RR_SURFACE_PLANAR:
                if (RrPlanarHasAlpha(sur))
                    p = RrSurfaceParent(sur);
                break;
            case RR_SURFACE_NONPLANAR:
                assert(0);
            }

            if (p) sur = p;
            else break;
        }
        /* also do this for transparent children */
        steal_children_exposes(inst, sur);
        RrPaint(sur, 0);
    } else
        RrDebug("Unable to find surface for window 0x%lx\n", e->window);
}

/*! Paints the surface, and all its children */
void RrPaint(struct RrSurface *sur, int recurse_always)
{
    struct RrInstance *inst;
    struct RrSurface *p;
    int ok, i;
    int surx, sury;
    int x, y, w, h, e;
    GSList *it;

    inst = RrSurfaceInstance(sur);

    /* can't paint a prototype! */
    assert(inst);
    if (!inst) return;

    if (!RrSurfaceVisible(sur)) return;

    ok = glXMakeCurrent(RrDisplay(inst), RrSurfaceWindow(sur),RrContext(inst));
    assert(ok);

    glViewport(0, 0, RrScreenWidth(inst)-1, RrScreenHeight(inst)-1);
/*
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(RrSurfaceX(sur), RrSurfaceX(sur) + RrSurfaceWidth(sur)-1,
            RrSurfaceY(sur), RrSurfaceY(sur) + RrSurfaceHeight(sur)-1,
            0, 10);
    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, RrSurfaceWidth(sur)-1, RrSurfaceHeight(sur)-1);
*/
    glPushMatrix();
    glTranslatef(-RrSurfaceX(sur), -RrSurfaceY(sur), 0);

    p = sur;
    surx = sury = 0;
    while (p) {
        surx += RrSurfaceX(p);
        sury += RrSurfaceY(p);
        p = p->parent;
    }

    switch (RrSurfaceType(sur)) {
    case RR_SURFACE_PLANAR:
        RrPlanarPaint(sur, surx, sury);
        e = RrPlanarEdgeWidth(sur);
        x = RrSurfaceX(sur) + e;
        y = RrSurfaceY(sur) + e;
        w = RrSurfaceWidth(sur) - e * 2;
        h = RrSurfaceHeight(sur) - e * 2;
        break;
    case RR_SURFACE_NONPLANAR:
        assert(0);
        break;
    case RR_SURFACE_NONE:
        x = RrSurfaceX(sur);
        y = RrSurfaceY(sur);
        w = RrSurfaceWidth(sur);
        h = RrSurfaceHeight(sur);
        break;
    }

    glEnable(GL_BLEND);
    for (i = 0; i < sur->ntextures; ++i)
        RrTexturePaint(sur, &sur->texture[i], x, y, w, h);
    glDisable(GL_BLEND);

    glPopMatrix();

    glXSwapBuffers(RrDisplay(inst), RrSurfaceWindow(sur));

    /* recurse and paint children */
    for (it = RrSurfaceChildren(sur); it; it = g_slist_next(it)) {
        if (recurse_always)
            RrPaint(it->data, 1);
        else {
            switch (RrSurfaceType(((struct RrSurface*)it->data))) {
            case RR_SURFACE_NONE:
                break;
            case RR_SURFACE_PLANAR:
                if (RrPlanarHasAlpha(it->data))
                    RrPaint(it->data, 0);
                break;
            case RR_SURFACE_NONPLANAR:
                assert(0);
                break;
            }
        }
    }
}
