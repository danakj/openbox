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

const int OBActions::BUTTONS;

OBActions::OBActions()
  : _button(0)
{
  for (int i=0; i<BUTTONS; ++i)
    _posqueue[i] = new ButtonPressAction();

  for (int i = 0; i < NUM_EVENTS; ++i)
    _callback[i] = 0;
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
    

  // XXX: dont load this every time!!@*
  long dblclick;
  if (!python_get_long("double_click_delay", &dblclick))
    dblclick = 300;

  if (e.time - _release.time < (unsigned)dblclick &&
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
  
  // run the ENTER python hook
  if (_callback[EventEnterWindow]) {
    EventData *data = new_event_data(e.window, EventEnterWindow, e.state);
    python_callback(_callback[EventEnterWindow], (PyObject*)data);
    Py_DECREF((PyObject*)data);
  }
}


void OBActions::leaveHandler(const XCrossingEvent &e)
{
  OtkEventHandler::leaveHandler(e);

  // run the LEAVE python hook
  if (_callback[EventLeaveWindow]) {
    EventData *data = new_event_data(e.window, EventLeaveWindow, e.state);
    python_callback(_callback[EventLeaveWindow], (PyObject*)data);
    Py_DECREF((PyObject*)data);
  }
}


void OBActions::keyPressHandler(const XKeyEvent &e)
{
  OtkEventHandler::keyPressHandler(e);

  unsigned int state = e.state & (ControlMask | ShiftMask | Mod1Mask |
                                  Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask);
  Openbox::instance->bindings()->fireKey(state, e.keycode, e.time);
}


void OBActions::motionHandler(const XMotionEvent &e)
{
  OtkEventHandler::motionHandler(e);

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
  OtkEventHandler::mapRequestHandler(e);

  if (_callback[EventNewWindow]) {
    EventData *data = new_event_data(e.window, EventNewWindow, 0);
    python_callback(_callback[EventNewWindow], (PyObject*)data);
    Py_DECREF((PyObject*)data);
  }
}

void OBActions::unmapHandler(const XUnmapEvent &e)
{
  OtkEventHandler::unmapHandler(e);

  if (_callback[EventCloseWindow]) {
    EventData *data = new_event_data(e.window, EventCloseWindow, 0);
    python_callback(_callback[EventCloseWindow], (PyObject*)data);
    Py_DECREF((PyObject*)data);
  }
}

void OBActions::destroyHandler(const XDestroyWindowEvent &e)
{
  OtkEventHandler::destroyHandler(e);

  if (_callback[EventCloseWindow]) {
    EventData *data = new_event_data(e.window, EventCloseWindow, 0);
    python_callback(_callback[EventCloseWindow], (PyObject*)data);
    Py_DECREF((PyObject*)data);
  }
}

bool OBActions::bind(EventAction action, PyObject *func)
{
  if (action < 0 || action >= NUM_EVENTS) {
    return false;
  }

  Py_XDECREF(_callback[action]);
  _callback[action] = func;
  Py_INCREF(func);
  return true;
}

bool OBActions::unbind(EventAction action)
{
  if (action < 0 || action >= NUM_EVENTS) {
    return false;
  }
  
  Py_XDECREF(_callback[action]);
  _callback[action] = 0;
  return true;
}

void OBActions::unbindAll()
{
  for (int i = 0; i < NUM_EVENTS; ++i) {
    Py_XDECREF(_callback[i]);
    _callback[i] = 0;
  }
}

}
