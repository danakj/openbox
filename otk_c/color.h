// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __color_h
#define __color_h

#include <X11/Xlib.h>
#include <Python.h>

extern PyTypeObject OtkColor_Type;

//!  OtkColor objects are immutable. DONT CHANGE THEM.
typedef struct OtkColor {
  PyObject_HEAD
  int red, green, blue;
  int screen;
  Bool allocated;
  unsigned long pixel;
} OtkColor;

PyObject *OtkColor_FromRGB(int r, int g, int b, int screen);
PyObject *OtkColor_FromName(const char *name, int screen);

unsigned long OtkColor_Pixel(OtkColor *self);

void OtkColor_CleanupColorCache();

#endif // __color_h
