// -*- mode: C++; indent-tabs-mode: nil; -*-
// Input.cc for Blackbox - an X11 Window manager
// Copyright (c) 2001 - 2002 Sean 'Shaleh' Perry <shaleh@debian.org>
// Copyright (c) 1997 - 2000 Brad Hughes (bhughes@tcac.net)
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
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "Input.hh"
#include "blackbox.hh"
#include "Window.hh"

BInput::BInput(Blackbox *b) {
  _blackbox = b;
  _display = b->getXDisplay();

  // hardcode blackbox's oldschool mouse bindings

  // buttons
  add(Button1, 0, IconifyButtonClick, Iconify);
  add(Button1, 0, CloseButtonClick, Close);

  add(Button1, 0, MaximizeButtonClick, ToggleMaximize);
  add(Button1, 0, MaximizeButtonClick, Raise);

  add(Button2, 0, MaximizeButtonClick, ToggleMaximizeVert);
  add(Button2, 0, MaximizeButtonClick, Raise);

  add(Button3, 0, MaximizeButtonClick, ToggleMaximizeHoriz);
  add(Button3, 0, MaximizeButtonClick, Raise);
 
  // title-bar
  
  add(Button1, ControlMask, WindowTitlePress, ToggleShade);
  add(Button2, 0, WindowTitlePress, Lower);
  add(Button1, 0, WindowTitleDoublePress, ToggleShade);

  // mouse wheel
  add(Button4, 0, WindowTitlePress, Shade);
  add(Button5, 0, WindowTitlePress, Unshade);

  // drag moving
  add(Button1, 0, WindowHandleDrag, BeginMove);
  add(Button1, 0, WindowTitleDrag, BeginMove);
  add(Button1, Mod1Mask, WindowDrag, BeginMove);

  // drag resizing
  add(Button3, Mod1Mask, WindowDrag, BeginResizeRelative);
  add(Button1, 0, WindowLeftGripDrag, BeginResizeLL);
  add(Button1, 0, WindowRightGripDrag, BeginResizeLR);

  // window menu
  add(Button3, 0, WindowTitlePress, ShowWindowMenu);
  add(Button3, 0, WindowFramePress, ShowWindowMenu);

  // focus/raising
  add(Button1, AnyModifier, WindowTitlePress, Raise);
  add(Button1, AnyModifier, WindowTitlePress, Focus);
  add(Button1, AnyModifier, WindowFramePress, Raise);
  add(Button1, AnyModifier, WindowFramePress, Focus);
}


BInput::~BInput() {
}


void BInput::add(unsigned int button, unsigned int state, MouseEvent event,
                 Action action) {
  _mousebind.push_back(MouseBinding(button, state, event, action));
}
  

void BInput::add(unsigned int button, unsigned int state, Action action) {
  _keybind.push_back(KeyBinding(button, state, action));
}
  

void BInput::remove(unsigned int button, unsigned int state, MouseEvent event,
                    Action action) {
  MouseBindingList::iterator it = _mousebind.begin();
  const MouseBindingList::iterator end = _mousebind.end();
  while (it != end) {
    if (it->button == button && it->state == state && it->action == action &&
        it->event == event) {
      MouseBindingList::iterator tmp = it;
      ++it;
      _mousebind.erase(tmp);
    } else {
      ++it;
    }
  }
}
  

void BInput::remove(unsigned int button, unsigned int state, Action action) {
  KeyBindingList::iterator it = _keybind.begin();
  const KeyBindingList::iterator end = _keybind.end();
  while (it != end) {
    if (it->button == button && it->state == state && it->action == action) {
      ++it;
      _keybind.erase(it);
    } else {
      ++it;
    }
  }
}
  

// execute a keyboard binding
bool BInput::doAction(BlackboxWindow *window, unsigned int keycode,
                      unsigned int state) const {
  bool ret = False;

  KeyBindingList::const_iterator it = _keybind.begin();
  const KeyBindingList::const_iterator end = _keybind.end();
  for (; it != end; ++it)
    if ((it->state == state || it->state == AnyModifier) &&
        it->button == keycode && it->action != NoAction) {
      doAction(window, it->action);
      ret = True;
    }
  return ret;
}


// determine if a keyboard binding exists
bool BInput::hasAction(unsigned int keycode, unsigned int state) const {
  KeyBindingList::const_iterator it = _keybind.begin();
  const KeyBindingList::const_iterator end = _keybind.end();
  for (; it != end; ++it)
    if ((it->state == state || it->state == AnyModifier) &&
        it->button == keycode && it->action != NoAction)
      return True;
  return False;
}


// execute a mouse binding
bool BInput::doAction(BlackboxWindow *window, unsigned int button,
                      unsigned int state, MouseEvent eventtype) const {
  bool ret = False;

  assert(button == Button1 || button == Button2 || button == Button3 ||
         button == Button4 || button == Button5);
  assert(eventtype >= 0 && eventtype < NUM_MOUSEEVENTS);

  MouseBindingList::const_iterator it = _mousebind.begin();
  const MouseBindingList::const_iterator end = _mousebind.end();
  for (; it != end; ++it)
    if ((it->state == state || it->state == AnyModifier) &&
        it->button == button && it->event == eventtype &&
        it->action != NoAction) {
      doAction(window, it->action);
      ret = True;
    }
  return ret;
}


