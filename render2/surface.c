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
    return sur;
}

static Window create_window(struct RrInstance *inst, Window parent)
{
    Window win = XCreateWindow(RrDisplay(inst), parent, 0, 0, 1, 1, 0,
                               RrDepth(inst), InputOutput, RrVisual(inst),
                               0, NULL);
    XMapWindow(RrDisplay(inst), win);
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
    sur->parentx = 0;
    sur->parenty = 0;
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
    return sur;
}

void RrSurfaceFree(struct RrSurface *sur)
{
    if (sur) {
        if (sur->ntextures)
            free(sur->texture);
        if (sur->parent && sur->win)
            XDestroyWindow(RrDisplay(sur->inst), sur->win);
        free(sur);
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
