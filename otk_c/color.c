// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "../config.h"
#include "color.h"
#include "display.h"
#include "screeninfo.h"

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

static Bool cleancache = False;
static PyObject *colorcache = NULL;

static void parseColorName(OtkColor *self, const char *name) {
  XColor xcol;

  // get rgb values from colorname
  xcol.red = 0;
  xcol.green = 0;
  xcol.blue = 0;
  xcol.pixel = 0;

  if (!XParseColor(OBDisplay->display,
                   OtkDisplay_ScreenInfo(OBDisplay, self->screen)->colormap,
                   name, &xcol)) {
    fprintf(stderr, "OtkColor: color parse error: \"%s\"\n", name);
    self->red = self->green = self->blue = 0;
  } else {
    self->red = xcol.red >> 8;
    self->green = xcol.green >> 8;
    self->blue = xcol.blue >> 8;
  }
}

static void doCacheCleanup() {
  unsigned long *pixels;
  int i, ppos;
  unsigned int count;
  PyObject *key; // this is a color too, but i dont need to use it as such
  OtkColor *color;

  // ### TODO - support multiple displays!
  if (!PyDict_Size(colorcache)) return; // nothing to do

  pixels = malloc(sizeof(unsigned long) * PyDict_Size(colorcache));

  for (i = 0; i < ScreenCount(OBDisplay->display); i++) {
    count = 0;
    ppos = 0;

    while (PyDict_Next(colorcache, &ppos, &key, (PyObject**)&color)) {
      // get the screen from the hash
      if (color->screen != i) continue; // wrong screen

      // does someone other than the cache have a reference? (the cache gets 2)
      if (color->ob_refcnt > 2)
        continue;

      pixels[count++] = color->pixel;
      PyDict_DelItem(colorcache, key);
      --ppos; // back up one in the iteration
    }

    if (count > 0)
      XFreeColors(OBDisplay->display,
                  OtkDisplay_ScreenInfo(OBDisplay, i)->colormap,
                  pixels, count, 0);
  }

  free(pixels);
  cleancache = False;
}

static void allocate(OtkColor *self) {
  XColor xcol;

  // allocate color from rgb values
  xcol.red =   self->red   | self->red   << 8;
  xcol.green = self->green | self->green << 8;
  xcol.blue =  self->blue  | self->blue  << 8;
  xcol.pixel = 0;
  
  if (!XAllocColor(OBDisplay->display,
                   OtkDisplay_ScreenInfo(OBDisplay, self->screen)->colormap,
                   &xcol)) {
    fprintf(stderr, "OtkColor: color alloc error: rgb:%x/%x/%x\n",
            self->red, self->green, self->blue);
    xcol.pixel = 0;
  }
  
  self->pixel = xcol.pixel;
  
  if (cleancache)
    doCacheCleanup();
}

PyObject *OtkColor_FromRGB(int r, int g, int b, int screen)
{
  OtkColor *self = PyObject_New(OtkColor, &OtkColor_Type);
  PyObject *cached;

  assert(screen >= 0); assert(r >= 0); assert(g >= 0); assert(b >= 0);
  assert(r <= 0xff); assert(g <= 0xff); assert(b <= 0xff);

  if (!colorcache) colorcache = PyDict_New();

  self->red = r;
  self->green = g;
  self->blue = b;
  self->screen = screen;

  // does this color already exist in the cache?
  cached = PyDict_GetItem(colorcache, (PyObject*)self);
  if (cached) {
    Py_INCREF(cached);
    return cached;
  }

  // add it to the cache
  PyDict_SetItem(colorcache, (PyObject*)self, (PyObject*)self);
  allocate(self);
  return (PyObject*)self;
}

PyObject *OtkColor_FromName(const char *name, int screen)
{
  OtkColor *self = PyObject_New(OtkColor, &OtkColor_Type);
  PyObject *cached;

  assert(screen >= 0); assert(name);

  if (!colorcache) colorcache = PyDict_New();
  
  self->red = -1;
  self->green = -1;
  self->blue = -1;
  self->screen = screen;

  parseColorName(self, name);

  // does this color already exist in the cache?
  cached = PyDict_GetItem(colorcache, (PyObject*)self);
  if (cached) {
    Py_INCREF(cached);
    return cached;
  }

  // add it to the cache
  PyDict_SetItem(colorcache, (PyObject*)self, (PyObject*)self);
  allocate(self);
  return (PyObject*)self;
}

void OtkColor_CleanupColorCache()
{
  cleancache = True;
}



static void otkcolor_dealloc(OtkColor* self)
{
  // when this is called, the color has already been cleaned out of the cache
  PyObject_Del((PyObject*)self);
}

static int otkcolor_compare(OtkColor *c1, OtkColor *c2)
{
  long result;
  unsigned long p1, p2;

  p1 = c1->red << 16 | c1->green << 8 | c1->blue;
  p2 = c2->red << 16 | c2->green << 8 | c2->blue;

  if (p1 < p2)
    result = -1;
  else if (p1 > p2)
    result = 1;
  else
    result = 0;
  return result;
}

static PyObject *otkcolor_repr(OtkColor *self)
{
  return PyString_FromFormat("rgb:%x/%x/%x", self->red, self->green,
                             self->blue);
}

static long otkcolor_hash(OtkColor *self)
{
  return self->screen << 24 | self->red << 16 | self->green << 8 | self->blue;
}

PyTypeObject OtkColor_Type = {
  PyObject_HEAD_INIT(NULL)
  0,
  "OtkColor",
  sizeof(OtkColor),
  0,
  (destructor)otkcolor_dealloc, /*tp_dealloc*/
  0,                            /*tp_print*/
  0,                            /*tp_getattr*/
  0,                            /*tp_setattr*/
  (cmpfunc)otkcolor_compare,    /*tp_compare*/
  (reprfunc)otkcolor_repr,      /*tp_repr*/
  0,                            /*tp_as_number*/
  0,                            /*tp_as_sequence*/
  0,                            /*tp_as_mapping*/
  (hashfunc)otkcolor_hash,      /*tp_hash */
};
