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

OBActions::OBActions()
  : _button(0)
{
  // XXX: load a configuration out of somewhere

}


OBActions::~OBActions()
{
}


void OBActions::buttonPressHandler(const XButtonEvent &e)
{
  OtkEventHandler::buttonPressHandler(e);
  
  // run the PRESS python hook
  OBWidget *w = dynamic_cast<OBWidget*>
    (Openbox::instance->findHandler(e.window));

  doCallback(Action_ButtonPress, e.window,
             (OBWidget::WidgetType)(w ? w->type():-1),
             e.state, e.button, e.x_root, e.y_root, e.time);
    
  if (_button) return; // won't count toward CLICK events

  _button = e.button;
}
  

void OBActions::buttonReleaseHandler(const XButtonEvent &e)
{
  OtkEventHandler::buttonReleaseHandler(e);
  
  OBWidget *w = dynamic_cast<OBWidget*>
    (Openbox::instance->findHandler(e.window));

  // run the RELEASE python hook
  doCallback(Action_ButtonRelease, e.window,
             (OBWidget::WidgetType)(w ? w->type():-1),
             e.state, e.button, e.x_root, e.y_root, e.time);

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
  doCallback(Action_Click, e.window,
             (OBWidget::WidgetType)(w ? w->type():-1),
             e.state, e.button, e.x_root, e.y_root, e.time);

  if (e.time - _release.time < DOUBLECLICKDELAY &&
      _release.win == e.window && _release.button == e.button) {

    // run the DOUBLECLICK python hook
    doCallback(Action_DoubleClick, e.window,
               (OBWidget::WidgetType)(w ? w->type():-1),
               e.state, e.button, e.x_root, e.y_root, e.time);
    
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

  Openbox::instance->bindings()->fire(e.state, e.keycode, e.time);
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

  // XXX: i can envision all sorts of crazy shit with this.. gestures, etc
  //      maybe that should all be done via python tho.. (or radial menus!)
  // run the simple MOTION python hook for now...
  doCallback(Action_MouseMotion, e.window,
             (OBWidget::WidgetType)(w ? w->type():-1),
             e.state, (unsigned)-1, e.x_root, e.y_root, e.time);
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
  for (it = it_pair.first; it != it_pair.second; ++it)
    python_callback(it->second, action, window, type, state,
                    button, xroot, yroot, time);
}

bool OBActions::registerCallback(ActionType action, PyObject *func,
                                 bool atfront)
{
  if (action < 0 || action >= OBActions::NUM_ACTIONS ||
      action == OBActions::Action_KeyPress) {
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
  if (it == it_pair.second) // not already in there
    if (atfront)
      _callbacks.insert(_callbacks.begin(), CallbackMapPair(action, func));
    else
      _callbacks.insert(CallbackMapPair(action, func));
  Py_INCREF(func);
  return true;
}

bool OBActions::unregisterCallback(ActionType action, PyObject *func)
{
  if (action < 0 || action >= OBActions::NUM_ACTIONS ||
      action == OBActions::Action_KeyPress) {
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
  if (action < 0 || action >= OBActions::NUM_ACTIONS ||
      action == OBActions::Action_KeyPress) {
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
