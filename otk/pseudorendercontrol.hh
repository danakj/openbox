// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __pseudorendercontrol_hh
#define __pseudorendercontrol_hh

#include "rendercontrol.hh"

namespace otk {

class PseudoRenderControl : public RenderControl {
private:
  // color tables, meaning, 256 (possibly) different shades of each color,
  // based on the number of bits there are available for each color in the
  // visual
  unsigned char _red_color_table[256];
  unsigned char _green_color_table[256];
  unsigned char _blue_color_table[256];

  int _cpc; // colors-per-channel: must be a value between [2,6]
  int _bpp; // bits-per-pixel
  int _ncolors; // number of allocated colors, size of the XColor array

  XColor *_colors;
  
  virtual void reduceDepth(Surface &sf, XImage *im) const;
  
public:
  PseudoRenderControl(int screen);
  virtual ~PseudoRenderControl();

};

}

#endif // __pseudorendercontrol_hh
