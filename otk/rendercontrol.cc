// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

#include "rendercontrol.hh"
#include "truerendercontrol.hh"
#include "pseudorendercontrol.hh"
#include "rendertexture.hh"
#include "rendercolor.hh"
#include "renderstyle.hh"
#include "display.hh"
#include "screeninfo.hh"
#include "surface.hh"
#include "font.hh"
#include "ustring.hh"
#include "property.hh"

extern "C" {
#ifdef    HAVE_SYS_WAIT_H
#  include <sys/wait.h>
#endif // HAVE_SYS_WAIT_H

#ifdef    HAVE_UNISTD_H
#  include <unistd.h>
#endif // HAVE_UNISTD_H

#include "../src/gettext.h"
#define _(str) gettext(str)
}

#include <cstdlib>

namespace otk {

RenderControl *RenderControl::createRenderControl(int screen)
{
  // get the visual on the screen and return the correct type of RenderControl
  int vclass = display->screenInfo(screen)->visual()->c_class;
  switch (vclass) {
  case TrueColor:
    return new TrueRenderControl(screen);
  case PseudoColor:
  case StaticColor:
    return new PseudoRenderControl(screen);
  case GrayScale:
  case StaticGray:
    return new PseudoRenderControl(screen);
  default:
    printf(_("RenderControl: Unsupported visual %d specified. Aborting.\n"),
	   vclass);
    ::exit(1);
  }
}

RenderControl::RenderControl(int screen)
  : _screen(screen)
    
{
  printf("Initializing RenderControl\n");
  
}

RenderControl::~RenderControl()
{
  printf("Destroying RenderControl\n");
}

void RenderControl::drawString(Surface& sf, const Font &font, int x, int y,
			       const RenderColor &color,
                               const ustring &string) const
{
  assert(sf._screen == _screen);
  XftDraw *d = sf._xftdraw;
  assert(d); // this means that the background hasn't been rendered yet!
  
  if (font._shadow) {
    XftColor c;
    c.color.red = 0;
    c.color.green = 0;
    c.color.blue = 0;
    c.color.alpha = font._tint | font._tint << 8; // transparent shadow
    c.pixel = BlackPixel(**display, _screen);

    if (string.utf8())
      XftDrawStringUtf8(d, &c, font._xftfont, x + font._offset,
                        font._xftfont->ascent + y + font._offset,
                        (FcChar8*)string.c_str(), string.bytes());
    else
      XftDrawString8(d, &c, font._xftfont, x + font._offset,
                     font._xftfont->ascent + y + font._offset,
                     (FcChar8*)string.c_str(), string.bytes());
  }
    
  XftColor c;
  c.color.red = color.red() | color.red() << 8;
  c.color.green = color.green() | color.green() << 8;
  c.color.blue = color.blue() | color.blue() << 8;
  c.pixel = color.pixel();
  c.color.alpha = 0xff | 0xff << 8; // no transparency in Color yet

  if (string.utf8())
    XftDrawStringUtf8(d, &c, font._xftfont, x, font._xftfont->ascent + y,
                      (FcChar8*)string.c_str(), string.bytes());
  else
    XftDrawString8(d, &c, font._xftfont, x, font._xftfont->ascent + y,
                   (FcChar8*)string.c_str(), string.bytes());
  return;
}

void RenderControl::drawSolidBackground(Surface& sf,
                                        const RenderTexture& texture) const
{
  assert(_screen == sf._screen);
  assert(_screen == texture.color().screen());
  
  if (texture.parentRelative()) return;
  
  sf.setPixmap(texture.color());

  int width = sf.size().width(), height = sf.size().height();
  int left = 0, top = 0, right = width - 1, bottom = height - 1;

  if (texture.interlaced())
    for (int i = 0; i < height; i += 2)
      XDrawLine(**display, sf.pixmap(), texture.interlaceColor().gc(),
                0, i, width, i);

  switch (texture.relief()) {
  case RenderTexture::Raised:
    switch (texture.bevel()) {
    case RenderTexture::Bevel1:
      XDrawLine(**display, sf.pixmap(), texture.bevelDarkColor().gc(),
                left, bottom, right, bottom);
      XDrawLine(**display, sf.pixmap(), texture.bevelDarkColor().gc(),
                right, bottom, right, top);

      XDrawLine(**display, sf.pixmap(), texture.bevelLightColor().gc(),
                left, top, right, top);
      XDrawLine(**display, sf.pixmap(), texture.bevelLightColor().gc(),
                left, bottom, left, top);
      break;
    case RenderTexture::Bevel2:
      XDrawLine(**display, sf.pixmap(), texture.bevelDarkColor().gc(),
                left + 1, bottom - 2, right - 2, bottom - 2);
      XDrawLine(**display, sf.pixmap(), texture.bevelDarkColor().gc(),
                right - 2, bottom - 2, right - 2, top + 1);

      XDrawLine(**display, sf.pixmap(), texture.bevelLightColor().gc(),
                left + 1, top + 1, right - 2, top + 1);
      XDrawLine(**display, sf.pixmap(), texture.bevelLightColor().gc(),
                left + 1, bottom - 2, left + 1, top + 1);
      break;
    default:
      assert(false); // unhandled RenderTexture::BevelType
    }
    break;
  case RenderTexture::Sunken:
    switch (texture.bevel()) {
    case RenderTexture::Bevel1:
      XDrawLine(**display, sf.pixmap(), texture.bevelLightColor().gc(),
                left, bottom, right, bottom);
      XDrawLine(**display, sf.pixmap(), texture.bevelLightColor().gc(),
                right, bottom, right, top);

      XDrawLine(**display, sf.pixmap(), texture.bevelDarkColor().gc(),
                left, top, right, top);
      XDrawLine(**display, sf.pixmap(), texture.bevelDarkColor().gc(),
                left, bottom, left, top);
      break;
    case RenderTexture::Bevel2:
      XDrawLine(**display, sf.pixmap(), texture.bevelLightColor().gc(),
                left + 1, bottom - 2, right - 2, bottom - 2);
      XDrawLine(**display, sf.pixmap(), texture.bevelLightColor().gc(),
                right - 2, bottom - 2, right - 2, top + 1);

      XDrawLine(**display, sf.pixmap(), texture.bevelDarkColor().gc(),
                left + 1, top + 1, right - 2, top + 1);
      XDrawLine(**display, sf.pixmap(), texture.bevelDarkColor().gc(),
                left + 1, bottom - 2, left + 1, top + 1);
      break;
    default:
      assert(false); // unhandled RenderTexture::BevelType
    }
    break;
  case RenderTexture::Flat:
    if (texture.border())
      XDrawRectangle(**display, sf.pixmap(), texture.borderColor().gc(),
                     left, top, right, bottom);
    break;
  default:
    assert(false); // unhandled RenderTexture::ReliefType
  }
}

void RenderControl::drawMask(Surface &sf, const RenderColor &color,
                             const PixmapMask &mask) const
{
  assert(_screen == sf._screen);
  assert(_screen == color.screen());

  if (mask.mask == None) return; // no mask given

  int width = sf.size().width(), height = sf.size().height();
  
  // set the clip region
  int x = (width - mask.w) / 2, y = (height - mask.h) / 2;
  XSetClipMask(**display, color.gc(), mask.mask);
  XSetClipOrigin(**display, color.gc(), x, y);

  // fill in the clipped region
  XFillRectangle(**display, sf.pixmap(), color.gc(), x, y,
                 x + mask.w, y + mask.h);

  // unset the clip region
  XSetClipMask(**display, color.gc(), None);
  XSetClipOrigin(**display, color.gc(), 0, 0);
}

void RenderControl::drawGradientBackground(
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
  case RenderTexture::Horizontal:
    horizontalGradient(sf, texture);
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
  sf.setPixmap(im);
  XDestroyImage(im);
}

void RenderControl::verticalGradient(Surface &sf,
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

void RenderControl::horizontalGradient(Surface &sf,
                                         const RenderTexture &texture) const
{
  pixel32 *data = sf.pixelData();
  pixel32 current;
  float dr, dg, db;
  unsigned int r,g,b;
  int w = sf.size().width(), h = sf.size().height();

  dr = (float)(texture.secondary_color().red() - texture.color().red());
  dr/= (float)w;

  dg = (float)(texture.secondary_color().green() - texture.color().green());
  dg/= (float)w;

  db = (float)(texture.secondary_color().blue() - texture.color().blue());
  db/= (float)w;

  for (int x = 0; x < w; ++x, ++data) {
    r = texture.color().red() + (int)(dr * x);
    g = texture.color().green() + (int)(dg * x);
    b = texture.color().blue() + (int)(db * x);
    current = (r << default_red_shift)
            + (g << default_green_shift)
            + (b << default_blue_shift);
    for (int y = 0; y < h; ++y)
      *(data + y*w) = current;
  }
}

void RenderControl::diagonalGradient(Surface &sf,
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

void RenderControl::crossDiagonalGradient(
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

void RenderControl::highlight(pixel32 *x, pixel32 *y, bool raised) const
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

void RenderControl::drawBackground(Surface& sf,
				       const RenderTexture &texture) const
{
  assert(_screen == sf._screen);
  assert(_screen == texture.color().screen());

  if (texture.gradient() == RenderTexture::Solid)
    drawSolidBackground(sf, texture);
  else
    drawGradientBackground(sf, texture);
}


void RenderControl::drawImage(Surface &sf, int w, int h,
                                  unsigned long *data) const
{
  pixel32 *bg = sf.pixelData();
  int c, sfw, sfh;
  unsigned int i, e, bgi;
  sfw = sf.size().width();
  sfh = sf.size().height();

  if (w && h) {
    // scale it
    unsigned long *olddata = data;
    unsigned long newdata[sfw*sfh];
    double dx = w / (double)sfw;
    double dy = h / (double)sfh;
    double px = 0.0;
    double py = 0.0;
    int iy = 0;
    for (i = 0, c = 0, e = sfw*sfh; i < e; ++i) {
      newdata[i] = olddata[(int)px + iy];
      if (++c >= sfw) {
        c = 0;
        px = 0;
        py += dy;
        iy = (int)py * w;
      } else
        px += dx;
    }
    data = newdata;

    // apply the alpha channel
    for (i = 0, c = 0, e = sfw*sfh; i < e; ++i, ++bgi) {
      unsigned char alpha = data[i] >> 24;
      unsigned char r = data[i] >> 16;
      unsigned char g = data[i] >> 8;
      unsigned char b = data[i];

      // background color
      unsigned char bgr = bg[i] >> default_red_shift;
      unsigned char bgg = bg[i] >> default_green_shift;
      unsigned char bgb = bg[i] >> default_blue_shift;
      
      r = bgr + (((r - bgr) * alpha) >> 8);
      g = bgg + (((g - bgg) * alpha) >> 8);
      b = bgb + (((b - bgb) * alpha) >> 8);

      bg[i] = (r << default_red_shift) | (g << default_green_shift) |
        (b << default_blue_shift);
    }
  }

  const ScreenInfo *info = display->screenInfo(_screen);
  XImage *im = XCreateImage(**display, info->visual(), info->depth(),
                            ZPixmap, 0, NULL, sf.size().width(),
                            sf.size().height(), 32, 0);
  im->byte_order = endian;

  reduceDepth(sf, im);
  sf.setPixmap(im);
  XDestroyImage(im);
}

void RenderControl::drawImage(Surface &sf, Pixmap pixmap, Pixmap mask) const
{
  int junk, sfw, sfh, w, h, depth, mw, mh, mdepth;
  Window wjunk;
  const ScreenInfo *info = display->screenInfo(_screen);
  GC mgc = 0;

  assert(pixmap != None);

  sfw = sf.size().width();
  sfh = sf.size().height();

  XGetGeometry(**display, pixmap, &wjunk, &junk, &junk,
               (unsigned int*)&w, (unsigned int*)&h,
               (unsigned int*)&junk, (unsigned int*)&depth);
  if (mask != None) {
    XGetGeometry(**display, mask, &wjunk, &junk, &junk,
                 (unsigned int*)&mw, (unsigned int*)&mh,
                 (unsigned int*)&junk, (unsigned int*)&mdepth);
    if (mw != w || mh != h || mdepth != 1)
      return;
  }

  Pixmap p = XCreatePixmap(**display, info->rootWindow(), sfw, sfh,
                           info->depth());
  Pixmap m;
  if (mask == None)
    m = None;
  else {
    m = XCreatePixmap(**display, info->rootWindow(), sfw, sfh, 1);
    XGCValues gcv;
    gcv.subwindow_mode = IncludeInferiors;
    gcv.graphics_exposures = false;
    mgc = XCreateGC(**display, m, GCGraphicsExposures |
                    GCSubwindowMode, &gcv);
  }

  // scale it
  for (int y = sfh - 1; y >= 0; --y) {
    int yy = y * h / sfh;
    for (int x = sfw - 1; x >= 0; --x) {
      int xx = x * w / sfw;
      if (depth != info->depth()) {
        XCopyPlane(**display, pixmap, p, DefaultGC(**display, _screen),
                   xx, yy, 1, 1, x, y, 1);
      } else {
        XCopyArea(**display, pixmap, p, DefaultGC(**display, _screen),
                  xx, yy, 1, 1, x, y);
      }
      if (mask != None)
        XCopyArea(**display, mask, m, mgc, xx, yy, 1, 1, x, y);
    }
  }

  XSetClipMask(**display, DefaultGC(**display, _screen), m);
  XSetClipOrigin(**display, DefaultGC(**display, _screen), 0, 0);
  XCopyArea(**display, p, sf.pixmap(), DefaultGC(**display, _screen), 0, 0,
            sfw, sfh, 0, 0);
  XSetClipMask(**display, DefaultGC(**display, _screen), None);

  XFreePixmap(**display, p);
  if (m != None) XFreePixmap(**display, m);
}

}
