// -*- mode: C++; indent-tabs-mode: nil; -*-
// window.cc for Epistrophy - a key handler for NETWM/EWMH window managers.
// Copyright (c) 2002 - 2002 Ben Jansens <ben at orodu.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifdef    HAVE_CONFIG_H
#  include "../../config.h"
#endif // HAVE_CONFIG_H

#include <iostream>

using std::cout;
using std::endl;
using std::hex;
using std::dec;

#include "epist.hh"
#include "screen.hh"
#include "window.hh"
#include "../../src/XAtom.hh"

XWindow::XWindow(epist *epist, screen *screen, Window window)
  : _epist(epist), _screen(screen), _xatom(epist->xatom()), _window(window) {

  _unmapped = false;

  XSelectInput(_epist->getXDisplay(), _window,
               PropertyChangeMask | StructureNotifyMask);

  updateBlackboxAttributes();
  updateNormalHints();
  updateWMHints();
  updateDimentions();
  updateState();
  updateDesktop();
  updateTitle();
  updateClass();

  _epist->addWindow(this);
}


XWindow::~XWindow() {
  if (! _unmapped)
    XSelectInput(_epist->getXDisplay(), _window, None);
  _epist->removeWindow(this);
}


void XWindow::updateDimentions() {
  Window root, child;
  int x, y;
  unsigned int w, h, b, d;

  if (XGetGeometry(_epist->getXDisplay(), _window, &root, &x, &y, &w, &h,
                     &b, &d) &&
      XTranslateCoordinates(_epist->getXDisplay(), _window, root, x, y,
                            &x, &y, &child))
    _rect.setRect(x, y, w, h);
  else
    _rect.setRect(0, 0, 1, 1);
}


void XWindow::updateBlackboxAttributes() {
  unsigned long *data;
  unsigned long num = PropBlackboxAttributesElements;

  _decorated = true;

  if (_xatom->getValue(_window,
                       XAtom::blackbox_attributes, XAtom::blackbox_attributes,
                       num, &data)) {
    if (num == PropBlackboxAttributesElements)
      if (data[0] & AttribDecoration)
        _decorated = (data[4] != DecorNone);
    delete data;
  }
}


