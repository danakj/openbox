// -*- mode: C; indent-tabs-mode: nil; -*-

#include "../config.h"
#include "font.h"
#include "display.h"
#include "color.h"

#include "../src/gettext.h"

static Bool xft_init = False;
static const char *fallback = "fixed";

void OtkFont_Initialize()
{
  if (!XftInit(0)) {
    printf(_("Couldn't initialize Xft version %d.%d.%d.\n\n"),
	   XFT_MAJOR, XFT_MINOR, XFT_REVISION);
    exit(3);
  }
  printf(_("Using Xft %d.%d.%d.\n"), XFT_MAJOR, XFT_MINOR, XFT_REVISION);
  xft_init = True;
}

PyObject *OtkFont_New(int screen, const char *fontstring, Bool shadow,
		      unsigned char offset, unsigned char tint)
{
  OtkFont *self = PyObject_New(OtkFont, &OtkFont_Type);

  assert(xft_init);
  assert(screen >= 0);
  assert(fontstring);
  
  self->screen = screen;
  self->shadow = shadow;
  self->offset = offset;
  self->tint   = tint;

  if (!(self->xftfont = XftFontOpenName(OBDisplay->display, screen,
					fontstring))) {
    printf(_("Unable to load font: %s"), fontstring);
    printf(_("Trying fallback font: %s\n"), fallback);
    if (!(self->xftfont =
	  XftFontOpenName(OBDisplay->display, screen, fallback))) {
      printf(_("Unable to load font: %s"), fallback);
      printf(_("Aborting!.\n"));
      
      exit(3); // can't continue without a font
    }
  }

  return (PyObject*)self;
}

int OtkFont_MeasureString(OtkFont *self, const char *string)//, Bool utf8)
{
  XGlyphInfo info;

/*  if (utf8)*/
    XftTextExtentsUtf8(OBDisplay->display, self->xftfont,
                       (const FcChar8*)string, strlen(string), &info);
/*  else
    XftTextExtents8(OBDisplay->display, self->xftfont,
    (const FcChar8*)string, strlen(string), &info);*/

  return info.xOff + (self->shadow ? self->offset : 0);
}

void OtkFont_DrawString(OtkFont *self, XftDraw *d, int x, int y,
			OtkColor *color, const char *string)//, Bool utf8)
{
  assert(self);
  assert(d);

  if (self->shadow) {
    XftColor c;
    c.color.red = 0;
    c.color.green = 0;
    c.color.blue = 0;
    c.color.alpha = self->tint | self->tint << 8; // transparent shadow
    c.pixel = BlackPixel(OBDisplay->display, self->screen);

/*    if (utf8)*/
      XftDrawStringUtf8(d, &c, self->xftfont, x + self->offset,
                        self->xftfont->ascent + y + self->offset,
                        (const FcChar8*)string, strlen(string));
/*    else
      XftDrawString8(d, &c, self->xftfont, x + self->offset,
                     self->xftfont->ascent + y + self->offset,
                     (const FcChar8*)string, strlen(string));*/
  }
    
  XftColor c;
  c.color.red   = color->red   | color->red   << 8;
  c.color.green = color->green | color->green << 8;
  c.color.blue  = color->blue  | color->blue  << 8;
  c.pixel = color->pixel;
  c.color.alpha = 0xff | 0xff << 8; // no transparency in BColor yet

/*  if (utf8)*/
    XftDrawStringUtf8(d, &c, self->xftfont, x, self->xftfont->ascent + y,
                      (const FcChar8*)string, strlen(string));
/*  else
    XftDrawString8(d, &c, self->xftfont, x, self->xftfont->ascent + y,
    (const FcChar8*)string, strlen(string));*/
}




static PyObject *otkfont_measurestring(OtkFont* self, PyObject* args)
{
  char *s;
  
  if (!PyArg_ParseTuple(args, "s", &s))
    return NULL;
  return PyInt_FromLong(OtkFont_MeasureString(self, s));
}

static PyMethodDef get_methods[] = {
  {"measureString", (PyCFunction)otkfont_measurestring, METH_VARARGS,
   "Measure the length of a string with a font."},
  {NULL, NULL, 0, NULL}
};


static void otkfont_dealloc(OtkFont* self)
{
  // this is always set. cuz if it failed.. the app would exit!
  XftFontClose(OBDisplay->display, self->xftfont);
  PyObject_Del((PyObject*)self);
}

static PyObject *otkfont_getattr(PyObject *obj, char *name)
{
  return Py_FindMethod(get_methods, obj, name);
}

PyTypeObject Otkfont_Type = {
  PyObject_HEAD_INIT(NULL)
  0,
  "OtkFont",
  sizeof(OtkFont),
  0,
  (destructor)otkfont_dealloc,  /*tp_dealloc*/
  0,                            /*tp_print*/
  otkfont_getattr,              /*tp_getattr*/
  0,                            /*tp_setattr*/
  0,                            /*tp_compare*/
  0,                            /*tp_repr*/
  0,                            /*tp_as_number*/
  0,                            /*tp_as_sequence*/
  0,                            /*tp_as_mapping*/
  0,                            /*tp_hash */
};
