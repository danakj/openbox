// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

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
  const ScreenInfo *info = display->screenInfo(_screen);
  XImage *timage = XCreateImage(**display, info->visual(), info->depth(),
                                ZPixmap, 0, NULL, 1, 1, 32, 0);
  printf("Initializing TrueColor RenderControl\n");

  unsigned long red_mask, green_mask, blue_mask;

  // find the offsets for each color in the visual's masks
  red_mask = timage->red_mask;
  green_mask = timage->green_mask;
  blue_mask = timage->blue_mask;

  while (! (red_mask & 1))   { _red_offset++;   red_mask   >>= 1; }
  while (! (green_mask & 1)) { _green_offset++; green_mask >>= 1; }
  while (! (blue_mask & 1))  { _blue_offset++;  blue_mask  >>= 1; }

  _red_shift = _green_shift = _blue_shift = 8;
  while (red_mask)   { red_mask   >>= 1; _red_shift--;   }
  while (green_mask) { green_mask >>= 1; _green_shift--; }
  while (blue_mask)  { blue_mask  >>= 1; _blue_shift--;  }
  XFree(timage);
}

TrueRenderControl::~TrueRenderControl()
{
  printf("Destroying TrueColor RenderControl\n");
}

void TrueRenderControl::drawGradientBackground(
     Surface &sf, const RenderTexture &texture) const
{
  unsigned int r,g,b;
  int w = sf.size().width(), h = sf.size().height();
  int off, x;

  const ScreenInfo *info = display->screenInfo(_screen);
  XImage *im = XCreateImage(**display, info->visual(), info->depth(),
                            ZPixmap, 0, NULL, w, h, 32, 0);
  im->byte_order = endian;

  switch (texture.gradient()) {
  case RenderTexture::Vertical:
    verticalGradient(sf, texture);
    break;
  case RenderTexture::Diagonal:
    diagonalGradient(sf, texture);
    break;
  case RenderTexture::CrossDiagonal:
    crossDiagonalGradient(sf, texture);
    break;
  default:
    printf("unhandled gradient\n");
  }

  pixel32 *data = sf.pixelData();
  pixel32 current;
  
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

  reduceDepth(sf, im);

  im->data = (char*) data;

  sf.setPixmap(im);

  im->data = NULL;
  XDestroyImage(im);
}

void TrueRenderControl::verticalGradient(Surface &sf,
                                         const RenderTexture &texture) const
{
  pixel32 *data = sf.pixelData();
  pixel32 current;
  float dr, dg, db;
  unsigned int r,g,b;
  int w = sf.size().width(), h = sf.size().height();

  dr = (float)(texture.secondary_color().red() - texture.color().red());
  dr/= (float)h;

  dg = (float)(texture.secondary_color().green() - texture.color().green());
  dg/= (float)h;

  db = (float)(texture.secondary_color().blue() - texture.color().blue());
  db/= (float)h;

  for (int y = 0; y < h; ++y) {
    r = texture.color().red() + (int)(dr * y);
    g = texture.color().green() + (int)(dg * y);
    b = texture.color().blue() + (int)(db * y);
    current = (r << default_red_shift)
            + (g << default_green_shift)
            + (b << default_blue_shift);
    for (int x = 0; x < w; ++x, ++data)
      *data = current;
  }
}

