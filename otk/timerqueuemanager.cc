// -*- mode: C++; indent-tabs-mode: nil; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "timerqueuemanager.hh"
#include "display.hh"

namespace otk {

void OBTimerQueueManager::fire()
{
  fd_set rfds;
  timeval now, tm, *timeout = (timeval *) 0;

  const int xfd = ConnectionNumber(otk::OBDisplay::display);
  
  FD_ZERO(&rfds);
  FD_SET(xfd, &rfds); // break on any x events

  if (! timerList.empty()) {
    const OBTimer* const timer = timerList.top();

    gettimeofday(&now, 0);
    tm = timer->remainingTime(now);

    timeout = &tm;
  }

  select(xfd + 1, &rfds, 0, 0, timeout);

  // check for timer timeout
  gettimeofday(&now, 0);

  // there is a small chance for deadlock here:
  // *IF* the timer list keeps getting refreshed *AND* the time between
  // timer->start() and timer->shouldFire() is within the timer's period
  // then the timer will keep firing.  This should be VERY near impossible.
  while (! timerList.empty()) {
    OBTimer *timer = timerList.top();
    if (! timer->shouldFire(now))
      break;

    timerList.pop();

    timer->fire();
    if (timer->recurring())
      timer->start();
  }
}


void OBTimerQueueManager::addTimer(OBTimer *timer)
{
  assert(timer);
  timerList.push(timer);
}

void OBTimerQueueManager::removeTimer(OBTimer* timer)
{
  assert(timer);
  timerList.release(timer);
}

}
