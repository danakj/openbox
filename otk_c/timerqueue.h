// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __timerqueue_h
#define __timerqueue_h

#include "timer.h"

void OtkTimerQueue_Initialize();

//! Will wait for and fire the next timer in the queue.
/*!
  The function will stop waiting if an event is received from the X server.
*/
void OtkTimerQueue_Fire();

//! Adds a new timer to the queue
/*!
  @param timer An OtkTimer to add to the queue
*/
void OtkTimerQueue_Add(OtkTimer* timer);

//! Removes a timer from the queue
/*!
  @param timer An OtkTimer already in the queue to remove
*/
void OtkTimerQueue_Remove(OtkTimer* timer);

#endif // __timerqueue_h
