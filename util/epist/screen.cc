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

#include "../../src/XAtom.hh"
#include "screen.hh"
#include "epist.hh"


screen::screen(epist *epist, int number) {
  _epist = epist;
  _xatom = _epist->xatom();
  _number = number;
  _active = _clients.end();
  _root = RootWindow(_epist->getXDisplay(), _number);
  
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
  list<Action>::const_iterator it = _epist->actions().begin();
  list<Action>::const_iterator end = _epist->actions().end();
  for (; it != end; ++it) {
    if (e.xkey.keycode == it->keycode() &&
        e.xkey.state == it->modifierMask() )
      {
        switch (it->type()) {
        case Action::nextDesktop:
          cycleWorkspace(true);
          break;
        case Action::prevDesktop:
          cycleWorkspace(false);
          break;
        }
        break;
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
        _clients.insert(insert_point, new XWindow(_epist, rootclients[i]));
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

/*
 * use this when execing a command to have it on the right screen
      string dtmp = (string)"DISPLAY=" + display_name;
      if (putenv(const_cast<char*>(dtmp.c_str()))) {
        cout << "warning: couldn't set environment variable 'DISPLAY'\n";
        perror("putenv()");
      }
 */

void screen::cycleWorkspace(const bool forward) {
  cout << "blef" << endl;

  unsigned long currentDesktop = 0;
  unsigned long numDesktops = 0;
  
  if (_xatom->getValue(_root, XAtom::net_current_desktop, XAtom::cardinal,
                       currentDesktop)) {
    if (forward)     
      ++currentDesktop;
    else
      --currentDesktop;

    cout << currentDesktop << endl;

    
    _xatom->getValue(_root, XAtom::net_number_of_desktops, XAtom::cardinal,
                     numDesktops);
    
    if ( ( (signed)currentDesktop) == -1)
      currentDesktop = numDesktops - 1;
    else if (currentDesktop >= numDesktops)
      currentDesktop = 0;

    
    _xatom->sendClientMessage(_root, XAtom::net_current_desktop, _root,
                              currentDesktop);
    
  }
}
                                               
