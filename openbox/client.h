#ifndef __client_h
#define __client_h

#include "geom.h"
#include "stacking.h"
#include <glib.h>
#include <X11/Xlib.h>

struct ClientWrap;
struct Frame;


/*! Holds an icon in ARGB format */
typedef struct Icon {
    unsigned long w, h;
    unsigned long *data;
} Icon;
     
/*! The MWM Hints as retrieved from the window property
  This structure only contains 3 elements, even though the Motif 2.0
  structure contains 5. We only use the first 3, so that is all gets
  defined.
*/
typedef struct MwmHints {
    /*! A bitmask of Client::MwmFlags values */
    unsigned long flags;
    /*! A bitmask of Client::MwmFunctions values */
    unsigned long functions;
    /*! A bitmask of Client::MwmDecorations values */
    unsigned long decorations;
} MwmHints;
/*! The number of elements in the Client::MwmHints struct */
#define MWM_ELEMENTS 3
     
/*! Possible flags for MWM Hints (defined by Motif 2.0) */
typedef enum {
    MwmFlag_Functions   = 1 << 0, /*!< The MMW Hints define funcs */
    MwmFlag_Decorations = 1 << 1  /*!< The MWM Hints define decor */
} MwmFlags;

/*! Possible functions for MWM Hints (defined by Motif 2.0) */
typedef enum {
    MwmFunc_All      = 1 << 0, /*!< All functions */
    MwmFunc_Resize   = 1 << 1, /*!< Allow resizing */
    MwmFunc_Move     = 1 << 2, /*!< Allow moving */
    MwmFunc_Iconify  = 1 << 3, /*!< Allow to be iconfied */
    MwmFunc_Maximize = 1 << 4  /*!< Allow to be maximized */
    /*MwmFunc_Close    = 1 << 5 /!< Allow to be closed */
} MwmFunctions;

/*! Possible decorations for MWM Hints (defined by Motif 2.0) */
typedef enum {
    MwmDecor_All      = 1 << 0, /*!< All decorations */
    MwmDecor_Border   = 1 << 1, /*!< Show a border */
    MwmDecor_Handle   = 1 << 2, /*!< Show a handle (bottom) */
    MwmDecor_Title    = 1 << 3, /*!< Show a titlebar */
    /*MwmDecor_Menu     = 1 << 4, /!< Show a menu */
    MwmDecor_Iconify  = 1 << 5, /*!< Show an iconify button */
    MwmDecor_Maximize = 1 << 6  /*!< Show a maximize button */
} MemDecorations;

/*! Corners of the client window, used for anchor positions */
typedef enum {
    Corner_TopLeft,
    Corner_TopRight,
    Corner_BottomLeft,
    Corner_BottomRight
} Corner;

/*! Possible window types */
typedef enum {
    Type_Desktop, /*!< A desktop (bottom-most window) */
    Type_Dock,    /*!< A dock bar/panel window */
    Type_Toolbar, /*!< A toolbar window, pulled off an app */
    Type_Menu,    /*!< An unpinned menu from an app */
    Type_Utility, /*!< A small utility window such as a palette */
    Type_Splash,  /*!< A splash screen window */
    Type_Dialog,  /*!< A dialog window */
    Type_Normal   /*!< A normal application window */
} WindowType;

/*! The things the user can do to the client window */
typedef enum {
    Func_Resize     = 1 << 0, /*!< Allow resizing */
    Func_Move       = 1 << 1, /*!< Allow moving */
    Func_Iconify    = 1 << 2, /*!< Allow to be iconified */
    Func_Maximize   = 1 << 3, /*!< Allow to be maximized */
    Func_Shade      = 1 << 4, /*!< Allow to be shaded */
    Func_Fullscreen = 1 << 5, /*!< Allow to be made fullscreen */
    Func_Close      = 1 << 6  /*!< Allow to be closed */
} Function;

/*! The decorations the client window wants to be displayed on it */
typedef enum {
    Decor_Titlebar    = 1 << 0, /*!< Display a titlebar */
    Decor_Handle      = 1 << 1, /*!< Display a handle (bottom) */
    Decor_Border      = 1 << 2, /*!< Display a border */
    Decor_Icon        = 1 << 3, /*!< Display the window's icon */
    Decor_Iconify     = 1 << 4, /*!< Display an iconify button */
    Decor_Maximize    = 1 << 5, /*!< Display a maximize button */
    /*! Display a button to toggle the window's placement on
      all desktops */
    Decor_AllDesktops = 1 << 6,
    Decor_Close       = 1 << 7  /*!< Display a close button */
} Decoration;


typedef struct Client {
    Window  window;

    struct Frame *frame;

    /*! The number of unmap events to ignore on the window */
    int ignore_unmaps;

    /*! The id of the group the window belongs to */
    Window  group;
    /*! Whether or not the client is a transient window. This is guaranteed to 
      be TRUE if transient_for != NULL, but not guaranteed to be FALSE if
      transient_for == NULL. */
    gboolean transient;
    /*! The client which this client is a transient (child) for */
    struct Client *transient_for;
    /*! The clients which are transients (children) of this client */
    GSList *transients;
    /*! The desktop on which the window resides (0xffffffff for all
      desktops) */
    unsigned int desktop;

    /*! Normal window title */
    gchar *title;
    /*! Window title when iconified */
    gchar *icon_title;

    /*! The application that created the window */
    gchar *res_name;
    /*! The class of the window, can used for grouping */
    gchar *res_class;
    /*! The specified role of the window, used for identification */
    gchar *role;

    /*! The type of window (what its function is) */
    WindowType type;

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
    MwmHints mwmhints;
  
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
    /*! Does the client window have the input focus? */
    gboolean focused;

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
    StackLayer layer;

    /*! A bitmask of values in the Decoration enum
      The values in the variable are the decorations that the client wants to
      be displayed around it.
    */
    int decorations;

    /*! A bitmask of values in the Decoration enum.
      Specifies the decorations that should NOT be displayed on the client.
    */
    int disabled_decorations;

    /*! A bitmask of values in the Function enum
      The values in the variable specify the ways in which the user is allowed
      to modify this window.
    */
    int functions;

    /*! Icons for the client as specified on the client window */
    Icon *icons;
    /*! The number of icons in icons */
    int nicons;

    /*! The icon for the client specified in the WMHints or the KWM hints */
    Pixmap pixmap_icon;
    /*! The mask for the pixmap_icon, or None if its not masked */
    Pixmap pixmap_icon_mask;

    /* The instance of the wrapper class if one exists */
    struct ClientWrap *wrap;
} Client;

