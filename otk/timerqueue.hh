// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __timerqueue_hh
#define __timerqueue_hh

#ifndef   DOXYGEN_IGNORE

#include "timer.hh"

#include <queue>
#include <vector>
#include <algorithm>

namespace otk {

template <class _Tp, class _Sequence, class _Compare>
class _timer_queue: protected std::priority_queue<_Tp, _Sequence, _Compare> {
public:
  typedef std::priority_queue<_Tp, _Sequence, _Compare> _Base;

  _timer_queue(): _Base() {}
  ~_timer_queue() {}

  void release(const _Tp& value) {
    c.erase(std::remove(c.begin(), c.end(), value), c.end());
    // after removing the item we need to make the heap again
    std::make_heap(c.begin(), c.end(), comp);
  }
  bool empty() const { return _Base::empty(); }
  size_t size() const { return _Base::size(); }
  void push(const _Tp& value) { _Base::push(value); }
  void pop() { _Base::pop(); }
  const _Tp& top() const { return _Base::top(); }
private:
  // no copying!
  _timer_queue(const _timer_queue&) {}
  _timer_queue& operator=(const _timer_queue&) {}
};

struct TimerLessThan {
  bool operator()(const Timer* const l, const Timer* const r) const {
    return *r < *l;
  }
};

typedef _timer_queue<Timer*, std::vector<Timer*>, TimerLessThan> TimerQueue;

}

#endif // DOXYGEN_IGNORE

#endif // __timerqueue_hh
