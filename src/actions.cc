// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

#include "actions.hh"
#include "openbox.hh"
#include "client.hh"
#include "frame.hh"
#include "screen.hh"
#include "python.hh"
#include "bindings.hh"
#include "otk/display.hh"

#include <cstdio>
#include <algorithm>

namespace ob {

const int Actions::BUTTONS;

Actions::Actions()
  : _button(0),
    _dragging(false)
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
  a->pos = otk::Point(e.x_root, e.y_root);

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

  MouseContext::MC context;
  EventHandler *h = openbox->findHandler(e.window);
  Frame *f = dynamic_cast<Frame*>(h);
  if (f)
    context= f->mouseContext(e.window);
  else if (dynamic_cast<Client*>(h))
    context = MouseContext::Window;
  else if (dynamic_cast<Screen*>(h))
    context = MouseContext::Root;
  else
    return; // not a valid mouse context

  // run the PRESS python hook
  // kill off the Button1Mask etc, only want the modifiers
  unsigned int state = e.state & (ControlMask | ShiftMask | Mod1Mask |
                                  Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask);
  int screen;
  Client *c = openbox->findClient(e.window);
  if (c)
    screen = c->screen();
  else
    screen = otk::display->findScreen(e.root)->screen();
  MouseData data(screen, c, e.time, state, e.button, context,
                 MouseAction::Press);
  openbox->bindings()->fireButton(&data);
    
  if (_button) return; // won't count toward CLICK events

  _button = e.button;

  if (context == MouseContext::Window) {
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
  
  MouseContext::MC context;
  EventHandler *h = openbox->findHandler(e.window);
  Frame *f = dynamic_cast<Frame*>(h);
  if (f)
    context= f->mouseContext(e.window);
  else if (dynamic_cast<Client*>(h))
    context = MouseContext::Window;
  else if (dynamic_cast<Screen*>(h))
    context = MouseContext::Root;
  else
    return; // not a valid mouse context

  // run the RELEASE python hook
  // kill off the Button1Mask etc, only want the modifiers
  unsigned int state = e.state & (ControlMask | ShiftMask | Mod1Mask |
                                  Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask);
  int screen;
  Client *c = openbox->findClient(e.window);
  if (c)
    screen = c->screen();
  else
    screen = otk::display->findScreen(e.root)->screen();
  MouseData data(screen, c, e.time, state, e.button, context,
                 MouseAction::Release);
  openbox->bindings()->fireButton(&data);

  // not for the button we're watching?
  if (_button != e.button) return;

  _button = 0;
  _dragging = false;

  // find the area of the window
  XWindowAttributes attr;
  if (!XGetWindowAttributes(**otk::display, e.window, &attr)) return;

  // if not on the window any more, it isnt a CLICK
  if (!(e.same_screen && e.x >= 0 && e.y >= 0 &&
        e.x < attr.width && e.y < attr.height))
    return;

  // run the CLICK python hook
  data.action = MouseAction::Click;
  openbox->bindings()->fireButton(&data);
    

  // XXX: dont load this every time!!@*
  long dblclick;
  if (!python_get_long("DOUBLE_CLICK_DELAY", &dblclick))
    dblclick = 300;

  if (e.time - _release.time < (unsigned)dblclick &&
      _release.win == e.window && _release.button == e.button) {

    // run the DOUBLECLICK python hook
    data.action = MouseAction::DoubleClick;
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
    screen = otk::display->findScreen(e.root)->screen();
  EventData data(screen, c, EventAction::EnterWindow, e.state);
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
    screen = otk::display->findScreen(e.root)->screen();
  EventData data(screen, c, EventAction::LeaveWindow, e.state);
  openbox->bindings()->fireEvent(&data);
}


void Actions::keyPressHandler(const XKeyEvent &e)
{
  otk::EventHandler::keyPressHandler(e);

  // kill off the Button1Mask etc, only want the modifiers
  unsigned int state = e.state & (ControlMask | ShiftMask | Mod1Mask |
                                  Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask);
  openbox->bindings()->
    fireKey(otk::display->findScreen(e.root)->screen(),
            state, e.keycode, e.time, KeyAction::Press);
}


void Actions::keyReleaseHandler(const XKeyEvent &e)
{
  otk::EventHandler::keyReleaseHandler(e);

  // kill off the Button1Mask etc, only want the modifiers
  unsigned int state = e.state & (ControlMask | ShiftMask | Mod1Mask |
                                  Mod2Mask | Mod3Mask | Mod4Mask | Mod5Mask);

  // remove from the state the mask of the modifier being released, if it is
  // a modifier key being released (this is a little ugly..)
  const XModifierKeymap *map = otk::display->modifierMap();
  const int mask_table[] = {
    ShiftMask, LockMask, ControlMask, Mod1Mask,
    Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask
  };
  KeyCode *kp = map->modifiermap;
  for (int i = 0, n = sizeof(mask_table)/sizeof(mask_table[0]); i < n; ++i) {
    for (int k = 0; k < map->max_keypermod; ++k) {
      if (*kp == e.keycode) { // found the keycode
        state &= ~mask_table[i]; // remove the mask for it
        i = n; // cause the first loop to break;
        break; // get outta here!
      }
      ++kp;
    }
  }
  
  openbox->bindings()->
    fireKey(otk::display->findScreen(e.root)->screen(),
            state, e.keycode, e.time, KeyAction::Release);
}


void Actions::motionHandler(const XMotionEvent &e)
{
  otk::EventHandler::motionHandler(e);

  if (!e.same_screen) return; // this just gets stupid

  MouseContext::MC context;
  EventHandler *h = openbox->findHandler(e.window);
  Frame *f = dynamic_cast<Frame*>(h);
  if (f)
    context= f->mouseContext(e.window);
  else if (dynamic_cast<Client*>(h))
    context = MouseContext::Window;
  else if (dynamic_cast<Screen*>(h))
    context = MouseContext::Root;
  else
    return; // not a valid mouse context

  int x_root = e.x_root, y_root = e.y_root;
  
  // compress changes to a window into a single change
  XEvent ce;
  while (XCheckTypedWindowEvent(**otk::display, e.window, e.type, &ce)) {
    x_root = e.x_root;
    y_root = e.y_root;
  }

  if (!_dragging) {
    long threshold;
    int dx = x_root - _posqueue[0]->pos.x();
    int dy = y_root - _posqueue[0]->pos.y();
    // XXX: dont get this from python every time!
    if (!python_get_long("DRAG_THRESHOLD", &threshold))
      threshold = 0;
    if (!(std::abs(dx) >= threshold || std::abs(dy) >= threshold))
      return; // not at the threshold yet
  }
  _dragging = true; // in a drag now
  
  // check if the movement is more than the threshold

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
    screen = otk::display->findScreen(e.root)->screen();
  MouseData data(screen, c, e.time, state, button, context,
                 MouseAction::Motion, x_root, y_root,
                 _posqueue[0]->pos, _posqueue[0]->clientarea);
  openbox->bindings()->fireButton(&data);
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
    EventData data(screen, c, EventAction::Bell, 0);
    openbox->bindings()->fireEvent(&data);
    break;
  }
}
#endif // XKB

}

