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

//! A Timer class which will fire a function when its time elapses
class OBTimer {
private:
  //! The manager which to add ourself to and remove ourself after we are done
  OBTimerQueueManager *manager;
  //! The function to call when the time elapses
  OBTimeoutHandler handler;
  //! The data which gets passed along to the OBTimeoutHandler
  OBTimeoutData data;
  //! Determines if the timer is currently started
  bool timing;
  //! When this is true, the timer will reset itself to fire again every time
  bool recur;

  //! The time at which the timer started
  timeval _start;
  //! The time at which the timer is going to fire
  timeval _timeout;

  //! Disallows copying of OBTimer objects
  OBTimer(const OBTimer&);
  //! Disallows copying of OBTimer objects
  OBTimer& operator=(const OBTimer&);

public:
  //! Constructs a new OBTimer object
  /*!
    @param m The OBTimerQueueManager with which to associate. The manager
             specified will be resposible for making this timer fire.
    @param h The function to call when the timer fires
    @param d The data to pass along to the function call when the timer fires
  */
  OBTimer(OBTimerQueueManager *m, OBTimeoutHandler h, OBTimeoutData d);
  //! Destroys the OBTimer object
  virtual ~OBTimer();

  //! Fires the timer, calling its OBTimeoutHandler
  void fireTimeout();

  //! Returns if the OBTimer is started and timing
  inline bool isTiming() const { return timing; }
  //! Returns if the OBTimer is going to repeat
  inline bool isRecurring() const { return recur; }

  //! Gets the amount of time the OBTimer should last before firing
  inline const timeval &getTimeout() const { return _timeout; }
  //! Gets the time at which the OBTimer started
  inline const timeval &getStartTime() const { return _start; }

  //! Gets the amount of time left before the OBTimer fires
  timeval timeRemaining(const timeval &tm) const;
  //! Returns if the OBTimer is past its timeout time, and should fire
  bool shouldFire(const timeval &tm) const;

  //! Gets the time at which the OBTimer will fire
  timeval endpoint() const;

  //! Sets the OBTimer to repeat or not
  /*!
    @param b If true, the timer is set to repeat; otherwise, it will fire only
             once
  */
  inline void recurring(bool b) { recur = b; }

  //! Sets the amount of time for the OBTimer to last in milliseconds
  /*!
    @param t The number of milliseconds the timer should last
  */
  void setTimeout(long t);
  //! Sets the amount of time the OBTimer should last before firing
  /*!
    @param t The amount of time the timer should last
  */
  void setTimeout(const timeval &t);

  //! Causes the timer to begin
  /*!
    The timer fires after the time in OBTimer::getTimeout has passed since this
    function was called.
    Calling this function while the timer is already started will cause it to
    restart its countdown.
  */
  void start();  // manager acquires timer
  //! Causes the timer to stop
  /*!
    The timer will no longer fire once this function has been called.
    Calling this function more than once does not have any effect.
  */
  void stop();   // manager releases timer

  //! Determines if this OBTimer will fire before a second OBTimer object
  /*!
    @param other The second OBTimer with which to compare
    @return true if this OBTimer will fire before 'other'; otherwise, false
  */
  bool operator<(const OBTimer& other) const
  { return shouldFire(other.endpoint()); }
};

}

#endif // __timer_hh