void XWindow::updateNormalHints() {
  XSizeHints size;
  long ret;

  // defaults
  _gravity = NorthWestGravity;
  _inc_x = _inc_y = 1;
  _base_x = _base_y = 0;
  
  if (XGetWMNormalHints(_epist->getXDisplay(), _window, &size, &ret)) {
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


void XWindow::updateWMHints() {
  XWMHints *hints;

  if ((hints = XGetWMHints(_epist->getXDisplay(), _window)) != NULL) {
    _can_focus = hints->input;
    XFree(hints);
  } else {
    // assume a window takes input if it doesnt specify
    _can_focus = True;
  }
}


void XWindow::updateState() {
  // set the defaults
  _shaded = _iconic = _max_vert = _max_horz = false;
  
  unsigned long num = (unsigned) -1;
  Atom *state;
  if (! _xatom->getValue(_window, XAtom::net_wm_state, XAtom::atom,
                         num, &state))
    return;
  for (unsigned long i = 0; i < num; ++i) {
    if (state[i] == _xatom->getAtom(XAtom::net_wm_state_maximized_vert))
      _max_vert = true;
    if (state[i] == _xatom->getAtom(XAtom::net_wm_state_maximized_horz))
      _max_horz = true;
    if (state[i] == _xatom->getAtom(XAtom::net_wm_state_shaded))
      _shaded = true;
    if (state[i] == _xatom->getAtom(XAtom::net_wm_state_hidden))
      _iconic = true;
  }

  delete [] state;
}


void XWindow::updateDesktop() {
  if (! _xatom->getValue(_window, XAtom::net_wm_desktop, XAtom::cardinal,
                         static_cast<unsigned long>(_desktop)))
    _desktop = 0;
}


void XWindow::updateTitle() {
  _title = "";
  
  // try netwm
  if (! _xatom->getValue(_window, XAtom::net_wm_name, XAtom::utf8, _title)) {
    // try old x stuff
    _xatom->getValue(_window, XAtom::wm_name, XAtom::ansi, _title);
  }

  if (_title.empty())
    _title = "Unnamed";
}


void XWindow::updateClass() {
  // set the defaults
  _app_name = _app_class = "";

  XAtom::StringVect v;
  unsigned long num = 2;

  if (! _xatom->getValue(_window, XAtom::wm_class, XAtom::ansi, num, v))
    return;

  if (num > 0) _app_name = v[0];
  if (num > 1) _app_class = v[1];
}


void XWindow::processEvent(const XEvent &e) {
  assert(e.xany.window == _window);

  switch (e.type) {
  case ConfigureNotify:
    updateDimentions();
    break;
  case PropertyNotify:
    if (e.xproperty.atom == XA_WM_NORMAL_HINTS)
      updateNormalHints();
    if (e.xproperty.atom == XA_WM_HINTS)
      updateWMHints();
    else if (e.xproperty.atom == _xatom->getAtom(XAtom::net_wm_state))
      updateState();
    else if (e.xproperty.atom == _xatom->getAtom(XAtom::net_wm_desktop))
      updateDesktop();
    else if (e.xproperty.atom == _xatom->getAtom(XAtom::net_wm_name) ||
             e.xproperty.atom == _xatom->getAtom(XAtom::wm_name))
      updateTitle();
    else if (e.xproperty.atom == _xatom->getAtom(XAtom::wm_class))
      updateClass();
    break;
  case DestroyNotify:
  case UnmapNotify:
    _unmapped = true;
    break;
  }
}


void XWindow::shade(const bool sh) const {
  _xatom->sendClientMessage(_screen->rootWindow(), XAtom::net_wm_state,
                            _window, (sh ? 1 : 0),
                            _xatom->getAtom(XAtom::net_wm_state_shaded));
}


void XWindow::close() const {
  _xatom->sendClientMessage(_screen->rootWindow(), XAtom::net_close_window,
                            _window);
}


void XWindow::raise() const {
  XRaiseWindow(_epist->getXDisplay(), _window);
}


void XWindow::lower() const {
  XLowerWindow(_epist->getXDisplay(), _window);
}


void XWindow::iconify() const {
  _xatom->sendClientMessage(_screen->rootWindow(), XAtom::wm_change_state,
                            _window, IconicState);
}


void XWindow::focus() const {
  // this will cause the window to be uniconified also
  _xatom->sendClientMessage(_screen->rootWindow(), XAtom::net_active_window,
                            _window);
 
  //XSetInputFocus(_epist->getXDisplay(), _window, None, CurrentTime);
}


void XWindow::sendTo(unsigned int dest) const {
  _xatom->sendClientMessage(_screen->rootWindow(), XAtom::net_wm_desktop,
                            _window, dest);
}


void XWindow::move(int x, int y) const {
  // get the window's decoration sizes (margins)
  Strut margins;
  Window win = _window, parent, root, last = None;
  Window *children = 0;
  unsigned int nchildren;
  XWindowAttributes wattr;
  
  while (XQueryTree(_epist->getXDisplay(), win, &root, &parent, &children,
                    &nchildren)) {
    if (children && nchildren > 0)
      XFree(children); // don't care about the children

    if (! parent) // no parent!?
      return;

    // if the parent window is the root window, stop here
    if (parent == root)
      break;

    last = win;
    win = parent;
  }

  if (! (XTranslateCoordinates(_epist->getXDisplay(), last, win, 0, 0,
                               (int *) &margins.left,
                               (int *) &margins.top,
                               &parent) &&
         XGetWindowAttributes(_epist->getXDisplay(), win, &wattr)))
    return;

  margins.right = wattr.width - _rect.width() - margins.left;
  margins.bottom = wattr.height - _rect.height() - margins.top;

  margins.left += wattr.border_width;
  margins.right += wattr.border_width;
  margins.top += wattr.border_width;
  margins.bottom += wattr.border_width;

  // this makes things work. why? i don't know. but you need them.
  margins.right -= 2;
  margins.bottom -= 2;
  
  // find the frame's reference position based on the window's gravity
  switch (_gravity) {
  case NorthWestGravity:
    x -= margins.left;
    y -= margins.top;
    break;
  case NorthGravity:
    x += (margins.left + margins.right) / 2;
    y -= margins.top;
    break;
  case NorthEastGravity:
    x += margins.right;
    y -= margins.top;
  case WestGravity:
    x -= margins.left;
    y += (margins.top + margins.bottom) / 2;
    break;
  case CenterGravity:
    x += (margins.left + margins.right) / 2;
    y += (margins.top + margins.bottom) / 2;
    break;
  case EastGravity:
    x += margins.right;
    y += (margins.top + margins.bottom) / 2;
  case SouthWestGravity:
    x -= margins.left;
    y += margins.bottom;
    break;
  case SouthGravity:
    x += (margins.left + margins.right) / 2;
    y += margins.bottom;
    break;
  case SouthEastGravity:
    x += margins.right;
    y += margins.bottom;
    break;
  default:
    break;
  }
  
  XMoveWindow(_epist->getXDisplay(), _window, x, y);
}


void XWindow::resizeRel(int dwidth, int dheight) const {
  // resize in increments if requested by the window
  unsigned int width = _rect.width(), height = _rect.height();
  
  unsigned int wdest = width + (dwidth * _inc_x) / _inc_x * _inc_x + _base_x;
  unsigned int hdest = height + (dheight * _inc_y) / _inc_y * _inc_y + _base_y;

  XResizeWindow(_epist->getXDisplay(), _window, wdest, hdest);
}


void XWindow::resizeAbs(unsigned int width, unsigned int height) const {
  // resize in increments if requested by the window

  unsigned int wdest = width / _inc_x * _inc_x + _base_x;
  unsigned int hdest = height / _inc_y * _inc_y + _base_y;

  if (width > wdest) {
    while (width > wdest)
      wdest += _inc_x;
  } else {
    while (width < wdest)
      wdest -= _inc_x;
  }
  if (height > hdest) {
    while (height > hdest)
      hdest += _inc_y;
  } else {
    while (height < hdest)
      hdest -= _inc_y;
  }
  
  XResizeWindow(_epist->getXDisplay(), _window, wdest, hdest);
}


void XWindow::toggleMaximize(Max max) const {
  switch (max) {
  case Max_Full:
    _xatom->
      sendClientMessage(_screen->rootWindow(), XAtom::net_wm_state,
                        _window, (_max_vert == _max_horz ? 2 : 1),
                        _xatom->getAtom(XAtom::net_wm_state_maximized_horz),
                        _xatom->getAtom(XAtom::net_wm_state_maximized_vert));
    break;

  case Max_Horz:
    _xatom->
      sendClientMessage(_screen->rootWindow(), XAtom::net_wm_state,
                        _window, 2,
                        _xatom->getAtom(XAtom::net_wm_state_maximized_horz));
    break;

  case Max_Vert:
    _xatom->
      sendClientMessage(_screen->rootWindow(), XAtom::net_wm_state,
                        _window, 2,
                        _xatom->getAtom(XAtom::net_wm_state_maximized_vert));
    break;
    
  case Max_None:
    assert(false);  // you should not do this. it is pointless and probly a bug
    break;
  }
}


void XWindow::maximize(Max max) const {
  switch (max) {
  case Max_None:
    _xatom->
      sendClientMessage(_screen->rootWindow(), XAtom::net_wm_state,
                        _window, 0,
                        _xatom->getAtom(XAtom::net_wm_state_maximized_horz),
                        _xatom->getAtom(XAtom::net_wm_state_maximized_vert));
    break;

  case Max_Full:
    _xatom->
      sendClientMessage(_screen->rootWindow(), XAtom::net_wm_state,
                        _window, 1,
                        _xatom->getAtom(XAtom::net_wm_state_maximized_horz),
                        _xatom->getAtom(XAtom::net_wm_state_maximized_vert));
    break;

  case Max_Horz:
    _xatom->
      sendClientMessage(_screen->rootWindow(), XAtom::net_wm_state,
                        _window, 1,
                        _xatom->getAtom(XAtom::net_wm_state_maximized_horz));
    break;

  case Max_Vert:
    _xatom->
      sendClientMessage(_screen->rootWindow(), XAtom::net_wm_state,
                        _window, 1,
                        _xatom->getAtom(XAtom::net_wm_state_maximized_vert));
    break;
  }
}


void XWindow::decorate(bool d) const {
  _xatom->sendClientMessage(_screen->rootWindow(),
                            XAtom::blackbox_change_attributes,
                            _window, AttribDecoration,
                            0, 0, 0, (d ? DecorNormal : DecorNone));
}
