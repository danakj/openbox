// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "surface.hh"
#include "display.hh"
#include "screeninfo.hh"
#include "rendercolor.hh"

extern "C" {
#include <X11/Xutil.h>
}

namespace otk {

Surface::Surface(int screen, const Point &size)
  : _screen(screen),
    _size(size),
    _pixmap(None),
    _xftdraw(0)
{
}

Surface::~Surface()
{
  destroyObjects();
}

void Surface::setPixmap(const RenderColor &color)
{
  if (_pixmap == None)
    createObjects();

  XFillRectangle(**display, _pixmap, color.gc(), 0, 0,
                 _size.x(), _size.y());
}

void Surface::setPixmap(XImage *image)
{
  assert(image->width == _size.x());
  assert(image->height == _size.y());
  
  if (_pixmap == None)
    createObjects();

  XPutImage(**display, _pixmap, DefaultGC(**display, _screen),
            image, 0, 0, 0, 0, _size.x(), _size.y());
}

void Surface::createObjects()
{
  assert(_pixmap == None); assert(!_xftdraw);

  const ScreenInfo *info = display->screenInfo(_screen);
  
  _pixmap = XCreatePixmap(**display, info->rootWindow(),
                          _size.x(), _size.y(), info->depth());
    
  _xftdraw = XftDrawCreate(**display, _pixmap,
                           info->visual(), info->colormap());
}

void Surface::destroyObjects()
{
  if (_xftdraw) {
    XftDrawDestroy(_xftdraw);
    _xftdraw = 0;
  }

  if (_pixmap != None) {
    XFreePixmap(**display, _pixmap);
    _pixmap = None;
  }
}

}
