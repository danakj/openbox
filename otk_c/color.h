// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __color_h
#define __color_h

#include <X11/Xlib.h>
#include <Python.h>


typedef struct OtkColor {
  int red, green, blue;
  int screen;
  Bool allocated;
  unsigned long pixel;
  PyObject *colorname; // PyStringObject
  Colormap colormap;
} OtkColor;

OtkColor *OtkColor_New(int screen);
OtkColor *OtkColor_FromRGB(int r, int g, int b, int screen);
OtkColor *OtkColor_FromName(const char *name, int screen);

void OtkColor_Destroy(OtkColor *self);

void OtkColor_SetRGB(OtkColor *self, int r, int g, int b);
void OtkColor_SetScreen(OtkColor *self, int screen);
Bool OtkColor_IsValid(OtkColor *self);
unsigned long OtkColor_Pixel(OtkColor *self);

void OtkColor_InitializeCache();
void OtkColor_DestroyCache();
void OtkColor_CleanupColorCache();

#endif // __color_h
