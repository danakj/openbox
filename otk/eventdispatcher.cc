// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef HAVE_CONFIG_H
# include "../config.h"
#endif

#include "eventdispatcher.hh"
#include "display.hh"
#include <iostream>

namespace otk {

OtkEventDispatcher::OtkEventDispatcher()
  : _fallback(0)
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
#include <stdio.h>
void OtkEventDispatcher::dispatchEvents(void)
{
  XEvent e;
  OtkEventHandler *handler;
  OtkEventMap::iterator it;

  while (XPending(OBDisplay::display)) {
    XNextEvent(OBDisplay::display, &e);

    it = _map.find(e.xany.window);

    if (it != _map.end())
      handler = it->second;
    else
      handler = _fallback;

    if (handler)
      handler->handle(e);
    else {
      // some events have to be handled anyways!
      if (e.type == ConfigureRequest) {
        XWindowChanges xwc;

        xwc.x = e.xconfigurerequest.x;
        xwc.y = e.xconfigurerequest.y;
        xwc.width = e.xconfigurerequest.width;
        xwc.height = e.xconfigurerequest.height;
        xwc.border_width = e.xconfigurerequest.border_width;
        xwc.sibling = e.xconfigurerequest.above;
        xwc.stack_mode = e.xconfigurerequest.detail;

        XConfigureWindow(OBDisplay::display, e.xconfigurerequest.window,
                         e.xconfigurerequest.value_mask, &xwc);
      }
    }
  }
}

}
