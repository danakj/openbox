// -*- mode: C; indent-tabs-mode: nil; -*-
#ifndef   __display_h
#define   __display_h

#include <X11/Xlib.h>
#include <Python.h>

struct OtkScreenInfo;
struct OtkGCCache;
struct OtkDisplay;

struct OtkDisplay *OBDisplay; // the global display XXX: move this to app.h and ob.h?

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
  PyObject *screenInfoList; // PyListObject

  //! A cache for re-using GCs, used by the drawing objects
  /*!
    @see BPen
    @see BFont
    @see BImage
    @see BImageControl
    @see BTexture
  */
  struct OtkGCCache *gccache;
} OtkDisplay;

//! Creates a struct, opens the X display
/*!
  @see OBDisplay::display
  @param name The name of the X display to open. If it is null, the DISPLAY
  environment variable is used instead.
*/
PyObject *OtkDisplay_New(char *name);

//! Grabs the display
void OtkDisplay_Grab(OtkDisplay *self);

//! Ungrabs the display
void OtkDisplay_Ungrab(OtkDisplay *self);

//! Get the screen info for a specific screen
struct OtkScreenInfo *OtkDisplay_ScreenInfo(OtkDisplay *self, int num);

#endif // __display_h
