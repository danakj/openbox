// -*- mode: C; indent-tabs-mode: nil; -*-
#ifndef   __rect_h
#define   __rect_h

#include <Python.h>

typedef struct OtkRect {
  PyObject_HEAD
  int x, y, width, height;
} OtkRect;

PyObject *OtkRect_New(int x, int y, int width, int height);

#endif // __rect_h
