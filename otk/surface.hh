// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __surface_hh
#define __surface_hh

#include "point.hh"
#include "truerendercontrol.hh"

extern "C" {
#include <X11/Xlib.h>
#define _XFT_NO_COMPAT_ // no Xft 1 API
#include <X11/Xft/Xft.h>
}

namespace otk {

class ScreenInfo;

class Surface {
  int _screen;
  Point _size;
  XImage *_im;
  Pixmap _pm;
  XftDraw *_xftdraw;

  void createObjects();
  void destroyObjects();
  
protected:
  Surface(int screen);
  Surface(int screen, const Point &size);

  virtual void setSize(int w, int h);

public:
  virtual ~Surface();

  inline int screen(void) const { return _screen; }
  virtual const Point& size() const { return _size; }
  virtual int width() const { return _size.x(); }
  virtual int height() const { return _size.y(); }
  virtual Pixmap pixmap() const { return _pm; } // TEMP

  // The RenderControl classes use the internal objects in this class to render
  // to it. Noone else needs them tho, so they are private.
  friend class RenderControl;
  friend class TrueRenderControl;
};

}

#endif // __surface_hh
