// -*- mode: C++; indent-tabs-mode: nil; -*-
#ifndef __actions_hh
#define __actions_hh

/*! @file actions.hh
  @brief The action interface for user-available actions
*/

#include "otk/point.hh"
#include "otk/rect.hh"
#include "otk/eventhandler.hh"

extern "C" {
#include <X11/Xlib.h>
}

namespace ob {

//! The action interface for user-available actions
/*!
  When these actions are fired, hooks to the guile engine are fired so that
  guile code is run.
*/
class OBActions : public otk::OtkEventHandler {
public:
  enum ActionType {
    Action_ButtonPress,
    Action_ButtonRelease,
    Action_EnterWindow,
    Action_LeaveWindow,
    Action_KeyPress,
    Action_MouseMotion,
    NUM_ACTIONS
  };
  
  struct ButtonReleaseAction {
    Window win;
    unsigned int button;
    Time time;
    ButtonReleaseAction() { win = 0; button = 0; time = 0; }
  };

  struct ButtonPressAction {
    unsigned int button;
    otk::Point pos;
    otk::Rect clientarea;
    ButtonPressAction() { button = 0; }
  };
  
private:
  // milliseconds XXX: config option
  static const unsigned int DOUBLECLICKDELAY;
  static const int BUTTONS = 5;
  
  //! The mouse button currently being watched from a press for a CLICK
  unsigned int _button;
  //! The last button release processed for CLICKs
  ButtonReleaseAction _release;
  //! The point where the mouse was when each mouse button was pressed
  /*!
    Used for motion events as the starting position.
  */
  ButtonPressAction *_posqueue[BUTTONS];
  //! The delta x/y of the last motion sequence
  int _dx, _dy;

  //! Insert a button/position in the _posqueue
  void insertPress(const XButtonEvent &e);
  //! Remove a button/position from the _posqueue
  void removePress(const XButtonEvent &e);
  
public:
  //! Constructs an OBActions object
  OBActions();
  //! Destroys the OBActions object
  virtual ~OBActions();

  virtual void buttonPressHandler(const XButtonEvent &e);
  virtual void buttonReleaseHandler(const XButtonEvent &e);
  
  virtual void enterHandler(const XCrossingEvent &e);
  virtual void leaveHandler(const XCrossingEvent &e);

  virtual void keyPressHandler(const XKeyEvent &e);

  virtual void motionHandler(const XMotionEvent &e);
};

}

#endif // __actions_hh
