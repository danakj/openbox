// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

#include "surface.hh"
#include "display.hh"
#include "screeninfo.hh"
#include "rendercolor.hh"

extern "C" {
#include <X11/Xutil.h>
#include <cstring>
}

namespace otk {

Surface::Surface(int screen, const Size &size)
  : _screen(screen),
    _size(size),
    _pixel_data(new pixel32[size.width()*size.height()]),
    _pixmap(None),
    _xftdraw(0)
{
}

Surface::~Surface()
{
  destroyObjects();
  freePixelData();
}

void Surface::freePixelData()
{
  if (_pixel_data) {
    delete [] _pixel_data;
    _pixel_data = 0;
  }
}

void Surface::setPixmap(const RenderColor &color)
{
  assert(_pixel_data);
  if (_pixmap == None)
    createObjects();
  
  XFillRectangle(**display, _pixmap, color.gc(), 0, 0,
                 _size.width(), _size.height());

  pixel32 val = 0; // XXX set this from the color and shift amounts!
  for (unsigned int i = 0, s = _size.width() * _size.height(); i < s; ++i) {
    unsigned char *p = (unsigned char*)&_pixel_data[i];
    *p = (unsigned char) (val >> 24);
    *++p = (unsigned char) (val >> 16);
    *++p = (unsigned char) (val >> 8);
    *++p = (unsigned char) val;
  }
}

void Surface::setPixmap(XImage *image)
{
  assert(_pixel_data);
  assert(image->width == _size.width());
  assert(image->height == _size.height());
  
  if (_pixmap == None)
    createObjects();

  XPutImage(**display, _pixmap, DefaultGC(**display, _screen),
            image, 0, 0, 0, 0, _size.width(), _size.height());
}

void Surface::createObjects()
{
  assert(_pixmap == None); assert(!_xftdraw);

  const ScreenInfo *info = display->screenInfo(_screen);
  
  _pixmap = XCreatePixmap(**display, info->rootWindow(),
                          _size.width(), _size.height(), info->depth());
  assert(_pixmap != None);
    
  _xftdraw = XftDrawCreate(**display, _pixmap,
                           info->visual(), info->colormap());
  assert(_xftdraw);
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
