// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __eventdispatcher
#define __eventdispatcher

#include "eventhandler.hh"
#include <map>
#include <utility>

namespace otk {

typedef std::map<unsigned int, EventHandler *> EventMap;

class EventDispatcher {
public:

  EventDispatcher();
  virtual ~EventDispatcher();

  virtual void clearAllHandlers(void);
  virtual void registerHandler(Window id, EventHandler *handler);
  virtual void clearHandler(Window id);
  //! Dispatch events from the X server to the appropriate EventHandlers
  /*!
    @param remote Is the Xserver on a remote (low bandwidth) connection or on a
                  local (high bandwidth) connection. This allows you to specify
                  'false' in which case slightly different semantics are used
                  for event retrieval.<br>
                  The default is 'true' since this should generally be used,
                  only the Openbox window manager should need to specify
                  'false' here.
  */
  virtual void dispatchEvents(bool remote = true);

  inline void setFallbackHandler(EventHandler *fallback)
  { _fallback = fallback; }
  EventHandler *getFallbackHandler(void) const { return _fallback; }

  //! Sets an event handler that gets all events for all handlers after
  //! any specific handlers have received them
  inline void setMasterHandler(EventHandler *master)
  { _master = master; }
  EventHandler *getMasterHandler(void) const { return _master; }

  EventHandler *findHandler(Window win);

  inline Time lastTime() const { return _lasttime; }
  
private:
  EventMap _map;
  EventHandler *_fallback;
  EventHandler *_master;

  //! The time at which the last XEvent with a time was received
  Time _lasttime;

  void dispatch(Window win, const XEvent &e);
  void dispatchFocus(const XEvent &e);
};

}

#endif
