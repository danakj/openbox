// -*- mode: C++; indent-tabs-mode: nil; -*-
#ifndef   __timer_hh
#define   __timer_hh

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

namespace otk {

class OBTimerQueueManager;

//! The data passed to the OBTimeoutHandler function.
/*!
  Note: this is a very useful place to put an object instance, and set the
  event handler to a static function in the same class.
*/
typedef void *OBTimeoutData;
//! The type of function which can be set as the callback for an OBTimer firing
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

}

#endif // __timer_hh
