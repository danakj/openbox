// -*- mode: C; indent-tabs-mode: nil; -*-

#include "../config.h"
#include "color.h"
#include "display.h"
#include "screeninfo.h"

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

static Bool cleancache = False;
static PyObject *colorcache;

// global color allocator/deallocator
typedef struct RGB {
  PyObject_HEAD
  int screen;
  int r, g, b;
} RGB;

static void rgb_dealloc(PyObject* self)
{
  PyObject_Del(self);
}

static int rgb_compare(PyObject *py1, PyObject *py2)
{
  long result;
  unsigned long p1, p2;
  RGB *r1, *r2;

  r1 = (RGB*) r1;
  r2 = (RGB*) r2;
  p1 = (r1->screen << 24 | r1->r << 16 | r1->g << 8 | r1->b) & 0x00ffffff;
  p2 = (r2->screen << 24 | r2->r << 16 | r2->g << 8 | r2->b) & 0x00ffffff;

  if (p1 < p2)
    result = -1;
  else if (p1 > p2)
    result = 1;
  else
    result = 0;
  return result;
}

static PyTypeObject RGB_Type = {
  PyObject_HEAD_INIT(NULL)
  0,
  "RGB",
  sizeof(RGB),
  0,
  rgb_dealloc, /*tp_dealloc*/
  0,          /*tp_print*/
  0,          /*tp_getattr*/
  0,          /*tp_setattr*/
  rgb_compare,          /*tp_compare*/
  0,          /*tp_repr*/
  0,          /*tp_as_number*/
  0,          /*tp_as_sequence*/
  0,          /*tp_as_mapping*/
  0,          /*tp_hash */
};

static PyObject *RGB_New(int screen, int r, int g, int b) {
  RGB *self = (RGB*) PyObject_New(RGB, &RGB_Type);
  self->screen = screen;
  self->r = r;
  self->g = g;
  self->b = b;
  return (PyObject*)self;
}

typedef struct PixelRef {
  unsigned long p;
  unsigned int count;
} PixelRef;

static PixelRef *PixelRef_New(unsigned long p) {
  PixelRef* self = malloc(sizeof(PixelRef));
  self->p = p;
  self->count = 1;
  return self;
}

static void OtkColor_ParseColorName(OtkColor *self) {
  XColor xcol;

  if (!self->colorname) {
    fprintf(stderr, "OtkColor: empty colorname, cannot parse (using black)\n");
    OtkColor_SetRGB(self, 0, 0, 0);
  }

  // get rgb values from colorname
  xcol.red = 0;
  xcol.green = 0;
  xcol.blue = 0;
  xcol.pixel = 0;

  if (!XParseColor(OBDisplay->display, self->colormap,
		   PyString_AsString(self->colorname), &xcol)) {
    fprintf(stderr, "BColor::allocate: color parse error: \"%s\"\n",
            PyString_AsString(self->colorname));
    OtkColor_SetRGB(self, 0, 0, 0);
    return;
  }

  OtkColor_SetRGB(self, xcol.red >> 8, xcol.green >> 8, xcol.blue >> 8);
}

