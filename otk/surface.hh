// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __surface_hh
#define __surface_hh

#include "size.hh"

extern "C" {
#include <X11/Xlib.h>
#define _XFT_NO_COMPAT_ // no Xft 1 API
#include <X11/Xft/Xft.h>

#ifdef HAVE_STDINT_H
#  include <stdint.h>
#else
#  ifdef HAVE_SYS_TYPES_H
#    include <sys/types.h>
#  endif
#endif
}

namespace otk {

class ScreenInfo;
class RenderColor;
class RenderControl;
class TrueRenderControl;
class PseudoRenderControl;

#ifdef HAVE_STDINT_H
typedef uint32_t pixel32;
typedef uint16_t pixel16;
#else
typedef u_int32_t pixel32;
typedef u_int16_t pixel16;
#endif /* HAVE_STDINT_H */

class Surface {
  int _screen;
  Size _size;
  pixel32 *_pixel_data;
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

  const Size& size() const { return _size; }

  Pixmap pixmap() const { return _pixmap; }

  pixel32 *pixelData() { return _pixel_data; }

  // The RenderControl classes use the internal objects in this class to render
  // to it. Noone else needs them tho, so they are private.
  friend class RenderControl;
  friend class TrueRenderControl;
  friend class PseudoRenderControl;
};

}

#endif // __surface_hh
