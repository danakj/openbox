// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "client.hh"
#include "screen.hh"
#include "openbox.hh"
#include "otk/display.hh"
#include "otk/property.hh"

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <assert.h>

#include "gettext.h"
#define _(str) gettext(str)
}

namespace ob {

OBClient::OBClient(Window window)
  : _window(window)
{
  assert(window);

  // update EVERYTHING the first time!!

  // the state is kinda assumed to be normal. is this right? XXX
  _wmstate = NormalState;
  
  getArea();
  getDesktop();
  getType();
  getState();
  getShaped();

  updateNormalHints();
  updateWMHints();
  // XXX: updateTransientFor();
  updateTitle();
  updateClass();

#ifdef DEBUG
  printf("Mapped window: 0x%lx\n"
         "  title:         \t%s\t  icon title:    \t%s\n"
         "  app name:      \t%s\t\t  class:         \t%s\n"
         "  position:      \t%d, %d\t\t  size:          \t%d, %d\n"
         "  desktop:       \t%lu\t\t  group:         \t0x%lx\n"
         "  type:          \t%d\t\t  min size       \t%d, %d\n"
         "  base size      \t%d, %d\t\t  max size       \t%d, %d\n"
         "  size incr      \t%d, %d\t\t  gravity        \t%d\n"
         "  wm state       \t%ld\t\t  can be focused:\t%s\n"
         "  notify focus:  \t%s\t\t  urgent:        \t%s\n"
         "  shaped:        \t%s\t\t  modal:         \t%s\n"
         "  shaded:        \t%s\t\t  iconic:        \t%s\n"
         "  vert maximized:\t%s\t\t  horz maximized:\t%s\n"
         "  fullscreen:    \t%s\t\t  floating:      \t%s\n",
         _window,
         _title.c_str(),
         _icon_title.c_str(),
         _app_name.c_str(),
         _app_class.c_str(),
         _area.x(), _area.y(),
         _area.width(), _area.height(),
         _desktop,
         _group,
         _type,
         _min_x, _min_y,
         _base_x, _base_y,
         _max_x, _max_y,
         _inc_x, _inc_y,
         _gravity,
         _wmstate,
         _can_focus ? "yes" : "no",
         _focus_notify ? "yes" : "no",
         _urgent ? "yes" : "no",
         _shaped ? "yes" : "no",
         _modal ? "yes" : "no",
         _shaded ? "yes" : "no",
         _iconic ? "yes" : "no",
         _max_vert ? "yes" : "no",
         _max_horz ? "yes" : "no",
         _fullscreen ? "yes" : "no",
         _floating ? "yes" : "no");
#endif
}


OBClient::~OBClient()
{
  const otk::OBProperty *property = Openbox::instance->property();

  // these values should not be persisted across a window unmapping/mapping
  property->erase(_window, otk::OBProperty::net_wm_desktop);
  property->erase(_window, otk::OBProperty::net_wm_state);
}


void OBClient::getDesktop()
{
  const otk::OBProperty *property = Openbox::instance->property();

  // defaults to the current desktop
  _desktop = 0; // XXX: change this to the current desktop!

  property->get(_window, otk::OBProperty::net_wm_desktop,
                otk::OBProperty::Atom_Cardinal,
                &_desktop);
}


void OBClient::getType()
{
  const otk::OBProperty *property = Openbox::instance->property();

  _type = (WindowType) -1;
  
  unsigned long *val;
  unsigned long num = (unsigned) -1;
  if (property->get(_window, otk::OBProperty::net_wm_window_type,
                    otk::OBProperty::Atom_Atom,
                    &num, &val)) {
    // use the first value that we know about in the array
    for (unsigned long i = 0; i < num; ++i) {
      if (val[i] ==
          property->atom(otk::OBProperty::net_wm_window_type_desktop))
        _type = Type_Desktop;
      else if (val[i] ==
               property->atom(otk::OBProperty::net_wm_window_type_dock))
        _type = Type_Dock;
      else if (val[i] ==
               property->atom(otk::OBProperty::net_wm_window_type_toolbar))
        _type = Type_Toolbar;
      else if (val[i] ==
               property->atom(otk::OBProperty::net_wm_window_type_menu))
        _type = Type_Menu;
      else if (val[i] ==
               property->atom(otk::OBProperty::net_wm_window_type_utility))
        _type = Type_Utility;
      else if (val[i] ==
               property->atom(otk::OBProperty::net_wm_window_type_splash))
        _type = Type_Splash;
      else if (val[i] ==
               property->atom(otk::OBProperty::net_wm_window_type_dialog))
        _type = Type_Dialog;
      else if (val[i] ==
               property->atom(otk::OBProperty::net_wm_window_type_normal))
        _type = Type_Normal;
//      else if (val[i] ==
//               property->atom(otk::OBProperty::kde_net_wm_window_type_override))
//        mwm_decorations = 0; // prevent this window from getting any decor
      // XXX: make this work again
    }
    delete val;
  }
    
  if (_type == (WindowType) -1) {
    /*
     * the window type hint was not set, which means we either classify ourself
     * as a normal window or a dialog, depending on if we are a transient.
     */
    // XXX: make this code work!
    //if (isTransient())
    //  _type = Type_Dialog;
    //else
      _type = Type_Normal;
  }
}


void OBClient::getArea()
{
  XWindowAttributes wattrib;
  assert(XGetWindowAttributes(otk::OBDisplay::display, _window, &wattrib));

  _area.setRect(wattrib.x, wattrib.y, wattrib.width, wattrib.height);
}


void OBClient::getState()
{
  const otk::OBProperty *property = Openbox::instance->property();

  _modal = _shaded = _max_horz = _max_vert = _fullscreen = _floating = false;
  
  unsigned long *state;
  unsigned long num = (unsigned) -1;
  
  if (property->get(_window, otk::OBProperty::net_wm_state,
                    otk::OBProperty::Atom_Atom, &num, &state)) {
    for (unsigned long i = 0; i < num; ++i) {
      if (state[i] == property->atom(otk::OBProperty::net_wm_state_modal))
        _modal = true;
      else if (state[i] ==
               property->atom(otk::OBProperty::net_wm_state_shaded))
        _shaded = true;
      else if (state[i] ==
               property->atom(otk::OBProperty::net_wm_state_fullscreen))
        _fullscreen = true;
      else if (state[i] ==
               property->atom(otk::OBProperty::net_wm_state_maximized_vert))
        _max_vert = true;
      else if (state[i] ==
               property->atom(otk::OBProperty::net_wm_state_maximized_horz))
        _max_horz = true;
    }

    delete [] state;
  }
}


void OBClient::getShaped()
{
  _shaped = false;
#ifdef   SHAPE
  if (otk::OBDisplay::shape()) {
    int foo;
    unsigned int ufoo;

    XShapeQueryExtents(otk::OBDisplay::display, client.window, &_shaped, &foo,
                       &foo, &ufoo, &ufoo, &foo, &foo, &foo, &ufoo, &ufoo);
  }
#endif // SHAPE
}


void OBClient::updateNormalHints()
{
  XSizeHints size;
  long ret;

  // defaults
  _gravity = NorthWestGravity;
  _inc_x = _inc_y = 1;
  _base_x = _base_y = 0;
  _min_x = _min_y = 0;
  _max_x = _max_y = (unsigned) -1;

  // get the hints from the window
  if (XGetWMNormalHints(otk::OBDisplay::display, _window, &size, &ret)) {
    if (size.flags & PWinGravity)
      _gravity = size.win_gravity;
    if (size.flags & PMinSize) {
      _min_x = size.min_width;
      _min_y = size.min_height;
    }
    if (size.flags & PMaxSize) {
      _max_x = size.max_width;
      _max_y = size.max_height;
    }
    if (size.flags & PBaseSize) {
      _base_x = size.base_width;
      _base_y = size.base_height;
    }
    if (size.flags & PResizeInc) {
      _inc_x = size.width_inc;
      _inc_y = size.height_inc;
    }
  }
}


void OBClient::updateWMHints()
{
  XWMHints *hints;

  // assume a window takes input if it doesnt specify
  _can_focus = true;
  _urgent = false;
  
  if ((hints = XGetWMHints(otk::OBDisplay::display, _window)) != NULL) {
    if (hints->flags & InputHint)
      _can_focus = hints->input;

    if (hints->flags & XUrgencyHint)
      _urgent = true;

    if (hints->flags & WindowGroupHint) {
      if (hints->window_group != _group) {
        // XXX: remove from the old group if there was one
        _group = hints->window_group;
        // XXX: do stuff with the group
      }
    } else // no group!
      _group = None;

    XFree(hints);
  }
}


void OBClient::updateTitle()
{
  const otk::OBProperty *property = Openbox::instance->property();

  _title = "";
  
  // try netwm
  if (! property->get(_window, otk::OBProperty::net_wm_name,
                      otk::OBProperty::utf8, &_title)) {
    // try old x stuff
    property->get(_window, otk::OBProperty::wm_name,
                  otk::OBProperty::ascii, &_title);
  }

  if (_title.empty())
    _title = _("Unnamed Window");
}


void OBClient::updateClass()
{
  const otk::OBProperty *property = Openbox::instance->property();

  // set the defaults
  _app_name = _app_class = "";

  otk::OBProperty::StringVect v;
  unsigned long num = 2;

  if (! property->get(_window, otk::OBProperty::wm_class,
                      otk::OBProperty::ascii, &num, &v))
    return;

  if (num > 0) _app_name = v[0];
  if (num > 1) _app_class = v[1];
}


void OBClient::update(const XPropertyEvent &e)
{
  const otk::OBProperty *property = Openbox::instance->property();

  if (e.atom == XA_WM_NORMAL_HINTS)
    updateNormalHints();
  else if (e.atom == XA_WM_HINTS)
    updateWMHints();
  else if (e.atom == property->atom(otk::OBProperty::net_wm_name) ||
           e.atom == property->atom(otk::OBProperty::wm_name) ||
           e.atom == property->atom(otk::OBProperty::net_wm_icon_name) ||
           e.atom == property->atom(otk::OBProperty::wm_icon_name))
    updateTitle();
  else if (e.atom == property->atom(otk::OBProperty::wm_class))
    updateClass();
  // XXX: transient for hint
}


void OBClient::setWMState(long state)
{
  if (state == _wmstate) return; // no change
  
  switch (state) {
  case IconicState:
    // XXX: cause it to iconify
    break;
  case NormalState:
    // XXX: cause it to uniconify
    break;
  }
  _wmstate = state;
}


void OBClient::setDesktop(long target)
{
  assert(target >= 0);
  //assert(target == 0xffffffff || target < MAX);
  
  // XXX: move the window to the new desktop
  _desktop = target;
}


void OBClient::setState(StateAction action, long data1, long data2)
{
  const otk::OBProperty *property = Openbox::instance->property();

  if (!(action == State_Add || action == State_Remove ||
        action == State_Toggle))
    return; // an invalid action was passed to the client message, ignore it

  for (int i = 0; i < 2; ++i) {
    Atom state = i == 0 ? data1 : data2;
    
    if (! state) continue;

    // if toggling, then pick whether we're adding or removing
    if (action == State_Toggle) {
      if (state == property->atom(otk::OBProperty::net_wm_state_modal))
        action = _modal ? State_Remove : State_Add;
      else if (state ==
               property->atom(otk::OBProperty::net_wm_state_maximized_vert))
        action = _max_vert ? State_Remove : State_Add;
      else if (state ==
               property->atom(otk::OBProperty::net_wm_state_maximized_horz))
        action = _max_horz ? State_Remove : State_Add;
      else if (state == property->atom(otk::OBProperty::net_wm_state_shaded))
        action = _shaded ? State_Remove : State_Add;
      else if (state ==
               property->atom(otk::OBProperty::net_wm_state_fullscreen))
        action = _fullscreen ? State_Remove : State_Add;
      else if (state == property->atom(otk::OBProperty::net_wm_state_floating))
        action = _floating ? State_Remove : State_Add;
    }
    
    if (action == State_Add) {
      if (state == property->atom(otk::OBProperty::net_wm_state_modal)) {
        if (_modal) continue;
        _modal = true;
        // XXX: give it focus if another window has focus that shouldnt now
      } else if (state ==
                 property->atom(otk::OBProperty::net_wm_state_maximized_vert)){
        if (_max_vert) continue;
        _max_vert = true;
        // XXX: resize the window etc
      } else if (state ==
                 property->atom(otk::OBProperty::net_wm_state_maximized_horz)){
        if (_max_horz) continue;
        _max_horz = true;
        // XXX: resize the window etc
      } else if (state ==
                 property->atom(otk::OBProperty::net_wm_state_shaded)) {
        if (_shaded) continue;
        _shaded = true;
        // XXX: hide the client window
      } else if (state ==
                 property->atom(otk::OBProperty::net_wm_state_fullscreen)) {
        if (_fullscreen) continue;
        _fullscreen = true;
        // XXX: raise the window n shit
      } else if (state ==
                 property->atom(otk::OBProperty::net_wm_state_floating)) {
        if (_floating) continue;
        _floating = true;
        // XXX: raise the window n shit
      }

    } else { // action == State_Remove
      if (state == property->atom(otk::OBProperty::net_wm_state_modal)) {
        if (!_modal) continue;
        _modal = false;
      } else if (state ==
                 property->atom(otk::OBProperty::net_wm_state_maximized_vert)){
        if (!_max_vert) continue;
        _max_vert = false;
        // XXX: resize the window etc
      } else if (state ==
                 property->atom(otk::OBProperty::net_wm_state_maximized_horz)){
        if (!_max_horz) continue;
        _max_horz = false;
        // XXX: resize the window etc
      } else if (state ==
                 property->atom(otk::OBProperty::net_wm_state_shaded)) {
        if (!_shaded) continue;
        _shaded = false;
        // XXX: show the client window
      } else if (state ==
                 property->atom(otk::OBProperty::net_wm_state_fullscreen)) {
        if (!_fullscreen) continue;
        _fullscreen = false;
        // XXX: lower the window to its proper layer
      } else if (state ==
                 property->atom(otk::OBProperty::net_wm_state_floating)) {
        if (!_floating) continue;
        _floating = false;
        // XXX: lower the window to its proper layer
      }
    }
  }
}


void OBClient::update(const XClientMessageEvent &e)
{
  if (e.format != 32) return;

  const otk::OBProperty *property = Openbox::instance->property();
  
  if (e.message_type == property->atom(otk::OBProperty::wm_change_state))
    setWMState(e.data.l[0]);
  else if (e.message_type ==
             property->atom(otk::OBProperty::net_wm_desktop))
    setDesktop(e.data.l[0]);
  else if (e.message_type == property->atom(otk::OBProperty::net_wm_state))
    setState((StateAction)e.data.l[0], e.data.l[1], e.data.l[2]);
}


void OBClient::setArea(const otk::Rect &area)
{
  _area = area;
}

}
