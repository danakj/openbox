#ifndef __client_h
#define __client_h

#include "misc.h"
#include "mwm.h"
#include "geom.h"
#include "stacking.h"
#include "render/color.h"

#include <glib.h>
#include <X11/Xlib.h>

struct _ObFrame;
struct _ObGroup;

typedef struct _ObClient     ObClient;
typedef struct _ObClientIcon ObClientIcon;

/* The value in client.transient_for indicating it is a transient for its
   group instead of for a single window */
#define OB_TRAN_GROUP ((void*)~0l)

/*! Holds an icon in ARGB format */
struct _ObClientIcon
{
    gint width;
    gint height;
    RrPixel32 *data;
};
     
/*! Possible window types */
typedef enum
{
    OB_CLIENT_TYPE_DESKTOP, /*!< A desktop (bottom-most window) */
    OB_CLIENT_TYPE_DOCK,    /*!< A dock bar/panel window */
    OB_CLIENT_TYPE_TOOLBAR, /*!< A toolbar window, pulled off an app */
    OB_CLIENT_TYPE_MENU,    /*!< An unpinned menu from an app */
    OB_CLIENT_TYPE_UTILITY, /*!< A small utility window such as a palette */
    OB_CLIENT_TYPE_SPLASH,  /*!< A splash screen window */
    OB_CLIENT_TYPE_DIALOG,  /*!< A dialog window */
    OB_CLIENT_TYPE_NORMAL   /*!< A normal application window */
} ObClientType;

/*! The things the user can do to the client window */
typedef enum
{
    OB_CLIENT_FUNC_RESIZE     = 1 << 0, /*!< Allow user resizing */
    OB_CLIENT_FUNC_MOVE       = 1 << 1, /*!< Allow user moving */
    OB_CLIENT_FUNC_ICONIFY    = 1 << 2, /*!< Allow to be iconified */
    OB_CLIENT_FUNC_MAXIMIZE   = 1 << 3, /*!< Allow to be maximized */
    OB_CLIENT_FUNC_SHADE      = 1 << 4, /*!< Allow to be shaded */
    OB_CLIENT_FUNC_FULLSCREEN = 1 << 5, /*!< Allow to be made fullscreen */
    OB_CLIENT_FUNC_CLOSE      = 1 << 6  /*!< Allow to be closed */
} ObFunctions;

struct _ObClient
{
    ObWindow obwin;

    Window  window;

    /*! The window's decorations. NULL while the window is being managed! */
    struct _ObFrame *frame;

    /*! The number of unmap events to ignore on the window */
    int ignore_unmaps;

    /*! The id of the group the window belongs to */
    struct _ObGroup *group;
    /*! Whether or not the client is a transient window. This is guaranteed to 
      be TRUE if transient_for != NULL, but not guaranteed to be FALSE if
      transient_for == NULL. */
    gboolean transient;
    /*! The client which this client is a transient (child) for.
      A value of TRAN_GROUP signifies that the window is a transient for all
      members of its ObGroup, and is not a valid pointer to be followed in this
      case.
     */
    ObClient *transient_for;
    /*! The clients which are transients (children) of this client */
    GSList *transients;
    /*! The desktop on which the window resides (0xffffffff for all
      desktops) */
    guint desktop;

    /*! Normal window title */
    gchar *title;
    /*! The count for the title. When another window with the same title
      exists, a count will be appended to it. */
    guint title_count;
    /*! Window title when iconified */
    gchar *icon_title;

    /*! The application that created the window */
    gchar *name;
    /*! The class of the window, can used for grouping */
    gchar *class;
    /*! The specified role of the window, used for identification */
    gchar *role;

    /*! The type of window (what its function is) */
    ObClientType type;

    /*! Position and size of the window
      This will not always be the actual position of the window on screen, it
      is, rather, the position requested by the client, to which the window's
      gravity is applied.
    */
    Rect    area;

    /*! The window's strut
      The strut defines areas of the screen that are marked off-bounds for
      window placement. In theory, where this window exists.
    */
    Strut   strut;
     
