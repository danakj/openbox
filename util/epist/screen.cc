// -*- mode: C++; indent-tabs-mode: nil; -*-
// screen.cc for Epistophy - a key handler for NETWM/EWMH window managers.
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
#ifdef    HAVE_STDIO_H
#  include <stdio.h>
#endif // HAVE_STDIO_H

#ifdef    HAVE_UNISTD_H
#  include <sys/types.h>
#  include <unistd.h>
#endif // HAVE_UNISTD_H
}

#include <iostream>
#include <string>

using std::cout;
using std::endl;
using std::hex;
using std::dec;
using std::string;

#include "../../src/BaseDisplay.hh"
#include "../../src/XAtom.hh"
#include "screen.hh"
#include "epist.hh"


screen::screen(epist *epist, int number) {
  _epist = epist;
  _xatom = _epist->xatom();
  _number = number;
  _active = _clients.end();
  _info = _epist->getScreenInfo(_number);
  _root = _info->getRootWindow();
  
  // find a window manager supporting NETWM, waiting for it to load if we must
  int count = 20;  // try for 20 seconds
  _managed = false;
  while (! (_epist->doShutdown() || _managed || count <= 0)) {
    if (! (_managed = findSupportingWM()))
      usleep(1000);
    --count;
  }
  if (_managed)
    cout << "Found compatible window manager '" << _wm_name << "' for screen "
      << _number << ".\n";
  else {
    cout << "Unable to find a compatible window manager for screen " <<
      _number << ".\n";
    return;
  }
 
  XSelectInput(_epist->getXDisplay(), _root, PropertyChangeMask);

  updateNumDesktops();
  updateActiveDesktop();
  updateClientList();
  updateActiveWindow();
}


screen::~screen() {
  if (_managed)
    XSelectInput(_epist->getXDisplay(), _root, None);
}


bool screen::findSupportingWM() {
  Window support_win;
  if (! _xatom->getValue(_root, XAtom::net_supporting_wm_check, XAtom::window,
                         support_win) || support_win == None)
    return false;

  string title;
  _xatom->getValue(support_win, XAtom::net_wm_name, XAtom::utf8, title);
  _wm_name = title;
  return true;
}


XWindow *screen::findWindow(const XEvent &e) const {
  assert(_managed);

  WindowList::const_iterator it, end = _clients.end();
  for (it = _clients.begin(); it != end; ++it)
    if (**it == e.xany.window)
      break;
  if(it == end)
    return 0;
  return *it;
}


void screen::processEvent(const XEvent &e) {
  assert(_managed);
  assert(e.xany.window == _root);

  XWindow *window = 0;
  if (e.xany.window != _root) {
    window = findWindow(e);  // find the window
    assert(window); // we caught an event for a window we don't know about!?
  }

  switch (e.type) {
  case PropertyNotify:
    // root window
    if (e.xproperty.atom == _xatom->getAtom(XAtom::net_number_of_desktops))
      updateNumDesktops();
    if (e.xproperty.atom == _xatom->getAtom(XAtom::net_current_desktop))
      updateActiveDesktop();
    if (e.xproperty.atom == _xatom->getAtom(XAtom::net_active_window))
      updateActiveWindow();
    if (e.xproperty.atom == _xatom->getAtom(XAtom::net_client_list)) {
      // catch any window unmaps first
      XEvent ev;
      if (XCheckTypedWindowEvent(_epist->getXDisplay(), e.xany.window,
                                 DestroyNotify, &ev) ||
          XCheckTypedWindowEvent(_epist->getXDisplay(), e.xany.window,
                                 UnmapNotify, &ev)) {
        processEvent(ev);
      }

      updateClientList();
    }
    break;
  case KeyPress:
    handleKeypress(e);
    break;
  }
}

