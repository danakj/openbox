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

#include "../src/gettext.h"
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
  unsigned int r,g,b;
  int w = sf.width(), h = sf.height(), off, x;

  const ScreenInfo *info = display->screenInfo(_screen);
  XImage *im = XCreateImage(**display, info->visual(), info->depth(),
                            ZPixmap, 0, NULL, w, h, 32, 0);
  
  pixel32 *data = new pixel32[sf.height()*sf.width()];
  pixel32 current;

  switch (texture.gradient()) {
  case RenderTexture::Vertical:
    verticalGradient(sf, texture, data);
    break;
  case RenderTexture::Diagonal:
    diagonalGradient(sf, texture, data);
    break;
  case RenderTexture::CrossDiagonal:
    crossDiagonalGradient(sf, texture, data);
    break;
  default:
    printf("unhandled gradient\n");
  }

  if (texture.relief() == RenderTexture::Flat && texture.border()) {
    r = texture.borderColor().red();
    g = texture.borderColor().green();
    b = texture.borderColor().blue();
    current = (r << default_red_shift)
            + (g << default_green_shift)
            + (b << default_blue_shift);
    for (off = 0, x = 0; x < w; ++x, off++) {
      *(data + off) = current;
      *(data + off + ((h-1) * w)) = current;
    }
    for (off = 0, x = 0; x < h; ++x, off++) {
      *(data + (off * w)) = current;
      *(data + (off * w) + w - 1) = current;
    }
  }

  if (texture.relief() != RenderTexture::Flat) {
    if (texture.bevel() == RenderTexture::Bevel1) {
      for (off = 1, x = 1; x < w - 1; ++x, off++)
        highlight(data + off,
                  data + off + (h-1) * w,
                  texture.relief()==RenderTexture::Raised);
      for (off = 0, x = 0; x < h; ++x, off++)
        highlight(data + off * w,
                  data + off * w + w - 1,
                  texture.relief()==RenderTexture::Raised);
    }

    if (texture.bevel() == RenderTexture::Bevel2) {
      for (off = 2, x = 2; x < w - 2; ++x, off++)
        highlight(data + off + w,
                  data + off + (h-2) * w,
                  texture.relief()==RenderTexture::Raised);
      for (off = 1, x = 1; x < h-1; ++x, off++)
        highlight(data + off * w + 1,
                  data + off * w + w - 2,
                  texture.relief()==RenderTexture::Raised);
    }
  }

  reduceDepth(im, data);

  im->data = (char*) data;

  sf.setPixmap(im);

  delete [] im->data;
  im->data = NULL;
  XDestroyImage(im);
}

void TrueRenderControl::verticalGradient(Surface &sf,
                                         const RenderTexture &texture,
                                         pixel32 *data) const
{
  pixel32 current;
  float dr, dg, db;
  unsigned int r,g,b;

  dr = (float)(texture.secondary_color().red() - texture.color().red());
  dr/= (float)sf.height();

  dg = (float)(texture.secondary_color().green() - texture.color().green());
  dg/= (float)sf.height();

  db = (float)(texture.secondary_color().blue() - texture.color().blue());
  db/= (float)sf.height();

  for (int y = 0; y < sf.height(); ++y) {
    r = texture.color().red() + (int)(dr * y);
    g = texture.color().green() + (int)(dg * y);
    b = texture.color().blue() + (int)(db * y);
    current = (r << default_red_shift)
            + (g << default_green_shift)
            + (b << default_blue_shift);
    for (int x = 0; x < sf.width(); ++x, ++data)
      *data = current;
  }
}

