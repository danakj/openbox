// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __screeninfo_h
#define   __screeninfo_h

#include <X11/Xlib.h>
#include <Python.h>

extern PyTypeObject OtkScreenInfo_Type;

struct OtkRect;

typedef struct OtkScreenInfo {
  int screen;
  Window root_window;

  int depth;
  Visual *visual;
  Colormap colormap;

  PyStringObject *display_string;
  struct OtkRect *rect; // OtkRect
#ifdef XINERAMA
  PyListObject *xinerama_areas; // holds OtkRect's
  Bool xinerama_active;
#endif
} OtkScreenInfo;

//! Creates an OtkScreenInfo for a screen
/*!
  @param num The number of the screen on the display for which to fill the
             struct with information. Must be a value >= 0.
*/
PyObject *OtkScreenInfo_New(int num);

#endif // __screeninfo_h
