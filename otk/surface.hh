// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __surface_hh
#define __surface_hh

#include "point.hh"
#include "truerendercontrol.hh"

extern "C" {
#include <X11/Xlib.h>
}

namespace otk {

class Surface {
  Point _size;
  Pixmap _pm;

protected:
  Surface();
  Surface(const Point &size);

  virtual void setSize(int w, int h);

public:
  virtual ~Surface();

  virtual const Point& size() const { return _size; }
  virtual int width() const { return _size.x(); }
  virtual int height() const { return _size.y(); }
  virtual Pixmap pixmap() const { return _pm; } // TEMP

  friend class TrueRenderControl;
};

}

#endif // __surface_hh
