// -*- mode: C++; indent-tabs-mode: nil; -*-
// epist.hh for Epistory - a key handler for NETWM/EWMH window managers.
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

#ifndef   __epist_hh
#define   __epist_hh

extern "C" {
#include <X11/Xlib.h>
}

#include <string>
#include <map>

#include <actions.hh>

#include "../../src/BaseDisplay.hh"

class XAtom;
class screen;
class XWindow;

class epist : public BaseDisplay {
private:
  std::string     _rc_file;
  XAtom          *_xatom;
  char          **_argv;

  typedef std::vector<screen *> ScreenList;
  ScreenList      _screens;

  typedef std::map<Window, XWindow*> WindowLookup;
  typedef WindowLookup::value_type WindowLookupPair;
  WindowLookup    _windows;

  ActionList _actions;
  
  virtual void process_event(XEvent *e);
  virtual bool handleSignal(int sig);

  void activateGrabs();
public:
  epist(char **argv, char *display_name, char *rc_file);
  virtual ~epist();

  inline XAtom *xatom() { return _xatom; }

  void addWindow(XWindow *window);
  void removeWindow(XWindow *window);
  XWindow *findWindow(Window window) const;

  list<Action> actions(void) { return _actions; }
};

#endif // __epist_hh
