#ifndef __instance_h
#define __instance_h

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>

struct RrInstance {
    Display *display;
    int screen;
    XVisualInfo visinfo;
    Colormap cmap;
    GLXContext glx_context;
};

struct RrInstance *RrInstanceNew(Display *display,
                                 int screen,
                                 XVisualInfo visinfo);
void RrInstanceFree(struct RrInstance *inst);


#define RrDisplay(i)  ((i)->display)
#define RrScreen(i)   ((i)->screen)
#define RrDepth(i)    ((i)->visinfo.depth)
#define RrVisual(i)   ((i)->visinfo.visual)
#define RrColormap(i) ((i)->cmap)
#define RrContext(i)  ((i)->glx_context)

#endif
