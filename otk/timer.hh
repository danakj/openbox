// -*- mode: C++; indent-tabs-mode: nil; -*-
#ifndef   _BLACKBOX_Timer_hh
#define   _BLACKBOX_Timer_hh

extern "C" {
#ifdef    TIME_WITH_SYS_TIME
#  include <sys/time.h>
#  include <time.h>
#else // !TIME_WITH_SYS_TIME
#  ifdef    HAVE_SYS_TIME_H
#    include <sys/time.h>
#  else // !HAVE_SYS_TIME_H
#    include <time.h>
#  endif // HAVE_SYS_TIME_H
#endif // TIME_WITH_SYS_TIME
}

#include <queue>
#include <algorithm>
#include <vector>

namespace otk {

// forward declaration
class OBTimerQueueManager;

typedef void *OBTimeoutData;
typedef void (*OBTimeoutHandler)(OBTimeoutData);

class OBTimer {
private:
  OBTimerQueueManager *manager;
  OBTimeoutHandler handler;
  OBTimeoutData data;
  bool timing, recur;

  timeval _start, _timeout;

  OBTimer(const OBTimer&);
  OBTimer& operator=(const OBTimer&);

public:
  OBTimer(OBTimerQueueManager *m, OBTimeoutHandler h, OBTimeoutData d);
  virtual ~OBTimer();

  void fireTimeout();

  inline bool isTiming() const { return timing; }
  inline bool isRecurring() const { return recur; }

  inline const timeval &getTimeout() const { return _timeout; }
  inline const timeval &getStartTime() const { return _start; }

  timeval timeRemaining(const timeval &tm) const;
  bool shouldFire(const timeval &tm) const;
  timeval endpoint() const;

  inline void recurring(bool b) { recur = b; }

  void setTimeout(long t);
  void setTimeout(const timeval &t);

  void start();  // manager acquires timer
  void stop();   // manager releases timer
  void halt();   // halts the timer

  bool operator<(const OBTimer& other) const
  { return shouldFire(other.endpoint()); }
};


template <class _Tp, class _Sequence, class _Compare>
class _timer_queue: protected std::priority_queue<_Tp, _Sequence, _Compare> {
public:
  typedef std::priority_queue<_Tp, _Sequence, _Compare> _Base;

  _timer_queue(): _Base() {}
  ~_timer_queue() {}

  void release(const _Tp& value) {
    c.erase(std::remove(c.begin(), c.end(), value), c.end());
    // after removing the item we need to make the heap again
    std::make_heap(c.begin(), c.end(), comp);
  }
  bool empty() const { return _Base::empty(); }
  size_t size() const { return _Base::size(); }
  void push(const _Tp& value) { _Base::push(value); }
  void pop() { _Base::pop(); }
  const _Tp& top() const { return _Base::top(); }
private:
  // no copying!
  _timer_queue(const _timer_queue&) {}
  _timer_queue& operator=(const _timer_queue&) {}
};

struct TimerLessThan {
  bool operator()(const OBTimer* const l, const OBTimer* const r) const {
    return *r < *l;
  }
};

typedef _timer_queue<OBTimer*,
                     std::vector<OBTimer*>, TimerLessThan> TimerQueue;

//! Manages a queue of OBTimer objects
/*!
  All OBTimer objects add themself to an OBTimerQueueManager. The manager is
  what fires the timers when their time has elapsed. This is done by having the
  application call the OBTimerQueueManager::fire class in its main event loop.
*/
class OBTimerQueueManager {
private:
  //! A priority queue of all timers being managed by this class.
  TimerQueue timerList;
public:
  //! Constructs a new OBTimerQueueManager
  OBTimerQueueManager() {}
  //! Destroys the OBTimerQueueManager
  virtual ~OBTimerQueueManager() {}

  //! Will wait for and fire the next timer in the queue.
  /*!
    The function will stop waiting if an event is received from the X server.
  */
  virtual void fire();

  //! Adds a new timer to the queue
  /*!
    @param timer An OBTimer to add to the queue
  */
  virtual void addTimer(OBTimer* timer);
  //! Removes a timer from the queue
  /*!
    @param timer An OBTimer already in the queue to remove
  */
  virtual void removeTimer(OBTimer* timer);
};

}

#endif // _BLACKBOX_Timer_hh