    /*! The logical size of the window
      The "logical" size of the window is refers to the user's perception of
      the size of the window, and is the value that should be displayed to the
      user. For example, with xterms, this value it the number of characters
      being displayed in the terminal, instead of the number of pixels.
    */
    Size   logical_size;

    /*! Width of the border on the window.
      The window manager will set this to 0 while the window is being managed,
      but needs to restore it afterwards, so it is saved here.
    */
    guint border_width;

    /*! The minimum aspect ratio the client window can be sized to.
      A value of 0 means this is ignored.
    */
    float min_ratio;
    /*! The maximum aspect ratio the client window can be sized to.
      A value of 0 means this is ignored.
    */
    float max_ratio;
  
    /*! The minimum size of the client window
      If the min is > the max, then the window is not resizable
    */
    Size min_size;
    /*! The maximum size of the client window
      If the min is > the max, then the window is not resizable
    */
    Size max_size;
    /*! The size of increments to resize the client window by */
    Size size_inc;
    /*! The base size of the client window
      This value should be subtracted from the window's actual size when
      displaying its size to the user, or working with its min/max size
    */
    Size base_size;

    /*! Window decoration and functionality hints */
    ObMwmHints mwmhints;
  
    /*! Where to place the decorated window in relation to the undecorated
      window */
    int gravity;

    /*! The state of the window, one of WithdrawnState, IconicState, or
      NormalState */
    long wmstate;

    /*! True if the client supports the delete_window protocol */
    gboolean delete_window;
  
    /*! Was the window's position requested by the application? if not, we
      should place the window ourselves when it first appears */
    gboolean positioned;
  
    /*! Can the window receive input focus? */
    gboolean can_focus;
    /*! Urgency flag */
    gboolean urgent;
    /*! Notify the window when it receives focus? */
    gboolean focus_notify;

    /*! The window uses shape extension to be non-rectangular? */
    gboolean shaped;

    /*! The window is modal, so it must be processed before any windows it is
      related to can be focused */
    gboolean modal;
    /*! Only the window's titlebar is displayed */
    gboolean shaded;
    /*! The window is iconified */
    gboolean iconic;
    /*! The window is maximized to fill the screen vertically */
    gboolean max_vert;
    /*! The window is maximized to fill the screen horizontally */
    gboolean max_horz;
    /*! The window should not be displayed by pagers */
    gboolean skip_pager;
    /*! The window should not be displayed by taskbars */
    gboolean skip_taskbar;
    /*! The window is a 'fullscreen' window, and should be on top of all
      others */
    gboolean fullscreen;
    /*! The window should be on top of other windows of the same type.
      above takes priority over below. */
    gboolean above;
    /*! The window should be underneath other windows of the same type.
      above takes priority over below. */
    gboolean below;

    /*! The layer in which the window will be stacked, windows in lower layers
      are always below windows in higher layers. */
    ObStackingLayer layer;

    /*! A bitmask of values in the ObFrameDecorations enum
      The values in the variable are the decorations that the client wants to
      be displayed around it.
    */
    guint decorations;

    /*! A user option. When this is set to FALSE the client will not ever
      be decorated.
    */
    gboolean decorate;

    /*! A bitmask of values in the ObFunctions enum
      The values in the variable specify the ways in which the user is allowed
      to modify this window.
    */
    guint functions;

    /*! Icons for the client as specified on the client window */
    ObClientIcon *icons;
    /*! The number of icons in icons */
    guint nicons;
};

extern GList *client_list;

void client_startup();
void client_shutdown();

/*! Manages all existing windows */
void client_manage_all();
/*! Manages a given window */
void client_manage(Window win);
/*! Unmanages all managed windows */
void client_unmanage_all();
/*! Unmanages a given client */
void client_unmanage(ObClient *client);

/*! Sets the client list on the root window from the client_list */
void client_set_list();

