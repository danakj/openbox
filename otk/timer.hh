// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
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

class TimerQueueManager;

//! The data passed to the TimeoutHandler function.
/*!
  Note: this is a very useful place to put an object instance, and set the
  event handler to a static function in the same class.
*/
typedef void *TimeoutData;
//! The type of function which can be set as the callback for a Timer firing
typedef void (*TimeoutHandler)(TimeoutData);

//! A Timer class which will fire a function when its time elapses
class Timer {
private:
  //! The manager which to add ourself to and remove ourself after we are done
  TimerQueueManager *_manager;
  //! The function to call when the time elapses
  TimeoutHandler _handler;
  //! The data which gets passed along to the TimeoutHandler
  TimeoutData _data;
  //! Determines if the timer is currently started
  bool _timing;
  //! When this is true, the timer will reset itself to fire again every time
  bool _recur;

  //! The time at which the timer started
  timeval _start;
  //! The time at which the timer is going to fire
  timeval _timeout;

  //! Disallows copying of Timer objects
  Timer(const Timer&);
  //! Disallows copying of Timer objects
  Timer& operator=(const Timer&);

public:
  //! Constructs a new Timer object
  /*!
    @param m The TimerQueueManager with which to associate. The manager
             specified will be resposible for making this timer fire.
    @param h The function to call when the timer fires
    @param d The data to pass along to the function call when the timer fires
  */
  Timer(TimerQueueManager *m, TimeoutHandler h, TimeoutData d);
  //! Destroys the Timer object
  virtual ~Timer();

  //! Fires the timer, calling its TimeoutHandler
  void fire();

  //! Returns if the Timer is started and timing
  inline bool timing() const { return _timing; }
  //! Returns if the Timer is going to repeat
  inline bool recurring() const { return _recur; }

  //! Gets the amount of time the Timer should last before firing
  inline const timeval &timeout() const { return _timeout; }
  //! Gets the time at which the Timer started
  inline const timeval &startTime() const { return _start; }

  //! Gets the amount of time left before the Timer fires
  timeval remainingTime(const timeval &tm) const;
  //! Returns if the Timer is past its timeout time, and should fire
  bool shouldFire(const timeval &tm) const;

  //! Gets the time at which the Timer will fire
  timeval endTime() const;

  //! Sets the Timer to repeat or not
  /*!
    @param b If true, the timer is set to repeat; otherwise, it will fire only
             once
  */
  inline void setRecurring(bool b) { _recur = b; }

  //! Sets the amount of time for the Timer to last in milliseconds
  /*!
    @param t The number of milliseconds the timer should last
  */
  void setTimeout(long t);
  //! Sets the amount of time the Timer should last before firing
  /*!
    @param t The amount of time the timer should last
  */
  void setTimeout(const timeval &t);

  //! Causes the timer to begin
  /*!
    The timer fires after the time in Timer::getTimeout has passed since this
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

  //! Determines if this Timer will fire before a second Timer object
  /*!
    @param other The second Timer with which to compare
    @return true if this Timer will fire before 'other'; otherwise, false
  */
  bool operator<(const Timer& other) const
  { return shouldFire(other.endTime()); }
};

}

#endif // __timer_hh
