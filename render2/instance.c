#include "instance.h"
#include <glib.h>

struct RrInstance *RrInstanceNew(Display *display,
                                 int screen,
                                 XVisualInfo visinfo)
{
    struct RrInstance *inst;

    inst = g_new(struct RrInstance, 1);
    inst->display = display;
    inst->screen = screen;
    inst->visinfo = visinfo;
    inst->cmap = XCreateColormap(display, RootWindow(display, screen),
                                 RrVisual(inst), AllocNone);
    inst->glx_context = glXCreateContext(display, &visinfo, NULL, TRUE);

    g_assert(inst->glx_context);

    return inst;
}

void RrInstanceFree(struct RrInstance *inst)
{
    if (inst) {
        glXDestroyContext(inst->display, inst->glx_context);
        XFreeColormap(inst->display, inst->cmap);
        g_free(inst);
    }
}