/*! Determines if the client should be shown or hidden currently.
  @return TRUE if it should be visible; otherwise, FALSE.
*/
gboolean client_should_show(ObClient *self);

/*! Returns if the window should be treated as a normal window.
  Some windows (desktops, docks, splash screens) have special rules applied
  to them in a number of places regarding focus or user interaction. */
gboolean client_normal(ObClient *self);

/* Returns if the window is focused */
gboolean client_focused(ObClient *self);

#define client_configure(self, anchor, x, y, w, h, user, final) \
  client_configure_full(self, anchor, x, y, w, h, user, final, FALSE)

/*! Move and/or resize the window.
  This also maintains things like the client's minsize, and size increments.
  @param anchor The corner to keep in the same position when resizing.
  @param x The x coordiante of the new position for the client.
  @param y The y coordiante of the new position for the client.
  @param w The width component of the new size for the client.
  @param h The height component of the new size for the client.
  @param user Specifies whether this is a user-requested change or a
              program requested change. For program requested changes, the
	      constraints are not checked.
  @param final If user is true, then this should specify if this is a final
               configuration. e.g. Final should be FALSE if doing an
	       interactive move/resize, and then be TRUE for the last call
	       only.
  @param force_reply Send a ConfigureNotify to the client regardless of if
                     the position changed.
*/
void client_configure_full(ObClient *self, ObCorner anchor,
                           int x, int y, int w, int h,
                           gboolean user, gboolean final,
                           gboolean force_reply);

void client_reconfigure(ObClient *self);

/*! Finds coordinates to keep a client on the screen.
  @param self The client
  @param x The x coord of the client, may be changed.
  @param y The y coord of the client, may be changed.
  @param w The width of the client.
  @param w The height of the client.
  @param rude Be rude about it. If false, it is only moved if it is entirely
              not visible. If true, then make sure the window is inside the
              struts if possible.
  @return true if the client was moved to be on-screen; false if not.
*/
gboolean client_find_onscreen(ObClient *self, int *x, int *y, int w, int h,
                              gboolean rude);

/*! Moves a client so that it is on screen if it is entirely out of the
  viewable screen.
  @param self The client to move
  @param rude Be rude about it. If false, it is only moved if it is entirely
              not visible. If true, then make sure the window is inside the
              struts if possible.
*/
void client_move_onscreen(ObClient *self, gboolean rude);

/*! Fullscreen's or unfullscreen's the client window
  @param fs true if the window should be made fullscreen; false if it should
            be returned to normal state.
  @param savearea true to have the client's current size and position saved;
                  otherwise, they are not. You should not save when mapping a
		  new window that is set to fullscreen. This has no effect
		  when restoring a window from fullscreen.
*/
void client_fullscreen(ObClient *self, gboolean fs, gboolean savearea);

/*! Iconifies or uniconifies the client window
  @param iconic true if the window should be iconified; false if it should be
                restored.
  @param curdesk If iconic is FALSE, then this determines if the window will
                 be uniconified to the current viewable desktop (true) or to
		 its previous desktop (false)
*/
void client_iconify(ObClient *self, gboolean iconic, gboolean curdesk);

/*! Maximize or unmaximize the client window
  @param max true if the window should be maximized; false if it should be
             returned to normal size.
  @param dir 0 to set both horz and vert, 1 to set horz, 2 to set vert.
  @param savearea true to have the client's current size and position saved;
                  otherwise, they are not. You should not save when mapping a
		  new window that is set to fullscreen. This has no effect
		  when unmaximizing a window.
*/
void client_maximize(ObClient *self, gboolean max, int dir,
		     gboolean savearea);

/*! Shades or unshades the client window
  @param shade true if the window should be shaded; false if it should be
               unshaded.
*/
void client_shade(ObClient *self, gboolean shade);

/*! Request the client to close its window */
void client_close(ObClient *self);

/*! Kill the client off violently */
void client_kill(ObClient *self);

/*! Sends the window to the specified desktop
  @param donthide If TRUE, the window will not be shown/hidden after its
         desktop has been changed. Generally this should be FALSE. */
