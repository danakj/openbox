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

void OtkEventDispatcher::dispatchEvents(void)
{
  XEvent e;
  OtkEventHandler *handler;
  OtkEventMap::iterator it;

  while (XPending(OBDisplay::display)) {
    XNextEvent(OBDisplay::display, &e);
    it = _map.find(e.xany.window);

    if (it == _map.end())
      handler = _fallback;
    else
      handler = it->second;

    if (handler)
      handler->handle(e);
  }
}

}
