// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
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
  // update the same enum in openbox.i when making changes to this
  enum ActionType {
    Action_ButtonPress,
    Action_ButtonRelease,
    Action_Click,
    Action_DoubleClick,
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

private:
  // milliseconds XXX: config option
  static const unsigned int DOUBLECLICKDELAY;
  
  //! The mouse button currently being watched from a press for a CLICK
  unsigned int _button;
  //! The last button release processed for CLICKs
  ButtonReleaseAction _release;

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
