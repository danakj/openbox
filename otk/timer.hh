// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __timer_hh
#define   __timer_hh

/*! @file timer.hh
  @brief Contains the Timer class, used for timed callbacks.
*/

extern "C" {
#include <ctime>
}

#include <queue>
#include <vector>

namespace otk {

//! The Timer class implements timed callbacks.
/*!
  The Timer class can be used to have a callback fire after a given time
  interval. A created Timer will fire repetitively until it is destroyed.
*/
class Timer {
public:
  //! Data type of Timer callback
  typedef void (*TimeoutHandler)(void *data);

private:
  //! Compares two timeval structs
  struct TimerCompare {
     //! Compares two timeval structs
     inline bool operator()(const Timer *a, const Timer *b) const {
       return ((&a->_timeout)->tv_sec == (&b->_timeout)->tv_sec) ?
         ((&a->_timeout)->tv_usec > (&b->_timeout)->tv_usec) :
         ((&a->_timeout)->tv_sec > (&b->_timeout)->tv_sec);
     }
  };
  friend struct TimerCompare; // give access to _timeout for shitty compilers

  typedef
  std::priority_queue<Timer*, std::vector<Timer*>, TimerCompare> TimerQ;

  //! Milliseconds between timer firings
  long _delay;
  //! Callback for timer expiry
  TimeoutHandler _action;
  //! Data sent to callback
  void *_data;
  //! We overload the delete operator to just set this to true
  bool _del_me;
  //! The time the last fire should've been at
  struct timeval _last;
  //! When this timer will next trigger
  struct timeval _timeout;

  //! Queue of pending timers
  static TimerQ _q;
  //! Time next timer will expire
  static timeval _nearest_timeout;
  //! Time at start of current processing loop
  static timeval _now;

  //! Really delete something (not just flag for later)
  /*!
    @param self Timer to be deleted.
  */
  static void realDelete(Timer *self);

  //! Adds a millisecond delay to a timeval structure
  /*!
    @param a Amount of time to increment.
    @param msec Number of milliseconds to increment by.
  */
  static void timevalAdd(timeval &a, long msec);

public:
  //! Constructs a new running timer and queues it
  /*!
    @param delay Time in milliseconds between firings
    @param cb The function to be called on fire.
    @param data Data to be passed to the callback on fire.
  */
  Timer(long delay, TimeoutHandler cb, void *data);

  //! Overloaded delete so we can leave deleted objects in queue for later reap
  /*!
    @param self Pointer to current instance of Timer.
  */
  void operator delete(void *self);

  //! Dispatches all elligible timers, then optionally waits for X events
  /*!
    @param wait Whether to wait for X events after processing timers.
  */
  static void dispatchTimers(bool wait = true);

  //! Returns a relative timeval (to pass select) of the next timer
  /*!
    @param tm Changed to hold the time until next timer.
    @return true if there are any timers queued, and the timeout is being
            returned in 'tm'. false if there are no timers queued.
  */
  static bool nearestTimeout(struct timeval &tm);

  //! Initializes internal data before use
  static void initialize();

  //! Deletes all waiting timers
  static void destroy();
};

}

#endif // __timer.hh
