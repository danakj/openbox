// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __actions_hh
#define __actions_hh

/*! @file actions.hh
  @brief The action interface for user-available actions
*/

#include "widget.hh"
#include "otk/point.hh"
#include "otk/rect.hh"
#include "otk/eventhandler.hh"

extern "C" {
#include <X11/Xlib.h>
#include <Python.h>
}

#include <map>

namespace ob {

//! The action interface for user-available actions
/*!
  When these actions are fired, hooks to the guile engine are fired so that
  guile code is run.
*/
class Actions : public otk::EventHandler {
public:
#ifndef   SWIG // get rid of a swig warning
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
#endif // SWIG
private:
  // milliseconds XXX: config option
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
  //! This is set to true once a drag has started and false when done to make
  //! sure the threshold isnt checked anymore once a drag is underway
  bool _dragging;

  
  void insertPress(const XButtonEvent &e);
  void removePress(const XButtonEvent &e);

public:
  //! Constructs an Actions object
  Actions();
  //! Destroys the Actions object
  virtual ~Actions();

  virtual void buttonPressHandler(const XButtonEvent &e);
  virtual void buttonReleaseHandler(const XButtonEvent &e);
  
  virtual void enterHandler(const XCrossingEvent &e);
  virtual void leaveHandler(const XCrossingEvent &e);

  virtual void keyPressHandler(const XKeyEvent &e);

  virtual void motionHandler(const XMotionEvent &e);

#ifdef    XKB
  virtual void xkbHandler(const XkbEvent &e);
#endif // XKB

};

}

#endif // __actions_hh
