// -*- mode: C++; indent-tabs-mode: nil; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "otk/display.hh"
#include "timer.hh"
#include "util.hh"

namespace ob {

BTimer::BTimer(OBTimerQueueManager *m, TimeoutHandler *h) {
  manager = m;
  handler = h;

  recur = timing = False;
}


BTimer::~BTimer(void) {
  if (timing) stop();
}


void BTimer::setTimeout(long t) {
  _timeout.tv_sec = t / 1000;
  _timeout.tv_usec = t % 1000;
  _timeout.tv_usec *= 1000;
}


void BTimer::setTimeout(const timeval &t) {
  _timeout.tv_sec = t.tv_sec;
  _timeout.tv_usec = t.tv_usec;
}


void BTimer::start(void) {
  gettimeofday(&_start, 0);

  if (! timing) {
    timing = True;
    manager->addTimer(this);
  }
}


void BTimer::stop(void) {
  timing = False;

  manager->removeTimer(this);
}


void BTimer::halt(void) {
  timing = False;
}


void BTimer::fireTimeout(void) {
  if (handler)
    handler->timeout();
}


timeval BTimer::timeRemaining(const timeval &tm) const {
  timeval ret = endpoint();

  ret.tv_sec  -= tm.tv_sec;
  ret.tv_usec -= tm.tv_usec;

  return normalizeTimeval(ret);
}


timeval BTimer::endpoint(void) const {
  timeval ret;

  ret.tv_sec = _start.tv_sec + _timeout.tv_sec;
  ret.tv_usec = _start.tv_usec + _timeout.tv_usec;

  return normalizeTimeval(ret);
}


bool BTimer::shouldFire(const timeval &tm) const {
  timeval end = endpoint();

  return ! ((tm.tv_sec < end.tv_sec) ||
            (tm.tv_sec == end.tv_sec && tm.tv_usec < end.tv_usec));
}


void OBTimerQueueManager::go()
{
  fd_set rfds;
  timeval now, tm, *timeout = (timeval *) 0;

  const int xfd = ConnectionNumber(otk::OBDisplay::display);
  
  FD_ZERO(&rfds);
  FD_SET(xfd, &rfds); // break on any x events

  if (! timerList.empty()) {
    const BTimer* const timer = timerList.top();

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
    BTimer *timer = timerList.top();
    if (! timer->shouldFire(now))
      break;

    timerList.pop();

    timer->fireTimeout();
    timer->halt();
    if (timer->isRecurring())
      timer->start();
  }
}


void OBTimerQueueManager::addTimer(BTimer *timer)
{
  assert(timer);
  timerList.push(timer);
}

void OBTimerQueueManager::removeTimer(BTimer* timer)
{
  assert(timer);
  timerList.release(timer);
}

}
