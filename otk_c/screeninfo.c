// -*- mode: C; indent-tabs-mode: nil; -*-

#include "../config.h"
#include "screeninfo.h"
#include "display.h"
#include "rect.h"

#include <X11/Xutil.h>

#ifdef HAVE_STRING_H
# include <string.h>
#endif

#include "../src/gettext.h"

extern PyTypeObject OtkScreenInfo_Type;

PyObject *OtkScreenInfo_New(int num)
{
  OtkScreenInfo* self;
  char *dstr, *dstr2;
  int i;

  self = PyObject_New(OtkScreenInfo, &OtkScreenInfo_Type);

  self->screen = num;
  self->root_window = RootWindow(OBDisplay->display, self->screen);
  self->rect = OtkRect_New(0, 0,
			   WidthOfScreen(ScreenOfDisplay(OBDisplay->display,
							 self->screen)),
			   HeightOfScreen(ScreenOfDisplay(OBDisplay->display,
							  self->screen)));
  
  /*
    If the default depth is at least 8 we will use that,
    otherwise we try to find the largest TrueColor visual.
    Preference is given to 24 bit over larger depths if 24 bit is an option.
  */

  self->depth = DefaultDepth(OBDisplay->display, self->screen);
  self->visual = DefaultVisual(OBDisplay->display, self->screen);
  self->colormap = DefaultColormap(OBDisplay->display, self->screen);
  
  if (self->depth < 8) {
    // search for a TrueColor Visual... if we can't find one...
    // we will use the default visual for the screen
    XVisualInfo vinfo_template, *vinfo_return;
    int vinfo_nitems;
    int best = -1;

    vinfo_template.screen = self->screen;
    vinfo_template.class = TrueColor;

    vinfo_return = XGetVisualInfo(OBDisplay->display,
                                  VisualScreenMask | VisualClassMask,
                                  &vinfo_template, &vinfo_nitems);
    if (vinfo_return) {
      int max_depth = 1;
      for (i = 0; i < vinfo_nitems; ++i) {
        if (vinfo_return[i].depth > max_depth) {
          if (max_depth == 24 && vinfo_return[i].depth > 24)
            break;          // prefer 24 bit over 32
          max_depth = vinfo_return[i].depth;
          best = i;
        }
      }
      if (max_depth < self->depth) best = -1;
    }

    if (best != -1) {
      self->depth = vinfo_return[best].depth;
      self->visual = vinfo_return[best].visual;
      self->colormap = XCreateColormap(OBDisplay->display, self->root_window,
				       self->visual, AllocNone);
    }

    XFree(vinfo_return);
  }

  // get the default display string and strip the screen number
  self->display_string =
    PyString_FromFormat("DISPLAY=%s",DisplayString(OBDisplay->display));
  dstr = PyString_AsString(self->display_string);
  dstr2 = strrchr(dstr, '.');
  if (dstr2) {
    PyObject *str;
    
    PyString_Resize(self->display_string, dstr2 - dstr);
    str = PyString_FromFormat(".%d", self->screen);
    PyString_Concat(&self->display_string, str);
  }

#ifdef    XINERAMA
  self->xinerama_active = False;

  if (OtkDisplay->hasXineramaExtensions()) {
    if (OtkDisplay->getXineramaMajorVersion() == 1) {
      // we know the version 1(.1?) protocol

      /*
        in this version of Xinerama, we can't query on a per-screen basis, but
        in future versions we should be able, so the 'activeness' is checked
        on a pre-screen basis anyways.
      */
      if (XineramaIsActive(OBDisplay->display)) {
        /*
          If Xinerama is being used, there there is only going to be one screen
          present. We still, of course, want to use the screen class, but that
          is why no screen number is used in this function call. There should
          never be more than one screen present with Xinerama active.
        */
        int num;
        XineramaScreenInfo *info = XineramaQueryScreens(OBDisplay->display,
                                                        &num);
        if (num > 0 && info) {
	  self->xinerama_areas = PyList_New(num);
          for (i = 0; i < num; ++i) {
	    PyList_SetItem(self->xinerama_areas, i,
			   OtkRect_New(info[i].x_org, info[i].y_org,
				       info[i].width, info[i].height));
          }
          XFree(info);

          // if we can't find any xinerama regions, then we act as if it is not
          // active, even though it said it was
          self->xinerama_active = True;
        }
      }
    }
  }
#endif // XINERAMA
    
  return (PyObject*)self;
}



static PyObject *otkscreeninfo_getscreen(OtkScreenInfo* self, PyObject* args)
{
  if (!PyArg_ParseTuple(args, ":getScreen"))
    return NULL;
  return PyInt_FromLong(self->screen);
}

static PyObject *otkscreeninfo_getrect(OtkScreenInfo* self, PyObject* args)
{
  if (!PyArg_ParseTuple(args, ":getRect"))
    return NULL;
  return self->rect;
}


static PyMethodDef get_methods[] = {
  {"getScreen", (PyCFunction)otkscreeninfo_getscreen, METH_VARARGS,
   "Get the screen number."},
  {"getRect", (PyCFunction)otkscreeninfo_getrect, METH_VARARGS,
   "Get the area taken up by the screen."},
  {NULL, NULL, 0, NULL}
};



static void otkscreeninfo_dealloc(PyObject* self)
{
  Py_DECREF(((OtkScreenInfo*) self)->display_string);
  Py_DECREF(((OtkScreenInfo*) self)->rect);
#ifdef XINERAMA
  Py_DECREF(((OtkScreenInfo*) self)->xinerama_areas);
#endif
  PyObject_Del(self);
}

static PyObject *otkscreeninfo_getattr(PyObject *obj, char *name)
{
  return Py_FindMethod(get_methods, obj, name);
}


PyTypeObject OtkScreenInfo_Type = {
  PyObject_HEAD_INIT(NULL)
  0,
  "OtkScreenInfo",
  sizeof(OtkScreenInfo),
  0,
  otkscreeninfo_dealloc, /*tp_dealloc*/
  0,          /*tp_print*/
  otkscreeninfo_getattr, /*tp_getattr*/
  0,          /*tp_setattr*/
  0,          /*tp_compare*/
  0,          /*tp_repr*/
  0,          /*tp_as_number*/
  0,          /*tp_as_sequence*/
  0,          /*tp_as_mapping*/
  0,          /*tp_hash */
};
