// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "../config.h"
#include "timer.h"
#include "timerqueue.h"

PyObject *OtkTimer_New(OtkTimeoutHandler handler, OtkTimeoutData data)
{
  OtkTimer *self = PyObject_New(OtkTimer, &OtkTimer_Type);

  assert(handler); assert(data);
  self->handler = handler;
  self->data = data;
  self->recur = self->timing = False;

  return (PyObject*)self;
}

void OtkTimer_Start(OtkTimer *self)
{
  gettimeofday(&(self->start), 0);

  self->end.tv_sec = self->start.tv_sec + self->timeout / 1000;
  self->end.tv_usec = self->start.tv_usec + self->timeout % 1000 * 1000;

  if (! self->timing) {
    self->timing = True;
    OtkTimerQueue_Add(self);
  }
}

void OtkTimer_Stop(OtkTimer *self)
{
  if (self->timing) {
    self->timing = False;
    OtkTimerQueue_Remove(self);
  }
}





static void otktimer_dealloc(OtkTimer* self)
{
  OtkTimer_Stop(self);  
  // when this is called, the color has already been cleaned out of the cache
  PyObject_Del((PyObject*)self);
}

static int otktimer_compare(OtkTimer *t1, OtkTimer *t2)
{
  if (t1->end.tv_sec == t2->end.tv_sec && t1->end.tv_usec == t2->end.tv_usec)
    return 0;
  else if ((t1->end.tv_sec < t2->end.tv_sec) ||
	   (t1->end.tv_sec == t2->end.tv_sec &&
	    t1->end.tv_usec < t2->end.tv_usec))
    return -1;
  else
    return 1;
}

PyTypeObject OtkTimer_Type = {
  PyObject_HEAD_INIT(NULL)
  0,
  "OtkTimer",
  sizeof(OtkTimer),
  0,
  (destructor)otktimer_dealloc, /*tp_dealloc*/
  0,                            /*tp_print*/
  0,                            /*tp_getattr*/
  0,                            /*tp_setattr*/
  (cmpfunc)otktimer_compare,    /*tp_compare*/
  0,                            /*tp_repr*/
  0,                            /*tp_as_number*/
  0,                            /*tp_as_sequence*/
  0,                            /*tp_as_mapping*/
  0,                            /*tp_hash */
};