void screen::handleKeypress(const XEvent &e) {
  ActionList::const_iterator it = _epist->actions().begin();
  ActionList::const_iterator end = _epist->actions().end();
  for (; it != end; ++it) {
    if (e.xkey.keycode == it->keycode() &&
        e.xkey.state == it->modifierMask()) {
      switch (it->type()) {
      case Action::nextWorkspace:
        cycleWorkspace(true);
        return;

      case Action::prevWorkspace:
        cycleWorkspace(false);
        return;

      case Action::nextWindow:
        cycleWindow(true);
        return;

      case Action::prevWindow:
        cycleWindow(false);
        return;

      case Action::nextWindowOnAllWorkspaces:
        cycleWindow(true, true);
        return;

      case Action::prevWindowOnAllWorkspaces:
        cycleWindow(false, true);
        return;

      case Action::nextWindowOfClass:
        cycleWindow(true, false, true, it->string());
        return;

      case Action::prevWindowOfClass:
        cycleWindow(false, false, true, it->string());
        return;

      case Action::nextWindowOfClassOnAllWorkspaces:
        cycleWindow(true, true, true, it->string());
        return;

      case Action::prevWindowOfClassOnAllWorkspaces:
        cycleWindow(false, true, true, it->string());
        return;

      case Action::changeWorkspace:
        changeWorkspace(it->number());
        return;

      case Action::execute:
        execCommand(it->string());
        return;

      default:
        break;
      }

      // these actions require an active window
      if (_active != _clients.end()) {
        XWindow *window = *_active;

        switch (it->type()) {
        case Action::iconify:
          window->iconify();
          return;

        case Action::close:
          window->close();
          return;

        case Action::raise:
          window->raise();
          return;

        case Action::lower:
          window->lower();
          return;

        case Action::sendToWorkspace:
          window->sendTo(it->number());
          return;

        case Action::toggleomnipresent:
          if (window->desktop() == 0xffffffff)
            window->sendTo(_active_desktop);
          else
            window->sendTo(0xffffffff);
          return;

        case Action::moveWindowUp:
          window->move(0, -it->number());
          return;
      
        case Action::moveWindowDown:
          window->move(0, it->number());
          return;
      
        case Action::moveWindowLeft:
          window->move(-it->number(), 0);
          return;
      
        case Action::moveWindowRight:
          window->move(it->number(), 0);
          return;
      
        case Action::toggleshade:
          window->shade(! window->shaded());
          return;
      
        case Action::toggleMaximizeHorizontal:
          window->toggleMaximize(XWindow::Max_Horz);
          return;
      
        case Action::toggleMaximizeVertical:
          window->toggleMaximize(XWindow::Max_Vert);
          return;
      
        case Action::toggleMaximizeFull:
          window->toggleMaximize(XWindow::Max_Full);
          return;
      
        default:
          assert(false);  // unhandled action type!
          break;
        }
      }
    }
  }
}

// do we want to add this window to our list?
bool screen::doAddWindow(Window window) const {
  assert(_managed);

  Atom type;
  if (! _xatom->getValue(window, XAtom::net_wm_window_type, XAtom::atom,
                         type))
    return True;

  if (type == _xatom->getAtom(XAtom::net_wm_window_type_dock) ||
      type == _xatom->getAtom(XAtom::net_wm_window_type_menu))
    return False;

  return True;
}


void screen::updateNumDesktops() {
  assert(_managed);

  if (! _xatom->getValue(_root, XAtom::net_number_of_desktops, XAtom::cardinal,
                         (unsigned long)_num_desktops))
    _num_desktops = 1;  // assume that there is at least 1 desktop!
}


void screen::updateActiveDesktop() {
  assert(_managed);

  if (! _xatom->getValue(_root, XAtom::net_current_desktop, XAtom::cardinal,
                         (unsigned long)_active_desktop))
    _active_desktop = 0;  // there must be at least one desktop, and it must
                          // be the current one
}


