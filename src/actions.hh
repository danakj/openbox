// -*- mode: C++; indent-tabs-mode: nil; -*-
#ifndef __actions_hh
#define __actions_hh

/*! @file actions.hh
  @brief The action interface for user-available actions
*/

#include "otk/display.hh"
#include "otk/point.hh"
#include "otk/rect.hh"
#include "otk/eventhandler.hh"

namespace ob {

//! The action interface for user-available actions
/*!
  When these actions are fired, hooks to the guile engine are fired so that
  guile code is run.
*/
class OBActions : public otk::OtkEventHandler {
public:
  struct MouseButtonAction {
    Window win;
    unsigned int button;
    Time time;
    MouseButtonAction() { win = 0; button = 0; time = 0; }
  };
  
private:
  // milliseconds XXX: config option
  static const unsigned int DOUBLECLICKDELAY;
  
  //! The last 2 button release processed for CLICKs
  MouseButtonAction _release;
  //! The mouse button currently being watched from a press for a CLICK
  unsigned int _button;

  void insertPress(Window win, unsigned int button, Time time);
  
public:
  OBActions();
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
