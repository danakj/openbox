// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "rendercontrol.hh"
#include "truerendercontrol.hh"
#include "rendertexture.hh"
#include "display.hh"
#include "screeninfo.hh"
#include "surface.hh"
#include "color.hh"
#include "font.hh"
#include "ustring.hh"

extern "C" {
#ifdef    HAVE_STDLIB_H
#  include <stdlib.h>
#endif // HAVE_STDLIB_H

#include "gettext.h"
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
//    return new PseudoRenderControl(screen);
  case GrayScale:
  case StaticGray:
//    return new GrayRenderControl(screen);
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

void RenderControl::drawString(Surface *sf, const Font &font, int x, int y,
			       const Color &color, const ustring &string) const
{
  assert(sf);
  assert(sf->_screen == _screen);
  XftDraw *d = sf->_xftdraw;
  assert(d);
  
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

}
