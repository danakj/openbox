// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "truerendercontrol.hh"
#include "display.hh"
#include "screeninfo.hh"
#include "surface.hh"
#include "rendertexture.hh"

extern "C" {
#ifdef    HAVE_STDLIB_H
#  include <stdlib.h>
#endif // HAVE_STDLIB_H

#include "gettext.h"
#define _(str) gettext(str)
}

namespace otk {

TrueRenderControl::TrueRenderControl(int screen)
  : RenderControl(screen),
    _red_offset(0),
    _green_offset(0),
    _blue_offset(0)
{
  printf("Initializing TrueColor RenderControl\n");

  Visual *visual = display->screenInfo(_screen)->visual();
  unsigned long red_mask, green_mask, blue_mask;

  // find the offsets for each color in the visual's masks
  red_mask = visual->red_mask;
  green_mask = visual->green_mask;
  blue_mask = visual->blue_mask;

  while (! (red_mask & 1))   { _red_offset++;   red_mask   >>= 1; }
  while (! (green_mask & 1)) { _green_offset++; green_mask >>= 1; }
  while (! (blue_mask & 1))  { _blue_offset++;  blue_mask  >>= 1; }

  _red_shift = _green_shift = _blue_shift = 8;
  while (red_mask)   { red_mask   >>= 1; _red_shift--;   }
  while (green_mask) { green_mask >>= 1; _green_shift--; }
  while (blue_mask)  { blue_mask  >>= 1; _blue_shift--;  }
}

TrueRenderControl::~TrueRenderControl()
{
  printf("Destroying TrueColor RenderControl\n");


}


static inline void renderPixel(XImage *im, unsigned char *dp,
			       unsigned long pixel)
{
  unsigned int bpp = im->bits_per_pixel + (im->byte_order == MSBFirst ? 1 : 0);

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

void TrueRenderControl::drawGradientBackground(
     Surface &sf, const RenderTexture &texture) const
{
    int w = sf.width(), h = sf.height();

    const ScreenInfo *info = display->screenInfo(_screen);
    XImage *im = XCreateImage(**display, info->visual(), info->depth(),
                              ZPixmap, 0, NULL, w, h, 32, 0);
  
    pixel32 *data = new pixel32[sf.height()*sf.width()];
    pixel32 current;
    pixel32 *dp = data;
    float dr, dg, db;
    unsigned int r,g,b;

    dr = (float)(texture.secondary_color().red() - texture.color().red());
    dr/= (float)sf.height();

    dg = (float)(texture.secondary_color().green() - texture.color().green());
    dg/= (float)sf.height();

    db = (float)(texture.secondary_color().blue() - texture.color().blue());
    db/= (float)sf.height();

    for (int y = 0; y < h; ++y) {
      r = texture.color().red() + (int)(dr * y);
      g = texture.color().green() + (int)(dg * y);
      b = texture.color().blue() + (int)(db * y);
      current = (r << 16)
              + (g << 8)
              + b;
      for (int x = 0; x < w; ++x, dp ++)
        *dp = current;
    }

    im->data = (char*) data;

    sf.setPixmap(im);

    delete [] im->data;
    im->data = NULL;
    XDestroyImage(im);
}

void TrueRenderControl::drawBackground(Surface& sf,
				       const RenderTexture &texture) const
{
  assert(_screen == sf._screen);
  assert(_screen == texture.color().screen());

  if (texture.gradient() == RenderTexture::Solid) {
    drawSolidBackground(sf, texture);
  } else {
    drawGradientBackground(sf, texture);
  }
}

}
