// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __timerqueuemanager_hh
#define __timerqueuemanager_hh

#include "timerqueue.hh"

namespace otk {

//! Manages a queue of Timer objects
/*!
  All Timer objects add themself to a TimerQueueManager. The manager is
  what fires the timers when their time has elapsed. This is done by having the
  application call the TimerQueueManager::fire class in its main event loop.
*/
class TimerQueueManager {
private:
  //! A priority queue of all timers being managed by this class.
  TimerQueue timerList;
public:
  //! Constructs a new TimerQueueManager
  TimerQueueManager() {}
  //! Destroys the TimerQueueManager
  virtual ~TimerQueueManager() {}

  //! Fire the next timer in the queue.
  /*!
    @param wait If true, this function will wait for the next timer, breaking
                on any events from the X server.
  */
  virtual void fire(bool wait = true);

  //! Adds a new timer to the queue
  /*!
    @param timer An Timer to add to the queue
  */
  virtual void addTimer(Timer* timer);
  //! Removes a timer from the queue
  /*!
    @param timer An Timer already in the queue to remove
  */
  virtual void removeTimer(Timer* timer);
};

}

#endif // __timerqueuemanager_hh
