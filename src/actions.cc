// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "actions.hh"
#include "widgetbase.hh"
#include "openbox.hh"
#include "client.hh"
#include "screen.hh"
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

  // kill off the Button1Mask etc, only want the modifiers
  unsigned int state = e.state & (ControlMask | ShiftMask | Mod1Mask |
                                  Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask);
  int screen;
  OBClient *c = Openbox::instance->findClient(e.window);
  if (c)
    screen = c->screen();
  else
    screen = otk::OBDisplay::findScreen(e.root)->screen();
  MouseData data(screen, c, e.time, state, e.button, w->mcontext(),
                 MousePress);
  Openbox::instance->bindings()->fireButton(&data);
    
  if (_button) return; // won't count toward CLICK events

  _button = e.button;

  if (w->mcontext() == MC_Window) {
    /*
      Because of how events are grabbed on the client window, we can't get
      ButtonRelease events, so instead we simply manufacture them here, so that
      clicks/doubleclicks etc still work.
    */
    //XButtonEvent ev = e;
    //ev.type = ButtonRelease;
    buttonReleaseHandler(e);
  }
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
  // kill off the Button1Mask etc, only want the modifiers
  unsigned int state = e.state & (ControlMask | ShiftMask | Mod1Mask |
                                  Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask);
  int screen;
  OBClient *c = Openbox::instance->findClient(e.window);
  if (c)
    screen = c->screen();
  else
    screen = otk::OBDisplay::findScreen(e.root)->screen();
  MouseData data(screen, c, e.time, state, e.button, w->mcontext(),
                 MouseClick);
  Openbox::instance->bindings()->fireButton(&data);
    

  // XXX: dont load this every time!!@*
  long dblclick;
  if (!python_get_long("double_click_delay", &dblclick))
    dblclick = 300;

  if (e.time - _release.time < (unsigned)dblclick &&
      _release.win == e.window && _release.button == e.button) {

    // run the DOUBLECLICK python hook
    data.action = MouseDoubleClick;
    Openbox::instance->bindings()->fireButton(&data);
    
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
  
  // run the ENTER python hook
  int screen;
  OBClient *c = Openbox::instance->findClient(e.window);
  if (c)
    screen = c->screen();
  else
    screen = otk::OBDisplay::findScreen(e.root)->screen();
  EventData data(screen, c, EventEnterWindow, e.state);
  Openbox::instance->bindings()->fireEvent(&data);
}


void OBActions::leaveHandler(const XCrossingEvent &e)
{
  OtkEventHandler::leaveHandler(e);

  // run the LEAVE python hook
  int screen;
  OBClient *c = Openbox::instance->findClient(e.window);
  if (c)
    screen = c->screen();
  else
    screen = otk::OBDisplay::findScreen(e.root)->screen();
  EventData data(screen, c, EventLeaveWindow, e.state);
  Openbox::instance->bindings()->fireEvent(&data);
}


void OBActions::keyPressHandler(const XKeyEvent &e)
{
  OtkEventHandler::keyPressHandler(e);

  // kill off the Button1Mask etc, only want the modifiers
  unsigned int state = e.state & (ControlMask | ShiftMask | Mod1Mask |
                                  Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask);
  Openbox::instance->bindings()->
    fireKey(otk::OBDisplay::findScreen(e.root)->screen(),
            state, e.keycode, e.time);
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
  // kill off the Button1Mask etc, only want the modifiers
  unsigned int state = e.state & (ControlMask | ShiftMask | Mod1Mask |
                                  Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask);
  unsigned int button = _posqueue[0]->button;
  int screen;
  OBClient *c = Openbox::instance->findClient(e.window);
  if (c)
    screen = c->screen();
  else
    screen = otk::OBDisplay::findScreen(e.root)->screen();
  MouseData data(screen, c, e.time, state, button, w->mcontext(), MouseMotion,
                 x_root, y_root, _posqueue[0]->pos, _posqueue[0]->clientarea);
  Openbox::instance->bindings()->fireButton(&data);
}

void OBActions::mapRequestHandler(const XMapRequestEvent &e)
{
  OtkEventHandler::mapRequestHandler(e);
  // do this in OBScreen::manageWindow
}

void OBActions::unmapHandler(const XUnmapEvent &e)
{
  OtkEventHandler::unmapHandler(e);
  // do this in OBScreen::unmanageWindow
}

void OBActions::destroyHandler(const XDestroyWindowEvent &e)
{
  OtkEventHandler::destroyHandler(e);
  // do this in OBScreen::unmanageWindow
}

#ifdef    XKB
void OBActions::xkbHandler(const XkbEvent &e)
{
  Window w;
  int screen;
  
  OtkEventHandler::xkbHandler(e);

  switch (((XkbAnyEvent*)&e)->xkb_type) {
  case XkbBellNotify:
    w = ((XkbBellNotifyEvent*)&e)->window;
    OBClient *c = Openbox::instance->findClient(w);
    if (c)
      screen = c->screen();
    else
      screen = Openbox::instance->focusedScreen()->number();
    EventData data(screen, c, EventBell, 0);
    Openbox::instance->bindings()->fireEvent(&data);
    break;
  }
}
#endif // XKB

}

