#ifndef __eventdispatcher
#define __eventdispatcher

#include "eventhandler.hh"
#include <map>
#include <utility>

namespace otk {

typedef std::map<unsigned int, OtkEventHandler *> OtkEventMap;

class OtkEventDispatcher {
public:

  OtkEventDispatcher();
  virtual ~OtkEventDispatcher();

  virtual void clearAllHandlers(void);
  virtual void registerHandler(Window id, OtkEventHandler *handler);
  virtual void clearHandler(Window id);
  virtual void dispatchEvents(void);

  inline void setFallbackHandler(OtkEventHandler *fallback)
  { _fallback = fallback; }
  OtkEventHandler *getFallbackHandler(void) const { return _fallback; }

private:
  OtkEventMap _map;
  OtkEventHandler *_fallback;

};

}

#endif
