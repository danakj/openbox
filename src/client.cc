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

OBClient::OBClient(BScreen *screen, Window window)
  : _screen(screen), _window(window)
{
  assert(_screen);
  assert(window);

  // initialize vars to false/invalid values
  _group = 0;
  _gravity = _base_x = _base_y = _inc_x = _inc_y = _max_x = _max_y = _min_x =
    _min_y = -1;
  _state = -1;
  _type = Type_Normal;
  _desktop = 0xffffffff - 1;
  _can_focus = _urgent = _focus_notify = _shaped = _modal = _shaded =
    _max_horz = _max_vert = _fullscreen = _floating = false;
  
  
  // update EVERYTHING the first time!!
}

OBClient::~OBClient()
{
}


void OBClient::updateNormalHints()
{
  XSizeHints size;
  long ret;

  // defaults
  _gravity = NorthWestGravity;
  _inc_x = _inc_y = 1;
  _base_x = _base_y = 0;

  // get the hints from the window
  if (XGetWMNormalHints(otk::OBDisplay::display, _window, &size, &ret)) {
    if (size.flags & PWinGravity)
      _gravity = size.win_gravity;
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
    if (hints->flags & WindowGroupHint)
      if (hints->window_group != _group) {
        // XXX: remove from the old group if there was one
        _group = hints->window_group;
        // XXX: do stuff with the group
      }
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
}


void OBClient::setWMState(long state)
{
  if (state == _state) return; // no change
  
  switch (state) {
  case IconicState:
    // XXX: cause it to iconify
    break;
  case NormalState:
    // XXX: cause it to uniconify
    break;
  }
  _state = state;
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

}
