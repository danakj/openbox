// -*- mode: C++; indent-tabs-mode: nil; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "actions.hh"
#include "widget.hh"
#include "openbox.hh"
#include "client.hh"
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

  // XXX: load a configuration out of somewhere

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
  // if it's not defined, they should have clicked on the root window, so this
  // area would be meaningless anyways
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
  
  // XXX: run the PRESS guile hook
  OBWidget *w = dynamic_cast<OBWidget*>
    (Openbox::instance->findHandler(e.window));

  printf("GUILE: PRESS: win %lx type %d modifiers %u button %u time %lx\n",
         (long)e.window, (w ? w->type():-1), e.state, e.button, e.time);
    
  if (_button) return; // won't count toward CLICK events

  _button = e.button;
}
  

void OBActions::buttonReleaseHandler(const XButtonEvent &e)
{
  OtkEventHandler::buttonReleaseHandler(e);
  removePress(e);
  
  // XXX: run the RELEASE guile hook
  OBWidget *w = dynamic_cast<OBWidget*>
    (Openbox::instance->findHandler(e.window));

  printf("GUILE: RELEASE: win %lx type %d, modifiers %u button %u time %lx\n",
         (long)e.window, (w ? w->type():-1), e.state, e.button, e.time);

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

  // XXX: run the CLICK guile hook
  printf("GUILE: CLICK: win %lx type %d modifiers %u button %u time %lx\n",
         (long)e.window, (w ? w->type():-1), e.state, e.button, e.time);

  if (e.time - _release.time < DOUBLECLICKDELAY &&
      _release.win == e.window && _release.button == e.button) {

    // XXX: run the DOUBLECLICK guile hook
    printf("GUILE: DOUBLECLICK: win %lx type %d modifiers %u button %u time %lx\n",
           (long)e.window, (w ? w->type():-1), e.state, e.button, e.time);

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
  
  // XXX: run the ENTER guile hook
  OBWidget *w = dynamic_cast<OBWidget*>
    (Openbox::instance->findHandler(e.window));

  printf("GUILE: ENTER: win %lx type %d modifiers %u\n",
         (long)e.window, (w ? w->type():-1), e.state);
}


void OBActions::leaveHandler(const XCrossingEvent &e)
{
  OtkEventHandler::leaveHandler(e);

  // XXX: run the LEAVE guile hook
  OBWidget *w = dynamic_cast<OBWidget*>
    (Openbox::instance->findHandler(e.window));

  printf("GUILE: LEAVE: win %lx type %d modifiers %u\n",
         (long)e.window, (w ? w->type():-1), e.state);
}


void OBActions::keyPressHandler(const XKeyEvent &e)
{
  // XXX: run the KEY guile hook
  OBWidget *w = dynamic_cast<OBWidget*>
    (Openbox::instance->findHandler(e.window));

  printf("GUILE: KEY: win %lx type %d modifiers %u keycode %u\n",
         (long)e.window, (w ? w->type():-1), e.state, e.keycode);
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

  _dx = x_root - _posqueue[0]->pos.x();
  _dy = y_root - _posqueue[0]->pos.y();
  
  // XXX: i can envision all sorts of crazy shit with this.. gestures, etc
  printf("GUILE: MOTION: win %lx type %d  modifiers %u x %d y %d\n",
         (long)e.window, (w ? w->type():-1), e.state, _dx, _dy);

  OBClient *c = Openbox::instance->findClient(e.window);
  if (w && c) {
    switch (w->type()) {
    case OBWidget::Type_Titlebar:
    case OBWidget::Type_Label:
      c->move(_posqueue[0]->clientarea.x() + _dx,
              _posqueue[0]->clientarea.y() + _dy);
      break;
    case OBWidget::Type_LeftGrip:
      c->resize(OBClient::TopRight,
                _posqueue[0]->clientarea.width() - _dx,
                _posqueue[0]->clientarea.height() + _dy);
      break;
    case OBWidget::Type_RightGrip:
      c->resize(OBClient::TopLeft,
                _posqueue[0]->clientarea.width() + _dx,
                _posqueue[0]->clientarea.height() + _dy);
      break;
    default:
      break;
    }
  }
}


}
