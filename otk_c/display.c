// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "../config.h"
#include "display.h"
#include "screeninfo.h"

#include <X11/keysym.h>

#ifdef    SHAPE
#include <X11/extensions/shape.h>
#endif // SHAPE

#ifdef    HAVE_STDIO_H
#  include <stdio.h>
#endif // HAVE_STDIO_H

#ifdef    HAVE_STDLIB_H
#  include <stdlib.h>
#endif // HAVE_STDLIB_H

#ifdef    HAVE_FCNTL_H
#  include <fcntl.h>
#endif // HAVE_FCNTL_H

#ifdef    HAVE_UNISTD_H
#  include <sys/types.h>
#  include <unistd.h>
#endif // HAVE_UNISTD_H

#include "../src/gettext.h"

extern PyTypeObject OtkDisplay_Type;

static int xerrorHandler(Display *d, XErrorEvent *e);

struct OtkDisplay *OBDisplay = NULL;

void OtkDisplay_Initialize(char *name)
{
  OtkDisplay* self;
  PyObject *disp_env;
  XModifierKeymap *modmap;
  unsigned int NumLockMask = 0, ScrollLockMask = 0;
  size_t cnt;
  int i;
  int junk;
  (void) junk;

  self = PyObject_New(OtkDisplay, &OtkDisplay_Type);

  // Open the X display
  if (!(self->display = XOpenDisplay(name))) {
    printf(_("Unable to open connection to the X server. Please set the \n\
DISPLAY environment variable approriately, or use the '-display' command \n\
line argument.\n\n"));
    exit(1);
  }
  if (fcntl(ConnectionNumber(self->display), F_SETFD, 1) == -1) {
    printf(_("Couldn't mark display connection as close-on-exec.\n\n"));
    exit(1);
  }

  XSetErrorHandler(xerrorHandler);

  // set the DISPLAY environment variable for any lauched children, to the
  // display we're using, so they open in the right place.
  disp_env = PyString_FromFormat("DISPLAY=%s", DisplayString(self->display));
  if (putenv(PyString_AsString(disp_env))) {
    printf(_("warning: couldn't set environment variable 'DISPLAY'\n"));
    perror("putenv()");
  }
  Py_DECREF(disp_env);
  
  // find the availability of X extensions we like to use
#ifdef SHAPE
  self->shape = XShapeQueryExtension(self->display,
				     &self->shape_event_basep, &junk);
#endif

#ifdef XINERAMA
  self->xinerama = XineramaQueryExtension(self->display,
					  &self->xinerama_event_basep, &junk);
#endif // XINERAMA

  // get lock masks that are defined by the display (not constant)
  modmap = XGetModifierMapping(self->display);
  if (modmap && modmap->max_keypermod > 0) {
    const int mask_table[] = {
      ShiftMask, LockMask, ControlMask, Mod1Mask,
      Mod2Mask, Mod3Mask, Mod4Mask, Mod5Mask
    };
    const size_t size = (sizeof(mask_table) / sizeof(mask_table[0])) *
      modmap->max_keypermod;
    // get the values of the keyboard lock modifiers
    // Note: Caps lock is not retrieved the same way as Scroll and Num lock
    // since it doesn't need to be.
    const KeyCode num_lock = XKeysymToKeycode(self->display, XK_Num_Lock);
    const KeyCode scroll_lock = XKeysymToKeycode(self->display,
						 XK_Scroll_Lock);

    for (cnt = 0; cnt < size; ++cnt) {
      if (! modmap->modifiermap[cnt]) continue;

      if (num_lock == modmap->modifiermap[cnt])
        NumLockMask = mask_table[cnt / modmap->max_keypermod];
      if (scroll_lock == modmap->modifiermap[cnt])
        ScrollLockMask = mask_table[cnt / modmap->max_keypermod];
    }
  }

  if (modmap) XFreeModifiermap(modmap);

  self->mask_list[0] = 0;
  self->mask_list[1] = LockMask;
  self->mask_list[2] = NumLockMask;
  self->mask_list[3] = LockMask | NumLockMask;
  self->mask_list[4] = ScrollLockMask;
  self->mask_list[5] = ScrollLockMask | LockMask;
  self->mask_list[6] = ScrollLockMask | NumLockMask;
  self->mask_list[7] = ScrollLockMask | LockMask | NumLockMask;

  // set the global var, for the new screeninfo's
  OBDisplay = self;
  
  // Get information on all the screens which are available.
  self->screenInfoList = (PyListObject*)PyList_New(ScreenCount(self->display));
  for (i = 0; i < ScreenCount(self->display); ++i)
    PyList_SetItem((PyObject*)self->screenInfoList, i, OtkScreenInfo_New(i));

  Py_INCREF(OBDisplay); // make sure it stays around!!
}

void OtkDisplay_Grab(OtkDisplay *self)
{
  assert(self);
  if (self->grab_count == 0)
    XGrabServer(self->display);
  self->grab_count++;
}

void OtkDisplay_Ungrab(OtkDisplay *self)
{
  if (self->grab_count == 0) return;
  self->grab_count--;
  if (self->grab_count == 0)
    XUngrabServer(self->display);
}

OtkScreenInfo *OtkDisplay_ScreenInfo(OtkDisplay *self, int num)
{
  return (OtkScreenInfo*)PyList_GetItem((PyObject*)self->screenInfoList, num);
}


static PyObject *otkdisplay_grab(OtkDisplay* self, PyObject* args)
{
  if (!PyArg_ParseTuple(args, ":grab"))
    return NULL;
  OtkDisplay_Grab(self);
  return Py_None;
}

static PyObject *otkdisplay_ungrab(OtkDisplay* self, PyObject* args)
{
  if (!PyArg_ParseTuple(args, ":ungrab"))
    return NULL;
  OtkDisplay_Ungrab(self);
  return Py_None;
}

static PyMethodDef get_methods[] = {
  {"grab", (PyCFunction)otkdisplay_grab, METH_VARARGS,
   "Grab the X display."},
  {"ungrab", (PyCFunction)otkdisplay_ungrab, METH_VARARGS,
   "Ungrab/Release the X display."},
  {NULL, NULL, 0, NULL}
};



static void otkdisplay_dealloc(PyObject* self)
{
  XCloseDisplay(((OtkDisplay*) self)->display);
  Py_DECREF(((OtkDisplay*) self)->screenInfoList);
  PyObject_Del(self);
}

static PyObject *otkdisplay_getattr(PyObject *obj, char *name)
{
  return Py_FindMethod(get_methods, obj, name);
}


PyTypeObject OtkDisplay_Type = {
  PyObject_HEAD_INIT(NULL)
  0,
  "OtkDisplay",
  sizeof(OtkDisplay),
  0,
  otkdisplay_dealloc, /*tp_dealloc*/
  0,          /*tp_print*/
  otkdisplay_getattr, /*tp_getattr*/
  0,          /*tp_setattr*/
  0,          /*tp_compare*/
  0,          /*tp_repr*/
  0,          /*tp_as_number*/
  0,          /*tp_as_sequence*/
  0,          /*tp_as_mapping*/
  0,          /*tp_hash */
};


static int xerrorHandler(Display *d, XErrorEvent *e)
{
#ifdef DEBUG
  char errtxt[128];

  //if (e->error_code != BadWindow)
  {
    XGetErrorText(d, e->error_code, errtxt, 128);
    printf("X Error: %s\n", errtxt);
  }
#else
  (void)d;
  (void)e;
#endif

  return False;
}
