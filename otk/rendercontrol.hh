// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __rendercontrol_hh
#define __rendercontrol_hh

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}

namespace otk {

class ScreenInfo;

class RenderControl {
protected:
  const ScreenInfo *_screen;

  // the number of bits (1-255) that each shade of color spans across. best
  // case is 1, which gives 255 shades
  int _red_bits;
  int _green_bits;
  int _blue_bits;

  // color tables, meaning, 256 (possibly) different shades of each color,
  // based on the number of bits there are available for each color in the
  // visual
  unsigned char _red_color_table[256];
  unsigned char _green_color_table[256];
  unsigned char _blue_color_table[256];

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

  RenderControl(const ScreenInfo *screen);

public:
  virtual ~RenderControl();

  static RenderControl *getRenderControl(int screen);

  virtual void render(::Window w) = 0;
};

}

#endif // __rendercontrol_hh