void TrueRenderControl::diagonalGradient(Surface &sf,
                                         const RenderTexture &texture) const
{
  pixel32 *data = sf.pixelData();
  pixel32 current;
  float drx, dgx, dbx, dry, dgy, dby;
  unsigned int r,g,b;
  int w = sf.size().width(), h = sf.size().height();

  for (int y = 0; y < h; ++y) {
    drx = (float)(texture.secondary_color().red() - texture.color().red());
    dry = drx/(float)h;
    drx/= (float)w;

    dgx = (float)(texture.secondary_color().green() - texture.color().green());
    dgy = dgx/(float)h;
    dgx/= (float)w;

    dbx = (float)(texture.secondary_color().blue() - texture.color().blue());
    dby = dbx/(float)h;
    dbx/= (float)w;
    for (int x = 0; x < w; ++x, ++data) {
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

void TrueRenderControl::crossDiagonalGradient(
  Surface &sf, const RenderTexture &texture) const
{
  pixel32 *data = sf.pixelData();
  pixel32 current;
  float drx, dgx, dbx, dry, dgy, dby;
  unsigned int r,g,b;
  int w = sf.size().width(), h = sf.size().height();

  for (int y = 0; y < h; ++y) {
    drx = (float)(texture.secondary_color().red() - texture.color().red());
    dry = drx/(float)h;
    drx/= (float)w;

    dgx = (float)(texture.secondary_color().green() - texture.color().green());
    dgy = dgx/(float)h;
    dgx/= (float)w;

    dbx = (float)(texture.secondary_color().blue() - texture.color().blue());
    dby = dbx/(float)h;
    dbx/= (float)w;
    for (int x = w; x > 0; --x, ++data) {
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

void TrueRenderControl::reduceDepth(Surface &sf, XImage *im) const
{
// since pixel32 is the largest possible pixel size, we can share the array
  int r, g, b;
  int x,y;
  pixel32 *data = sf.pixelData();
  pixel16 *p = (pixel16*) data;
  switch (im->bits_per_pixel) {
  case 32:
    if ((_red_offset != default_red_shift) ||
        (_blue_offset != default_blue_shift) ||
        (_green_offset != default_green_shift)) {
      printf("cross endian conversion\n");
      for (y = 0; y < im->height; y++) {
        for (x = 0; x < im->width; x++) {
          r = (data[x] >> default_red_shift) & 0xFF;
          g = (data[x] >> default_green_shift) & 0xFF;
          b = (data[x] >> default_blue_shift) & 0xFF;
          data[x] = (r << _red_offset) + (g << _green_offset) +
            (b << _blue_offset);
        }
        data += im->width;
      } 
   }
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

  if (texture.gradient() == RenderTexture::Solid)
    drawSolidBackground(sf, texture);
  else
    drawGradientBackground(sf, texture);
}


void TrueRenderControl::drawImage(Surface &sf, int w, int h,
                                  unsigned long *data) const
{
  pixel32 *bg = sf.pixelData();
  int startx, x, y, c;
  unsigned int i, e;
  x = (sf.size().width() - w) / 2;
  y = (sf.size().height() - h) / 2;

  if (x < 0) x = 0;
  if (y < 0) y = 0;

  // XX SCALING!@!&*(@! to make it fit on the surface

  startx = x;
  
  for (i = 0, c = 0, e = w*h; i < e; ++i) {
    unsigned char alpha = data[i] >> 24;
    unsigned char r = data[i] >> 16;
    unsigned char g = data[i] >> 8;
    unsigned char b = data[i];

    // background color
    unsigned char bgr = bg[i] >> default_red_shift;
    unsigned char bgg = bg[i] >> default_green_shift;
    unsigned char bgb = bg[i] >> default_blue_shift;
      
    r = bgr + (r - bgr) * alpha >> 8;
    g = bgg + (g - bgg) * alpha >> 8;
    b = bgb + (b - bgb) * alpha >> 8;

    bg[i] = (r << default_red_shift) | (g << default_green_shift) |
      (b << default_blue_shift);

    if (++c >= w) {
      ++y;
      x = startx;
      c = 0;
    } else
      ++x;
  }

  const ScreenInfo *info = display->screenInfo(_screen);
  XImage *im = XCreateImage(**display, info->visual(), info->depth(),
                            ZPixmap, 0, NULL, sf.size().width(),
                            sf.size().height(), 32, 0);
  im->byte_order = endian;

  reduceDepth(sf, im);

  im->data = (char*) bg;

  sf.setPixmap(im);

  im->data = NULL;
  XDestroyImage(im);
}

}
