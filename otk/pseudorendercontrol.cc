// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

#include "pseudorendercontrol.hh"
#include "display.hh"
#include "screeninfo.hh"
#include "surface.hh"
#include "rendertexture.hh"

extern "C" {
#ifdef    HAVE_STDLIB_H
#  include <stdlib.h>
#endif // HAVE_STDLIB_H

#include "../src/gettext.h"
#define _(str) gettext(str)
}

namespace otk {

PseudoRenderControl::PseudoRenderControl(int screen)
  : RenderControl(screen)
{
  printf("Initializing PseudoColor RenderControl\n");
  const ScreenInfo *info = display->screenInfo(_screen);
  int depth = info->depth();

  // determine the number of colors and the bits-per-color
  int bpc = 2; // XXX THIS SHOULD BE A USER OPTION
  assert(bpc >= 1);
  _ncolors = 1 << (bpc * 3);

  if (_ncolors > 1 << depth) {
    fprintf(stderr,
            _("PseudoRenderControl: Invalid colormap size. Resizing.\n"));
    bpc = 1 << (depth/3) >> 3;
    _ncolors = 1 << (bpc * 3);
  }

  // build a color cube
  _colors = new XColor[_ncolors];

  int cpc = 1 << bpc; // colors per channel
  for (int n = _ncolors - 1,
         r = (1 << (bpc + 1)) -1, i = 0; i < cpc; r >>= 1, ++i)
    for (int g = (1 << (bpc + 1)) -1, j = 0; j < cpc; g >>= 1, ++j)
      for (int b = (1 << (bpc + 1)) -1, k = 0; k < cpc; b >>= 1, ++k, --n) {
        _colors[n].red = r | r << 8;
        _colors[n].green = g | g << 8;
        _colors[n].blue = b | b << 8;
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

  unsigned long *pixels = new unsigned long [ncolors], *p = pixels;
  for (int i = 0; i < _ncolors; ++i, ++p)
    *p = _colors[i].pixel;
  XFreeColors(**display, display->screenInfo(_screen)->colormap(), pixels,
              _ncolors, 0);
  delete [] colors;
}

void PseudoRenderControl::reduceDepth(Surface &sf, XImage *im) const
{
}

}
