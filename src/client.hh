// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __client_hh
#define   __client_hh

/*! @file client.hh
  @brief The OBClient class maintains the state of a client window by handling
  property changes on the window and some client messages
*/

extern "C" {
#include <X11/Xlib.h>

#ifdef    SHAPE
#include <X11/extensions/shape.h>
#endif // SHAPE
}

#include <string>

#include "otk/point.hh"
#include "otk/strut.hh"
#include "otk/rect.hh"
#include "otk/eventhandler.hh"
#include "widget.hh"

namespace ob {

class OBFrame;

//! Maintains the state of a client window.
/*!
  OBClient maintains the state of a client window. The state consists of the
  hints that the application sets on the window, such as the title, or window
  gravity.
  <p>
  OBClient also manages client messages for the client window. When the
  application (or any application) requests something to be changed for the
  client, it will call the ActionHandler (for client messages) or update the
  class' member variables and call whatever is nessary to complete the
  change (such as causing a redraw of the titlebar after the title is changed).
*/
class OBClient : public otk::OtkEventHandler, public OBWidget {
public:

  //! The frame window which decorates around the client window
  /*!
    NOTE: This should NEVER be used inside the client class's constructor!
  */
  OBFrame *frame;

  //! Corners of the client window, used for anchor positions
  enum Corner { TopLeft,
                TopRight,
                BottomLeft,
                BottomRight };

  //! Possible window types
  enum WindowType { Type_Desktop, //!< A desktop (bottom-most window)
                    Type_Dock,    //!< A dock bar/panel window
                    Type_Toolbar, //!< A toolbar window, pulled off an app
                    Type_Menu,    //!< A sticky menu from an app
                    Type_Utility, //!< A small utility window such as a palette
                    Type_Splash,  //!< A splash screen window
                    Type_Dialog,  //!< A dialog window
                    Type_Normal   //!< A normal application window
  };

  //! Possible flags for MWM Hints (defined by Motif 2.0)
  enum MwmFlags { MwmFlag_Functions   = 1 << 0, //!< The MMW Hints define funcs
                  MwmFlag_Decorations = 1 << 1  //!< The MWM Hints define decor
  };

  //! Possible functions for MWM Hints (defined by Motif 2.0)
  enum MwmFunctions { MwmFunc_All      = 1 << 0, //!< All functions
                      MwmFunc_Resize   = 1 << 1, //!< Allow resizing
                      MwmFunc_Move     = 1 << 2, //!< Allow moving
                      MwmFunc_Iconify  = 1 << 3, //!< Allow to be iconfied
                      MwmFunc_Maximize = 1 << 4  //!< Allow to be maximized
                      //MwmFunc_Close    = 1 << 5 //!< Allow to be closed
  };

  //! Possible decorations for MWM Hints (defined by Motif 2.0)
  enum MemDecorations { MwmDecor_All      = 1 << 0, //!< All decorations
                        MwmDecor_Border   = 1 << 1, //!< Show a border
                        MwmDecor_Handle   = 1 << 2, //!< Show a handle (bottom)
                        MwmDecor_Title    = 1 << 3, //!< Show a titlebar
                        //MwmDecor_Menu     = 1 << 4, //!< Show a menu
                        MwmDecor_Iconify  = 1 << 5, //!< Show an iconify button
                        MwmDecor_Maximize = 1 << 6  //!< Show a maximize button
  };

  //! The things the user can do to the client window
  enum Function { Func_Resize   = 1 << 0, //!< Allow resizing
                  Func_Move     = 1 << 1, //!< Allow moving
                  Func_Iconify  = 1 << 2, //!< Allow to be iconified
                  Func_Maximize = 1 << 3, //!< Allow to be maximized
                  Func_Close    = 1 << 4  //!< Allow to be closed
  };
  //! Holds a bitmask of OBClient::Function values
  typedef unsigned char FunctionFlags;

  //! The decorations the client window wants to be displayed on it
  enum Decoration { Decor_Titlebar = 1 << 0, //!< Display a titlebar
                    Decor_Handle   = 1 << 1, //!< Display a handle (bottom)
                    Decor_Border   = 1 << 2, //!< Display a border
                    Decor_Iconify  = 1 << 3, //!< Display an iconify button
                    Decor_Maximize = 1 << 4, //!< Display a maximize button
                    Decor_Sticky   = 1 << 5, //!< Display a sticky button
                    Decor_Close    = 1 << 6  //!< Display a close button
  };
  //! Holds a bitmask of OBClient::Decoration values
  typedef unsigned char DecorationFlags;

  //! The MWM Hints as retrieved from the window property
  /*!
    This structure only contains 3 elements, even though the Motif 2.0
    structure contains 5. We only use the first 3, so that is all gets defined.
  */
  typedef struct MwmHints {
    //! The number of elements in the OBClient::MwmHints struct
    static const unsigned int elements = 3;
    unsigned long flags;      //!< A bitmask of OBClient::MwmFlags values
    unsigned long functions;  //!< A bitmask of OBClient::MwmFunctions values
    unsigned long decorations;//!< A bitmask of OBClient::MwmDecorations values
  };

