// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __rect_h
#define   __rect_h

#include <Python.h>

extern PyTypeObject OtkRect_Type;

typedef struct OtkRect {
  PyObject_HEAD
  int x, y, width, height;
} OtkRect;

PyObject *OtkRect_New(int x, int y, int width, int height);

#endif // __rect_h
