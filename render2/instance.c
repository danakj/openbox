#include "instance.h"
#include "surface.h"
#include "debug.h"
#include "glft/glft.h"
#include <stdlib.h>
#include <assert.h>

static int glft_init = 0;

static int glx_rating(Display *display, XVisualInfo *v)
{
    int rating = 0;
    int val;
    RrDebug("evaluating visual %d\n", (int)v->visualid);
    glXGetConfig(display, v, GLX_BUFFER_SIZE, &val);
    RrDebug("buffer size %d\n", val);

    switch (val) {
    case 32:
        rating += 300;
    break;
    case 24:
        rating += 200;
    break;
    case 16:
        rating += 100;
    break;
    }

    glXGetConfig(display, v, GLX_LEVEL, &val);
    RrDebug("level %d\n", val);
    if (val != 0)
        rating = -10000;

    glXGetConfig(display, v, GLX_DEPTH_SIZE, &val);
    RrDebug("depth size %d\n", val);
    switch (val) {
    case 32:
        rating += 30;
    break;
    case 24:
        rating += 20;
    break;
    case 16:
        rating += 10;
    break;
    case 0:
        rating -= 10000;
    }

    glXGetConfig(display, v, GLX_DOUBLEBUFFER, &val);
    RrDebug("double buffer %d\n", val);
    if (val)
        rating++;
    return rating;
}

struct RrInstance *RrInstanceNew(Display *display, int screen)
{
    int count, i = 0, val, best = 0, rate = 0, temp, ok;
    XVisualInfo vimatch, *vilist;

    vimatch.screen = screen;
    vimatch.class = TrueColor;
    vilist = XGetVisualInfo(display, VisualScreenMask | VisualClassMask,
                            &vimatch, &count);

    if (vilist) {
        RrDebug("looking for a GL visual in %d visuals\n", count);
        for (i = 0; i < count; i++) {
            glXGetConfig(display, &vilist[i], GLX_USE_GL, &val);
            if (val) {
                temp = glx_rating(display, &vilist[i]);
                if (temp > rate) {
                    best = i;
                    rate = temp;
                }
            }
        }
    }
    if (rate > 0) {
        struct RrInstance *inst;

        RrDebug("picked visual %d with rating %d\n", best, rate);

        if (!glft_init) {
            if (!GlftInit())
                return NULL;
            glft_init = 1;
        }

        inst = malloc(sizeof(struct RrInstance));
        inst->display = display;
        inst->screen = screen;
        inst->visinfo = vilist[best];
        inst->cmap = XCreateColormap(display, RootWindow(display, screen),
                                     RrVisual(inst), AllocNone);
        inst->glx_context = glXCreateContext(display, &vilist[best],
                                             NULL, True);
        inst->shape_window = XCreateSimpleWindow(display,
                                                 RootWindow(display, screen),
                                                 0, 0, 1, 1, 0, 0, 0);
        /* make the context current on anything we can so we can dl 
           textures */

        ok = glXMakeCurrent(display, inst->shape_window, inst->glx_context);
        assert(ok);
        inst->surface_map = g_hash_table_new(g_int_hash, g_int_equal);

        assert(inst->glx_context);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, RrScreenWidth(inst), 0, RrScreenHeight(inst), 0, 10);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        return inst;
    }

    RrDebug("unable to find a suitable GL visual\n");
    return NULL;
}

void RrInstanceFree(struct RrInstance *inst)
{
    if (inst) {
        g_hash_table_destroy(inst->surface_map);
        glXDestroyContext(inst->display, inst->glx_context);
        XFreeColormap(inst->display, inst->cmap);
        free(inst);
    }
}

int RrInstanceDepth(struct RrInstance *inst)
{
    return inst->visinfo.depth;
}

Colormap RrInstanceColormap(struct RrInstance *inst)
{
    return inst->cmap;
}

Visual *RrInstanceVisual(struct RrInstance *inst)
{
    return inst->visinfo.visual;
}

void RrInstaceAddSurface(struct RrSurface *sur)
{
    g_hash_table_replace(RrSurfaceInstance(sur)->surface_map, &sur->win, sur);
}

void RrInstaceRemoveSurface(struct RrSurface *sur)
{
    g_hash_table_remove(RrSurfaceInstance(sur)->surface_map, &sur->win);
}

struct RrSurface *RrInstaceLookupSurface(struct RrInstance *inst, Window win)
{
    return g_hash_table_lookup(inst->surface_map, &win);
}