  //! Possible actions that can be made with the _NET_WM_STATE client message
  enum StateAction { State_Remove = 0, //!< _NET_WM_STATE_REMOVE
                     State_Add,        //!< _NET_WM_STATE_ADD
                     State_Toggle      //!< _NET_WM_STATE_TOGGLE
  };

  //! The event mask to grab on client windows
  static const long event_mask = PropertyChangeMask | FocusChangeMask |
                                 StructureNotifyMask;

  //! The mask of events to not let propogate past the client
  /*!
    This makes things like xprop work on the client window, but means we have
    to explicitly grab clicks that we want.
  */
  static const long no_propagate_mask = ButtonPressMask | ButtonReleaseMask |
                                        ButtonMotionMask;

  //! The number of unmap events to ignore on the window
  int ignore_unmaps;
  
private:
  //! The screen number on which the client resides
  int      _screen;
  
  //! The actual window that this class is wrapping up
  Window   _window;

  //! The id of the group the window belongs to
  Window   _group;

  // XXX: transient_for, transients

  //! The desktop on which the window resides (0xffffffff for all desktops)
  unsigned long _desktop;

  //! Normal window title
  std::string  _title; // XXX: Have to keep track if this string is Utf8 or not
  //! Window title when iconifiged
  std::string  _icon_title;

  //! The application that created the window
  std::string  _app_name;
  //! The class of the window, can used for grouping
  std::string  _app_class;

  //! The type of window (what its function is)
  WindowType   _type;

  //! Position and size of the window
  /*!
    This will not always be the actual position of the window on screen, it is
    rather, the position requested by the client, to which the window's gravity
    is applied.
  */
  otk::Rect    _area;

  //! The logical size of the window
  /*!
    The "logical" size of the window is refers to the user's perception of the
    size of the window, and is the value that should be displayed to the user.
    For example, with xterms, this value it the number of characters being
    displayed in the terminal, instead of the number of pixels.
  */
  otk::Point   _logical_size;

  //! Width of the border on the window.
  /*!
    The window manager will set this to 0 while the window is being managed,
    but needs to restore it afterwards, so it is saved here.
  */
  int _border_width;

  //! The minimum size of the client window
  /*!
    If the min is > the max, then the window is not resizable
  */
  otk::Point _min_size;
  //! The maximum size of the client window
  /*!
    If the min is > the max, then the window is not resizable
  */
  otk::Point _max_size;
  //! The size of increments to resize the client window by
  otk::Point _size_inc;
  //! The base size of the client window
  /*!
    This value should be subtracted from the window's actual size when
    displaying its size to the user, or working with its min/max size
  */
  otk::Point _base_size;

  //! Where to place the decorated window in relation to the undecorated window
  int _gravity;

  //! The state of the window, one of WithdrawnState, IconicState, or
  //! NormalState
  long _wmstate;

  //! Was the window's position requested by the application? if not, we should
  //! place the window ourselves when it first appears
  bool _positioned;
  
  //! Can the window receive input focus?
  bool _can_focus;
  //! Urgency flag
  bool _urgent;
  //! Notify the window when it receives focus?
  bool _focus_notify;

  //! The window uses shape extension to be non-rectangular?
  bool _shaped;

  //! The window is modal, so it must be processed before any windows it is
  //! related to can be focused
  bool _modal;
  //! Only the window's titlebar is displayed
  bool _shaded;
  //! The window is iconified
  bool _iconic;
  //! The window is maximized to fill the screen vertically
  bool _max_vert;
  //! The window is maximized to fill the screen horizontally
  bool _max_horz;
  //! The window is a 'fullscreen' window, and should be on top of all others
  bool _fullscreen;
  //! The window should be on top of other windows of the same type
  bool _floating;

  //! A bitmask of values in the OBClient::Decoration enum
  /*!
    The values in the variable are the decorations that the client wants to be
    displayed around it.
  */
  DecorationFlags _decorations;

  //! A bitmask of values in the OBClient::Function enum
  /*!
    The values in the variable specify the ways in which the user is allowed to
    modify this window.
  */
  FunctionFlags _functions;

  //! Retrieves the desktop hint's value and sets OBClient::_desktop
  void getDesktop();
  //! Retrieves the window's type and sets OBClient::_type
  void getType();
  //! Gets the MWM Hints and adjusts OBClient::_functions and
  //! OBClient::_decorations
  void getMwmHints();
  //! Gets the position and size of the window and sets OBClient::_area
  void getArea();
  //! Gets the net_state hint and sets the boolean flags for any states set in
  //! the hint
  void getState();
  //! Determines if the window uses the Shape extension and sets
  //! OBClient::_shaped
  void getShaped();

  //! Sets the wm_state to the specified value
  void setWMState(long state);
  //! Sends the window to the specified desktop
  void setDesktop(long desktop);
  //! Adjusts the window's net_state
  void setState(StateAction action, long data1, long data2);

