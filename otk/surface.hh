// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __surface_hh
#define __surface_hh

#include "size.hh"
#include "truerendercontrol.hh"
#include "pseudorendercontrol.hh"

extern "C" {
#include <X11/Xlib.h>
#define _XFT_NO_COMPAT_ // no Xft 1 API
#include <X11/Xft/Xft.h>
}

namespace otk {

class ScreenInfo;
class RenderColor;

class Surface {
  int _screen;
  Size _size;
  Pixmap _pixmap;
  XftDraw *_xftdraw;

protected:
  void createObjects();
  void destroyObjects();

  void setPixmap(XImage *image);
  void setPixmap(const RenderColor &color);
  
public:
  Surface(int screen, const Size &size);
  virtual ~Surface();

  inline int screen(void) const { return _screen; }

  virtual const Size& size() const { return _size; }

  virtual Pixmap pixmap() const { return _pixmap; }

  // The RenderControl classes use the internal objects in this class to render
  // to it. Noone else needs them tho, so they are private.
  friend class RenderControl;
  friend class TrueRenderControl;
  friend class PseudoRenderControl;
};

}

#endif // __surface_hh
