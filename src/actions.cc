// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "actions.hh"
#include "widget.hh"
#include "openbox.hh"
#include "client.hh"
#include "python.hh"
#include "bindings.hh"
#include "otk/display.hh"

#include <stdio.h>

namespace ob {

const unsigned int OBActions::DOUBLECLICKDELAY = 300;
const int OBActions::BUTTONS;

OBActions::OBActions()
  : _button(0)
{
  for (int i=0; i<BUTTONS; ++i)
    _posqueue[i] = new ButtonPressAction();
}


OBActions::~OBActions()
{
  for (int i=0; i<BUTTONS; ++i)
    delete _posqueue[i];
}


void OBActions::insertPress(const XButtonEvent &e)
{
  ButtonPressAction *a = _posqueue[BUTTONS - 1];
  for (int i=BUTTONS-1; i>0;)
    _posqueue[i] = _posqueue[--i];
  _posqueue[0] = a;
  a->button = e.button;
  a->pos.setPoint(e.x_root, e.y_root);

  OBClient *c = Openbox::instance->findClient(e.window);
  if (c) a->clientarea = c->area();
}

void OBActions::removePress(const XButtonEvent &e)
{
  ButtonPressAction *a = 0;
  for (int i=0; i<BUTTONS; ++i) {
    if (_posqueue[i]->button == e.button)
      a = _posqueue[i];
    if (a) // found one and removed it
      _posqueue[i] = _posqueue[i+1];
  }
  if (a) { // found one
    _posqueue[BUTTONS-1] = a;
    a->button = 0;
  }
}

void OBActions::buttonPressHandler(const XButtonEvent &e)
{
  OtkEventHandler::buttonPressHandler(e);
  insertPress(e);
  
  // run the PRESS python hook
  OBWidget *w = dynamic_cast<OBWidget*>
    (Openbox::instance->findHandler(e.window));
  assert(w); // everything should be a widget

  unsigned int state = e.state & (ControlMask | ShiftMask | Mod1Mask |
                                  Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask);
  ButtonData *data = new_button_data(e.window, e.time, state, e.button,
                                     w->mcontext(), MousePress);
  Openbox::instance->bindings()->fireButton(data);
  Py_DECREF((PyObject*)data);
    
  if (_button) return; // won't count toward CLICK events

  _button = e.button;
}
  

void OBActions::buttonReleaseHandler(const XButtonEvent &e)
{
  OtkEventHandler::buttonReleaseHandler(e);
  removePress(e);
  
  OBWidget *w = dynamic_cast<OBWidget*>
    (Openbox::instance->findHandler(e.window));
  assert(w); // everything should be a widget

  // not for the button we're watching?
  if (_button != e.button) return;

  _button = 0;

  // find the area of the window
  XWindowAttributes attr;
  if (!XGetWindowAttributes(otk::OBDisplay::display, e.window, &attr)) return;

  // if not on the window any more, it isnt a CLICK
  if (!(e.same_screen && e.x >= 0 && e.y >= 0 &&
        e.x < attr.width && e.y < attr.height))
    return;

  // run the CLICK python hook
  unsigned int state = e.state & (ControlMask | ShiftMask | Mod1Mask |
                                  Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask);
  ButtonData *data = new_button_data(e.window, e.time, state, e.button,
                                     w->mcontext(), MouseClick);
  Openbox::instance->bindings()->fireButton(data);
    

  if (e.time - _release.time < DOUBLECLICKDELAY &&
      _release.win == e.window && _release.button == e.button) {

    // run the DOUBLECLICK python hook
    data->action = MouseDoubleClick;
    Openbox::instance->bindings()->fireButton(data);
    
    // reset so you cant triple click for 2 doubleclicks
    _release.win = 0;
    _release.button = 0;
    _release.time = 0;
  } else {
    // save the button release, might be part of a double click
    _release.win = e.window;
    _release.button = e.button;
    _release.time = e.time;
  }

  Py_DECREF((PyObject*)data);
}


void OBActions::enterHandler(const XCrossingEvent &e)
{
  OtkEventHandler::enterHandler(e);
  
  OBWidget *w = dynamic_cast<OBWidget*>
    (Openbox::instance->findHandler(e.window));

  // run the ENTER python hook
  doCallback(Action_EnterWindow, e.window,
             (OBWidget::WidgetType)(w ? w->type():-1), e.state, 0, 0, 0, 0);
}


void OBActions::leaveHandler(const XCrossingEvent &e)
{
  OtkEventHandler::leaveHandler(e);

  OBWidget *w = dynamic_cast<OBWidget*>
    (Openbox::instance->findHandler(e.window));

  // run the LEAVE python hook
  doCallback(Action_LeaveWindow, e.window,
             (OBWidget::WidgetType)(w ? w->type():-1), e.state, 0, 0, 0, 0);
}


void OBActions::keyPressHandler(const XKeyEvent &e)
{
//  OBWidget *w = dynamic_cast<OBWidget*>
//    (Openbox::instance->findHandler(e.window));

  unsigned int state = e.state & (ControlMask | ShiftMask | Mod1Mask |
                                  Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask);
  Openbox::instance->bindings()->fireKey(state, e.keycode, e.time);
}


void OBActions::motionHandler(const XMotionEvent &e)
{
  if (!e.same_screen) return; // this just gets stupid

  int x_root = e.x_root, y_root = e.y_root;
  
  // compress changes to a window into a single change
  XEvent ce;
  while (XCheckTypedEvent(otk::OBDisplay::display, e.type, &ce)) {
    if (ce.xmotion.window != e.window) {
      XPutBackEvent(otk::OBDisplay::display, &ce);
      break;
    } else {
      x_root = e.x_root;
      y_root = e.y_root;
    }
  }

  OBWidget *w = dynamic_cast<OBWidget*>
    (Openbox::instance->findHandler(e.window));
  assert(w); // everything should be a widget

  // run the MOTION python hook
  unsigned int state = e.state & (ControlMask | ShiftMask | Mod1Mask |
                                  Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask);
  unsigned int button = _posqueue[0]->button;
  MotionData *data = new_motion_data(e.window, e.time, state, button,
                                     w->mcontext(), MouseMotion,
                                     x_root, y_root, _posqueue[0]->pos,
                                     _posqueue[0]->clientarea);
  Openbox::instance->bindings()->fireButton((ButtonData*)data);
  Py_DECREF((PyObject*)data);
}

void OBActions::mapRequestHandler(const XMapRequestEvent &e)
{
  doCallback(Action_NewWindow, e.window, (OBWidget::WidgetType)-1,
             0, 0, 0, 0, 0);
}

void OBActions::unmapHandler(const XUnmapEvent &e)
{
  (void)e;
  doCallback(Action_CloseWindow, e.window, (OBWidget::WidgetType)-1,
             0, 0, 0, 0, 0);
}

void OBActions::destroyHandler(const XDestroyWindowEvent &e)
{
  (void)e;
  doCallback(Action_CloseWindow, e.window, (OBWidget::WidgetType)-1,
             0, 0, 0, 0, 0);
}

void OBActions::doCallback(ActionType action, Window window,
                           OBWidget::WidgetType type, unsigned int state,
                           unsigned int button, int xroot, int yroot,
                           Time time)
{
  std::pair<CallbackMap::iterator, CallbackMap::iterator> it_pair =
    _callbacks.equal_range(action);

  CallbackMap::iterator it;
//  for (it = it_pair.first; it != it_pair.second; ++it)
//    python_callback(it->second, action, window, type, state,
//                    button, xroot, yroot, time);
  // XXX do a callback
}

bool OBActions::registerCallback(ActionType action, PyObject *func,
                                 bool atfront)
{
  if (action < 0 || action >= OBActions::NUM_ACTIONS) {
    return false;
  }
  if (!func)
    return false;

  std::pair<CallbackMap::iterator, CallbackMap::iterator> it_pair =
    _callbacks.equal_range(action);

  CallbackMap::iterator it;
  for (it = it_pair.first; it != it_pair.second; ++it)
    if (it->second == func)
      return true; // already in there
  if (atfront)
    _callbacks.insert(_callbacks.begin(), CallbackMapPair(action, func));
  else
    _callbacks.insert(CallbackMapPair(action, func));
  Py_INCREF(func);
  return true;
}

bool OBActions::unregisterCallback(ActionType action, PyObject *func)
{
  if (action < 0 || action >= OBActions::NUM_ACTIONS) {
    return false;
  }
  if (!func)
    return false;
  
  std::pair<CallbackMap::iterator, CallbackMap::iterator> it_pair =
    _callbacks.equal_range(action);
  
  CallbackMap::iterator it;
  for (it = it_pair.first; it != it_pair.second; ++it)
    if (it->second == func)
      break;
  if (it != it_pair.second) { // its been registered before
    Py_DECREF(func);
    _callbacks.erase(it);
  }
  return true;
}

bool OBActions::unregisterAllCallbacks(ActionType action)
{
  if (action < 0 || action >= OBActions::NUM_ACTIONS) {
    return false;
  }

  while (!_callbacks.empty()) {
    CallbackMap::iterator it = _callbacks.begin();
    Py_DECREF(it->second);
    _callbacks.erase(it);
  }
  return true;
}

}
