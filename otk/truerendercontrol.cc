// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "truerendercontrol.hh"
#include "display.hh"
#include "screeninfo.hh"

extern "C" {
#ifdef    HAVE_STDLIB_H
#  include <stdlib.h>
#endif // HAVE_STDLIB_H

#include "gettext.h"
#define _(str) gettext(str)
}

namespace otk {

TrueRenderControl::TrueRenderControl(const ScreenInfo *screen)
  : RenderControl(screen)
{
  printf("Initializing TrueColor RenderControl\n");

  unsigned long red_mask, green_mask, blue_mask;

  // find the offsets for each color in the visual's masks
  red_mask = screen->visual()->red_mask;
  green_mask = screen->visual()->green_mask;
  blue_mask = screen->visual()->blue_mask;

  while (! (red_mask & 1)) { _red_offset++; red_mask >>= 1; }
  while (! (green_mask & 1)) { _green_offset++; green_mask >>= 1; }
  while (! (blue_mask & 1)) { _blue_offset++; blue_mask >>= 1; }

  // use the mask to determine the number of bits for each shade of color
  // so, best case, red_mask == 0xff (255), with each bit as a different
  // shade!
  _red_bits = 255 / red_mask;
  _green_bits = 255 / green_mask;
  _blue_bits = 255 / blue_mask;

  // compute color tables, based on the number of bits for each shade
  for (int i = 0; i < 256; i++) {
    _red_color_table[i] = i / _red_bits;
    _green_color_table[i] = i / _green_bits;
    _blue_color_table[i] = i / _blue_bits;
  }
}

TrueRenderControl::~TrueRenderControl()
{
  printf("Destroying TrueColor RenderControl\n");


}

void TrueRenderControl::render(::Drawable d)
{
  XGCValues gcv;
  gcv.cap_style = CapProjecting;

  int w = 255, h = 32;
  Pixmap p = XCreatePixmap(**display, d, w, h, _screen->depth());
  XImage *im = XCreateImage(**display, _screen->visual(), _screen->depth(),
			    ZPixmap, 0, NULL, w, h, 32, 0);
  //GC gc = XCreateGC(**display, _screen->rootWindow(), GCCapStyle, &gcv);

  im->data = new char[im->bytes_per_line * (h + 1)]; // XXX  + 1?
  char *dp = im->data;

  for (int x = 0; x < w; ++x, ++dp)
    *dp = 0;
  for (int y = 0; y < 10; ++h)
    for (int x = 0; x < w; ++x, ++dp)
      *dp = _red_color_table[x] << _red_offset;
  for (int y = 0; y < 10; ++h)
    for (int x = 0; x < w; ++x, ++dp)
      *dp = _green_color_table[x] << _green_offset;
  for (int y = 0; y < 10; ++h)
    for (int x = 0; x < w; ++x, ++dp)
      *dp = _blue_color_table[x] << _blue_offset;
  for (int x = 0; x < w; ++x, ++dp)
    *dp = 0;

  XPutImage(**display, p, DefaultGC(**display, _screen->screen()),
            im, 0, 0, 0, 0, w, h);

  //delete [] image->data;
  //image->data = NULL;
  XDestroyImage(im);

  XCopyArea(**display, p, d, DefaultGC(**display, _screen->screen()),
	    0, 0, w, h,  0, 0);

  XFreePixmap(**display, p);
}

}
