#ifndef __stacking_h
#define __stacking_h

#include <glib.h>

struct Client;

/*! The possible stacking layers a client window can be a part of */
typedef enum {
    Layer_Icon,       /*!< 0 - iconified windows, in any order at all */
    Layer_Desktop,    /*!< 1 - desktop windows */
    Layer_Below,      /*!< 2 - normal windows w/ below */
    Layer_Normal,     /*!< 3 - normal windows */
    Layer_Above,      /*!< 4 - normal windows w/ above */
    Layer_Top,        /*!< 5 - always-on-top-windows (docks?) */
    Layer_Fullscreen, /*!< 6 - fullscreeen windows */
    Layer_Internal    /*!< 7 - openbox windows/menus */
} StackLayer;

/* list of Client*s in stacking order from highest to lowest */
extern GList  *stacking_list;

/*! Sets the client stacking list on the root window from the
  stacking_clientlist */
void stacking_set_list();

/*! Raises a client window above all others in its stacking layer
  raiseWindow has a couple of constraints that lowerWindow does not.<br>
  1) raiseWindow can be called after changing a Client's stack layer, and
     the list will be reorganized properly.<br>
  2) raiseWindow guarantees that XRestackWindows() will <i>always</i> be
     called for the specified client.
*/
void stacking_raise(struct Client *client);

/*! Lowers a client window below all others in its stacking layer */
void stacking_lower(struct Client *client);

#endif
