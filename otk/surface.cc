// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "surface.hh"
#include "display.hh"

namespace otk {

Surface::Surface()
  : _size(1, 1),
    _pm(None)
{
}

Surface::Surface(const Point &size)
  : _size(size),
    _pm(None)
{
}

Surface::~Surface()
{
  if (_pm != None) XFreePixmap(**display, _pm);
}

void Surface::setSize(int w, int h)
{
  _size.setPoint(w, h);
}

}
