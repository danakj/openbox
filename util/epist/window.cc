// -*- mode: C++; indent-tabs-mode: nil; -*-
// window.cc for Epistory - a key handler for NETWM/EWMH window managers.
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

#include "window.hh"
#include "epist.hh"
#include "../../src/XAtom.hh"


XWindow::XWindow(Window window) : _window(window) {
  XSelectInput(_display, _window, PropertyChangeMask);
  updateState();
}


XWindow::~XWindow() {
  XSelectInput(_display, _window, None);
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