void TrueRenderControl::diagonalGradient(Surface &sf,
                                         const RenderTexture &texture,
                                         pixel32 *data) const
{
  pixel32 current;
  float drx, dgx, dbx, dry, dgy, dby;
  unsigned int r,g,b;


  for (int y = 0; y < sf.height(); ++y) {
    drx = (float)(texture.secondary_color().red() - texture.color().red());
    dry = drx/(float)sf.height();
    drx/= (float)sf.width();

    dgx = (float)(texture.secondary_color().green() - texture.color().green());
    dgy = dgx/(float)sf.height();
    dgx/= (float)sf.width();

    dbx = (float)(texture.secondary_color().blue() - texture.color().blue());
    dby = dbx/(float)sf.height();
    dbx/= (float)sf.width();
    for (int x = 0; x < sf.width(); ++x, ++data) {
      r = texture.color().red() + ((int)(drx * x) + (int)(dry * y))/2;
      g = texture.color().green() + ((int)(dgx * x) + (int)(dgy * y))/2;
      b = texture.color().blue() + ((int)(dbx * x) + (int)(dby * y))/2;
      current = (r << default_red_shift)
              + (g << default_green_shift)
              + (b << default_blue_shift);
      *data = current;
    }
  }
}

void TrueRenderControl::crossDiagonalGradient(Surface &sf,
                                         const RenderTexture &texture,
                                         pixel32 *data) const
{
  pixel32 current;
  float drx, dgx, dbx, dry, dgy, dby;
  unsigned int r,g,b;

  for (int y = 0; y < sf.height(); ++y) {
    drx = (float)(texture.secondary_color().red() - texture.color().red());
    dry = drx/(float)sf.height();
    drx/= (float)sf.width();

    dgx = (float)(texture.secondary_color().green() - texture.color().green());
    dgy = dgx/(float)sf.height();
    dgx/= (float)sf.width();

    dbx = (float)(texture.secondary_color().blue() - texture.color().blue());
    dby = dbx/(float)sf.height();
    dbx/= (float)sf.width();
    for (int x = sf.width(); x > 0; --x, ++data) {
      r = texture.color().red() + ((int)(drx * (x-1)) + (int)(dry * y))/2;
      g = texture.color().green() + ((int)(dgx * (x-1)) + (int)(dgy * y))/2;
      b = texture.color().blue() + ((int)(dbx * (x-1)) + (int)(dby * y))/2;
      current = (r << default_red_shift)
              + (g << default_green_shift)
              + (b << default_blue_shift);
      *data = current;
    }
  }
}
void TrueRenderControl::reduceDepth(XImage *im, pixel32 *data) const
{
// since pixel32 is the largest possible pixel size, we can share the array
  int r, g, b;
  int x,y;
  pixel16 *p = (pixel16 *)data;
  switch (im->bits_per_pixel) {
  case 32:
    return;
  case 16:
    for (y = 0; y < im->height; y++) {
      for (x = 0; x < im->width; x++) {
        r = (data[x] >> default_red_shift) & 0xFF;
        r = r >> _red_shift;
        g = (data[x] >> default_green_shift) & 0xFF;
        g = g >> _green_shift;
        b = (data[x] >> default_blue_shift) & 0xFF;
        b = b >> _blue_shift;
        p[x] = (r << _red_offset) + (g << _green_offset) + (b << _blue_offset);
      }
      data += im->width;
      p += im->bytes_per_line/2;
    }
    break;
  default:
    printf("your bit depth is currently unhandled\n");
  }
}

void TrueRenderControl::highlight(pixel32 *x, pixel32 *y, bool raised) const
{
  int r, g, b;

  pixel32 *up, *down;
  if (raised) {
    up = x;
    down = y;
  } else {
    up = y;
    down = x;
  }
  r = (*up >> default_red_shift) & 0xFF;
  r += r >> 1;
  g = (*up >> default_green_shift) & 0xFF;
  g += g >> 1;
  b = (*up >> default_blue_shift) & 0xFF;
  b += b >> 1;
  if (r > 255) r = 255;
  if (g > 255) g = 255;
  if (b > 255) b = 255;
  *up = (r << default_red_shift) + (g << default_green_shift)
      + (b << default_blue_shift);
  
  r = (*down >> default_red_shift) & 0xFF;
  r = (r >> 1) + (r >> 2);
  g = (*down >> default_green_shift) & 0xFF;
  g = (g >> 1) + (g >> 2);
  b = (*down >> default_blue_shift) & 0xFF;
  b = (b >> 1) + (b >> 2);
  *down = (r << default_red_shift) + (g << default_green_shift)
        + (b << default_blue_shift);
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