void client_set_desktop(ObClient *self, guint target, gboolean donthide);

/*! Validate client, by making sure no Destroy or Unmap events exist in
  the event queue for the window.
  @return true if the client is valid; false if the client has already
          been unmapped/destroyed, and so is invalid.
*/
gboolean client_validate(ObClient *self);

/*! Sets the wm_state to the specified value */
void client_set_wm_state(ObClient *self, long state);

/*! Adjusts the window's net_state
  This should not be called as part of the window mapping process! It is for
  use when updating the state post-mapping.<br>
  client_apply_startup_state is used to do the same things during the mapping
  process.
*/
void client_set_state(ObClient *self, Atom action, long data1, long data2);

/* Given a ObClient, find the client that focus would actually be sent to if
   you wanted to give focus to the specified ObClient. Will return the same
   ObClient passed to it or another ObClient if appropriate. */
ObClient *client_focus_target(ObClient *self);

/*! Returns what client_focus would return if passed the same client, but
  without focusing it or modifying the focus order lists. */
gboolean client_can_focus(ObClient *self);

/*! Attempt to focus the client window */
gboolean client_focus(ObClient *self);

/*! Remove focus from the client window */
void client_unfocus(ObClient *self);

/*! Activates the client for use, focusing, uniconifying it, etc. To be used
  when the user deliberately selects a window for use. */
void client_activate(ObClient *self);

/*! Calculates the stacking layer for the client window */
void client_calc_layer(ObClient *self);

/*! Updates the window's transient status, and any parents of it */
void client_update_transient_for(ObClient *self);
/*! Update the protocols that the window supports and adjusts things if they
  change */
void client_update_protocols(ObClient *self);
/*! Updates the WMNormalHints and adjusts things if they change */
void client_update_normal_hints(ObClient *self);

/*! Updates the WMHints and adjusts things if they change
  @param initstate Whether to read the initial_state property from the
                   WMHints. This should only be used during the mapping
		   process.
*/
void client_update_wmhints(ObClient *self);
/*! Updates the window's title and icon title */
void client_update_title(ObClient *self);
/*! Updates the window's application name and class */
void client_update_class(ObClient *self);
/*! Updates the strut for the client */
void client_update_strut(ObClient *self);
/*! Updates the window's icons */
void client_update_icons(ObClient *self);

/*! Set up what decor should be shown on the window and what functions should
  be allowed (ObClient::decorations and ObClient::functions).
  This also updates the NET_WM_ALLOWED_ACTIONS hint.
*/
void client_setup_decor_and_functions(ObClient *self);

/*! Retrieves the window's type and sets ObClient->type */
void client_get_type(ObClient *self);

ObClientIcon *client_icon(ObClient *self, int w, int h);

/*! Searches a client's transients for a focused window. The function does not
  check for the passed client, only for its transients.
  If no focused transient is found, NULL is returned.
*/
ObClient *client_search_focus_tree(ObClient *self);

/*! Searches a client's transient tree for a focused window. The function
  searches up the tree and down other branches as well as the passed client's.
  If no focused client is found, NULL is returned.
*/
ObClient *client_search_focus_tree_full(ObClient *self);

/*! Return a modal child of the client window that can be focused.
    @return A modal child of the client window that can be focused, or 0 if
            none was found.
*/
ObClient *client_search_modal_child(ObClient *self);

ObClient *client_search_top_transient(ObClient *self);

ObClient *client_search_transient(ObClient *self, ObClient *search);

/*! Return the "closest" client in the given direction */
ObClient *client_find_directional(ObClient *c, ObDirection dir);

/*! Set a client window to be above/below other clients.
  @layer < 0 indicates the client should be placed below other clients.<br>
         = 0 indicates the client should be placed with other clients.<br>
         > 0 indicates the client should be placed above other clients.
*/
void client_set_layer(ObClient *self, int layer);

guint client_monitor(ObClient *self);

gchar* client_get_sm_client_id(ObClient *self);

#endif
