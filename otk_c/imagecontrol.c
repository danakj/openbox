// -*- mode: C; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "../config.h"
#include "imagecontrol.h"
#include "timer.h"
#include "screeninfo.h"
#include "display.h"

typedef struct CachedImage {
  Pixmap pixmap;
  
  unsigned int count, width, height;
  unsigned long pixel1, pixel2, texture;
} CachedImage;

static void timeout(OtkImageControl *self);
static void initColors(OtkImageControl *self, Visual *visual);

PyObject *OtkImageControl_New(int screen)
{
  OtkImageControl *self;
  int count, i;
  XPixmapFormatValues *pmv;

  self = PyObject_New(OtkImageControl, &OtkImageControl_Type);

  self->screen = OtkDisplay_ScreenInfo(OBDisplay, screen);

  self->timer = (OtkTimer*)OtkTimer_New((OtkTimeoutHandler)timeout, self);
  self->timer->timeout = 300000;
  OtkTimer_Start(self->timer);
  self->cache_max = 200;
  
  self->dither = True; // default value
  self->cpc = 4; // default value

  // get the BPP from the X server
  self->bpp = 0;
  if ((pmv = XListPixmapFormats(OBDisplay->display, &count))) {
    for (i = 0; i < count; i++)
      if (pmv[i].depth == self->screen->depth) {
	self->bpp = pmv[i].bits_per_pixel;
	break;
      }
    XFree(pmv);
  }
  if (!self->bpp) self->bpp = self->screen->depth;
  if (self->bpp >= 24) self->dither = False; // don't need dither at >= 24 bpp

  self->grad_xbuffer = self->grad_ybuffer = NULL;
  self->grad_buffer_width = self->grad_buffer_height = 0;
  self->sqrt_table = NULL;

  initColors(self, self->screen->visual);

  return (PyObject*)self;
}