extern GSList *client_list;
extern GHashTable *client_map;

void client_startup();
void client_shutdown();

/*! Manages all existing windows */
void client_manage_all();
/*! Manages a given window */
void client_manage(Window win);
/*! Unmanages all managed windows */
void client_unmanage_all();
/*! Unmanages a given client */
void client_unmanage(Client *client);

/*! Sets the client list on the root window from the client_list */
void client_set_list();

/*! Reapplies the maximized state to the window
  Use this to make the window readjust its maximized size to new
  surroundings (struts, etc). */
void client_remaximize(Client *self);

/*! Shows the window if it should be shown, or hides it
  Used when changing desktops, the window's state, etc. */
void client_showhide(Client *self, gboolean firehook);

/*! Returns if the window should be treated as a normal window.
  Some windows (desktops, docks, splash screens) have special rules applied
  to them in a number of places regarding focus or user interaction. */
gboolean client_normal(Client *self);

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
*/
void client_configure(Client *self, Corner anchor, int x, int y, int w, int h,
		      gboolean user, gboolean final);

/*! Fullscreen's or unfullscreen's the client window
  @param fs true if the window should be made fullscreen; false if it should
            be returned to normal state.
  @param savearea true to have the client's current size and position saved;
                  otherwise, they are not. You should not save when mapping a
		  new window that is set to fullscreen. This has no effect
		  when restoring a window from fullscreen.
*/
void client_fullscreen(Client *self, gboolean fs, gboolean savearea);

/*! Iconifies or uniconifies the client window
  @param iconic true if the window should be iconified; false if it should be
                restored.
  @param curdesk If iconic is FALSE, then this determines if the window will
                 be uniconified to the current viewable desktop (true) or to
		 its previous desktop (false)
*/
void client_iconify(Client *self, gboolean iconic, gboolean curdesk);

/*! Maximize or unmaximize the client window
  @param max true if the window should be maximized; false if it should be
             returned to normal size.
  @param dir 0 to set both horz and vert, 1 to set horz, 2 to set vert.
  @param savearea true to have the client's current size and position saved;
                  otherwise, they are not. You should not save when mapping a
		  new window that is set to fullscreen. This has no effect
		  when unmaximizing a window.
*/
void client_maximize(Client *self, gboolean max, int dir,
		     gboolean savearea);

/*! Shades or unshades the client window
  @param shade true if the window should be shaded; false if it should be
               unshaded.
*/
void client_shade(Client *self, gboolean shade);

/*! Request the client to close its window. */
void client_close(Client *self);

/*! Sends the window to the specified desktop */
void client_set_desktop(Client *self, unsigned int target);

/*! Return a modal child of the client window
    @return A modal child of the client window, or 0 if none was found.
*/
Client *client_find_modal_child(Client *self);

/*! Validate client, by making sure no Destroy or Unmap events exist in
  the event queue for the window.
  @return true if the client is valid; false if the client has already
          been unmapped/destroyed, and so is invalid.
*/
gboolean client_validate(Client *self);

/*! Sets the wm_state to the specified value */
void client_set_wm_state(Client *self, long state);

/*! Adjusts the window's net_state
  This should not be called as part of the window mapping process! It is for
  use when updating the state post-mapping.<br>
  client_apply_startup_state is used to do the same things during the mapping
  process.
*/
void client_set_state(Client *self, Atom action, long data1, long data2);

/*! Attempt to focus the client window */
gboolean client_focus(Client *self);

/*! Remove focus from the client window */
void client_unfocus(Client *self);

/*! Calculates the stacking layer for the client window */
void client_calc_layer(Client *self);

/*! Updates the window's transient status, and any parents of it */
void client_update_transient_for(Client *self);
/*! Update the protocols that the window supports and adjusts things if they
  change */
void client_update_protocols(Client *self);
/*! Updates the WMNormalHints and adjusts things if they change */
void client_update_normal_hints(Client *self);

/*! Updates the WMHints and adjusts things if they change
  @param initstate Whether to read the initial_state property from the
                   WMHints. This should only be used during the mapping
		   process.
*/
void client_update_wmhints(Client *self);
/*! Updates the window's title */
void client_update_title(Client *self);
/*! Updates the window's icon title */
void client_update_icon_title(Client *self);
/*! Updates the window's application name and class */
void client_update_class(Client *self);
/*! Updates the strut for the client */
void client_update_strut(Client *self);
/*! Updates the window's icons */
void client_update_icons(Client *self);
/*! Updates the window's kwm icon */
void client_update_kwm_icon(Client *self);

/*! Set up what decor should be shown on the window and what functions should
  be allowed (Client::decorations and Client::functions).
  This also updates the NET_WM_ALLOWED_ACTIONS hint.
*/
void client_setup_decor_and_functions(Client *self);

/*! Retrieves the window's type and sets Client->type */
void client_get_type(Client *self);

#endif
