#include "planar.h"
#include "surface.h"
#include "texture.h"
#include "color.h"
#include "debug.h"
#include "font.h"
#include "instance.h"
#include <string.h>
#include <assert.h>
#include <GL/glx.h>

static void RrLineWidth(struct RrInstance *i, int w)
{
    if (i->gl_linewidth != w) {
        glLineWidth(w);
        i->gl_linewidth = w;
    }
}

void RrPlanarSet(struct RrSurface *sur,
                 enum RrSurfaceColorType type,
                 enum RrBevelType bevel,
                 struct RrColor *primary,
                 struct RrColor *secondary,
                 int borderwidth,
                 struct RrColor *border)
{
    sur->data.planar.colortype = type;
    sur->data.planar.bevel = bevel;
    sur->data.planar.primary = *primary;
    if (!(type == RR_PLANAR_NONE || type == RR_PLANAR_SOLID))
        sur->data.planar.secondary = *secondary;
    assert(borderwidth >= 0);
    sur->data.planar.borderwidth = borderwidth >= 0 ? borderwidth : 0;
    if (borderwidth)
        sur->data.planar.border = *border;
}

int RrPlanarHasAlpha(struct RrSurface *sur)
{
    if (!(RrPlanarColorType(sur) == RR_PLANAR_NONE)) {
        if (RrColorHasAlpha(RrPlanarPrimaryColor(sur))) return 1;
        if (!RrPlanarColorType(sur) == RR_PLANAR_SOLID)
            if (RrColorHasAlpha(RrPlanarSecondaryColor(sur))) return 1;
    }
    return 0;
}

static void copy_parent(struct RrSurface *sur)
{
    if (RrPlanarHasAlpha(sur)) {
        /*
        struct RrSurface *parent = RrSurfaceParent(sur);

        XXX COPY PARENT PLS
        */
        RrDebug("copy parent here pls\n");
    }
}

static void RrBevelPaint(struct RrSurface *s, int inset, int raise)
{
    int offset = RrPlanarBorderWidth(s) + inset;
    int x, y, w, h;
    x = RrSurfaceX(s);
    y = RrSurfaceY(s);
    w = RrSurfaceWidth(s) - 1;
    h = RrSurfaceHeight(s) - 1;

    RrLineWidth(RrSurfaceInstance(s), 1);

    if (raise)
        glColor4f(1.0, 1.0, 1.0, 0.25);
    else
        glColor4f(0.0, 0.0, 0.0, 0.25);

    glBegin(GL_LINES);
    glVertex2i(x + offset, y + offset);
    glVertex2i(x + offset, y + h - offset);

    glVertex2i(x + offset, y + h - offset);
    glVertex2i(x + w - offset, y + h - offset);

    if (!raise)
        glColor4f(1.0, 1.0, 1.0, 0.25);
    else
        glColor4f(0.0, 0.0, 0.0, 0.25);

    glVertex2i(x + w - offset, y + h - offset);
    glVertex2i(x + w - offset,  y + offset);
               
    glVertex2i(x + w - offset, y + offset);
    glVertex2i(x + offset, y + offset);
    glEnd();
}

static void RrBorderPaint(struct RrSurface *s)
{
    int x, y, w, h, offset, bwidth;

    offset = RrPlanarBorderWidth(s) / 2;
    bwidth = RrPlanarBorderWidth(s);
    x = RrSurfaceX(s);
    y = RrSurfaceY(s);
    w = RrSurfaceWidth(s) - 1;
    h = RrSurfaceHeight(s) - 1;

    RrColor4f(&RrPlanarBorderColor(s));

    RrLineWidth(RrSurfaceInstance(s), bwidth);

    glBegin(GL_LINE_LOOP);
    glVertex2i(x + offset, y + offset);
    glVertex2i(x + offset, y + h - offset);
    glVertex2i(x + w - offset, y + h - offset);
    glVertex2i(x + w - offset, y + offset);
    glEnd();
}

