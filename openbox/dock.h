#ifndef __dock_h
#define __dock_h

#include "window.h"
#include "stacking.h"
#include "geom.h"
#include "render/render.h"

#include <glib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef struct _ObDock    ObDock;
typedef struct _ObDockApp ObDockApp;

struct _ObDock
{
    ObWindow obwin;

    Window frame;
    RrAppearance *a_frame;

    /* actual position (when not auto-hidden) */
    gint x;
    gint y;
    gint w;
    gint h;

    gboolean hidden;

    GList *dock_apps;
};

struct _ObDockApp {
    ObWindow obwin;

    gint ignore_unmaps;

    Window icon_win;
    Window win;

    gchar *name;
    gchar *class;

    gint x;
    gint y;
    gint w;
    gint h;
};

extern StrutPartial dock_strut;

void dock_startup(gboolean reconfig);
void dock_shutdown(gboolean reconfig);

void dock_configure();
void dock_hide(gboolean hide);

void dock_add(Window win, XWMHints *wmhints);

void dock_remove_all();
void dock_remove(ObDockApp *app, gboolean reparent);

void dock_app_drag(ObDockApp *app, XMotionEvent *e);
void dock_app_configure(ObDockApp *app, gint w, gint h);

#endif