  //! Update the protocols that the window supports and adjusts things if they
  //! change
  void updateProtocols();
  //! Updates the WMNormalHints and adjusts things if they change
  void updateNormalHints();
  //! Updates the WMHints and adjusts things if they change
  void updateWMHints();
  //! Updates the window's title
  void updateTitle();
  //! Updates the window's icon title
  void updateIconTitle();
  //! Updates the window's application name and class
  void updateClass();
  // XXX: updateTransientFor();

public:
  //! Constructs a new OBClient object around a specified window id
  /*!
    @param window The window id that the OBClient class should handle
    @param screen The screen on which the window resides
  */
  OBClient(int screen, Window window);
  //! Destroys the OBClient object
  virtual ~OBClient();

  //! Returns the screen on which the clien resides
  inline int screen() const { return _screen; }
  
  //! Returns the window id that the OBClient object is handling
  inline Window window() const { return _window; }

  //! Returns the type of the window, one of the OBClient::WindowType values
  inline WindowType type() const { return _type; }
  //! Returns the desktop on which the window resides
  /*!
    This value is a 0-based index.<br>
    A value of 0xffffffff indicates that the window exists on all desktops.
  */
  inline unsigned long desktop() const { return _desktop; }
  //! Returns the window's title
  inline const std::string &title() const { return _title; }
  //! Returns the window's title when it is iconified
  inline const std::string &iconTitle() const { return _title; }
  //! Returns the application's name to whom the window belongs
  inline const std::string &appName() const { return _app_name; }
  //! Returns the class of the window
  inline const std::string &appClass() const { return _app_class; }
  //! Returns if the window can be focused
  /*!
    @return true if the window can receive focusl otherwise, false
  */
  inline bool canFocus() const { return _can_focus; }
  //! Returns if the window has indicated that it needs urgent attention
  inline bool urgent() const { return _urgent; }
  //! Returns if the window wants to be notified when it receives focus
  inline bool focusNotify() const { return _focus_notify; }
  //! Returns if the window uses the Shape extension
  inline bool shaped() const { return _shaped; }
  //! Returns the window's gravity
  /*!
    This value determines where to place the decorated window in relation to
    its position without decorations.<br>
    One of: NorthWestGravity, SouthWestGravity, EastGravity, ...,
    SouthGravity, StaticGravity, ForgetGravity
  */
  inline int gravity() const { return _gravity; }
  //! Returns if the application requested the initial position for the window
  /*!
    If the application did not request a position (this function returns false)
    then the window should be placed intelligently by the window manager
    initially
  */
  inline bool positionRequested() const { return _positioned; }
  //! Returns the decorations that the client window wishes to be displayed on
  //! it
  inline DecorationFlags decorations() const { return _decorations; }
  //! Returns the functions that the user can perform on the window
  inline FunctionFlags funtions() const { return _functions; }

  //! Returns if the window is modal
  /*!
    If the window is modal, then no other windows that it is related to can get
    focus while it exists/remains modal.
  */
  inline bool modal() const { return _modal; }
  //! Returns if the window is shaded
  /*!
    When the window is shaded, only its titlebar is visible, the client itself
    is not mapped
  */
  inline bool shaded() const { return _shaded; }
  //! Returns if the window is iconified
  /*!
    When the window is iconified, it is not visible at all (except in iconbars/
    panels/etc that want to show lists of iconified windows
  */
  inline bool iconic() const { return _iconic; }
  //! Returns if the window is maximized vertically
  inline bool maxVert() const { return _max_vert; }
  //! Returns if the window is maximized horizontally
  inline bool maxHorz() const { return _max_horz; }
  //! Returns if the window is fullscreen
  /*!
    When the window is fullscreen, it is kept above all others
  */
  inline bool fullscreen() const { return _fullscreen; }
  //! Returns if the window is floating
  /*!
    When the window is floating, it is kept above all others in the same
    stacking layer as it
  */
  inline bool floating() const { return _floating; }

  //! Removes or reapplies the client's border to its window
  /*!
    Used when managing and unmanaging a window.
    @param addborder true if adding the border to the client; false if removing
                     from the client
  */
  void toggleClientBorder(bool addborder);

  //! Returns the position and size of the client relative to the root window
  inline const otk::Rect &area() const { return _area; }

  //! Move the client window
  void move(int x, int y);
  
  //! Resizes the client window, anchoring it in a given corner
  /*!
    This also maintains things like the client's minsize, and size increments.
    @param anchor The corner to keep in the same position when resizing
    @param x The X component of the new size for the client
    @param y The Y component of the new size for the client
  */
  void resize(Corner anchor, int x, int y);
  
  virtual void propertyHandler(const XPropertyEvent &e);
  virtual void clientMessageHandler(const XClientMessageEvent &e);
  virtual void shapeHandler(const XShapeEvent &e);
  virtual void configureRequestHandler(const XConfigureRequestEvent &e);
  virtual void unmapHandler(const XUnmapEvent &e);
  virtual void destroyHandler(const XDestroyWindowEvent &e);
};

}

#endif // __client_hh
