// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __timer_h
#define __timer_h

#include <X11/Xlib.h>
#include <Python.h>

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

extern PyTypeObject OtkTimer_Type;

//! The data passed to the OtkTimeoutHandler function.
/*!
  Note: this is a very useful place to put an object instance, and set the
  event handler to a static function in the same class.
*/
typedef void *OtkTimeoutData;
//! The type of function which can be set as the callback for an OtkTimer
//! firing
typedef void (*OtkTimeoutHandler)(OtkTimeoutData);

typedef struct OtkTimer {
  PyObject_HEAD
  OtkTimeoutHandler handler;
  OtkTimeoutData data;
  Bool recur;
  long timeout;

  // don't edit these
  Bool timing;
  struct timeval start;
  struct timeval end;
} OtkTimer;

PyObject *OtkTimer_New(OtkTimeoutHandler handler, OtkTimeoutData data);

//! Causes the timer to begin
void OtkTimer_Start(OtkTimer *self);

//! Causes the timer to stop
void OtkTimer_Stop(OtkTimer *self);

#endif // __timer_h
