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
    _im(0),
    _pm(None),
    _xftdraw(0)
{
  createObjects();
}

Surface::Surface(int screen, const Point &size)
  : _screen(screen),
    _size(size),
    _im(0),
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
  assert(!_im); assert(_pm == None); assert(!_xftdraw);

  const ScreenInfo *info = display->screenInfo(_screen);
  
  _im = XCreateImage(**display, info->visual(), info->depth(),
		     ZPixmap, 0, NULL, _size.x(), _size.y(), 32, 0);

  _pm = XCreatePixmap(**display, info->rootWindow(), _size.x(), _size.y(),
		      info->depth());

  _xftdraw = XftDrawCreate(**display, _pm, info->visual(), info->colormap());
}

void Surface::destroyObjects()
{
  assert(_im); assert(_pm != None); assert(_xftdraw);

  // do the delete ourselves cuz we alloc it with new not malloc
  delete [] _im->data;
  _im->data = NULL;
  XDestroyImage(_im);
  _im = 0;

  XFreePixmap(**display, _pm);
  _pm = None;

  XftDrawDestroy(_xftdraw);
  _xftdraw = 0;
}

void Surface::setSize(int w, int h)
{
  if (w == _size.x() && h == _size.y()) return; // no change
  
  _size.setPoint(w, h);
  destroyObjects();
  createObjects();
}

}
