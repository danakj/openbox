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

  /*
  // color tables, meaning, 256 (possibly) different shades of each color,
  // based on the number of bits there are available for each color in the
  // visual
  unsigned char _red_color_table[256];
  unsigned char _green_color_table[256];
  unsigned char _blue_color_table[256];
  */

/*
  Bool _dither;

  int _cpc; // colors-per-channel: must be a value between [2,6]
  int _bpp; // bits-per-pixel

  unsigned int *_grad_xbuffer;
  unsigned int *_grad_ybuffer;
  unsigned int _grad_buffer_width;
  unsigned int _grad_buffer_height;

  unsigned long *_sqrt_table;

  // These values are all determined based on a visual

  int _red_bits;    // the number of bits (1-255) that each shade of color
  int _green_bits;  // spans across. best case is 1, which gives 255 shades.
  int _blue_bits;
  unsigned char _red_color_table[256];
  unsigned char _green_color_table[256];
  unsigned char _blue_color_table[256];

  // These are only used for TrueColor visuals
  int _red_offset;  // the offset of each color in a color mask
  int _green_offset;
  int _blue_offset;

  // These are only used for !TrueColor visuals
  XColor *_colors;
  int _ncolors;
*/

  RenderControl(int screen);

  virtual void reduceDepth(Surface &sf, XImage *im) const = 0;
  
  inline void highlight(pixel32 *x, pixel32 *y, bool raised) const;
  void verticalGradient(Surface &sf, const RenderTexture &texture) const;
  void diagonalGradient(Surface &sf, const RenderTexture &texture) const;
  void crossDiagonalGradient(Surface &sf, const RenderTexture &texture) const;
  virtual void drawGradientBackground(Surface &sf,
                                      const RenderTexture &texture) const;
  virtual void drawSolidBackground(Surface& sf,
                                   const RenderTexture& texture) const;

public:
  virtual ~RenderControl();

  static RenderControl *getRenderControl(int screen);

  virtual void drawRoot(const RenderColor &color) const;
  
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
};

}

#endif // __rendercontrol_hh
