// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "eventdispatcher.hh"
#include "display.hh"
#include <iostream>

namespace otk {

EventDispatcher::EventDispatcher()
  : _fallback(0), _master(0)
{
}

EventDispatcher::~EventDispatcher()
{
}

void EventDispatcher::clearAllHandlers(void)
{
  _map.clear();
}

void EventDispatcher::registerHandler(Window id, EventHandler *handler)
{
  _map.insert(std::pair<Window, EventHandler*>(id, handler));
}

void EventDispatcher::clearHandler(Window id)
{
  _map.erase(id);
}

void EventDispatcher::dispatchEvents(void)
{
  XEvent e;

  while (XPending(**display)) {
    XNextEvent(**display, &e);

#if 0//defined(DEBUG)
    printf("Event %d window %lx\n", e.type, e.xany.window);
#endif

    if (e.type == FocusIn || e.type == FocusOut) {
      // focus events are a beast all their own.. yuk, hate, etc.
      dispatchFocus(e);
    } else {    
      Window win;

      // pick a window
      switch (e.type) {
      case UnmapNotify:
        win = e.xunmap.window;
        break;
      case DestroyNotify:
        win = e.xdestroywindow.window;
        break;
      case ConfigureRequest:
        win = e.xconfigurerequest.window;
        break;
      default:
        win = e.xany.window;
      }
    
      // grab the lasttime and hack up the modifiers
      switch (e.type) {
      case ButtonPress:
      case ButtonRelease:
        _lasttime = e.xbutton.time;
        e.xbutton.state &= ~(LockMask | display->numLockMask() |
                             display->scrollLockMask());
        break;
      case KeyPress:
        e.xkey.state &= ~(LockMask | display->numLockMask() |
                          display->scrollLockMask());
        break;
      case MotionNotify:
        _lasttime = e.xmotion.time;
        e.xmotion.state &= ~(LockMask | display->numLockMask() |
                             display->scrollLockMask());
        break;
      case PropertyNotify:
        _lasttime = e.xproperty.time;
        break;
      case EnterNotify:
      case LeaveNotify:
        _lasttime = e.xcrossing.time;
        if (e.xcrossing.mode != NotifyNormal)
          continue; // skip me!
        break;
      }

      dispatch(win, e);
    }
  }
}

void EventDispatcher::dispatchFocus(const XEvent &e)
{
  printf("focus %s detail %d -> 0x%lx\n",
         (e.xfocus.type == FocusIn ? "IN" : "OUT"),
         e.xfocus.detail, e.xfocus.window);
  // ignore focus changes from grabs
  if (e.xfocus.mode != NotifyNormal) //|| e.xfocus.mode == NotifyUngrab ||
      // From Metacity, from WindowMaker, ignore all funky pointer root events
      // its commented out cuz I don't think we need this at all. If problems
      // arise we can look into it
      //e.xfocus.detail > NotifyNonlinearVirtual)
    return;
  
  if (e.type == FocusIn) {
    //printf("Got FocusIn!\n");

    // send a FocusIn to whatever was just focused
    dispatch(e.xfocus.window, e);
    //printf("Sent FocusIn 0x%lx\n", e.xfocus.window);

  } else if (e.type == FocusOut) {
    //printf("Got FocusOut!\n");

    // FocusOut events just make us look for FocusIn events. They are ignored
    // otherwise.
    XEvent fi;
    if (XCheckTypedEvent(**display, FocusIn, &fi)) {
      //printf("Found FocusIn\n");
      dispatchFocus(fi);
      // dont unfocus the window we just focused!
      if (fi.xfocus.window == e.xfocus.window)
        return;
    }

    dispatch(e.xfocus.window, e);
    //printf("Sent FocusOut 0x%lx\n", e.xfocus.window);
  }
}

void EventDispatcher::dispatch(Window win, const XEvent &e)
{
  EventHandler *handler = 0;
  EventMap::iterator it;

  // master gets everything first
  if (_master)
    _master->handle(e);

  // find handler for the chosen window
  it = _map.find(win);

  if (it != _map.end()) {
    // if we found a handler
    handler = it->second;
  } else if (e.type == ConfigureRequest) {
    // unhandled configure requests must be used to configure the window
    // directly
    XWindowChanges xwc;
      
    xwc.x = e.xconfigurerequest.x;
    xwc.y = e.xconfigurerequest.y;
    xwc.width = e.xconfigurerequest.width;
    xwc.height = e.xconfigurerequest.height;
    xwc.border_width = e.xconfigurerequest.border_width;
    xwc.sibling = e.xconfigurerequest.above;
    xwc.stack_mode = e.xconfigurerequest.detail;
      
    XConfigureWindow(**display, e.xconfigurerequest.window,
                     e.xconfigurerequest.value_mask, &xwc);
  } else {
    // grab a falback if it exists
    handler = _fallback;
  }

  if (handler)
    handler->handle(e);
}

EventHandler *EventDispatcher::findHandler(Window win)
{
  EventMap::iterator it = _map.find(win);
  if (it != _map.end())
    return it->second;
  return 0;
}

}
