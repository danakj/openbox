// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
// screen.cc for Epistrophy - a key handler for NETWM/EWMH window managers.
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
#include "config.hh"

screen::screen(epist *epist, int number) 
  : _clients(epist->clientsList()),
    _active(epist->activeWindow()) {
  _epist = epist;
  _xatom = _epist->xatom();
  _last_active = _clients.end();
  _number = number;
  _info = _epist->getScreenInfo(_number);
  _root = _info->getRootWindow();

  // find a window manager supporting NETWM, waiting for it to load if we must
  int count = 20;  // try for 20 seconds
  _managed = false;
  while (! (_epist->doShutdown() || _managed || count <= 0)) {
    if (! (_managed = findSupportingWM()))
      sleep(1);
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
  int scrolllockMask, numlockMask;
  _epist->getLockModifiers(numlockMask, scrolllockMask);
  
  // Mask out the lock modifiers. We want our keys to always work
  // This should be made an option
  unsigned int state = e.xkey.state & ~(LockMask|scrolllockMask|numlockMask);
  const Action *it = _epist->getKeyTree().getAction(e, state, this);
  
  if (!it)
    return;

  switch (it->type()) {
  case Action::nextScreen:
    _epist->cycleScreen(_number, true);
    return;

  case Action::prevScreen:
    _epist->cycleScreen(_number, false);
    return;

  case Action::nextWorkspace:
    cycleWorkspace(true, it->number() != 0 ? it->number(): 1);
    return;

  case Action::prevWorkspace:
    cycleWorkspace(false, it->number() != 0 ? it->number(): 1);
    return;

  case Action::nextWindow:
    
    cycleWindow(true, it->number() != 0 ? it->number(): 1);
    return;

  case Action::prevWindow:
    cycleWindow(false, it->number() != 0 ? it->number(): 1);
    return;

  case Action::nextWindowOnAllWorkspaces:
    cycleWindow(true, it->number() != 0 ? it->number(): 1,  false, true);
    return;

  case Action::prevWindowOnAllWorkspaces:
    cycleWindow(false, it->number() != 0 ? it->number(): 1, false, true);
    return;

  case Action::nextWindowOnAllScreens:
    cycleWindow(true, it->number() != 0 ? it->number(): 1, true);
    return;

  case Action::prevWindowOnAllScreens:
    cycleWindow(false, it->number() != 0 ? it->number(): 1, true);
    return;

  case Action::nextWindowOfClass:
    cycleWindow(true, it->number() != 0 ? it->number(): 1,
                false, false, true, it->string());
    return;

  case Action::prevWindowOfClass:
    cycleWindow(false, it->number() != 0 ? it->number(): 1,
                false, false, true, it->string());
    return;
      
  case Action::nextWindowOfClassOnAllWorkspaces:
    cycleWindow(true, it->number() != 0 ? it->number(): 1,
                false, true, true, it->string());
    return;
      
  case Action::prevWindowOfClassOnAllWorkspaces:
    cycleWindow(false, it->number() != 0 ? it->number(): 1,
                false, true, true, it->string());
    return;

  case Action::changeWorkspace:
    changeWorkspace(it->number());
    return;

  case Action::upWorkspace:
    changeWorkspaceVert(-1);
    return;

  case Action::downWorkspace:
    changeWorkspaceVert(1);
    return;

  case Action::leftWorkspace:
    changeWorkspaceHorz(-1);
    return;

  case Action::rightWorkspace:
    changeWorkspaceHorz(1);
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
      window->move(window->x(), window->y() - it->number());
      return;
      
    case Action::moveWindowDown:
      window->move(window->x(), window->y() + it->number());
      return;
      
    case Action::moveWindowLeft:
      window->move(window->x() - it->number(), window->y());
      return;
      
    case Action::moveWindowRight:
      window->move(window->x() + it->number(), window->y());
      return;
      
    case Action::resizeWindowWidth:
      window->resizeRel(it->number(), 0);
      return;
      
    case Action::resizeWindowHeight:
      window->resizeRel(0, it->number());
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


void screen::updateEverything() {
  updateNumDesktops();
  updateActiveDesktop();
  updateClientList();
  updateActiveWindow();
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
                         &rootclients))
    num = 0;

  WindowList::iterator it;
  const WindowList::iterator end = _clients.end();
  unsigned long i;
  
  // insert new clients after the active window
  for (i = 0; i < num; ++i) {
    for (it = _clients.begin(); it != end; ++it)
      if (**it == rootclients[i])
        break;
    if (it == end) {  // didn't already exist
      if (doAddWindow(rootclients[i])) {
        //        cout << "Added window: 0x" << hex << rootclients[i] << dec << endl;
        _clients.insert(insert_point, new XWindow(_epist, this,
                                                  rootclients[i]));
      }
    }
  }

  // remove clients that no longer exist (that belong to this screen)
  for (it = _clients.begin(); it != end;) {
    WindowList::iterator it2 = it;
    ++it;

    // is on another screen?
    if ((*it2)->getScreen() != this)
      continue;

    for (i = 0; i < num; ++i)
      if (**it2 == rootclients[i])
        break;
    if (i == num)  { // no longer exists
      //      cout << "Removed window: 0x" << hex << (*it2)->window() << dec << endl;
      // watch for the active and last-active window
      if (it2 == _active)
        _active = _clients.end();
      if (it2 == _last_active)
        _last_active = _clients.end();
      delete *it2;
      _clients.erase(it2);
    }
  }

  if (rootclients) delete [] rootclients;
}


const XWindow *screen::lastActiveWindow() const {
  if (_last_active != _clients.end())
    return *_last_active;

  // find a window if one exists
  WindowList::const_iterator it, end = _clients.end();
  for (it = _clients.begin(); it != end; ++it)
    if ((*it)->getScreen() == this && ! (*it)->iconic() &&
        ((*it)->desktop() == 0xffffffff || (*it)->desktop() == _active_desktop))
      return *it;

  // no windows on this screen
  return 0;
}


void screen::updateActiveWindow() {
  assert(_managed);

  Window a = None;
  _xatom->getValue(_root, XAtom::net_active_window, XAtom::window, a);
  
  WindowList::iterator it, end = _clients.end();
  for (it = _clients.begin(); it != end; ++it) {
    if (**it == a) {
      if ((*it)->getScreen() != this)
        return;
      break;
    }
  }
  _active = it;
  if (it != end)
    _last_active = it;

  /*  cout << "Active window is now: ";
      if (_active == _clients.end()) cout << "None\n";
      else cout << "0x" << hex << (*_active)->window() << dec << endl;
  */
}


void screen::execCommand(const string &cmd) const {
  pid_t pid;
  if ((pid = fork()) == 0) {
    // make the command run on the correct screen
    if (putenv(const_cast<char*>(_info->displayString().c_str()))) {
      cout << "warning: couldn't set environment variable 'DISPLAY'\n";
      perror("putenv()");
    }
    execl("/bin/sh", "sh", "-c", cmd.c_str(), NULL);
    exit(-1);
  } else if (pid == -1) {
    cout << _epist->getApplicationName() <<
      ": Could not fork a process for executing a command\n";
  }
}


void screen::cycleWindow(const bool forward, const int increment,
                         const bool allscreens, const bool alldesktops,
                         const bool sameclass, const string &cn) const {
  assert(_managed);
  assert(increment > 0);

  if (_clients.empty()) return;

  string classname(cn);
  if (sameclass && classname.empty() && _active != _clients.end())
    classname = (*_active)->appClass();

  WindowList::const_iterator target = _active,
    begin = _clients.begin(),
    end = _clients.end();

  const XWindow *t = 0;
  
  for (int x = 0; x < increment; ++x) {
    while (1) {
      if (forward) {
        if (target == end) {
          target = begin;
        } else {
          ++target;
        }
      } else {
        if (target == begin)
          target = end;
        --target;
      }

      // must be no window to focus
      if (target == _active)
        return;

      // start back at the beginning of the loop
      if (target == end)
        continue;

      // determine if this window is invalid for cycling to
      t = *target;
      if (t->iconic()) continue;
      if (! allscreens && t->getScreen() != this) continue;
      if (! alldesktops && ! (t->desktop() == _active_desktop ||
                              t->desktop() == 0xffffffff)) continue;
      if (sameclass && ! classname.empty() &&
          t->appClass() != classname) continue;
      if (! t->canFocus()) continue;

      // found a good window so break out of the while, and perhaps continue
      // with the for loop
      break;
    }
  }

  // phew. we found the window, so focus it.
  t->focus();
}


void screen::cycleWorkspace(const bool forward, const int increment,
                            const bool loop) const {
  assert(_managed);
  assert(increment > 0);

  unsigned int destination = _active_desktop;

  for (int x = 0; x < increment; ++x) {
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
  }

  if (destination != _active_desktop) 
    changeWorkspace(destination);
}


void screen::changeWorkspace(const int num) const {
  assert(_managed);

  _xatom->sendClientMessage(_root, XAtom::net_current_desktop, _root, num);
}

void screen::changeWorkspaceVert(const int num) const {
  assert(_managed);
  const Config *conf = _epist->getConfig();
  int width = conf->getNumberValue(Config::workspaceColumns);

  if (width > _num_desktops || width <= 0)
    return;

  int wnum;
  
  // a cookie to the person that makes this pretty
  if (num < 0) {
    wnum = _active_desktop - width;
    if (wnum < 0) {
      wnum = _num_desktops/width * width + _active_desktop;
      if (wnum >= _num_desktops)
        wnum = _num_desktops - 1;
    }
  }
  else {
    wnum = _active_desktop + width;
    if (wnum >= _num_desktops) {
      wnum = (_active_desktop + width) % _num_desktops - 1;
      if (wnum < 0)
        wnum = 0;
    }
  }
  changeWorkspace(wnum);
}

void screen::changeWorkspaceHorz(const int num) const {
  assert(_managed);
  const Config *conf = _epist->getConfig();
  int width = conf->getNumberValue(Config::workspaceColumns);
  int wnum;
  
  if (width > _num_desktops || width <= 0)
    return;

  if (num < 0) {
    if (_active_desktop % width != 0)
      changeWorkspace(_active_desktop - 1);
    else {
      wnum = _active_desktop + width - 1;
      if (wnum >= _num_desktops)
        wnum = _num_desktops - 1;
    }
  }
  else {
    if (_active_desktop % width != width - 1) {
      wnum = _active_desktop + 1;
      if (wnum >= _num_desktops)
        wnum = _num_desktops / width * width;
    }
    else
      wnum = _active_desktop - width + 1;
  }
  changeWorkspace(wnum);
}

void screen::grabKey(const KeyCode keyCode, const int modifierMask) const {

  Display *display = _epist->getXDisplay();
  int numlockMask, scrolllockMask;

  _epist->getLockModifiers(numlockMask, scrolllockMask);

  XGrabKey(display, keyCode, modifierMask,
           _root, True, GrabModeAsync, GrabModeAsync);
  XGrabKey(display, keyCode, 
           modifierMask|LockMask,
           _root, True, GrabModeAsync, GrabModeAsync);
  XGrabKey(display, keyCode, 
           modifierMask|scrolllockMask,
           _root, True, GrabModeAsync, GrabModeAsync);
  XGrabKey(display, keyCode, 
           modifierMask|numlockMask,
           _root, True, GrabModeAsync, GrabModeAsync);
    
  XGrabKey(display, keyCode, 
           modifierMask|LockMask|scrolllockMask,
           _root, True, GrabModeAsync, GrabModeAsync);
  XGrabKey(display, keyCode, 
           modifierMask|scrolllockMask|numlockMask,
           _root, True, GrabModeAsync, GrabModeAsync);
  XGrabKey(display, keyCode, 
           modifierMask|numlockMask|LockMask,
           _root, True, GrabModeAsync, GrabModeAsync);
    
  XGrabKey(display, keyCode, 
           modifierMask|numlockMask|LockMask|scrolllockMask,
           _root, True, GrabModeAsync, GrabModeAsync);
}

void screen::ungrabKey(const KeyCode keyCode, const int modifierMask) const {

  Display *display = _epist->getXDisplay();
  int numlockMask, scrolllockMask;

  _epist->getLockModifiers(numlockMask, scrolllockMask);

  XUngrabKey(display, keyCode, modifierMask, _root);
  XUngrabKey(display, keyCode, modifierMask|LockMask, _root);
  XUngrabKey(display, keyCode, modifierMask|scrolllockMask, _root);
  XUngrabKey(display, keyCode, modifierMask|numlockMask, _root);
  XUngrabKey(display, keyCode, modifierMask|LockMask|scrolllockMask, _root);
  XUngrabKey(display, keyCode, modifierMask|scrolllockMask|numlockMask, _root);
  XUngrabKey(display, keyCode, modifierMask|numlockMask|LockMask, _root);
  XUngrabKey(display, keyCode, modifierMask|numlockMask|LockMask|
             scrolllockMask, _root);
}
