// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "../config.h"
#include "timerqueue.h"
#include "display.h"

#include <X11/Xlib.h>
#include <Python.h>

static PyObject *list = NULL; // PyListObject

void OtkTimerQueue_Initialize()
{
  list = PyList_New(0);
}

void OtkTimerQueue_Add(OtkTimer* timer)
{
  PyList_Append(list, (PyObject*)timer);
  PyList_Sort(list);
}

void OtkTimerQueue_Remove(OtkTimer* timer)
{
  int index;

  index = PySequence_Index(list, (PyObject*)timer);
  if (index >= 0)
    PySequence_DelItem(list, index);
}

static Bool shouldFire(OtkTimer *timer, const struct timeval *now)
{
  return ! ((now->tv_sec < timer->end.tv_sec) ||
	    (now->tv_sec == timer->end.tv_sec &&
	     now->tv_usec < timer->end.tv_usec));
}

static void normalizeTimeval(struct timeval *time)
{
  while (time->tv_usec < 0) {
    if (time->tv_sec > 0) {
      --time->tv_sec;
      time->tv_usec += 1000000;
    } else {
      time->tv_usec = 0;
    }
  }

  if (time->tv_usec >= 1000000) {
    time->tv_sec += time->tv_usec / 1000000;
    time->tv_usec %= 1000000;
  }

  if (time->tv_sec < 0) time->tv_sec = 0;
}

void OtkTimerQueue_Fire()
{
  fd_set rfds;
  struct timeval now, tm, *timeout = NULL;

  const int xfd = ConnectionNumber(OBDisplay->display);
  
  FD_ZERO(&rfds);
  FD_SET(xfd, &rfds); // break on any x events

  // check for timer timeout
  gettimeofday(&now, 0);
  
  // there is a small chance for deadlock here:
  // *IF* the timer list keeps getting refreshed *AND* the time between
  // timer->start() and timer->shouldFire() is within the timer's period
  // then the timer will keep firing.  This should be VERY near impossible.
  while (PyList_Size(list)) {
    OtkTimer *timer = (OtkTimer*)PyList_GetItem(list, 0);
    
    if (! shouldFire(timer, &now)) {
      tm.tv_sec = timer->end.tv_sec - now.tv_sec;
      tm.tv_usec = timer->end.tv_usec - now.tv_usec;
      normalizeTimeval(&tm);
      timeout = &tm; // set the timeout for the select
      break; // go on and wait
    }

    // stop and remove the timer from the queue
    PySequence_DelItem(list, 0);
    timer->timing = False;

    if (timer->handler)
      timer->handler(timer->data);

    if (timer->recur)
      OtkTimer_Start(timer);
  }

  select(xfd + 1, &rfds, 0, 0, timeout);
}
