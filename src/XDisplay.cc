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
int XDisplay::errorHandler(Display *d, XErrorEvent *e) {
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

  XSetErrorHandler(errorHandler);

#ifdef    SHAPE
  int waste;
  _hasshape = XShapeQueryExtension(_display, &_shape_event_base, &waste);
#endif // SHAPE

#ifndef NOCLOBBER
  getLockModifiers();
#endif
}


XDisplay::~XDisplay() {
  std::for_each(_screens.begin(), _screens.end(), PointerAssassin());
  XCloseDisplay(_display);
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


/*
 * Gets the next event on the queue from the X server.
 * 
 * Returns: true if e contains a new event; false if there is no event to be
 *          returned.
 */
bool XDisplay::nextEvent(XEvent &e) {
  if(!XPending(_display))
    return false;
  XNextEvent(_display, &e);
  if (_last_bad_window != None) {
    if (e.xany.window == _last_bad_window) {
      cerr << "XDisplay::nextEvent(): Removing event for bad window from " <<
        "event queue\n";
      return false;
    } else
      _last_bad_window = None;
  }
  return true;
}


int XDisplay::connectionNumber() const {
  return ConnectionNumber(_display);
}


/*
 * Creates a font cursor in the X server and returns it.
 */
Cursor createCursor(unsigned int shape) const {
  return XCreateFontCursor(_display, shape);
}


#ifndef   NOCLOBBER
void XDisplay::getLockModifers() {
  NumLockMask = ScrollLockMask = 0;

  const XModifierKeymap* const modmap = XGetModifierMapping(display);
  if (modmap && modmap->max_keypermod > 0) {
    const int mask_table[] = {
      ShiftMask, LockMask, ControlMask, Mod1Mask,
      Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask
    };
    const size_t size = (sizeof(mask_table) / sizeof(mask_table[0])) *
      modmap->max_keypermod;
    // get the values of the keyboard lock modifiers
    // Note: Caps lock is not retrieved the same way as Scroll and Num lock
    // since it doesn't need to be.
    const KeyCode num_lock_code = XKeysymToKeycode(display, XK_Num_Lock);
    const KeyCode scroll_lock_code = XKeysymToKeycode(display, XK_Scroll_Lock);

    for (size_t cnt = 0; cnt < size; ++cnt) {
      if (! modmap->modifiermap[cnt]) continue;

      if (num_lock_code == modmap->modifiermap[cnt])
        NumLockMask = mask_table[cnt / modmap->max_keypermod];
      if (scroll_lock_code == modmap->modifiermap[cnt])
        ScrollLockMask = mask_table[cnt / modmap->max_keypermod];
    }
  }

  MaskList[0] = 0;
  MaskList[1] = LockMask;
  MaskList[2] = NumLockMask;
  MaskList[3] = ScrollLockMask;
  MaskList[4] = LockMask | NumLockMask;
  MaskList[5] = NumLockMask  | ScrollLockMask;
  MaskList[6] = LockMask | ScrollLockMask;
  MaskList[7] = LockMask | NumLockMask | ScrollLockMask;

  if (modmap) XFreeModifiermap(const_cast<XModifierKeymap*>(modmap));
}
#endif // NOCLOBBER
  
unsigned int XDisplay::stripModifiers(const unsigned int state) const {
#ifndef NOCLOBBER
  return state &= ~(NumLockMask() | ScrollLockMask | LockMask);
#else
  return state &= ~LockMask;
#endif
}


/*
 * Verifies that a window has not requested to be destroyed/unmapped, so
 * if it is a valid window or not.
 * Returns: true if the window is valid; false if it is no longer valid.
 */
bool XDisplay::validateWindow(Window window) {
  XEvent event;
  if (XCheckTypedWindowEvent(_display, window, DestroyNotify, &event)) {
    XPutBackEvent(display, &event);
    return false;
  }
  return true;
}


/*
 * Grabs a button, but also grabs the button in every possible combination with
 * the keyboard lock keys, so that they do not cancel out the event.
 */
void BaseDisplay::grabButton(unsigned int button, unsigned int modifiers,
                             Window grab_window, Bool owner_events,
                             unsigned int event_mask, int pointer_mode,
                             int keybaord_mode, Window confine_to,
                             Cursor cursor) const
{
#ifndef   NOCLOBBER
  for (size_t cnt = 0; cnt < 8; ++cnt)
    XGrabButton(_display, button, modifiers | MaskList[cnt], grab_window,
                owner_events, event_mask, pointer_mode, keybaord_mode,
                confine_to, cursor);
#else  // NOCLOBBER
  XGrabButton(_display, button, modifiers, grab_window,
              owner_events, event_mask, pointer_mode, keybaord_mode,
              confine_to, cursor);
#endif // NOCLOBBER
}


/*
 * Releases the grab on a button, and ungrabs all possible combinations of the
 * keyboard lock keys.
 */
void BaseDisplay::ungrabButton(unsigned int button, unsigned int modifiers,
                               Window grab_window) const {
#ifndef   NOCLOBBER
  for (size_t cnt = 0; cnt < 8; ++cnt)
    XUngrabButton(display, button, modifiers | MaskList[cnt], grab_window);
#else  // NOCLOBBER
  XUngrabButton(display, button, modifiers, grab_window);
#endif // NOCLOBBER
}
