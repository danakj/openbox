#ifndef __stacking_h
#define __stacking_h

#include "window.h"

#include <glib.h>
#include <X11/Xlib.h>

/*! The possible stacking layers a client window can be a part of */
typedef enum {
    Layer_Desktop,    /*!< 0 - desktop windows */
    Layer_Below,      /*!< 1 - normal windows w/ below */
    Layer_Normal,     /*!< 2 - normal windows */
    Layer_Above,      /*!< 3 - normal windows w/ above */
    Layer_Top,        /*!< 4 - always-on-top-windows (docks?) */
    Layer_Fullscreen, /*!< 5 - fullscreeen windows */
    Layer_Internal    /*!< 6 - openbox windows/menus */
} StackLayer;

/* list of ObWindow*s in stacking order from highest to lowest */
extern GList  *stacking_list;

/*! Sets the window stacking list on the root window from the
  stacking_list */
void stacking_set_list();

#define stacking_add(win) stacking_list = g_list_append(stacking_list, win);
#define stacking_remove(win) stacking_list = g_list_remove(stacking_list, win);

/*! Raises a window above all others in its stacking layer
  raiseWindow has a couple of constraints that lowerWindow does not.<br>
  1) raiseWindow can be called after changing a Window's stack layer, and
     the list will be reorganized properly.<br>
  2) raiseWindow guarantees that XRestackWindows() will <i>always</i> be
     called for the specified window.
*/
void stacking_raise(ObWindow *window);

/*! Lowers a client window below all others in its stacking layer */
void stacking_lower(ObWindow *window);

#endif
