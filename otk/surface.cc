// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "surface.hh"
#include "display.hh"
#include "screeninfo.hh"

extern "C" {
#include <X11/Xutil.h>
}

namespace otk {

Surface::Surface(int screen)
  : _screen(screen),
    _size(1, 1),
    _pm(None),
    _xftdraw(0)
{
  createObjects();
}

Surface::Surface(int screen, const Point &size)
  : _screen(screen),
    _size(size),
    _pm(None),
    _xftdraw(0)
{
  createObjects();
}

Surface::~Surface()
{
  destroyObjects();
}

void Surface::createObjects()
{
  assert(_pm == None); assert(!_xftdraw);

  const ScreenInfo *info = display->screenInfo(_screen);
  
  _pm = XCreatePixmap(**display, info->rootWindow(), _size.x(), _size.y(),
		      info->depth());

  _xftdraw = XftDrawCreate(**display, _pm, info->visual(), info->colormap());
}

void Surface::destroyObjects()
{
  assert(_pm != None); assert(_xftdraw);

  XftDrawDestroy(_xftdraw);
  _xftdraw = 0;

  XFreePixmap(**display, _pm);
  _pm = None;
}

void Surface::setSize(int w, int h)
{
  if (w == _size.x() && h == _size.y()) return; // no change
  
  _size.setPoint(w, h);
  destroyObjects();
  createObjects();
}

}
