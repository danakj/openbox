// -*- mode: C++; indent-tabs-mode: nil; -*-
// screen.hh for Epistory - a key handler for NETWM/EWMH window managers.
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

#ifndef   __screen_hh
#define   __screen_hh

extern "C" {
#include "X11/Xlib.h"
}

#include <vector>

#include "window.hh"

class epist;
class screen;
class XAtom;

class screen {
  epist *_epist;
  XAtom *_xatom;
  int _number;
  Window _root;

  std::string _wm_name;
  
  WindowList _clients;
  WindowList::iterator _active;

  bool _managed;

  XWindow *findWindow(const XEvent &e) const;
  void updateClientList();
  void updateActiveWindow();
  bool doAddWindow(Window window) const;
  bool findSupportingWM();

public:
  screen(epist *epist, int number);
  virtual ~screen();
  
  inline Window rootWindow() const { return _root; }
  inline bool managed() const { return _managed; }
  
  void processEvent(const XEvent &e);
};

#endif // __screen_hh

