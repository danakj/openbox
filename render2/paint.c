#include "render.h"
#include "surface.h"
#include "instance.h"
#include "debug.h"
#include <assert.h>
#include <GL/glx.h>

void planar_copy_parent(struct RrSurface *sur)
{
    int ncols;
    int copy;

    switch (RrPlanarColorType(sur)) {
    case RR_SURFACE_NONE:
        return;
    case RR_SURFACE_SOLID:
        ncols = 1;
        break;
    case RR_SURFACE_HORIZONTAL:
        ncols = 2;
        break;
    case RR_SURFACE_VERTICAL:
        ncols = 2;
        break;
    case RR_SURFACE_DIAGONAL:
        ncols = 2;
        break;
    case RR_SURFACE_CROSSDIAGONAL:
        ncols = 2;
        break;
    case RR_SURFACE_PYRAMID:
        ncols = 2;
        break;
    case RR_SURFACE_PIPECROSS:
        ncols = 2;
        break;
    case RR_SURFACE_RECTANGLE:
        ncols = 2;
        break;
    }

    copy = 0;
    if (ncols >= 1 && RrColorHasAlpha(RrPlanarPrimaryColor(sur)))
        copy = 1;
    if (ncols >= 1 && RrColorHasAlpha(RrPlanarSecondaryColor(sur)))
        copy = 1;
    if (copy) {
        /*
        struct RrSurface *parent = RrSurfaceParent(sur);

        XXX COPY PARENT PLS
        */
    }
}

void paint_planar(struct RrSurface *sur, int x, int y, int w, int h)
{   
    float pr,pg,pb;
    float sr, sg, sb;
    float ar, ag, ab;

    pr = RrColorRed(RrPlanarPrimaryColor(sur));
    pg = RrColorGreen(RrPlanarPrimaryColor(sur));
    pb = RrColorBlue(RrPlanarPrimaryColor(sur));
    sr = RrColorRed(RrPlanarSecondaryColor(sur));
    sg = RrColorGreen(RrPlanarSecondaryColor(sur));
    sb = RrColorBlue(RrPlanarSecondaryColor(sur));
    switch (RrPlanarColorType(sur)) {
    case RR_SURFACE_NONE:
        return;
    case RR_SURFACE_SOLID:
        glBegin(GL_TRIANGLES);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glVertex2i(x+w, y);
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        glVertex2i(x, y+h);
        glVertex2i(x, y);
        glEnd();
        break;
    case RR_SURFACE_HORIZONTAL:
        glBegin(GL_TRIANGLES);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w, y);
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y+h);
        glVertex2i(x, y);
        glEnd();
        break;
    case RR_SURFACE_VERTICAL:
        glBegin(GL_TRIANGLES);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glVertex2i(x+w, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        glVertex2i(x, y+h);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glEnd();
        break;
    case RR_SURFACE_DIAGONAL:
        ar = (pr + sr) / 2.0;
        ag = (pg + sg) / 2.0;
        ab = (pb + sb) / 2.0;
        glBegin(GL_TRIANGLES);
        glColor3f(ar, ag, ab);
        glVertex2i(x, y);
        glColor3f(pr, pg, pb);
        glVertex2i(x+w, y);
        glColor3f(ar, ag, ab);
        glVertex2i(x+w, y+h);

        glColor3f(ar, ag, ab);
        glVertex2i(x+w, y+h);
        glColor3f(sr, sg, sb);
        glVertex2i(x, y+h);
        glColor3f(ar, ag, ab);
        glVertex2i(x, y);
        glEnd();
        break;
    case RR_SURFACE_CROSSDIAGONAL:
        ar = (pr + sr) / 2.0;
        ag = (pg + sg) / 2.0;
        ab = (pb + sb) / 2.0;
        glBegin(GL_TRIANGLES);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glColor3f(ar, ag, ab);
        glVertex2i(x+w, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w, y+h);

        glColor3f(sr, sg, sb);
        glVertex2i(x+w, y+h);
        glColor3f(ar, ag, ab);
        glVertex2i(x, y+h);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glEnd();
        break;
    case RR_SURFACE_PYRAMID:
        ar = (pr + sr) / 2.0;
        ag = (pg + sg) / 2.0;
        ab = (pb + sb) / 2.0;
        glBegin(GL_TRIANGLES);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(ar, ag, ab);
        glVertex2i(x, y+h/2);

        glVertex2i(x, y+h/2);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y+h);

        glVertex2i(x, y+h);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(ar, ag, ab);
        glVertex2i(x+w/2, y+h);

        glVertex2i(x+w/2, y+h);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(ar, ag, ab);
        glVertex2i(x+w, y+h/2);

        glVertex2i(x+w, y+h/2);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x+w, y);

        glVertex2i(x+w, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(ar, ag, ab);
        glVertex2i(x+w/2, y);

        glVertex2i(x+w/2, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glEnd();
        break;
    case RR_SURFACE_PIPECROSS:
        glBegin(GL_TRIANGLES);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glVertex2i(x, y+h/2);

        glVertex2i(x, y+h/2);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y+h);

        glVertex2i(x, y+h);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glVertex2i(x+w/2, y+h);

        glVertex2i(x+w/2, y+h);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glVertex2i(x+w, y+h/2);

        glVertex2i(x+w, y+h/2);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x+w, y);

        glVertex2i(x+w, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glVertex2i(x+w/2, y);

        glVertex2i(x+w/2, y);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glEnd();
        break;
    case RR_SURFACE_RECTANGLE:
        glBegin(GL_TRIANGLES);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y+h);

        glVertex2i(x, y+h);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x+w, y);

        glVertex2i(x+w, y);
        glColor3f(sr, sg, sb);
        glVertex2i(x+w/2, y+h/2);
        glColor3f(pr, pg, pb);
        glVertex2i(x, y);

        glEnd();
        break;
    }
}

/*! Paints the surface, and all its children */
void RrSurfacePaint(struct RrSurface *sur)
{
    RrSurfacePaintArea(sur, 0, 0, RrSurfaceWidth(sur), RrSurfaceHeight(sur));
}

/*! Paints the surface, and all its children, but only in the given area. */
void RrSurfacePaintArea(struct RrSurface *sur,
                        int x,
                        int y,
                        int w,
                        int h)
{
    struct RrInstance *inst;
    int ok;

    inst = RrSurfaceInstance(sur);

    /* can't paint a prototype! */
    assert(inst);
    if (!inst) return;

    assert(x >= 0 && y >= 0);
    if (!(x >= 0 && y >= 0)) return;
    assert(x + w < RrSurfaceWidth(sur) && y + h < RrSurfaceHeight(sur));
    if (!(x + w < RrSurfaceWidth(sur) && y + h < RrSurfaceHeight(sur))) return;

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
        planar_copy_parent(sur);
        paint_planar(sur, x, y, w, h);
        break;
    case RR_SURFACE_NONPLANAR:
        assert(0);
        break;
    }

    glXSwapBuffers(RrDisplay(inst), RrSurfaceWindow(sur));
}
