// -*- mode: C++; indent-tabs-mode: nil; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "actions.hh"

#include <stdio.h>

namespace ob {

const unsigned int OBActions::DOUBLECLICKDELAY = 300;

OBActions::OBActions()
  : _button(0), _enter_win(0)
{

  // XXX: load a configuration out of somewhere

}


OBActions::~OBActions()
{
}


void OBActions::buttonPressHandler(const XButtonEvent &e)
{
  // XXX: run the PRESS guile hook
  printf("GUILE: PRESS: win %lx modifiers %u button %u time %lx\n",
         (long)e.window, e.state, e.button, e.time);
    
  if (_button) return; // won't count toward CLICK events

  _button = e.button;
}
  

void OBActions::buttonReleaseHandler(const XButtonEvent &e)
{
  // XXX: run the RELEASE guile hook
  printf("GUILE: RELEASE: win %lx modifiers %u button %u time %lx\n",
         (long)e.window, e.state, e.button, e.time);

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
  printf("GUILE: CLICK: win %lx modifiers %u button %u time %lx\n",
         (long)e.window, e.state, e.button, e.time);

  if (e.time - _release.time < DOUBLECLICKDELAY &&
      _release.win == e.window && _release.button == e.button) {

    // XXX: run the DOUBLECLICK guile hook
    printf("GUILE: DOUBLECLICK: win %lx modifiers %u button %u time %lx\n",
           (long)e.window, e.state, e.button, e.time);

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


void OBActions::enter(Window win, unsigned int modifiers)
{
  _enter_win = win;

  (void)modifiers;
  // XXX: run the ENTER guile hook
  printf("GUILE: ENTER: win %lx modifiers %u\n", (long)win, modifiers);

}


void OBActions::leave(unsigned int modifiers)
{
  (void)modifiers;
  // XXX: run the LEAVE guile hook
  printf("GUILE: LEAVE: win %lx modifiers %u\n", (long)_enter_win, modifiers);

  _enter_win = 0;
}


void OBActions::drag(Window win, otk::Point delta, unsigned int modifiers,
                     unsigned int button, Time time)
{
  (void)win;(void)delta;(void)modifiers;(void)button;(void)time;

  // XXX: some guile shit...
}


void OBActions::key(Window win, unsigned int modifiers, unsigned int keycode)
{
  (void)win;(void)modifiers;(void)keycode;

  // XXX: some guile shit...
}


}
