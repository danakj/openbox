// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __client_hh
#define   __client_hh

extern "C" {
#include <X11/Xlib.h>
}

#include <string>

#include "otk/strut.hh"
#include "otk/rect.hh"

namespace ob {

class OBClient {
public:
  enum Max { Max_None,
             Max_Horz,
             Max_Vert,
             Max_Full };

  enum WindowType { Type_Desktop,
                    Type_Dock,
                    Type_Toolbar,
                    Type_Menu,
                    Type_Utility,
                    Type_Splash,
                    Type_Dialog,
                    Type_Normal };

  enum MwmFlags { Functions   = 1 << 0,
                  Decorations = 1 << 1 };

  enum MwmFunctions { MwmFunc_All      = 1 << 0,
                      MwmFunc_Resize   = 1 << 1,
                      MwmFunc_Move     = 1 << 2,
                      MwmFunc_Iconify  = 1 << 3,
                      MwmFunc_Maximize = 1 << 4,
                      MwmFunc_Close    = 1 << 5 };

  enum MemDecorations { MemDecor_All      = 1 << 0,
                        MemDecor_Border   = 1 << 1,
                        MemDecor_Handle   = 1 << 2,
                        MemDecor_Title    = 1 << 3,
                        //MemDecor_Menu     = 1 << 4,
                        MemDecor_Iconify  = 1 << 5,
                        MemDecor_Maximize = 1 << 6 };

  // this structure only contains 3 elements... the Motif 2.0 structure
  // contains 5... we only need the first 3... so that is all we will define
  typedef struct MwmHints {
    static const int elements = 3;
    unsigned long flags;
    unsigned long functions;
    unsigned long decorations;
  };

  enum StateAction { State_Remove = 0, // _NET_WM_STATE_REMOVE
                     State_Add,        // _NET_WM_STATE_ADD
                     State_Toggle      // _NET_WM_STATE_TOGGLE
  };

private:
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

  //! Position and size of the window (relative to the root window)
  otk::Rect    _area;

  // size bounds
  // if min > max, then the window is not resizable
  int _min_x, _min_y; // minumum size
  int _max_x, _max_y; // maximum size
  int _inc_x, _inc_y; // resize increments
  int _base_x, _base_y; // base size

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

  // XXX: motif decoration hints!

  void getDesktop();
  void getType();
  void getArea();
  void getState();
  void getShaped();

  void setWMState(long state);
  void setDesktop(long desktop);
  void setState(StateAction action, long data1, long data2);
  
  void updateNormalHints();
  void updateWMHints();
  // XXX: updateTransientFor();
  void updateTitle();
  void updateClass();

public:
  OBClient(Window window);
  virtual ~OBClient();

  inline Window window() const { return _window; }

  inline WindowType type() const { return _type; }
  inline unsigned long desktop() const { return _desktop; }
  inline const std::string &title() const { return _title; }
  inline const std::string &iconTitle() const { return _title; }
  inline const std::string &appName() const { return _app_name; }
  inline const std::string &appClass() const { return _app_class; }
  inline bool canFocus() const { return _can_focus; }
  inline bool urgent() const { return _urgent; }
  inline bool focusNotify() const { return _focus_notify; }
  inline bool shaped() const { return _shaped; }
  inline int gravity() const { return _gravity; }

  // states
  inline bool modal() const { return _modal; }
  inline bool shaded() const { return _shaded; }
  inline bool iconic() const { return _iconic; }
  inline bool maxVert() const { return _max_vert; }
  inline bool maxHorz() const { return _max_horz; }
  inline bool fullscreen() const { return _fullscreen; }
  inline bool floating() const { return _floating; }

  inline int minX() const { return _min_x; }
  inline int minY() const { return _min_y; }
  inline int maxX() const { return _max_x; }
  inline int maxY() const { return _max_y; }
  inline int incrementX() const { return _inc_x; }
  inline int incrementY() const { return _inc_y; }
  inline int baseX() const { return _base_x; }
  inline int baseY() const { return _base_y; }

  inline const otk::Rect &area() const { return _area; }

  void update(const XPropertyEvent &e);
  void update(const XClientMessageEvent &e);

  void setArea(const otk::Rect &area);
};

}

#endif // __client_hh
