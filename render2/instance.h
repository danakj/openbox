#ifndef __render_instance_h
#define __render_instance_h

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/glx.h>

#include <glib.h>

struct RrInstance {
    Display *display;
    int screen;
    XVisualInfo visinfo;
    Colormap cmap;
    GLXContext glx_context;

    GHashTable *surface_map;
};

#define RrDisplay(i)  ((i)->display)
#define RrScreen(i)   ((i)->screen)
#define RrScreenWidth(i) (WidthOfScreen(ScreenOfDisplay((i)->display, \
                                                        (i)->screen)))
#define RrScreenHeight(i) (HeightOfScreen(ScreenOfDisplay((i)->display, \
                                                          (i)->screen)))
#define RrDepth(i)    ((i)->visinfo.depth)
#define RrVisual(i)   ((i)->visinfo.visual)
#define RrColormap(i) ((i)->cmap)
#define RrContext(i)  ((i)->glx_context)

struct RrSurface;

void RrInstaceAddSurface(struct RrSurface *sur);
void RrInstaceRemoveSurface(struct RrSurface *sur);
struct RrSurface *RrInstaceLookupSurface(struct RrInstance *inst, Window win);

#endif
