// -*- mode: C++; indent-tabs-mode: nil; -*-
// window.cc for Epistophy - a key handler for NETWM/EWMH window managers.
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
  case PropertyNotify:
    // a client window
    if (e.xproperty.atom == _xatom->getAtom(XAtom::net_wm_state))
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
  // this will also unshade the window..
  _xatom->sendClientMessage(_screen->rootWindow(), XAtom::net_active_window,
                            _window);
}
