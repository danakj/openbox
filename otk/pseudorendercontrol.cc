// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

#include "pseudorendercontrol.hh"
#include "display.hh"
#include "screeninfo.hh"
#include "surface.hh"
#include "rendertexture.hh"

extern "C" {
#include "../src/gettext.h"
#define _(str) gettext(str)
}

#include <cstdlib>

namespace otk {

PseudoRenderControl::PseudoRenderControl(int screen)
  : RenderControl(screen)
{
  printf("Initializing PseudoColor RenderControl\n");
  const ScreenInfo *info = display->screenInfo(_screen);
  int depth = info->depth();

  // determine the number of colors and the bits-per-color
  _bpc = 2; // XXX THIS SHOULD BE A USER OPTION
  assert(_bpc >= 1);
  _ncolors = 1 << (_bpc * 3);

  if (_ncolors > 1 << depth) {
    fprintf(stderr,
            _("PseudoRenderControl: Invalid colormap size. Resizing.\n"));
    _bpc = 1 << (depth/3) >> 3;
    _ncolors = 1 << (_bpc * 3);
  }

  // build a color cube
  _colors = new XColor[_ncolors];
int tr, tg, tb;
  int cpc = 1 << _bpc; // colors per channel
  for (int n = 0,
         r = 0; r < cpc; r++)
    for (int g = 0; g < cpc; g++)
      for (int b = 0; b < cpc; b++, n++) {
        tr = (int)(((float)(r)/(float)(cpc-1)) * 0xFF);
        tg = (int)(((float)(g)/(float)(cpc-1)) * 0xFF);
        tb = (int)(((float)(b)/(float)(cpc-1)) * 0xFF);
        _colors[n].red = tr | tr << 8;
        _colors[n].green = tg | tg << 8;
        _colors[n].blue = tb | tb << 8;
        _colors[n].flags = DoRed|DoGreen|DoBlue; // used to track allocation
      }

  // allocate the colors
  for (int i = 0; i < _ncolors; i++)
    if (!XAllocColor(**display, info->colormap(), &_colors[i]))
      _colors[i].flags = 0; // mark it as unallocated

  // try allocate any colors that failed allocation above

  // get the allocated values from the X server (only the first 256 XXX why!?)
  XColor icolors[256];
  int incolors = (((1 << depth) > 256) ? 256 : (1 << depth));
  for (int i = 0; i < incolors; i++)
    icolors[i].pixel = i;
  XQueryColors(**display, info->colormap(), icolors, incolors);

  // try match unallocated ones
  for (int i = 0; i < _ncolors; i++) {
    if (!_colors[i].flags) { // if it wasn't allocated...
      unsigned long closest = 0xffffffff, close = 0;
      for (int ii = 0; ii < incolors; ii++) {
        // find deviations
        int r = (_colors[i].red - icolors[ii].red) & 0xff;
        int g = (_colors[i].green - icolors[ii].green) & 0xff;
        int b = (_colors[i].blue - icolors[ii].blue) & 0xff;
        // find a weighted absolute deviation
        unsigned long dev = (r * r) + (g * g) + (b * b);

        if (dev < closest) {
          closest = dev;
          close = ii;
        }
      }

      _colors[i].red = icolors[close].red;
      _colors[i].green = icolors[close].green;
      _colors[i].blue = icolors[close].blue;
      _colors[i].pixel = icolors[close].pixel;

      // try alloc this closest color, it had better succeed!
      if (XAllocColor(**display, info->colormap(), &_colors[i]))
        _colors[i].flags = DoRed|DoGreen|DoBlue; // mark as alloced
      else
        assert(false); // wtf has gone wrong, its already alloced for chissake!
    }
  }
}

PseudoRenderControl::~PseudoRenderControl()
{
  printf("Destroying PseudoColor RenderControl\n");

  unsigned long *pixels = new unsigned long [_ncolors], *p = pixels;
  for (int i = 0; i < _ncolors; ++i, ++p)
    *p = _colors[i].pixel;
  XFreeColors(**display, display->screenInfo(_screen)->colormap(), pixels,
              _ncolors, 0);
  delete [] _colors;
}

inline const XColor *PseudoRenderControl::pickColor(int r, int g, int b) const
{
  r = (r & 0xff) >> (8-_bpc);
  g = (g & 0xff) >> (8-_bpc);
  b = (b & 0xff) >> (8-_bpc);
  return &_colors[(r << (2*_bpc)) + (g << (1*_bpc)) + b];
}

void PseudoRenderControl::reduceDepth(Surface &sf, XImage *im) const
{
  pixel32 *data = sf.pixelData();
  pixel32 *ret = (pixel32*)malloc(im->width * im->height * 4);
  char *p = (char *)ret;
  int x, y;
  for (y = 0; y < im->height; y++) {
    for (x = 0; x < im->width; x++) {
      p[x] = pickColor(data[x] >> default_red_shift,
                       data[x] >> default_green_shift,
                       data[x] >> default_blue_shift)->pixel;
    }
    data += im->width;
    p += im->bytes_per_line;
  }
  im->data = (char*)ret;
}

void PseudoRenderControl::allocateColor(XColor *color) const
{
  const XColor *c = pickColor(color->red, color->blue, color->green);

  color->red = c->red;
  color->green = c->green;
  color->blue = c->blue;
  color->pixel = c->pixel;

  if (XAllocColor(**display, display->screenInfo(_screen)->colormap(), color))
    color->flags = DoRed|DoGreen|DoBlue; // mark as alloced
  else
    assert(false); // wtf has gone wrong, its already alloced for chissake!

  return;
}

}