static void initColors(OtkImageControl *self, Visual *visual)
{
  // these are not used for !TrueColor
  self->red_offset = self->green_offset = self->blue_offset = 0;
  // these are not used for TrueColor
  self->colors = NULL;
  self->ncolors = 0;

  // figure out all our color settings based on the visual type
  switch (visual->class) {
  case TrueColor: {
    int i;
    unsigned long red_mask, green_mask, blue_mask;

    // find the offsets for each color in the visual's masks
    red_mask = visual->red_mask;
    green_mask = visual->green_mask;
    blue_mask = visual->blue_mask;

    while (! (red_mask & 1)) { self->red_offset++; red_mask >>= 1; }
    while (! (green_mask & 1)) { self->green_offset++; green_mask >>= 1; }
    while (! (blue_mask & 1)) { self->blue_offset++; blue_mask >>= 1; }

    // use the mask to determine the number of bits for each shade of color
    // so, best case, red_mask == 0xff (255), and so each bit is a different
    // shade!
    self->red_bits = 255 / red_mask;
    self->green_bits = 255 / green_mask;
    self->blue_bits = 255 / blue_mask;

    // compute color tables, based on the number of bits for each shade
    for (i = 0; i < 256; i++) {
      self->red_color_table[i] = i / self->red_bits;
      self->green_color_table[i] = i / self->green_bits;
      self->blue_color_table[i] = i / self->blue_bits;
    }
    break;
  }
/*
  case PseudoColor:
  case StaticColor: {
    ncolors = self->cpc * self->cpc * self->cpc; // cpc ^ 3

    if (ncolors > (1 << self->screen->depth)) {
      self->cpc = (1 << self->screen->depth) / 3;
      ncolors = self->cpc * self->cpc * self->cpc; // cpc ^ 3
    }

    if (self->cpc < 2 || self->ncolors > (1 << self->screen->depth)) {
      fprintf(stderr,
	      "OtkImageControl_New: invalid colormap size %d "
              "(%d/%d/%d) - reducing",
	      self->ncolors, self->cpc, self->cpc, self->cpc);

      self->cpc = (1 << self->screen->depth) / 3;
    }

    self->colors = malloc(sizeof(XColor) * self->ncolors);
    if (! self->colors) {
      fprintf(stderr, "OtkImageControl_New: error allocating colormap\n");
      exit(1);
    }

    int i = 0, ii, p, r, g, b,
      bits = 255 / (colors_per_channel - 1);

    red_bits = green_bits = blue_bits = bits;

    for (i = 0; i < 256; i++)
      red_color_table[i] = green_color_table[i] = blue_color_table[i] =
	i / bits;

    for (r = 0, i = 0; r < colors_per_channel; r++)
      for (g = 0; g < colors_per_channel; g++)
	for (b = 0; b < colors_per_channel; b++, i++) {
	  colors[i].red = (r * 0xffff) / (colors_per_channel - 1);
	  colors[i].green = (g * 0xffff) / (colors_per_channel - 1);
	  colors[i].blue = (b * 0xffff) / (colors_per_channel - 1);;
	  colors[i].flags = DoRed|DoGreen|DoBlue;
	}

    for (i = 0; i < ncolors; i++) {
      if (! XAllocColor(OBDisplay::display, colormap, &colors[i])) {
	fprintf(stderr, "couldn't alloc color %i %i %i\n",
		colors[i].red, colors[i].green, colors[i].blue);
	colors[i].flags = 0;
      } else {
	colors[i].flags = DoRed|DoGreen|DoBlue;
      }
    }

    XColor icolors[256];
    int incolors = (((1 << screen_depth) > 256) ? 256 : (1 << screen_depth));

    for (i = 0; i < incolors; i++)
      icolors[i].pixel = i;

    XQueryColors(OBDisplay::display, colormap, icolors, incolors);
    for (i = 0; i < ncolors; i++) {
      if (! colors[i].flags) {
	unsigned long chk = 0xffffffff, pixel, close = 0;

	p = 2;
	while (p--) {
	  for (ii = 0; ii < incolors; ii++) {
	    r = (colors[i].red - icolors[i].red) >> 8;
	    g = (colors[i].green - icolors[i].green) >> 8;
	    b = (colors[i].blue - icolors[i].blue) >> 8;
	    pixel = (r * r) + (g * g) + (b * b);

	    if (pixel < chk) {
	      chk = pixel;
	      close = ii;
	    }

	    colors[i].red = icolors[close].red;
	    colors[i].green = icolors[close].green;
	    colors[i].blue = icolors[close].blue;

	    if (XAllocColor(OBDisplay::display, colormap,
			    &colors[i])) {
	      colors[i].flags = DoRed|DoGreen|DoBlue;
	      break;
	    }
	  }
	}
      }
    }

    break;
  }

  case GrayScale:
  case StaticGray: {
    if (visual->c_class == StaticGray) {
      ncolors = 1 << screen_depth;
    } else {
      ncolors = colors_per_channel * colors_per_channel * colors_per_channel;

      if (ncolors > (1 << screen_depth)) {
	colors_per_channel = (1 << screen_depth) / 3;
	ncolors =
	  colors_per_channel * colors_per_channel * colors_per_channel;
      }
    }

    if (colors_per_channel < 2 || ncolors > (1 << screen_depth)) {
      fprintf(stderr,
              "BImageControl::BImageControl: invalid colormap size %d "
              "(%d/%d/%d) - reducing",
	      ncolors, colors_per_channel, colors_per_channel,
	      colors_per_channel);

      colors_per_channel = (1 << screen_depth) / 3;
    }

    colors = new XColor[ncolors];
    if (! colors) {
      fprintf(stderr,
              "BImageControl::BImageControl: error allocating colormap\n");
      exit(1);
    }

    int i = 0, ii, p, bits = 255 / (colors_per_channel - 1);
    red_bits = green_bits = blue_bits = bits;

    for (i = 0; i < 256; i++)
      red_color_table[i] = green_color_table[i] = blue_color_table[i] =
	i / bits;

    for (i = 0; i < ncolors; i++) {
      colors[i].red = (i * 0xffff) / (colors_per_channel - 1);
      colors[i].green = (i * 0xffff) / (colors_per_channel - 1);
      colors[i].blue = (i * 0xffff) / (colors_per_channel - 1);;
      colors[i].flags = DoRed|DoGreen|DoBlue;

      if (! XAllocColor(OBDisplay::display, colormap,
			&colors[i])) {
	fprintf(stderr, "couldn't alloc color %i %i %i\n",
		colors[i].red, colors[i].green, colors[i].blue);
	colors[i].flags = 0;
      } else {
	colors[i].flags = DoRed|DoGreen|DoBlue;
      }
    }

    XColor icolors[256];
    int incolors = (((1 << screen_depth) > 256) ? 256 :
		    (1 << screen_depth));

    for (i = 0; i < incolors; i++)
      icolors[i].pixel = i;

    XQueryColors(OBDisplay::display, colormap, icolors, incolors);
    for (i = 0; i < ncolors; i++) {
      if (! colors[i].flags) {
	unsigned long chk = 0xffffffff, pixel, close = 0;

	p = 2;
	while (p--) {
	  for (ii = 0; ii < incolors; ii++) {
	    int r = (colors[i].red - icolors[i].red) >> 8;
	    int g = (colors[i].green - icolors[i].green) >> 8;
	    int b = (colors[i].blue - icolors[i].blue) >> 8;
	    pixel = (r * r) + (g * g) + (b * b);

	    if (pixel < chk) {
	      chk = pixel;
	      close = ii;
	    }

	    colors[i].red = icolors[close].red;
	    colors[i].green = icolors[close].green;
	    colors[i].blue = icolors[close].blue;

	    if (XAllocColor(OBDisplay::display, colormap,
			    &colors[i])) {
	      colors[i].flags = DoRed|DoGreen|DoBlue;
	      break;
	    }
	  }
	}
      }
    }

    break;
  }
*/
  default:
    fprintf(stderr, "OtkImageControl: unsupported visual class: %d\n",
	    visual->class);
    exit(1);
  }
}


static void timeout(OtkImageControl *self)
{
  (void)self;
}



static void otkimagecontrol_dealloc(OtkImageControl* self)
{
  Py_DECREF(self->screen);
  Py_DECREF(self->timer);
  PyObject_Del((PyObject*)self);
}

PyTypeObject OtkImageControl_Type = {
  PyObject_HEAD_INIT(NULL)
  0,
  "OtkImageControl",
  sizeof(OtkImageControl),
  0,
  (destructor)otkimagecontrol_dealloc, /*tp_dealloc*/
  0,                            /*tp_print*/
  0,                            /*tp_getattr*/
  0,                            /*tp_setattr*/
  0,                            /*tp_compare*/
  0,                            /*tp_repr*/
  0,                            /*tp_as_number*/
  0,                            /*tp_as_sequence*/
  0,                            /*tp_as_mapping*/
  0,                            /*tp_hash */
};
