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

extern "C" {
#ifdef    HAVE_STDLIB_H
#  include <stdlib.h>
#endif // HAVE_STDLIB_H

#include "../src/gettext.h"
#define _(str) gettext(str)
}

namespace otk {

RenderControl *RenderControl::getRenderControl(int screen)
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

void RenderControl::drawRoot(const RenderColor &color) const
{
  Window root = display->screenInfo(_screen)->rootWindow();
  XSetWindowBackground(**display, root, color.pixel());
  XClearWindow(**display, root);
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

}
