// -*- mode: C++; indent-tabs-mode: nil; -*-
// process.cc for Epistory - a key handler for NETWM/EWMH window managers.
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

#include "process.hh"
#include "epist.hh"
#include "window.hh"

#ifdef    HAVE_CONFIG_H
#  include "../../config.h"
#endif // HAVE_CONFIG_H

#include <iostream>

using std::cout;
using std::endl;
using std::hex;
using std::dec;

#include "../../src/XAtom.hh"

WindowList _clients;
WindowList::iterator _active = _clients.end();


XWindow &findWindow(const XEvent &e) {
  WindowList::iterator it, end = _clients.end();
  for (it = _clients.begin(); it != end; ++it)
    if (**it == e.xany.window)
      break;
  assert(it != end);  // this means a client somehow got removed from the
                      // list!
  return **it;
}


void processEvent(const XEvent &e) {
  switch (e.type) {
  case PropertyNotify:
    if (e.xany.window == _root) {
      // root window
      if (e.xproperty.atom == _xatom->getAtom(XAtom::net_active_window))
        updateActiveWindow();
      if (e.xproperty.atom == _xatom->getAtom(XAtom::net_client_list)) {
        // catch any window unmaps first
        XEvent ev;
        if (XCheckTypedWindowEvent(_display, e.xany.window,
                                   DestroyNotify, &ev) ||
            XCheckTypedWindowEvent(_display, e.xany.window,
                                   UnmapNotify, &ev)) {
          processEvent(ev);
        }
        
        updateClientList();
      }
    } else {
      // a client window
      if (e.xproperty.atom == _xatom->getAtom(XAtom::net_wm_state))
        findWindow(e).updateState();
      if (e.xproperty.atom == _xatom->getAtom(XAtom::net_wm_desktop))
        findWindow(e).updateDesktop();
    }
    break;
  case DestroyNotify:
  case UnmapNotify:
    cout << "unmap notify\n";
    findWindow(e).setUnmapped(true);
    break;
  }
}


// do we want to add this window to our list?
bool doAddWindow(Window window) {
  Atom type;
  if (! _xatom->getValue(window, XAtom::net_wm_window_type, XAtom::atom,
                         type))
    return True;

  if (type == _xatom->getAtom(XAtom::net_wm_window_type_dock) ||
      type == _xatom->getAtom(XAtom::net_wm_window_type_menu))
    return False;

  return True;
}


void updateClientList() {
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
        _clients.insert(insert_point, new XWindow(rootclients[i]));
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


void updateActiveWindow() {
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
