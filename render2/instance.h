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
    int gl_linewidth;
    Window shape_window;

    int depth;
    Visual *visual;
    guint32 red_mask;
    guint32 green_mask;
    guint32 blue_mask;
    guint32 red_offset;
    guint32 green_offset;
    guint32 blue_offset;
    int red_shift;
    int green_shift;
    int blue_shift;

    gboolean render_to_pixmap;

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
#define RrVisualInfo(i) (&(i)->visinfo)
#define RrColormap(i) ((i)->cmap)
#define RrContext(i)  ((i)->glx_context)

#define RrRenderToPixmap(i) ((i)->render_to_pixmap)

#define RrShapeWindow(i) ((i)->shape_window)

struct RrSurface;

void RrInstaceAddSurface(struct RrSurface *sur);
void RrInstaceRemoveSurface(struct RrSurface *sur);
struct RrSurface *RrInstaceLookupSurface(struct RrInstance *inst, Window win);

#endif
