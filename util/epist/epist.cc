// -*- mode: C++; indent-tabs-mode: nil; -*-
// epist.cc for Epistophy - a key handler for NETWM/EWMH window managers.
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

extern "C" {
#ifdef    HAVE_UNISTD_H
#  include <sys/types.h>
#  include <unistd.h>
#endif // HAVE_UNISTD_H

#ifdef    HAVE_STDLIB_H
#  include <stdlib.h>
#endif // HAVE_STDLIB_H

#ifdef    HAVE_SIGNAL_H
#  include <signal.h>
#endif // HAVE_SIGNAL_H

#ifdef    HAVE_LIBGEN_H
#  include <libgen.h>
#endif // HAVE_LIBGEN_H
}

#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;

#include "epist.hh"
#include "screen.hh"
#include "window.hh"
#include "../../src/XAtom.hh"


epist::epist(char **argv, char *dpy_name, char *rc_file)
  : BaseDisplay(argv[0], dpy_name) {

  _argv = argv;

  if (rc_file)
    _rc_file = rc_file;
  else
    _rc_file = expandTilde("~/.openbox/epistrc");

  _xatom = new XAtom(getXDisplay());

  screen *s = new screen(this, DefaultScreen(getXDisplay()));
  if (s->managed())
    _screens.push_back(s);
  if (_screens.empty()) {
    cout << "No compatible window manager found on any screens. Aborting.\n";
    ::exit(1);
  }

  _actions.push_back(Action(Action::nextWorkspace,
                            XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("Tab")),
                            ControlMask));
  _actions.push_back(Action(Action::prevWorkspace,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("Tab")),
                           ControlMask | ShiftMask));
  _actions.push_back(Action(Action::toggleshade,
                            XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("F5")),
                            Mod1Mask));
  _actions.push_back(Action(Action::close,
                            XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("F4")),
                            Mod1Mask));
  _actions.push_back(Action(Action::nextWindow,
                            XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("Tab")),
                            Mod1Mask));
  _actions.push_back(Action(Action::prevWindow,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("Tab")),
                           Mod1Mask | ShiftMask));
  _actions.push_back(Action(Action::nextWindowOnAllWorkspaces,
                            XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("Tab")),
                            Mod1Mask | ControlMask));
  _actions.push_back(Action(Action::prevWindowOnAllWorkspaces,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("Tab")),
                           Mod1Mask | ShiftMask | ControlMask));
  _actions.push_back(Action(Action::raise,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("Up")),
                           Mod1Mask));
  _actions.push_back(Action(Action::lower,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("Down")),
                           Mod1Mask));
  _actions.push_back(Action(Action::iconify,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("I")),
                           Mod1Mask | ControlMask));
  _actions.push_back(Action(Action::toggleomnipresent,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("O")),
                           Mod1Mask | ControlMask));
  _actions.push_back(Action(Action::sendTo,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("1")),
                           Mod1Mask | ControlMask, 0));
  _actions.push_back(Action(Action::sendTo,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("2")),
                           Mod1Mask | ControlMask, 1));
  _actions.push_back(Action(Action::sendTo,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("3")),
                           Mod1Mask | ControlMask, 2));
  _actions.push_back(Action(Action::sendTo,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("4")),
                           Mod1Mask | ControlMask, 3));
  activateGrabs();
}


epist::~epist() {
  delete _xatom;
}

void epist::activateGrabs() {

  ScreenList::const_iterator scrit, scrend = _screens.end();
  
  for (scrit = _screens.begin(); scrit != scrend; ++scrit) {
    ActionList::const_iterator ait, end = _actions.end();

    for(ait = _actions.begin(); ait != end; ++ait) {
      XGrabKey(getXDisplay(), ait->keycode(), ait->modifierMask(),
               (*scrit)->rootWindow(), False, GrabModeAsync, GrabModeAsync);
    }
  }
}


bool epist::handleSignal(int sig) {
  switch (sig) {
  case SIGHUP:
    cout << "epist: Restarting on request.\n";
    execvp(_argv[0], _argv);
    execvp(basename(_argv[0]), _argv);
    return false;  // this should be unreachable

  case SIGTERM:
  case SIGINT:
  case SIGPIPE:
    shutdown();
    return true;
  }

  return false;
}


void epist::process_event(XEvent *e) {
  Window root;

  if (e->xany.type == KeyPress)
    root = e->xkey.root;
  else
    root = e->xany.window;
  
  ScreenList::const_iterator it, end = _screens.end();
  for (it = _screens.begin(); it != end; ++it) {
    if ((*it)->rootWindow() == root) {
      (*it)->processEvent(*e);
      return;
    }
  }

  // wasnt a root window, try for client windows
  XWindow *w = findWindow(e->xany.window);
  if (w) w->processEvent(*e);
}
  

void epist::addWindow(XWindow *window) {
  _windows.insert(WindowLookupPair(window->window(), window));
}


void epist::removeWindow(XWindow *window) {
  _windows.erase(window->window());
}


XWindow *epist::findWindow(Window window) const {
  WindowLookup::const_iterator it = _windows.find(window);
  if (it != _windows.end())
    return it->second;

  return 0;
}
