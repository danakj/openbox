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
  
  // run the PRESS guile hook
  OBWidget *w = dynamic_cast<OBWidget*>
    (Openbox::instance->findHandler(e.window));

  python_callback(Action_ButtonPress, e.window,
                  (OBWidget::WidgetType)(w ? w->type():-1),
                  e.state, e.button, e.x_root, e.y_root, e.time);
  if (w && w->type() == OBWidget::Type_Frame) // a binding
    Openbox::instance->bindings()->fire(Action_ButtonPress, e.window,
                                        e.state, e.button, e.time);
    
  if (_button) return; // won't count toward CLICK events

  _button = e.button;
}
  

void OBActions::buttonReleaseHandler(const XButtonEvent &e)
{
  OtkEventHandler::buttonReleaseHandler(e);
  
  OBWidget *w = dynamic_cast<OBWidget*>
    (Openbox::instance->findHandler(e.window));

  // run the RELEASE guile hook
  python_callback(Action_ButtonRelease, e.window,
                  (OBWidget::WidgetType)(w ? w->type():-1),
                  e.state, e.button, e.x_root, e.y_root, e.time);
  if (w && w->type() == OBWidget::Type_Frame) // a binding
    Openbox::instance->bindings()->fire(Action_ButtonRelease, e.window,
                                        e.state, e.button, e.time);

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

  // run the CLICK guile hook
  python_callback(Action_Click, e.window,
                  (OBWidget::WidgetType)(w ? w->type():-1),
                  e.state, e.button, e.time);
  if (w && w->type() == OBWidget::Type_Frame) // a binding
    Openbox::instance->bindings()->fire(Action_Click, e.window,
                                        e.state, e.button, e.time);

  if (e.time - _release.time < DOUBLECLICKDELAY &&
      _release.win == e.window && _release.button == e.button) {

    // run the DOUBLECLICK guile hook
    python_callback(Action_DoubleClick, e.window,
                  (OBWidget::WidgetType)(w ? w->type():-1),
                  e.state, e.button, e.time);
    if (w && w->type() == OBWidget::Type_Frame) // a binding
      Openbox::instance->bindings()->fire(Action_DoubleClick, e.window,
                                          e.state, e.button, e.time);
    
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

  // run the ENTER guile hook
  python_callback(Action_EnterWindow, e.window,
                  (OBWidget::WidgetType)(w ? w->type():-1), e.state);
}


void OBActions::leaveHandler(const XCrossingEvent &e)
{
  OtkEventHandler::leaveHandler(e);

  OBWidget *w = dynamic_cast<OBWidget*>
    (Openbox::instance->findHandler(e.window));

  // run the LEAVE guile hook
  python_callback(Action_LeaveWindow, e.window,
                  (OBWidget::WidgetType)(w ? w->type():-1), e.state);
}


void OBActions::keyPressHandler(const XKeyEvent &e)
{
//  OBWidget *w = dynamic_cast<OBWidget*>
//    (Openbox::instance->findHandler(e.window));

  Openbox::instance->bindings()->fire(Action_KeyPress, e.window,
                                      e.state, e.keycode, e.time);
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
  //      maybe that should all be done via python tho..
  // run the simple MOTION guile hook for now...
  python_callback(Action_MouseMotion, e.window,
                  (OBWidget::WidgetType)(w ? w->type():-1),
                  e.state, e.x_root, e.y_root, e.time);
}


}
