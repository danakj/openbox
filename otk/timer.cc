// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "timer.hh"
#include "timerqueuemanager.hh"

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
  _manager = m;
  _handler = h;
  _data = d;

  _recur = _timing = false;
}


OBTimer::~OBTimer(void)
{
  if (_timing) stop();
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

  if (! _timing) {
    _timing = true;
    _manager->addTimer(this);
  }
}


void OBTimer::stop(void)
{
  if (_timing) {
    _timing = false;

    _manager->removeTimer(this);
  }
}


void OBTimer::fire(void)
{
  if (_handler)
    _handler(_data);
}


timeval OBTimer::remainingTime(const timeval &tm) const
{
  timeval ret = endTime();

  ret.tv_sec  -= tm.tv_sec;
  ret.tv_usec -= tm.tv_usec;

  return normalizeTimeval(ret);
}


timeval OBTimer::endTime(void) const
{
  timeval ret;

  ret.tv_sec = _start.tv_sec + _timeout.tv_sec;
  ret.tv_usec = _start.tv_usec + _timeout.tv_usec;

  return normalizeTimeval(ret);
}


bool OBTimer::shouldFire(const timeval &tm) const
{
  timeval end = endTime();

  return ! ((tm.tv_sec < end.tv_sec) ||
            (tm.tv_sec == end.tv_sec && tm.tv_usec < end.tv_usec));
}

}
