// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "truerendercontrol.hh"
#include "display.hh"
#include "screeninfo.hh"
#include "widget.hh"

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

  // scale available colorspace to match our 256x256x256 cube
  _red_bits = 255 / red_mask;
  _green_bits = 255 / green_mask;
  _blue_bits = 255 / blue_mask;

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

static inline void renderPixel(XImage *im, unsigned char *dp,
			       unsigned long pixel)
{
  unsigned int bpp = im->bits_per_pixel + (im->byte_order == MSBFirst) ? 1 : 0;

  switch (bpp) {
  case  8: //  8bpp
    *dp++ = pixel;
    break;
  case 16: // 16bpp LSB
    *dp++ = pixel;
    *dp++ = pixel >> 8;
    break;
  case 17: // 16bpp MSB
    *dp++ = pixel >> 8;
    *dp++ = pixel;
    break;
  case 24: // 24bpp LSB
    *dp++ = pixel;
    *dp++ = pixel >> 8;
    *dp++ = pixel >> 16;
    break;
  case 25: // 24bpp MSB
    *dp++ = pixel >> 16;
    *dp++ = pixel >> 8;
    *dp++ = pixel;
    break;
  case 32: // 32bpp LSB
    *dp++ = pixel;
    *dp++ = pixel >> 8;
    *dp++ = pixel >> 16;
    *dp++ = pixel >> 24;
    break;
  case 33: // 32bpp MSB
    *dp++ = pixel >> 24;
    *dp++ = pixel >> 16;
    *dp++ = pixel >> 8;
    *dp++ = pixel;
    break;
  default:
    assert(false); // wtf?
  }
}

void TrueRenderControl::render(Widget *wi)
{
  assert(wi);
  
  XGCValues gcv;
  gcv.cap_style = CapProjecting;

  int w = 255, h = 31;
  Pixmap p = XCreatePixmap(**display, wi->window(), w, h, _screen->depth());
  XImage *im = XCreateImage(**display, _screen->visual(), _screen->depth(),
			    ZPixmap, 0, NULL, w, h, 32, 0);
  //GC gc = XCreateGC(**display, _screen->rootWindow(), GCCapStyle, &gcv);

  // XXX  + 1?
  unsigned char *data = new unsigned char[im->bytes_per_line * h];
  unsigned char *dp = data;

  for (int x = 0; x < w; ++x, dp += im->bits_per_pixel/8)
    renderPixel(im, dp, 0);
  for (int y = 0; y < 10; ++y)
    for (int x = 0; x < w; ++x, dp += im->bits_per_pixel/8)
      renderPixel(im, dp, _red_color_table[x] << _red_offset);
  for (int y = 0; y < 10; ++y)
    for (int x = 0; x < w; ++x, dp += im->bits_per_pixel/8)
      renderPixel(im, dp, _green_color_table[x] << _green_offset);
  for (int y = 0; y < 10; ++y)
    for (int x = 0; x < w; ++x, dp += im->bits_per_pixel/8)
      renderPixel(im, dp, _blue_color_table[x] << _blue_offset);

  printf("\nDone %d %d\n", im->bytes_per_line * h, dp - data);

  im->data = (char*) data;
  
  XPutImage(**display, p, DefaultGC(**display, _screen->screen()),
            im, 0, 0, 0, 0, w, h);

  //delete [] image->data;
  //image->data = NULL;
  XDestroyImage(im);

  XSetWindowBackgroundPixmap(**display, wi->window(), p);
  XClearWindow(**display, wi->window());

  XFreePixmap(**display, p);
}

}
