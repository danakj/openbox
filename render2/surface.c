#include "surface.h"
#include "instance.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#ifdef    SHAPE
#include <X11/extensions/shape.h>
#endif

/* doesn't set win or parent */
static struct RrSurface *surface_new(enum RrSurfaceType type,
                                     int numtex)
{
    struct RrSurface *sur;

    sur = malloc(sizeof(struct RrSurface));
    sur->type = type;
    sur->shape_base = None;
    sur->shape_base_x = 0;
    sur->shape_base_y = 0;
    sur->ntextures = numtex;
    if (numtex) {
        sur->texture = malloc(sizeof(struct RrTexture) * numtex);
        memset(sur->texture, 0, sizeof(struct RrTexture) * numtex);
    } else
        sur->texture = NULL;
    sur->x = 0;
    sur->y = 0;
    sur->w = 1;
    sur->h = 1;
    sur->visible = 0;
    sur->children = NULL;
    sur->pixmap = None;
    sur->glxpixmap = None;
    return sur;
}

static Window create_window(struct RrInstance *inst, Window parent)
{
    XSetWindowAttributes attrib;
    Window win;

    attrib.event_mask = ExposureMask;
    win = XCreateWindow(RrDisplay(inst), parent, 0, 0, 1, 1, 0,
                        RrDepth(inst), InputOutput, RrVisual(inst),
                        CWEventMask, &attrib);
    return win;
}

struct RrSurface *RrSurfaceNewProto(enum RrSurfaceType type,
                                    int numtex)
{
    struct RrSurface *sur;

    sur = surface_new(type, numtex);
    sur->inst = NULL;
    sur->win = None;
    sur->parent = NULL;
    sur->visible = 0;
    sur->pixmap = None;
    sur->glxpixmap = None;
    return sur;
}

struct RrSurface *RrSurfaceNew(struct RrInstance *inst,
                               enum RrSurfaceType type,
                               Window win,
                               int numtex)
{
    struct RrSurface *sur;

    sur = surface_new(type, numtex);
    sur->inst = inst;
    sur->win = win;
    sur->parent = NULL;
    sur->visible = 0;
    sur->pixmap = None;
    sur->glxpixmap = None;

    RrInstaceAddSurface(sur);
    return sur;
}

struct RrSurface *RrSurfaceNewChild(enum RrSurfaceType type,
                                    struct RrSurface *parent,
                                    int numtex)
{
    struct RrSurface *sur;

    /* can't be a child of a prototype! */
    assert(parent->inst);
    if (!parent->inst) return NULL;

    sur = surface_new(type, numtex);
    sur->inst = parent->inst;
    sur->win = create_window(sur->inst, parent->win);
    sur->parent = parent;
    sur->pixmap = None;
    sur->glxpixmap = None;
    RrSurfaceShow(sur);

    parent->children = g_slist_append(parent->children, sur);

    RrInstaceAddSurface(sur);
    return sur;
}

void RrSurfaceCopy(struct RrSurface *dest, struct RrSurface *src)
{
    int i;

    dest->type = src->type;
    switch (dest->type) {
    case RR_SURFACE_PLANAR:
        dest->data = src->data;
        break;
    case RR_SURFACE_NONPLANAR:
        assert(0);
        break;
    case RR_SURFACE_NONE:
        break;
    }
    for (i = 0; i < dest->ntextures; i++)
        RrTextureFreeContents(&dest->texture[i]);
    free(dest->texture);
    dest->ntextures = src->ntextures;
    if (dest->ntextures)
        dest->texture = malloc(sizeof(struct RrTexture) * dest->ntextures);
    memcpy(dest->texture, src->texture,
           sizeof(struct RrTexture) * dest->ntextures);
    if (dest->pixmap) {
        glXDestroyGLXPixmap(RrDisplay(dest->inst), dest->glxpixmap);
        XFreePixmap(RrDisplay(dest->inst), dest->pixmap);
    }
    dest->pixmap = None;
    dest->glxpixmap = None;
}

void RrSurfaceFree(struct RrSurface *sur)
{
    int i;
    if (sur) {
        if (sur->parent)
            sur->parent->children = g_slist_remove(sur->parent->children, sur);

        RrInstaceRemoveSurface(sur);
        for (i = 0; i < sur->ntextures; ++i)
            RrTextureFreeContents(&sur->texture[i]);
        if (sur->ntextures)
            free(sur->texture);
        if (sur->parent && sur->win)
            XDestroyWindow(RrDisplay(sur->inst), sur->win);
        if (sur->glxpixmap != None) {
            glXDestroyGLXPixmap(RrDisplay(sur->inst), sur->glxpixmap);
            sur->glxpixmap = None;
        }
        if (sur->pixmap != None) {
            XFreePixmap(RrDisplay(sur->inst), sur->pixmap);
            sur->pixmap = None;
        }
        free(sur);
    }
}

void RrSurfaceSetPos(struct RrSurface *sur,
                     int x,
                     int y)
{
    RrSurfaceSetArea(sur, x, y, sur->w, sur->h);
}

