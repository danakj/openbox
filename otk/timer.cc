// -*- mode: C++; indent-tabs-mode: nil; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "timer.hh"
#include "display.hh"
#include "util.hh"

namespace otk {

static timeval normalizeTimeval(const timeval &tm)
{
  timeval ret = tm;

  while (ret.tv_usec < 0) {
    if (ret.tv_sec > 0) {
      --ret.tv_sec;
      ret.tv_usec += 1000000;
    } else {
      ret.tv_usec = 0;
    }
  }

  if (ret.tv_usec >= 1000000) {
    ret.tv_sec += ret.tv_usec / 1000000;
    ret.tv_usec %= 1000000;
  }

  if (ret.tv_sec < 0) ret.tv_sec = 0;

  return ret;
}


OBTimer::OBTimer(OBTimerQueueManager *m, OBTimeoutHandler h, OBTimeoutData d)
{
  manager = m;
  handler = h;
  data = d;

  recur = timing = False;
}


OBTimer::~OBTimer(void)
{
  if (timing) stop();
}


void OBTimer::setTimeout(long t)
{
  _timeout.tv_sec = t / 1000;
  _timeout.tv_usec = t % 1000;
  _timeout.tv_usec *= 1000;
}


void OBTimer::setTimeout(const timeval &t)
{
  _timeout.tv_sec = t.tv_sec;
  _timeout.tv_usec = t.tv_usec;
}


void OBTimer::start(void)
{
  gettimeofday(&_start, 0);

  if (! timing) {
    timing = True;
    manager->addTimer(this);
  }
}


void OBTimer::stop(void)
{
  timing = False;

  manager->removeTimer(this);
}


void OBTimer::halt(void)
{
  timing = False;
}


void OBTimer::fireTimeout(void)
{
  if (handler)
    handler(data);
}


timeval OBTimer::timeRemaining(const timeval &tm) const
{
  timeval ret = endpoint();

  ret.tv_sec  -= tm.tv_sec;
  ret.tv_usec -= tm.tv_usec;

  return normalizeTimeval(ret);
}


timeval OBTimer::endpoint(void) const
{
  timeval ret;

  ret.tv_sec = _start.tv_sec + _timeout.tv_sec;
  ret.tv_usec = _start.tv_usec + _timeout.tv_usec;

  return normalizeTimeval(ret);
}


bool OBTimer::shouldFire(const timeval &tm) const
{
  timeval end = endpoint();

  return ! ((tm.tv_sec < end.tv_sec) ||
            (tm.tv_sec == end.tv_sec && tm.tv_usec < end.tv_usec));
}


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
    tm = timer->timeRemaining(now);

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

    timer->fireTimeout();
    timer->halt();
    if (timer->isRecurring())
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
