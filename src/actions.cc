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

const int Actions::BUTTONS;

Actions::Actions()
  : _button(0)
{
  for (int i=0; i<BUTTONS; ++i)
    _posqueue[i] = new ButtonPressAction();
}


Actions::~Actions()
{
  for (int i=0; i<BUTTONS; ++i)
    delete _posqueue[i];
}


void Actions::insertPress(const XButtonEvent &e)
{
  ButtonPressAction *a = _posqueue[BUTTONS - 1];
  // rm'd the last one, shift them all down one
  for (int i = BUTTONS-1; i > 0; --i) {
    _posqueue[i] = _posqueue[i-1];
  }
  _posqueue[0] = a;
  a->button = e.button;
  a->pos.setPoint(e.x_root, e.y_root);

  Client *c = openbox->findClient(e.window);
  if (c) a->clientarea = c->area();
}

void Actions::removePress(const XButtonEvent &e)
{
  int i;
  ButtonPressAction *a = 0;
  for (i=0; i<BUTTONS-1; ++i)
    if (_posqueue[i]->button == e.button) {
      a = _posqueue[i];
      break;
    }
  if (a) { // found one, remove it and shift the rest up one
    for (; i < BUTTONS-1; ++i)
      _posqueue[i] = _posqueue[i+1];
    _posqueue[BUTTONS-1] = a;
  }
  _posqueue[BUTTONS-1]->button = 0;
}

void Actions::buttonPressHandler(const XButtonEvent &e)
{
  otk::EventHandler::buttonPressHandler(e);
  insertPress(e);
  
  // run the PRESS python hook
  WidgetBase *w = dynamic_cast<WidgetBase*>
    (openbox->findHandler(e.window));
  assert(w); // everything should be a widget

  // kill off the Button1Mask etc, only want the modifiers
  unsigned int state = e.state & (ControlMask | ShiftMask | Mod1Mask |
                                  Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask);
  int screen;
  Client *c = openbox->findClient(e.window);
  if (c)
    screen = c->screen();
  else
    screen = otk::Display::findScreen(e.root)->screen();
  MouseData data(screen, c, e.time, state, e.button, w->mcontext(),
                 MousePress);
  openbox->bindings()->fireButton(&data);
    
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
  

void Actions::buttonReleaseHandler(const XButtonEvent &e)
{
  otk::EventHandler::buttonReleaseHandler(e);
  removePress(e);
  
  WidgetBase *w = dynamic_cast<WidgetBase*>
    (openbox->findHandler(e.window));
  assert(w); // everything should be a widget

  // not for the button we're watching?
  if (_button != e.button) return;

  _button = 0;

  // find the area of the window
  XWindowAttributes attr;
  if (!XGetWindowAttributes(otk::Display::display, e.window, &attr)) return;

  // if not on the window any more, it isnt a CLICK
  if (!(e.same_screen && e.x >= 0 && e.y >= 0 &&
        e.x < attr.width && e.y < attr.height))
    return;

  // run the CLICK python hook
  // kill off the Button1Mask etc, only want the modifiers
  unsigned int state = e.state & (ControlMask | ShiftMask | Mod1Mask |
                                  Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask);
  int screen;
  Client *c = openbox->findClient(e.window);
  if (c)
    screen = c->screen();
  else
    screen = otk::Display::findScreen(e.root)->screen();
  MouseData data(screen, c, e.time, state, e.button, w->mcontext(),
                 MouseClick);
  openbox->bindings()->fireButton(&data);
    

  // XXX: dont load this every time!!@*
  long dblclick;
  if (!python_get_long("double_click_delay", &dblclick))
    dblclick = 300;

  if (e.time - _release.time < (unsigned)dblclick &&
      _release.win == e.window && _release.button == e.button) {

    // run the DOUBLECLICK python hook
    data.action = MouseDoubleClick;
    openbox->bindings()->fireButton(&data);
    
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


void Actions::enterHandler(const XCrossingEvent &e)
{
  otk::EventHandler::enterHandler(e);
  
  // run the ENTER python hook
  int screen;
  Client *c = openbox->findClient(e.window);
  if (c)
    screen = c->screen();
  else
    screen = otk::Display::findScreen(e.root)->screen();
  EventData data(screen, c, EventEnterWindow, e.state);
  openbox->bindings()->fireEvent(&data);
}


void Actions::leaveHandler(const XCrossingEvent &e)
{
  otk::EventHandler::leaveHandler(e);

  // run the LEAVE python hook
  int screen;
  Client *c = openbox->findClient(e.window);
  if (c)
    screen = c->screen();
  else
    screen = otk::Display::findScreen(e.root)->screen();
  EventData data(screen, c, EventLeaveWindow, e.state);
  openbox->bindings()->fireEvent(&data);
}


void Actions::keyPressHandler(const XKeyEvent &e)
{
  otk::EventHandler::keyPressHandler(e);

  // kill off the Button1Mask etc, only want the modifiers
  unsigned int state = e.state & (ControlMask | ShiftMask | Mod1Mask |
                                  Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask);
  openbox->bindings()->
    fireKey(otk::Display::findScreen(e.root)->screen(),
            state, e.keycode, e.time);
}


void Actions::motionHandler(const XMotionEvent &e)
{
  otk::EventHandler::motionHandler(e);

  if (!e.same_screen) return; // this just gets stupid

  int x_root = e.x_root, y_root = e.y_root;
  
  // compress changes to a window into a single change
  XEvent ce;
  while (XCheckTypedEvent(otk::Display::display, e.type, &ce)) {
    if (ce.xmotion.window != e.window) {
      XPutBackEvent(otk::Display::display, &ce);
      break;
    } else {
      x_root = e.x_root;
      y_root = e.y_root;
    }
  }

  WidgetBase *w = dynamic_cast<WidgetBase*>
    (openbox->findHandler(e.window));
  assert(w); // everything should be a widget

  // run the MOTION python hook
  // kill off the Button1Mask etc, only want the modifiers
  unsigned int state = e.state & (ControlMask | ShiftMask | Mod1Mask |
                                  Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask);
  unsigned int button = _posqueue[0]->button;
  int screen;
  Client *c = openbox->findClient(e.window);
  if (c)
    screen = c->screen();
  else
    screen = otk::Display::findScreen(e.root)->screen();
  MouseData data(screen, c, e.time, state, button, w->mcontext(), MouseMotion,
                 x_root, y_root, _posqueue[0]->pos, _posqueue[0]->clientarea);
  openbox->bindings()->fireButton(&data);
}

void Actions::mapRequestHandler(const XMapRequestEvent &e)
{
  otk::EventHandler::mapRequestHandler(e);
  // do this in Screen::manageWindow
}

void Actions::unmapHandler(const XUnmapEvent &e)
{
  otk::EventHandler::unmapHandler(e);
  // do this in Screen::unmanageWindow
}

void Actions::destroyHandler(const XDestroyWindowEvent &e)
{
  otk::EventHandler::destroyHandler(e);
  // do this in Screen::unmanageWindow
}

#ifdef    XKB
void Actions::xkbHandler(const XkbEvent &e)
{
  Window w;
  int screen;
  
  otk::EventHandler::xkbHandler(e);

  switch (((XkbAnyEvent*)&e)->xkb_type) {
  case XkbBellNotify:
    w = ((XkbBellNotifyEvent*)&e)->window;
    Client *c = openbox->findClient(w);
    if (c)
      screen = c->screen();
    else
      screen = openbox->focusedScreen()->number();
    EventData data(screen, c, EventBell, 0);
    openbox->bindings()->fireEvent(&data);
    break;
  }
}
#endif // XKB

}

