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

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#ifdef HAVE_UNNISTD_H
# include <unistd.h>
#endif

#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif

#ifdef SHAPE
# include <X11/extensions/shape.h>
#endif

#include "XDisplay.h"
#include "XScreen.h"
#include "Util.h"
#include <iostream>
#include <algorithm>

using std::cerr;
using std::endl;

std::string XDisplay::_app_name;
Window      XDisplay::_last_bad_window = None;
  
/*
 * X error handler to handle all X errors while the application is
 * running.
 */
int XDisplay::XErrorHandler(Display *d, XErrorEvent *e) {
#ifdef DEBUG
  char errtxt[128];
  XGetErrorText(d, e->error_code, errtxt, sizeof(errtxt)/sizeof(char));
  cerr << _app_name.c_str() << ": X error: " << 
    errtxt << "(" << e->error_code << ") opcodes " <<
    e->request_code << "/" << e->minor_code << endl;
  cerr.flags(std::ios_base::hex);
  cerr << "  resource 0x" << e->resourceid << endl;
  cerr.flags(std::ios_base::dec);
#endif
  
  if (e->error_code == BadWindow)
    _last_bad_window = e->resourceid;
  
  return False;
}


XDisplay::XDisplay(const std::string &application_name, const char *dpyname) {
  _app_name = application_name;
  _grabs = 0;
  _hasshape = false;
  
  _display = XOpenDisplay(dpyname);
  if (_display == NULL) {
    cerr << "Could not open display. Connection to X server failed.\n";
    ::exit(2);
  }
  if (-1 == fcntl(ConnectionNumber(_display), F_SETFD, 1)) {
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
    _screens.push_back(new XScreen(this, s));
}


XDisplay::~XDisplay() {
  std::for_each(_screens.begin(), _screens.end(), PointerAssassin());
  XCloseDisplay(_display);
}


/*
 * Return information about a screen.
 */
XScreen *XDisplay::screen(unsigned int s) const {
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
