// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "eventdispatcher.hh"
#include "display.hh"
#include <iostream>

namespace otk {

OtkEventDispatcher::OtkEventDispatcher()
  : _fallback(0), _master(0)
{
}

OtkEventDispatcher::~OtkEventDispatcher()
{
}

void OtkEventDispatcher::clearAllHandlers(void)
{
  _map.clear();
}

void OtkEventDispatcher::registerHandler(Window id, OtkEventHandler *handler)
{
  _map.insert(std::pair<Window, OtkEventHandler*>(id, handler));
}

void OtkEventDispatcher::clearHandler(Window id)
{
  _map.erase(id);
}

void OtkEventDispatcher::dispatchEvents(void)
{
  XEvent e;
  OtkEventHandler *handler;
  OtkEventMap::iterator it;

  while (XPending(OBDisplay::display)) {
    XNextEvent(OBDisplay::display, &e);

#if 0
    printf("Event %d window %lx\n", e.type, e.xany.window);
#endif

    // these ConfigureRequests require some special attention
    if (e.type == ConfigureRequest) {
      // find the actual window! e.xany.window is the parent window
      it = _map.find(e.xconfigurerequest.window);

      if (it != _map.end())
        it->second->handle(e);
      else {
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

        XConfigureWindow(otk::OBDisplay::display, e.xconfigurerequest.window,
                         e.xconfigurerequest.value_mask, &xwc);
      }
    } else {
      // normal events
      
      it = _map.find(e.xany.window);

      if (it != _map.end())
        handler = it->second;
      else
        handler = _fallback;

      if (handler)
        handler->handle(e);
    }

    if (_master)
      _master->handle(e);
  }
}

OtkEventHandler *OtkEventDispatcher::findHandler(Window win)
{
  OtkEventMap::iterator it = _map.find(win);
  if (it != _map.end())
    return it->second;
  return 0;
}

}