// determine if a mouse binding exists
bool BInput::hasAction(unsigned int button, unsigned int state,
                       MouseEvent eventtype) const {
  assert(button == Button1 || button == Button2 || button == Button3 ||
         button == Button4 || button == Button5);
  assert(eventtype >= 0 && eventtype < NUM_MOUSEEVENTS);

  MouseBindingList::const_iterator it = _mousebind.begin();
  const MouseBindingList::const_iterator end = _mousebind.end();
  for (; it != end; ++it)
    if ((it->state == state || it->state == AnyModifier) &&
        it->button == button && it->event == eventtype &&
        it->action != NoAction)
      return True;
  return False;
}


void BInput::doAction(BlackboxWindow *window, Action action) const {
  switch (action) {
  case Raise:
    if (window) window->raise();
    return;

  case Lower:
    if (window) window->lower();
    return;

  case Stick:
    if (window && ! window->isStuck()) window->stick();
    return;

  case Unstick:
    if (window && window->isStuck()) window->stick();
    return;

  case ToggleStick:
    if (window) window->stick();
    return;

  case Shade:
    if (window && ! window->isShaded()) window->shade();
    return;

  case Unshade:
    if (window && window->isShaded()) window->shade();
    return;

  case ToggleShade:
    if (window) window->shade();
    return;

  case Focus:
    if (window && ! window->isFocused()) window->setInputFocus();
    return;

  case Iconify:
    if (window && ! window->isIconic()) window->iconify();
    return;

  case ToggleMaximizeVert:
    if (window) window->maximize(2);
    return;

  case ToggleMaximizeHoriz:
    if (window) window->maximize(3);
    return;

  case ToggleMaximize:
    if (window) window->maximize(1);
    return;

  case Close:
    if (window && window->isClosable()) window->close();
    return;

  case NextWorkspace: {
    BScreen *s;
    unsigned int w;
    s = _blackbox->getFocusedScreen();
    if (s) {
      w = s->getCurrentWorkspaceID();
      if (++w >= s->getWorkspaceCount())
        w = 0;
      s->changeWorkspaceID(w);
    }
    return;
  }

  case PrevWorkspace: {
    BScreen *s;
    int w;
    s = _blackbox->getFocusedScreen();
    if (s) {
      w = s->getCurrentWorkspaceID();
      if (w-- == 0)
        w = s->getWorkspaceCount() - 1;
      s->changeWorkspaceID(w);
    }
    return;
  }

  case BeginMove:
    if (window && window->isMovable()) {
      Window root_return, child_return;
      int root_x_return, root_y_return;
      int win_x_return, win_y_return;
      unsigned int mask_return;

      if (! XQueryPointer(_display, window->getClientWindow(),
                          &root_return, &child_return,
                          &root_x_return, &root_y_return,
                          &win_x_return, &win_y_return,
                          &mask_return))
        return;
      window->beginMove(root_x_return, root_y_return);
    }
    return;

  case BeginResizeUL:
  case BeginResizeUR:
  case BeginResizeLL:
  case BeginResizeLR:
  case BeginResizeRelative:
    if (window && window->isResizable()) {
      Window root_return, child_return;
      int root_x_return, root_y_return;
      int win_x_return, win_y_return;
      unsigned int mask_return;

      if (! XQueryPointer(_display, window->getClientWindow(),
                          &root_return, &child_return,
                          &root_x_return, &root_y_return,
                          &win_x_return, &win_y_return,
                          &mask_return))
        return;

      BlackboxWindow::Corner corner;
      switch (action) {
      case BeginResizeUL: corner = BlackboxWindow::TopLeft; break;
      case BeginResizeUR: corner = BlackboxWindow::TopRight; break;
      case BeginResizeLL: corner = BlackboxWindow::BottomLeft; break;
      case BeginResizeLR: corner = BlackboxWindow::BottomRight; break;
      case BeginResizeRelative: {
        const Rect &r = window->frameRect();
        if (! r.contains(root_x_return, root_y_return))
          return;

        bool left = root_x_return < r.x() + (signed)(r.width() / 2);
        bool top = root_y_return < r.y() + (signed)(r.height() / 2);
        if (left) {
          if (top) corner = BlackboxWindow::TopLeft;
          else corner = BlackboxWindow::BottomLeft;
        } else {
          if (top) corner = BlackboxWindow::TopRight;
          else corner = BlackboxWindow::BottomRight;
        }
        break;
      }
      default: assert(false); // unhandled action
      }
      window->beginResize(root_x_return, root_y_return, corner);
    }
    return;

  case ShowWindowMenu:
    if (window) {
      Window root_return, child_return;
      int root_x_return, root_y_return;
      int win_x_return, win_y_return;
      unsigned int mask_return;

      if (! XQueryPointer(_display, window->getClientWindow(),
                          &root_return, &child_return,
                          &root_x_return, &root_y_return,
                          &win_x_return, &win_y_return,
                          &mask_return))
        return;
      window->showWindowMenu(root_x_return, root_y_return);
    }
    return;

  default:
    assert(false);  // unhandled Action
  }
}