void RrSurfaceSetSize(struct RrSurface *sur,
                      int w,
                      int h)
{
    RrSurfaceSetArea(sur, sur->x, sur->y, w, h);
}

void RrSurfaceSetArea(struct RrSurface *sur,
                      int x,
                      int y,
                      int w,
                      int h)
{
    int m, r;

    assert(w > 0 && h > 0);
    if (!(w > 0 && h > 0)) return;

    m = sur->x != x || sur->y != y;
    r = sur->w != w || sur->h != h;

    sur->x = x;
    sur->y = y;
    sur->w = w;
    sur->h = h;
    if (sur->win) {
        if (m && r)
            XMoveResizeWindow(RrDisplay(sur->inst), sur->win, x, y, w, h);
        else if (m)
            XMoveWindow(RrDisplay(sur->inst), sur->win, x, y);
        else if (r)
            XResizeWindow(RrDisplay(sur->inst), sur->win, w, h);
    }
}

Window RrSurfaceWindow(struct RrSurface *sur)
{
    /* can't get a window for a prototype */
    assert(sur->inst);
    if (!sur->inst) return None;

    return sur->win;
}

struct RrTexture *RrSurfaceTexture(struct RrSurface *sur, int texnum)
{
    assert(texnum < sur->ntextures);
    return &(sur->texture[texnum]);
}

void RrSurfaceMinSize(struct RrSurface *sur, int *w, int *h)
{
    int i;
    int minw, minh;

    switch(sur->type) {
    case RR_SURFACE_NONE:
        *w = *h = 0;
        break;
    case RR_SURFACE_PLANAR:
        RrPlanarMinSize(sur, w, h);
        break;
    case RR_SURFACE_NONPLANAR:
        assert(0);
        break;
    }

    minw = minh = 0;
    for (i = 0; i < sur->ntextures; ++i) {
        switch (sur->texture[i].type) {
        case RR_TEXTURE_NONE:
            minw = MAX(minw, 0);
            minh = MAX(minh, 0);
            break;
        case RR_TEXTURE_TEXT:
            minw =
                MAX(minw,
                    RrFontMeasureString(sur->texture[i].data.text.font,
                                        sur->texture[i].data.text.string)); 
            minh = MAX(minh, RrFontHeight(sur->texture[i].data.text.font));
            break;
        case RR_TEXTURE_RGBA:
            minw = MAX(minw, (sur->texture[i].data.rgba.x +
                              sur->texture[i].data.rgba.w));
            minh = MAX(minw, (sur->texture[i].data.rgba.y +
                              sur->texture[i].data.rgba.h));
            break;
        }
    }

    *w += minw;
    *h += minh;
    /* zeros are bad. */
    if (*w == 0) *w = 1;
    if (*h == 0) *h = 1;
}

void RrSurfaceShow(struct RrSurface *sur)
{
    sur->visible = 1;
    if (sur->win)
        XMapWindow(RrDisplay(sur->inst), sur->win);
}

void RrSurfaceHide(struct RrSurface *sur)
{
    sur->visible = 0;
    if (sur->win)
        XUnmapWindow(RrDisplay(sur->inst), sur->win);
}

int RrSurfaceVisible(struct RrSurface *sur)
{
    assert(sur->inst);
    return sur->visible;
}

void RrSurfaceShapeSetBase(struct RrSurface *sur, Window base, int x, int y)
{
    assert(sur->inst);
    sur->shape_base = base;
    sur->shape_base_x = x;
    sur->shape_base_y = y;
}

void RrSurfaceShape(struct RrSurface *sur)
{
    GSList *it;

    assert(sur->inst);

#ifdef SHAPE
    XResizeWindow(RrDisplay(sur->inst), RrShapeWindow(sur->inst),
                  sur->w, sur->h);
    XShapeCombineShape(RrDisplay(sur->inst), RrShapeWindow(sur->inst),
                       ShapeBounding,
                       sur->shape_base_x, sur->shape_base_y,
                       sur->shape_base, ShapeBounding, ShapeSet);
    /* include the shape of the children */
    for (it = sur->children; it; it = g_slist_next(it)) {
        struct RrSurface *ch = it->data;
        if (ch->win)
            XShapeCombineShape(RrDisplay(sur->inst),
                               RrShapeWindow(sur->inst),
                               ShapeBounding, ch->x, ch->y, ch->win,
                               ShapeBounding, ShapeUnion);
    }
    switch (sur->type) {
    case RR_SURFACE_NONE:
        break;
    case RR_SURFACE_PLANAR:
        /* XXX shape me based on something! an alpha mask? */
        break;
    case RR_SURFACE_NONPLANAR:
        /* XXX shape me based on my GL form! */
        assert(0);
        break;
    }

    /* apply the final shape */
    XShapeCombineShape(RrDisplay(sur->inst), sur->win, ShapeBounding, 0, 0,
                       RrShapeWindow(sur->inst), ShapeBounding, ShapeSet);
#endif
}
