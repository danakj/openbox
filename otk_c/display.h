// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef   __display_h
#define   __display_h

#include <X11/Xlib.h>
#include <Python.h>

struct OtkScreenInfo;
struct OtkDisplay;

extern struct OtkDisplay *OBDisplay; // the global display XXX: move this to app.h and ob.h?

extern PyTypeObject OtkDisplay_Type;

typedef struct OtkDisplay {
  PyObject_HEAD
  
  //! The X display
  Display *display;
  
  //! Does the display have the Shape extention?
  Bool shape;
  //! Base for events for the Shape extention
  int  shape_event_basep;

  //! Does the display have the Xinerama extention?
  Bool xinerama;
  //! Base for events for the Xinerama extention
  int  xinerama_event_basep;

  //! A list of all possible combinations of keyboard lock masks
  unsigned int mask_list[8];

  //! The number of requested grabs on the display
  int grab_count;

  //! A list of information for all screens on the display
  PyListObject *screenInfoList;
} OtkDisplay;

//! Opens the X display, and sets the global OBDisplay variable
/*!
  @see OBDisplay::display
  @param name The name of the X display to open. If it is null, the DISPLAY
  environment variable is used instead.
*/
void OtkDisplay_Initialize(char *name);

//! Grabs the display
void OtkDisplay_Grab(OtkDisplay *self);

//! Ungrabs the display
void OtkDisplay_Ungrab(OtkDisplay *self);

//! Get the screen info for a specific screen
struct OtkScreenInfo *OtkDisplay_ScreenInfo(OtkDisplay *self, int num);

#endif // __display_h
