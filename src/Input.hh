// -*- mode: C++; indent-tabs-mode: nil; -*-
// Input.hh for Blackbox - an X11 Window manager
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

#ifndef   __Input_hh
#define   __Input_hh

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}

#include <list>

class Blackbox;
class BlackboxWindow;

class BInput {
public:
  enum MouseEvent {
    InvalidEvent = -1,

    IconifyButtonClick,
    MaximizeButtonClick,
    CloseButtonClick,
    
    WindowFramePress,
    WindowTitlePress,
    WindowTitleDoublePress,
    WindowClientPress,
    RootWindowPress,

    WindowDrag,
    WindowTitleDrag,
    WindowHandleDrag,
    WindowLeftGripDrag,
    WindowRightGripDrag,
    
    NUM_MOUSEEVENTS
  };

  enum Action {
    NoAction = 0,
    Raise,
    Lower,
    Shade,
    Unshade,
    Focus,
    Iconify,
    ToggleMaximizeVert,
    ToggleMaximizeHoriz,
    ToggleMaximize,
    ToggleShade,
    Close,
    BeginMove,
    BeginResizeUL,
    BeginResizeUR,
    BeginResizeLL,
    BeginResizeLR,
    BeginResizeRelative,  // picks a corner based on the mouse cursor's position
    ShowWindowMenu,
    NUM_ACTIONS
  };

  struct KeyBinding {
    unsigned int button;
    unsigned int state;

    // for keyboard events, this is applied to the focused window.
    // for mouse events, this is applied to the window that was clicked on.
    Action action;

    KeyBinding(unsigned int button, unsigned int state, Action action) {
      assert(button > 0);
      assert(action >= 0 && action < NUM_ACTIONS);
      this->button = button;
      this->state = state;
      this->action = action;
    }
  };

  struct MouseBinding : public KeyBinding {
    MouseEvent event;

    MouseBinding(unsigned int button, unsigned int state, MouseEvent event,
                 Action action) : KeyBinding(button, state, action) {
      assert(event >= 0 && event < NUM_MOUSEEVENTS);
      this->event = event;
    }
  };

  typedef std::list<MouseBinding> MouseBindingList;
  typedef std::list<KeyBinding> KeyBindingList;

private:
  Blackbox         *_blackbox;
  Display          *_display;

  MouseBindingList  _mousebind; 
  KeyBindingList    _keybind;

  void doAction(BlackboxWindow *window, Action action) const;

public:
  BInput(Blackbox *b);
  virtual ~BInput();

  void add(unsigned int button, unsigned int state, MouseEvent event,
           Action action);
  void remove(unsigned int button, unsigned int state, MouseEvent event,
              Action action);
  void add(unsigned int button, unsigned int state, Action action);
  void remove(unsigned int button, unsigned int state, Action action);
  
  // execute a keyboard binding
  // returns false if the specified binding doesnt exist
  bool doAction(BlackboxWindow *window, unsigned int keycode,
                unsigned int state) const;
    // execute a mouse binding
  // returns false if the specified binding doesnt exist
  bool doAction(BlackboxWindow *window, unsigned int keycode,
                unsigned int state, MouseEvent eventtype) const;

  // determine if a keyboard binding exists
  bool hasAction(unsigned int keycode, unsigned int state) const;
  // determine if a mouse binding exists
  bool hasAction(unsigned int button, unsigned int state,
                 MouseEvent eventtype) const;

  const MouseBindingList &getMouseBindings(void) { return _mousebind; }
  const KeyBindingList &getKeyBindings(void) { return _keybind; }
};

#endif // __Input_hh
