// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "../config.h"
#include "display.h"
#include "screeninfo.h"
#include "color.h"
#include "gccache.h"
#include "font.h"
#include "rect.h"
#include "timer.h"
#include "timerqueue.h"
#include "imagecontrol.h"

#include <X11/Xlib.h>
#include <Python.h>

static PyMethodDef otk_methods[] = {
//  {"new_noddy", noddy_new_noddy, METH_VARARGS,
//   "Create a new Noddy object."},

  {NULL, NULL, 0, NULL}
};

void initotk(char *display)
{
  OtkRect_Type.ob_type = &PyType_Type;
  OtkDisplay_Type.ob_type = &PyType_Type;
  OtkScreenInfo_Type.ob_type = &PyType_Type;
  OtkColor_Type.ob_type = &PyType_Type;
  OtkFont_Type.ob_type = &PyType_Type;
  OtkTimer_Type.ob_type = &PyType_Type;
  OtkImageControl_Type.ob_type = &PyType_Type;

  Py_InitModule("otk", otk_methods);

  OtkTimerQueue_Initialize();
  OtkDisplay_Initialize(display);
  assert(OBDisplay);
  OtkGCCache_Initialize();
  OtkFont_Initialize();
}