static void OtkColor_DoCacheCleanup() {
  unsigned long *pixels;
  int i;
  unsigned int count;
  PyObject *rgb, *pixref;
  int ppos;

  // ### TODO - support multiple displays!
  if (!PyDict_Size(colorcache)) {
    // nothing to do
    return;
  }

  pixels = malloc(sizeof(unsigned long) * PyDict_Size(colorcache));

  for (i = 0; i < ScreenCount(OBDisplay->display); i++) {
    count = 0;
    ppos = 0;

    while (PyDict_Next(colorcache, &ppos, &rgb, &pixref)) {
      if (((PixelRef*)pixref)->count != 0 || ((RGB*)rgb)->screen != i)
        continue;

      pixels[count++] = ((PixelRef*)pixref)->p;
      PyDict_DelItem(colorcache, rgb);
      free(pixref); // not really a PyObject, it just pretends
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

static void OtkColor_Allocate(OtkColor *self) {
  XColor xcol;
  PyObject *rgb, *pixref;

  if (!OtkColor_IsValid(self)) {
    if (!self->colorname) {
      fprintf(stderr, "BColor: cannot allocate invalid color (using black)\n");
      OtkColor_SetRGB(self, 0, 0, 0);
    } else {
      OtkColor_ParseColorName(self);
    }
  }

  // see if we have allocated this color before
  rgb = RGB_New(self->screen, self->red, self->green, self->blue);
  pixref = PyDict_GetItem((PyObject*)colorcache, rgb);
  if (pixref) {
    // found
    self->allocated = True;
    self->pixel = ((PixelRef*)pixref)->p;
    ((PixelRef*)pixref)->count++;
    return;
  }

  // allocate color from rgb values
  xcol.red =   self->red   | self->red   << 8;
  xcol.green = self->green | self->green << 8;
  xcol.blue =  self->blue  | self->blue  << 8;
  xcol.pixel = 0;

  if (!XAllocColor(OBDisplay->display, self->colormap, &xcol)) {
    fprintf(stderr, "BColor::allocate: color alloc error: rgb:%x/%x/%x\n",
            self->red, self->green, self->blue);
    xcol.pixel = 0;
  }

  self->pixel = xcol.pixel;
  self->allocated = True;

  PyDict_SetItem(colorcache, rgb, (PyObject*)PixelRef_New(self->pixel));

  if (cleancache)
    OtkColor_DoCacheCleanup();
}

static void OtkColor_Deallocate(OtkColor *self) {
  PyObject *rgb, *pixref;

  if (!self->allocated)
    return;

  rgb = RGB_New(self->screen, self->red, self->green, self->blue);
  pixref = PyDict_GetItem(colorcache, rgb);
  if (pixref) {
    if (((PixelRef*)pixref)->count >= 1)
      ((PixelRef*)pixref)->count--;
  }

  if (cleancache)
    OtkColor_DoCacheCleanup();

  self->allocated = False;
}


OtkColor *OtkColor_New(int screen)
{
  OtkColor *self = malloc(sizeof(OtkColor));

  self->allocated = False;
  self->red = -1;
  self->green = -1;
  self->blue = -1;
  self->pixel = 0;
  self->screen = screen;
  self->colorname = NULL;
  self->colormap = OtkDisplay_ScreenInfo(OBDisplay, self->screen)->colormap;

  return self;
}

OtkColor *OtkColor_FromRGB(int r, int g, int b, int screen)
{
  OtkColor *self = malloc(sizeof(OtkColor));

  self->allocated = False;
  self->red = r;
  self->green = g;
  self->blue = b;
  self->pixel = 0;
  self->screen = screen;
  self->colorname = NULL;
  self->colormap = OtkDisplay_ScreenInfo(OBDisplay, self->screen)->colormap;

  return self;
}

OtkColor *OtkColor_FromName(const char *name, int screen)
{
  OtkColor *self = malloc(sizeof(OtkColor));

  self->allocated = False;
  self->red = -1;
  self->green = -1;
  self->blue = -1;
  self->pixel = 0;
  self->screen = screen;
  self->colorname = PyString_FromString(name);
  self->colormap = OtkDisplay_ScreenInfo(OBDisplay, self->screen)->colormap;

  return self;
}

void OtkColor_Destroy(OtkColor *self)
{
  if (self->colorname)
    Py_DECREF(self->colorname);
  free(self);
}

void OtkColor_SetRGB(OtkColor *self, int r, int g, int b)
{
  OtkColor_Deallocate(self);
  self->red = r;
  self->green = g;
  self->blue = b;
}

void OtkColor_SetScreen(OtkColor *self, int screen)
{
  if (screen == self->screen) {
    // nothing to do
    return;
  }

  Otk_Deallocate(self);
  
  self->colormap = OtkDisplay_ScreenInfo(OBDisplay, self->screen)->colormap;

  self->screen = screen;

  if (self->colorname)
    parseColorName();
}

Bool OtkColor_IsValid(OtkColor *self)
{
  return self->red != -1 && self->blue != -1 && self->green != -1;
}

unsigned long OtkColor_Pixel(OtkColor *self)
{
  if (!self->allocated)
    OtkColor_Allocate(self);
  return self->pixel;
}

void OtkColor_InitializeCache()
{
  colorcache = PyDict_New();
}

void OtkColor_DestroyCache()
{
  Py_DECREF(colorcache);
  colorcache = NULL;
}

void OtkColor_CleanupColorCache()
{
  cleancache = True;
}
