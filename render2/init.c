#include "instance.h"
#include "render.h"

static int glXRating(Display *display, XVisualInfo *v)
{
    int rating = 0;
    int val;
    g_print("evaluating visual %d\n", (int)v->visualid);
    glXGetConfig(display, v, GLX_BUFFER_SIZE, &val);
    g_print("buffer size %d\n", val);

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
    g_print("level %d\n", val);
    if (val != 0)
        rating = -10000;

    glXGetConfig(display, v, GLX_DEPTH_SIZE, &val);
    g_print("depth size %d\n", val);
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
    g_print("double buffer %d\n", val);
    if (val)
        rating++;
    return rating;
}

struct RrInstance *RrInit(Display *display,
                          int screen)
{
    int count, i = 0, val, best = 0, rate = 0, temp;
    XVisualInfo vimatch, *vilist;
    struct RrInstance *ret = NULL;

    vimatch.screen = screen;
    vimatch.class = TrueColor;
    vilist = XGetVisualInfo(display, VisualScreenMask | VisualClassMask,
                            &vimatch, &count);

    if (vilist) {
        g_print("looking for a GL visual in %d visuals\n", count);
        for (i = 0; i < count; i++) {
            glXGetConfig(display, &vilist[i], GLX_USE_GL, &val);
            if (val) {
                temp = glXRating(display, &vilist[i]);
                if (temp > rate) {
                    best = i;
                    rate = temp;
                }
            }
        }
    }
    if (rate > 0) {
        g_print("picked visual %d with rating %d\n", best, rate);
        ret = RrInstanceNew(display, screen, vilist[best]);
    }
    return ret;
}

void RrDestroy(struct RrInstance *inst)
{
    RrInstanceFree(inst);
}
