// -*- mode: C++; indent-tabs-mode: nil; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "actions.hh"

#include <stdio.h>

namespace ob {

const unsigned int OBActions::DOUBLECLICKDELAY;

OBActions::OBActions()
  : _button(0), _enter_win(0)
{
  _presses[0] = new MousePressAction();
  _presses[1] = new MousePressAction();
  _presses[2] = new MousePressAction();

  // XXX: load a configuration out of somewhere

}


OBActions::~OBActions()
{
}


void OBActions::insertPress(Window win, unsigned int button, Time time)
{
  MousePressAction *a = _presses[1];
  _presses[1] = _presses[0];
  _presses[0] = a;
  a->win = win;
  a->button = button;
  a->time = time;
}


void OBActions::bpress(Window win, unsigned int modifiers, unsigned int button,
                       Time time)
{
  (void)modifiers;
  // XXX: run the PRESS guile hook
  printf("GUILE: PRESS: win %lx modifiers %ux button %ud time %lx",
         (long)win, modifiers, button, time);
    
  if (_button) return; // won't count toward CLICK events

  _button = button;

  insertPress(win, button, time);
}
  

void OBActions::brelease(Window win, const otk::Rect &area,
                         const otk::Point &mpos, 
                         unsigned int modifiers, unsigned int button,
                         Time time)
{
  (void)modifiers;
  // XXX: run the RELEASE guile hook
  printf("GUILE: RELEASE: win %lx modifiers %ux button %ud time %lx",
         (long)win, modifiers, button, time);

  if (_button && _button != button) return; // not for the button we're watchin

  _button = 0;

  if (!area.contains(mpos)) return; // not on the window any more

  // XXX: run the CLICK guile hook
  printf("GUILE: CLICK: win %lx modifiers %ux button %ud time %lx",
         (long)win, modifiers, button, time);

  if (_presses[0]->win == _presses[1]->win &&
      _presses[0]->button == _presses[1]->button &&
      time - _presses[1]->time < DOUBLECLICKDELAY) {

    // XXX: run the DOUBLECLICK guile hook
    printf("GUILE: DOUBLECLICK: win %lx modifiers %ux button %ud time %lx",
           (long)win, modifiers, button, time);

  }
}


void OBActions::enter(Window win, unsigned int modifiers)
{
  _enter_win = win;

  (void)modifiers;
  // XXX: run the ENTER guile hook
  printf("GUILE: ENTER: win %lx modifiers %ux", (long)win, modifiers);

}


void OBActions::leave(unsigned int modifiers)
{
  (void)modifiers;
  // XXX: run the LEAVE guile hook
  printf("GUILE: LEAVE: win %lx modifiers %ux", (long)_enter_win, modifiers);

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
