// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "timerqueuemanager.hh"
#include "display.hh"

namespace otk {

void TimerQueueManager::fire(bool wait)
{
  fd_set rfds;
  timeval now, tm, *timeout = (timeval *) 0;

  const int xfd = ConnectionNumber(**display);
  
  FD_ZERO(&rfds);
  FD_SET(xfd, &rfds); // break on any x events

  if (wait) {
    if (! timerList.empty()) {
      const Timer* const timer = timerList.top();
      
      gettimeofday(&now, 0);
      tm = timer->remainingTime(now);
      
      timeout = &tm;
    }

    select(xfd + 1, &rfds, 0, 0, timeout);
  }

  // check for timer timeout
  gettimeofday(&now, 0);

  // there is a small chance for deadlock here:
  // *IF* the timer list keeps getting refreshed *AND* the time between
  // timer->start() and timer->shouldFire() is within the timer's period
  // then the timer will keep firing.  This should be VERY near impossible.
  while (! timerList.empty()) {
    Timer *timer = timerList.top();
    if (! timer->shouldFire(now))
      break;

    timerList.pop();

    timer->fire();
    if (timer->recurring())
      timer->start();
  }
}


void TimerQueueManager::addTimer(Timer *timer)
{
  assert(timer);
  timerList.push(timer);
}

void TimerQueueManager::removeTimer(Timer* timer)
{
  assert(timer);
  timerList.release(timer);
}

}
