// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __rendercontrol_hh
#define __rendercontrol_hh

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}

#include "surface.hh"

namespace otk {

class ScreenInfo;
class RenderTexture;
class Font;
class RenderColor;
class ustring;
class PixmapMask;

class RenderControl {
protected:
  int _screen;

//  bool _dither;

  RenderControl(int screen);

  inline void highlight(pixel32 *x, pixel32 *y, bool raised) const;
  void verticalGradient(Surface &sf, const RenderTexture &texture) const;
  void diagonalGradient(Surface &sf, const RenderTexture &texture) const;
  void crossDiagonalGradient(Surface &sf, const RenderTexture &texture) const;
  virtual void drawGradientBackground(Surface &sf,
                                      const RenderTexture &texture) const;
  virtual void drawSolidBackground(Surface& sf,
                                   const RenderTexture& texture) const;

  //! Reduces a Surface's Surface::pixelData so that it will display correctly
  //! on the screen's depth
  /*!
    This function allocates and sets the im->data member. The allocated memory
    will be freed when XDetroyImage is called on the XImage.
  */
  virtual void reduceDepth(Surface &sf, XImage *im) const = 0;

public:
  virtual ~RenderControl();

  static RenderControl *getRenderControl(int screen);

  //! Draws a background onto a Surface, as specified by a RenderTexture
  /*!
    This function will overwrite the entire surface.
  */
  virtual void drawBackground(Surface &sf,
			      const RenderTexture &texture) const;

  //! Draws an image onto the surface
  /*!
    This function will overwrite the entire surface.<br>
    The image must be specified in 32-bit packed ARGB format. The current
    background will be used for applying the alpha.
  */
  virtual void drawImage(Surface &sf, int w, int h,
                         unsigned long *data) const;
  
  //! Draws a string onto a Surface
  virtual void drawString(Surface &sf, const Font &font, int x, int y,
			  const RenderColor &color,
                          const ustring &string) const;

  //! Draws a PixmapMask with a specified color onto a Surface
  virtual void drawMask(Surface &sf, const RenderColor &color,
                        const PixmapMask &mask) const;

  virtual void allocateColor(XColor *color) const = 0;
};

}

#endif // __rendercontrol_hh
