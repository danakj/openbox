// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

#include "timer.hh"
#include "display.hh"

extern "C" {
#ifdef    HAVE_SYS_SELECT_H
#  include <sys/select.h>
#endif // HAVE_SYS_SELECT_H

#ifdef    HAVE_SYS_TIME_H
#  include <sys/time.h>
#endif
}

namespace otk {

timeval Timer::_nearest_timeout, Timer::_now;
Timer::TimerQ Timer::_q;

void Timer::timevalAdd(timeval &a, long msec)
{
  a.tv_sec += msec / 1000;
  a.tv_usec += (msec % 1000) * 1000;
  a.tv_sec += a.tv_usec / 1000000;
  a.tv_usec %= 1000000;	
}

bool Timer::nearestTimeout(struct timeval &tm)
{
  if (_q.empty())
    return false;
  tm.tv_sec = _nearest_timeout.tv_sec - _now.tv_sec;
  tm.tv_usec = _nearest_timeout.tv_usec - _now.tv_usec;

  while (tm.tv_usec < 0) {
    tm.tv_usec += 1000000;
    tm.tv_sec--;
  }
  tm.tv_sec += tm.tv_usec / 1000000;
  tm.tv_usec %= 1000000;
  if (tm.tv_sec < 0)
    tm.tv_sec = 0;

  return true;
}

void Timer::dispatchTimers(bool wait)
{
  fd_set selset;
  int fd;
  timeval next;
  Timer *curr;

  gettimeofday(&_now, NULL);
  _nearest_timeout = _now;
  _nearest_timeout.tv_sec += 10000;

  while (!_q.empty()) {
    curr = _q.top();
    /* since we overload the destructor to keep from removing from the middle
       of the priority queue, set _del_me, we have to do our real delete in
       here.
    */
    if (curr->_del_me) {
      _q.pop();
      realDelete(curr);
      continue;
    }

    // the queue is sorted, so if this timer shouldn't fire, none are ready
    _nearest_timeout = curr->_timeout;
    if (!timercmp(&_now, &_nearest_timeout, >))
      break;

    /* we set the last fired time to delay msec after the previous firing, then
       re-insert.  timers maintain their order and may trigger more than once
       if they've waited more than one delay's worth of time.
    */
    _q.pop();
    timevalAdd(curr->_last, curr->_delay);
    curr->_action(curr->_data);
    timevalAdd(curr->_timeout, curr->_delay);
    _q.push(curr);

    /* if at least one timer fires, then don't wait on X events, as there may
       already be some in the queue from the timer callbacks.
    */
    wait = false;
  }

  if (wait) {
    // wait for the nearest trigger, or for X to do something interesting
    fd = ConnectionNumber(**display);
    FD_ZERO(&selset);
    FD_SET(fd, &selset);
    if (nearestTimeout(next)) {
      select(fd + 1, &selset, NULL, NULL, &next);
    } else
      select(fd + 1, &selset, NULL, NULL, NULL);
  }
}

Timer::Timer(long delay, Timer::TimeoutHandler action, void *data)
  : _delay(delay),
    _action(action),
    _data(data),
    _del_me(false),
    _last(_now),
    _timeout(_now)
{
  timevalAdd(_timeout, delay);
  _q.push(this);
}

void Timer::operator delete(void *self)
{
  Timer *t;
  t = (Timer *)self;
  t->_del_me = true;
}

void Timer::realDelete(Timer *me)
{
  ::delete me;
}

void Timer::initialize(void)
{
  gettimeofday(&_now, NULL);
  _nearest_timeout.tv_sec = 100000;
  _nearest_timeout.tv_usec = 0;
}

void Timer::destroy(void)
{
  while(!_q.empty()) {
    realDelete(_q.top());
    _q.pop();
  }
}

}
