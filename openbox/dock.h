#ifndef __dock_h
#define __dock_h

#include "timer.h"
#include "window.h"
#include "stacking.h"
#include "geom.h"
#include "render/render.h"

#include <glib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef enum {
    DockPos_Floating,
    DockPos_TopLeft,
    DockPos_Top,
    DockPos_TopRight,
    DockPos_Right,
    DockPos_BottomRight,
    DockPos_Bottom,
    DockPos_BottomLeft,
    DockPos_Left
} DockPosition;

typedef struct Dock {
    ObWindow obwin;

    Window frame;
    RrAppearance *a_frame;

    /* actual position (when not auto-hidden) */
    int x, y;
    int w, h;

    gboolean hidden;
    Timer *hide_timer;

    GList *dock_apps;
} Dock;

typedef struct DockApp {
    ObWindow obwin;

    int ignore_unmaps;

    Window icon_win;
    Window win;

    char *name;
    char *class;

    int x;
    int y;
    int w;
    int h;
} DockApp;

extern Strut dock_strut;

void dock_startup();
void dock_shutdown();

void dock_configure();
void dock_hide(gboolean hide);

void dock_add(Window win, XWMHints *wmhints);

void dock_remove_all();
void dock_remove(DockApp *app, gboolean reparent);

void dock_app_drag(DockApp *app, XMotionEvent *e);
void dock_app_configure(DockApp *app, int w, int h);

#endif
