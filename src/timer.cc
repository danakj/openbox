// -*- mode: C++; indent-tabs-mode: nil; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "basedisplay.hh"
#include "timer.hh"
#include "util.hh"

BTimer::BTimer(TimerQueueManager *m, TimeoutHandler *h) {
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
