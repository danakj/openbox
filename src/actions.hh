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
class OBActions : public otk::OtkEventHandler {
public:
  // update the same enum in openbox.i when making changes to this
  enum ActionType {
    Action_EnterWindow,
    Action_LeaveWindow,
    Action_NewWindow,
    Action_CloseWindow,
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

  
  void insertPress(const XButtonEvent &e);
  void removePress(const XButtonEvent &e);
  
  typedef std::multimap<ActionType, PyObject*> CallbackMap;
  typedef std::pair<ActionType, PyObject*> CallbackMapPair;
  CallbackMap _callbacks;

  void doCallback(ActionType action, Window window, OBWidget::WidgetType type,
                  unsigned int state, unsigned int button,
                  int xroot, int yroot, Time time);
  
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

  virtual void mapRequestHandler(const XMapRequestEvent &e);
  virtual void unmapHandler(const XUnmapEvent &e);
  virtual void destroyHandler(const XDestroyWindowEvent &e);

  //! Add a callback funtion to the back of the hook list
  /*!
    Registering functions for KeyPress events is pointless. Use
    OBSCript::bindKey instead to do this.
  */
  bool registerCallback(ActionType action, PyObject *func, bool atfront);

  //! Remove a callback function from the hook list
  bool unregisterCallback(ActionType action, PyObject *func);

  //! Remove all callback functions from the hook list
  bool unregisterAllCallbacks(ActionType action);
};

}

#endif // __actions_hh
