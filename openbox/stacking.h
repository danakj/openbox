#ifndef __stacking_h
#define __stacking_h

#include "window.h"

#include <glib.h>
#include <X11/Xlib.h>

/*! The possible stacking layers a client window can be a part of */
typedef enum {
    OB_STACKING_LAYER_DESKTOP,    /*!< 0 - desktop windows */
    OB_STACKING_LAYER_BELOW,      /*!< 1 - normal windows w/ below */
    OB_STACKING_LAYER_NORMAL,     /*!< 2 - normal windows */
    OB_STACKING_LAYER_ABOVE,      /*!< 3 - normal windows w/ above */
    OB_STACKING_LAYER_TOP,        /*!< 4 - always-on-top-windows (docks?) */
    OB_STACKING_LAYER_FULLSCREEN, /*!< 5 - fullscreeen windows */
    OB_STACKING_LAYER_INTERNAL,   /*!< 6 - openbox windows/menus */
    OB_NUM_STACKING_LAYERS
} ObStackingLayer;

/* list of ObWindow*s in stacking order from highest to lowest */
extern GList  *stacking_list;

/*! Sets the window stacking list on the root window from the
  stacking_list */
void stacking_set_list();

void stacking_add(ObWindow *win);
void stacking_add_nonintrusive(ObWindow *win);
#define stacking_remove(win) stacking_list = g_list_remove(stacking_list, win);

/*! Raises a window above all others in its stacking layer */
void stacking_raise(ObWindow *window);

/*! Lowers a window below all others in its stacking layer */
void stacking_lower(ObWindow *window);

/*! Moves a window below another if its in the same layer */
void stacking_below(ObWindow *window, ObWindow *below);

#endif
