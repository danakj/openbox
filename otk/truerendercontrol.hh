// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __truerendercontrol_hh
#define __truerendercontrol_hh

#include "rendercontrol.hh"

#include <vector>

namespace otk {

class TrueRenderControl : public RenderControl {
private:
  // the number of bits to shift a color value (from 0-255) to the right, to
  // fit it into the the color mask (do this before the offset)
  int _red_shift;
  int _green_shift;
  int _blue_shift;

  // the offset of each color in a color mask
  int _red_offset;
  int _green_offset;
  int _blue_offset;

  virtual void reduceDepth(Surface &sf, XImage *im) const;

public:
  TrueRenderControl(int screen);
  virtual ~TrueRenderControl();
};

}

#endif // __truerendercontrol_hh
