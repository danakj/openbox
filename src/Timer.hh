// -*- mode: C++; indent-tabs-mode: nil; -*-
// Timer.hh for Blackbox - An X11 Window Manager
// Copyright (c) 2001 - 2002 Sean 'Shaleh' Perry <shaleh@debian.org>
// Copyright (c) 1997 - 2000 Brad Hughes (bhughes@tcac.net)
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef   _BLACKBOX_Timer_hh
#define   _BLACKBOX_Timer_hh

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

// forward declaration
class TimerQueueManager;

class TimeoutHandler {
public:
  virtual void timeout(void) = 0;
};

class BTimer {
private:
  TimerQueueManager *manager;
  TimeoutHandler *handler;
  bool timing, recur;

  timeval _start, _timeout;

  BTimer(const BTimer&);
  BTimer& operator=(const BTimer&);

public:
  BTimer(TimerQueueManager *m, TimeoutHandler *h);
  virtual ~BTimer(void);

  void fireTimeout(void);

  inline bool isTiming(void) const { return timing; }
  inline bool isRecurring(void) const { return recur; }

  inline const timeval &getTimeout(void) const { return _timeout; }
  inline const timeval &getStartTime(void) const { return _start; }

  timeval timeRemaining(const timeval &tm) const;
  bool shouldFire(const timeval &tm) const;
  timeval endpoint(void) const;

  inline void recurring(bool b) { recur = b; }

  void setTimeout(long t);
  void setTimeout(const timeval &t);

  void start(void);  // manager acquires timer
  void stop(void);   // manager releases timer
  void halt(void);   // halts the timer

  bool operator<(const BTimer& other) const
  { return shouldFire(other.endpoint()); }
};


#include <queue>
#include <algorithm>

template <class _Tp, class _Sequence, class _Compare>
class _timer_queue: protected std::priority_queue<_Tp, _Sequence, _Compare> {
public:
  typedef std::priority_queue<_Tp, _Sequence, _Compare> _Base;

  _timer_queue(void): _Base() {}
  ~_timer_queue(void) {}

  void release(const _Tp& value) {
    c.erase(std::remove(c.begin(), c.end(), value), c.end());
    // after removing the item we need to make the heap again
    std::make_heap(c.begin(), c.end(), comp);
  }
  bool empty(void) const { return _Base::empty(); }
  size_t size(void) const { return _Base::size(); }
  void push(const _Tp& value) { _Base::push(value); }
  void pop(void) { _Base::pop(); }
  const _Tp& top(void) const { return _Base::top(); }
private:
  // no copying!
  _timer_queue(const _timer_queue&) {}
  _timer_queue& operator=(const _timer_queue&) {}
};

struct TimerLessThan {
  bool operator()(const BTimer* const l, const BTimer* const r) const {
    return *r < *l;
  }
};

#include <vector>
typedef _timer_queue<BTimer*, std::vector<BTimer*>, TimerLessThan> TimerQueue;

class TimerQueueManager {
public:
  virtual void addTimer(BTimer* timer) = 0;
  virtual void removeTimer(BTimer* timer) = 0;
};

#endif // _BLACKBOX_Timer_hh