void screen::updateClientList() {
  assert(_managed);

  WindowList::iterator insert_point = _active;
  if (insert_point != _clients.end())
    ++insert_point; // get to the item client the focused client
  
  // get the client list from the root window
  Window *rootclients = 0;
  unsigned long num = (unsigned) -1;
  if (! _xatom->getValue(_root, XAtom::net_client_list, XAtom::window, num,
                         &rootclients)) {
    while (! _clients.empty()) {
      delete _clients.front();
      _clients.erase(_clients.begin());
    }
    if (rootclients) delete [] rootclients;
    return;
  }
  
  WindowList::iterator it, end = _clients.end();
  unsigned long i;
  
  // insert new clients after the active window
  for (i = 0; i < num; ++i) {
    for (it = _clients.begin(); it != end; ++it)
      if (**it == rootclients[i])
        break;
    if (it == end) {  // didn't already exist
      if (doAddWindow(rootclients[i])) {
        cout << "Added window: 0x" << hex << rootclients[i] << dec << endl;
        _clients.insert(insert_point, new XWindow(_epist, this,
                                                  rootclients[i]));
      }
    }
  }

  // remove clients that no longer exist
  for (it = _clients.begin(); it != end;) {
    WindowList::iterator it2 = it++;
    for (i = 0; i < num; ++i)
      if (**it2 == rootclients[i])
        break;
    if (i == num)  { // no longer exists
      cout << "Removed window: 0x" << hex << (*it2)->window() << dec << endl;
      delete *it2;
      _clients.erase(it2);
    }
  }

  if (rootclients) delete [] rootclients;
}


void screen::updateActiveWindow() {
  assert(_managed);

  Window a = None;
  _xatom->getValue(_root, XAtom::net_active_window, XAtom::window, a);
  
  WindowList::iterator it, end = _clients.end();
  for (it = _clients.begin(); it != end; ++it) {
    if (**it == a)
      break;
  }
  _active = it;

  cout << "Active window is now: ";
  if (_active == _clients.end()) cout << "None\n";
  else cout << "0x" << hex << (*_active)->window() << dec << endl;
}


void screen::execCommand(const std::string &cmd) const {
  pid_t pid;
  if ((pid = fork()) == 0) {
    extern char **environ;

    char *const argv[] = {
      "sh",
      "-c",
      const_cast<char *>(cmd.c_str()),
      0
    };
    // make the command run on the correct screen
    if (putenv(const_cast<char*>(_info->displayString().c_str()))) {
      cout << "warning: couldn't set environment variable 'DISPLAY'\n";
      perror("putenv()");
    }
    execve("/bin/sh", argv, environ);
    exit(127);
  } else if (pid == -1) {
    cout << _epist->getApplicationName() <<
      ": Could not fork a process for executing a command\n";
  }
}


void screen::cycleWindow(const bool forward, const bool alldesktops,
                         const bool sameclass, const string &cn) const {
  assert(_managed);

  if (_clients.empty()) return;
    
  WindowList::const_iterator target = _active;

  string classname = cn;
  if (sameclass && classname.empty() && target != _clients.end())
    classname = (*target)->appClass();

  if (target == _clients.end())
    target = _clients.begin();
 
  do {
    if (forward) {
      ++target;
      if (target == _clients.end())
        target = _clients.begin();
    } else {
      if (target == _clients.begin())
        target = _clients.end();
      --target;
    }
  } while (target == _clients.end() ||
           (*target)->iconic() ||
           (! alldesktops && (*target)->desktop() != _active_desktop) ||
           (sameclass && ! classname.empty() &&
            (*target)->appClass() != classname));
  
  if (target != _clients.end())
    (*target)->focus();
}


void screen::cycleWorkspace(const bool forward, const bool loop) const {
  assert(_managed);

  unsigned int destination = _active_desktop;

  if (forward) {
    if (destination < _num_desktops - 1)
      ++destination;
    else if (loop)
      destination = 0;
  } else {
    if (destination > 0)
      --destination;
    else if (loop)
      destination = _num_desktops - 1;
  }

  if (destination != _active_desktop) 
    changeWorkspace(destination);
}


void screen::changeWorkspace(const int num) const {
  assert(_managed);

  _xatom->sendClientMessage(_root, XAtom::net_current_desktop, _root, num);
}
