// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// epist.cc for Epistrophy - a key handler for NETWM/EWMH window managers.
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

#ifdef    HAVE_UNISTD_H
#  include <sys/types.h>
#  include <unistd.h>
#endif // HAVE_UNISTD_H

#ifdef    HAVE_SYS_STAT_H
#  include <sys/types.h>
#  include <sys/stat.h>
#endif // HAVE_SYS_STAT_H
}

#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::string;

#include "actions.hh"
#include "epist.hh"
#include "screen.hh"
#include "window.hh"
#include "parser.hh"
#include "../../src/xatom.hh"


epist::epist(char **argv, char *dpy_name, char *rc_file)
  : BaseDisplay(argv[0], dpy_name) {
    
  _argv = argv;

  if (rc_file)
    _rc_file = rc_file;
  else
    _rc_file = expandTilde("~/.openbox/epistrc");

  struct stat buf;
  if (0 != stat(_rc_file.c_str(), &buf) ||
      !S_ISREG(buf.st_mode))
    _rc_file = DEFAULTRC;

  _xatom = new XAtom(getXDisplay());
  _active = _clients.end();

  _config = new Config;
  _ktree = new keytree(getXDisplay(), this);

  // set up the key tree
  parser p(_ktree, _config);
  p.parse(_rc_file);
  
  for (unsigned int i = 0; i < getNumberOfScreens(); ++i) {
    screen *s = new screen(this, i);
    if (s->managed()) {
      _screens.push_back(s);
      s->updateEverything();
    }
  }
  if (_screens.empty()) {
    cout << "No compatible window manager found on any screens. Aborting.\n";
    ::exit(1);
  }

  activateGrabs();
}


epist::~epist() {
  delete _xatom;
}

void epist::activateGrabs() {

  ScreenList::const_iterator scrit, scrend = _screens.end();
  
  for (scrit = _screens.begin(); scrit != scrend; ++scrit)
      _ktree->grabDefaults(*scrit);
}


bool epist::handleSignal(int sig) {
  switch (sig) {
  case SIGHUP: {
    cout << "epist: Restarting on request.\n";
    
    execvp(_argv[0], _argv);

    string base(basename(_argv[0]));
    execvp(base.c_str(), _argv);
    
    return false;  // this should be unreachable
  }

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


void epist::cycleScreen(int current, bool forward) const {
  unsigned int i;
  for (i = 0; i < _screens.size(); ++i)
    if (_screens[i]->number() == current) {
      current = i;
      break;
    }
  assert(i < _screens.size());  // current is for an unmanaged screen
  
  int dest = current + (forward ? 1 : -1);

  if (dest < 0) dest = (signed)_screens.size() - 1;
  else if (dest >= (signed)_screens.size()) dest = 0;

  const XWindow *target = _screens[dest]->lastActiveWindow();
  if (target) target->focus();
}
