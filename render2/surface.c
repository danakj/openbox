#include "surface.h"
#include "instance.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* doesn't set win or parent */
static struct RrSurface *surface_new(enum RrSurfaceType type,
                                     int numtex)
{
    struct RrSurface *sur;

    sur = malloc(sizeof(struct RrSurface));
    sur->type = type;
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
    RrSurfaceShow(sur);

    parent->children = g_slist_append(parent->children, sur);

    RrInstaceAddSurface(sur);
    return sur;
}

/* doesn't set win or parent */
static struct RrSurface *surface_copy(struct RrSurface *orig)
{
    struct RrSurface *sur;

    sur = malloc(sizeof(struct RrSurface));
    sur->type = orig->type;
    switch (sur->type) {
    case RR_SURFACE_PLANAR:
        sur->data = orig->data;
        break;
    case RR_SURFACE_NONPLANAR:
        assert(0);
        break;
    case RR_SURFACE_NONE:
        break;
    }
    sur->ntextures = orig->ntextures;
    sur->texture = malloc(sizeof(struct RrTexture) * sur->ntextures);
    memcpy(sur->texture, orig->texture,
           sizeof(struct RrTexture) * sur->ntextures);
    return sur;
}

struct RrSurface *RrSurfaceCopy(struct RrInstance *inst,
                                struct RrSurface *orig,
                                Window win)
{
    struct RrSurface *sur;

    sur = surface_copy(orig);
    sur->inst = inst;
    sur->win = win;
    sur->parent = NULL;
    sur->visible = 0;

    RrInstaceAddSurface(sur);
    return sur;
}

struct RrSurface *RrSurfaceCopyChild(struct RrSurface *orig,
                                     struct RrSurface *parent)
{
    struct RrSurface *sur;

    /* can't be a child of a prototype! */
    assert(parent->inst);
    if (!parent->inst) return NULL;

    sur = surface_copy(orig);
    sur->inst = parent->inst;
    sur->win = create_window(sur->inst, parent->win);
    sur->parent = parent;
    RrSurfaceShow(sur);

    parent->children = g_slist_append(parent->children, sur);

    RrInstaceAddSurface(sur);
    return sur;
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
        free(sur);
    }
}

void RrSurfaceSetArea(struct RrSurface *sur,
                      int x,
                      int y,
                      int w,
                      int h)
{
    assert(w > 0 && h > 0);
    if (!(w > 0 && h > 0)) return;

    sur->x = x;
    sur->y = y;
    sur->w = w;
    sur->h = h;
    if (sur->win)
        XMoveResizeWindow(RrDisplay(sur->inst), sur->win, x, y, w, h);
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
            /* XXX MEASUER STRING PLS */
            minw = MAX(minw, 100 /*MEASURESTRING*/); 
            minh = MAX(minh, 10  /*HEIGHTOFFONT*/);
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
    return sur->visible;
}