void RrPlanarPaint(struct RrSurface *sur, int absx, int absy)
{   
    struct RrColor *pri, *sec, avg;
    int x, y, w, h;

    copy_parent(sur);

    pri = &RrPlanarPrimaryColor(sur);
    sec = &RrPlanarSecondaryColor(sur);

    x = RrSurfaceX(sur);
    y = RrSurfaceY(sur);
    w = RrSurfaceWidth(sur);
    h = RrSurfaceHeight(sur);

    switch (RrPlanarColorType(sur)) {
    case RR_PLANAR_NONE:
        return;
    case RR_PLANAR_SOLID:
        glBegin(GL_TRIANGLES);
        RrColor3f(pri);
        glVertex2i(x, y);
        glVertex2i(x+w, y);
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        glVertex2i(x, y+h);
        glVertex2i(x, y);
        glEnd();
        break;
    case RR_PLANAR_HORIZONTAL:
        glBegin(GL_TRIANGLES);
        RrColor3f(pri);
        glVertex2i(x, y);
        RrColor3f(sec);
        glVertex2i(x+w, y);
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        RrColor3f(pri);
        glVertex2i(x, y+h);
        glVertex2i(x, y);
        glEnd();
        break;
    case RR_PLANAR_VERTICAL:
        glBegin(GL_TRIANGLES);
        RrColor3f(pri);
        glVertex2i(x, y);
        glVertex2i(x+w, y);
        RrColor3f(sec);
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        glVertex2i(x, y+h);
        RrColor3f(pri);
        glVertex2i(x, y);
        glEnd();
        break;
    case RR_PLANAR_DIAGONAL:
        RrColorAvg(&avg, pri, sec);
        glBegin(GL_TRIANGLES);
        RrColor3f(&avg);
        glVertex2i(x, y);
        RrColor3f(pri);
        glVertex2i(x+w, y);
        RrColor3f(&avg);
        glVertex2i(x+w, y+h);

        RrColor3f(&avg);
        glVertex2i(x+w, y+h);
        RrColor3f(sec);
        glVertex2i(x, y+h);
        RrColor3f(&avg);
        glVertex2i(x, y);
        glEnd();
        break;
    case RR_PLANAR_CROSSDIAGONAL:
        RrColorAvg(&avg, pri, sec);
        glBegin(GL_TRIANGLES);
        RrColor3f(pri);
        glVertex2i(x, y);
        RrColor3f(&avg);
        glVertex2i(x+w, y);
        RrColor3f(sec);
        glVertex2i(x+w, y+h);

        RrColor3f(sec);
        glVertex2i(x+w, y+h);
        RrColor3f(&avg);
        glVertex2i(x, y+h);
        RrColor3f(pri);
        glVertex2i(x, y);
        glEnd();
        break;
    case RR_PLANAR_PYRAMID:
        RrColorAvg(&avg, pri, sec);
        glBegin(GL_TRIANGLES);
        RrColor3f(pri);
        glVertex2i(x, y);
        RrColor3f(sec);
        glVertex2i(x+w/2, y+h/2);
        RrColor3f(&avg);
        glVertex2i(x, y+h/2);

        glVertex2i(x, y+h/2);
        RrColor3f(sec);
        glVertex2i(x+w/2, y+h/2);
        RrColor3f(pri);
        glVertex2i(x, y+h);

        glVertex2i(x, y+h);
        RrColor3f(sec);
        glVertex2i(x+w/2, y+h/2);
        RrColor3f(&avg);
        glVertex2i(x+w/2, y+h);

        glVertex2i(x+w/2, y+h);
        RrColor3f(sec);
        glVertex2i(x+w/2, y+h/2);
        RrColor3f(pri);
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        RrColor3f(sec);
        glVertex2i(x+w/2, y+h/2);
        RrColor3f(&avg);
        glVertex2i(x+w, y+h/2);

        glVertex2i(x+w, y+h/2);
        RrColor3f(sec);
        glVertex2i(x+w/2, y+h/2);
        RrColor3f(pri);
        glVertex2i(x+w, y);

        glVertex2i(x+w, y);
        RrColor3f(sec);
        glVertex2i(x+w/2, y+h/2);
        RrColor3f(&avg);
        glVertex2i(x+w/2, y);

        glVertex2i(x+w/2, y);
        RrColor3f(sec);
        glVertex2i(x+w/2, y+h/2);
        RrColor3f(pri);
        glVertex2i(x, y);
        glEnd();
        break;
    case RR_PLANAR_PIPECROSS:
        glBegin(GL_TRIANGLES);
        RrColor3f(pri);
        glVertex2i(x, y);
        RrColor3f(sec);
        glVertex2i(x+w/2, y+h/2);
        glVertex2i(x, y+h/2);

        glVertex2i(x, y+h/2);
        glVertex2i(x+w/2, y+h/2);
        RrColor3f(pri);
        glVertex2i(x, y+h);

        glVertex2i(x, y+h);
        RrColor3f(sec);
        glVertex2i(x+w/2, y+h/2);
        glVertex2i(x+w/2, y+h);

        glVertex2i(x+w/2, y+h);
        glVertex2i(x+w/2, y+h/2);
        RrColor3f(pri);
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        RrColor3f(sec);
        glVertex2i(x+w/2, y+h/2);
        glVertex2i(x+w, y+h/2);

        glVertex2i(x+w, y+h/2);
        glVertex2i(x+w/2, y+h/2);
        RrColor3f(pri);
        glVertex2i(x+w, y);

        glVertex2i(x+w, y);
        RrColor3f(sec);
        glVertex2i(x+w/2, y+h/2);
        glVertex2i(x+w/2, y);

        glVertex2i(x+w/2, y);
        glVertex2i(x+w/2, y+h/2);
        RrColor3f(pri);
        glVertex2i(x, y);
        glEnd();
        break;
    case RR_PLANAR_RECTANGLE:
        glBegin(GL_TRIANGLES);
        RrColor3f(pri);
        glVertex2i(x, y);
        RrColor3f(sec);
        glVertex2i(x+w/2, y+h/2);
        RrColor3f(pri);
        glVertex2i(x, y+h);

        glVertex2i(x, y+h);
        RrColor3f(sec);
        glVertex2i(x+w/2, y+h/2);
        RrColor3f(pri);
        glVertex2i(x+w, y+h);

        glVertex2i(x+w, y+h);
        RrColor3f(sec);
        glVertex2i(x+w/2, y+h/2);
        RrColor3f(pri);
        glVertex2i(x+w, y);

        glVertex2i(x+w, y);
        RrColor3f(sec);
        glVertex2i(x+w/2, y+h/2);
        RrColor3f(pri);
        glVertex2i(x, y);

        glEnd();
        break;
    }

    switch (RrPlanarBevelType(sur)) {
    case RR_SUNKEN_OUTER:
        RrBevelPaint(sur, 0, 0);
        break;
    case RR_SUNKEN_INNER:
        RrBevelPaint(sur, 1, 0);
        break;
    case RR_RAISED_OUTER:
        RrBevelPaint(sur, 0, 1);
        break;
    case RR_RAISED_INNER:
        RrBevelPaint(sur, 1, 1);
        break;
    case RR_BEVEL_NONE:
        break;
    }

    if (RrPlanarBorderWidth(sur))
        RrBorderPaint(sur);
}

int RrPlanarEdgeWidth(struct RrSurface *sur)
{
    int w;
    w = RrPlanarBorderWidth(sur);
    switch (RrPlanarBevelType(sur)) {
    case RR_SUNKEN_OUTER:
        w++;
        break;
    case RR_SUNKEN_INNER:
        w += 2;
        break;
    case RR_RAISED_OUTER:
        w += 2;
        break;
    case RR_RAISED_INNER:
        w++;
        break;
    case RR_BEVEL_NONE:
        break;
    }
    return w;
}

void RrPlanarMinSize(struct RrSurface *sur, int *w, int *h)
{
    *w = *h = 2 * RrPlanarEdgeWidth(sur);
}
