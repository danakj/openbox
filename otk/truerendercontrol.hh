// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __truerendercontrol_hh
#define __truerendercontrol_hh

#include "rendercontrol.hh"

namespace otk {

class TrueRenderControl : public RenderControl {
private:
  // the offset of each color in a color mask
  int _red_offset;
  int _green_offset;
  int _blue_offset;

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
  
public:
  TrueRenderControl(const ScreenInfo *screen);
  virtual ~TrueRenderControl();

  virtual void render(::Drawable d);
};

}

#endif // __truerendercontrol_hh
