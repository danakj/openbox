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

  for (unsigned int i = 0; i < getNumberOfScreens(); ++i) {
    screen *s = new screen(this, i);
    if (s->managed())
      _screens.push_back(s);
  }
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
  _actions.push_back(Action(Action::moveWindowUp,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("Up")),
                           Mod1Mask | ControlMask, 1));
  _actions.push_back(Action(Action::moveWindowDown,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("Down")),
                           Mod1Mask | ControlMask, 1));
  _actions.push_back(Action(Action::moveWindowLeft,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("Left")),
                           Mod1Mask | ControlMask, 1));
  _actions.push_back(Action(Action::moveWindowRight,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("Right")),
                           Mod1Mask | ControlMask, 1));
  _actions.push_back(Action(Action::resizeWindowHeight,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("Up")),
                           ShiftMask | Mod1Mask | ControlMask, -1));
  _actions.push_back(Action(Action::resizeWindowHeight,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("Down")),
                           ShiftMask | Mod1Mask | ControlMask, 1));
  _actions.push_back(Action(Action::resizeWindowWidth,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("Left")),
                           ShiftMask | Mod1Mask | ControlMask, -1));
  _actions.push_back(Action(Action::resizeWindowWidth,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("Right")),
                           ShiftMask | Mod1Mask | ControlMask, 1));
  _actions.push_back(Action(Action::iconify,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("I")),
                           Mod1Mask | ControlMask));
  _actions.push_back(Action(Action::toggleomnipresent,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("O")),
                           Mod1Mask | ControlMask));
  _actions.push_back(Action(Action::toggleMaximizeHorizontal,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("X")),
                           ShiftMask | Mod1Mask));
  _actions.push_back(Action(Action::toggleMaximizeVertical,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("X")),
                           ShiftMask | ControlMask));
  _actions.push_back(Action(Action::toggleMaximizeFull,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("X")),
                           Mod1Mask | ControlMask));
  _actions.push_back(Action(Action::changeWorkspace,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("1")),
                           Mod1Mask | ControlMask, 0));
  _actions.push_back(Action(Action::changeWorkspace,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("2")),
                           Mod1Mask | ControlMask, 1));
  _actions.push_back(Action(Action::changeWorkspace,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("3")),
                           Mod1Mask | ControlMask, 2));
  _actions.push_back(Action(Action::changeWorkspace,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("4")),
                           Mod1Mask | ControlMask, 3));
  _actions.push_back(Action(Action::sendToWorkspace,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("1")),
                           Mod1Mask | ControlMask | ShiftMask, 0));
  _actions.push_back(Action(Action::sendToWorkspace,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("2")),
                           Mod1Mask | ControlMask | ShiftMask, 1));
  _actions.push_back(Action(Action::sendToWorkspace,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("3")),
                           Mod1Mask | ControlMask | ShiftMask, 2));
  _actions.push_back(Action(Action::sendToWorkspace,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("4")),
                           Mod1Mask | ControlMask | ShiftMask, 3));
  _actions.push_back(Action(Action::execute,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("Escape")),
                           Mod1Mask | ControlMask,
                           "sleep 1 && xset dpms force off"));
  _actions.push_back(Action(Action::execute,
                           XKeysymToKeycode(getXDisplay(),
                                             XStringToKeysym("space")),
                           Mod1Mask, "rxvt"));
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
      (*scrit)->grabKey(ait->keycode(), ait->modifierMask());
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
  ScreenList::const_iterator it, end = _screens.end();
  for (it = _screens.begin(); it != end; ++it) {
    if ((*it)->rootWindow() == e->xany.window) {
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
