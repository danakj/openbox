#ifndef __slit_h
#define __slit_h

#include <glib.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef struct Slit Slit;

typedef struct SlitApp {
    int ignore_unmaps;

    Slit *slit;
    Window icon_win;
    Window win;
    int x;
    int y;
    int w;
    int h;
} SlitApp;

typedef enum {
    SlitPos_Floating,
    SlitPos_TopLeft,
    SlitPos_Top,
    SlitPos_TopRight,
    SlitPos_Right,
    SlitPos_BottomRight,
    SlitPos_Bottom,
    SlitPos_BottomLeft,
    SlitPos_Left
} SlitPosition;

extern GHashTable *slit_map;
extern GHashTable *slit_app_map;

void slit_startup();
void slit_shutdown();

void slit_configure_all();
void slit_hide(Slit *self, gboolean hide);

void slit_add(Window win, XWMHints *wmhints);

void slit_remove_all();
void slit_remove(SlitApp *app, gboolean reparent);

void slit_app_drag(SlitApp *app, XMotionEvent *e);
void slit_app_configure(SlitApp *app, int w, int h);

#endif
