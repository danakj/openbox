// XDisplay.cc for Openbox
// Copyright (c) 2002 - 2002 Ben Janens (ben at orodu.net)
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

#include "Xdisplay.h"
#include "XScreen.h"
#include "Util.h"
#include <iostream>
#include <algorithm>

using std::cerr;

Xdisplay::Xdisplay(const char *dpyname) {
  _grabs = 0;
  _hasshape = false;
  
  _display = XOpenDisplay(dpy_name);
  if (_display == NULL) {
    cerr << "Could not open display. Connection to X server failed.\n";
    ::exit(2);
  }
  if (-1 == fcntl(ConnectionNumber(display), F_SETFD, 1)) {
    cerr << "Could not mark display connection as close-on-exec.\n";
    ::exit(2);
  }
  _name = XDisplayName(dpyname);

  XSetErrorHandler(XErrorHandler);

#ifdef    SHAPE
  int waste;
  _hasshape = XShapeQueryExtension(_display, &_shape_event_base, &waste);
#endif // SHAPE

  const unsigned int scount = ScreenCount(_display);
  _screens.reserve(scount);
  for (unsigned int s = 0; s < scount; s++)
    _screens.push_back(new XScreen(_display, s));
}


Xdisplay::~Xdisplay() {
  std::for_each(_screens.begin(), _screens.end(), PointerAssassin());
  XCloseDisplay(_display);
}


/*
 * Return information about a screen.
 */
XScreen *Xdisplay::screen(unsigned int s) const {
  ASSERT(s < _screens.size());
  return _screens[s];
}


/*
 * Grab the X server
 */
void XDisplay::grab() {
  if (_grabs++ == 0)
    XGrabServer(_display);
}


/*
 * Release the X server from a grab
 */
void XDisplay::ungrab() {
  if (--_grabs == 0)
    XUngrabServer(_display);
}
