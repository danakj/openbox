// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __truerendercontrol_hh
#define __truerendercontrol_hh

#include "rendercontrol.hh"

extern "C" {

#ifdef HAVE_STDINT_H
#  include <stdint.h>
#else
#  ifdef HAVE_SYS_TYPES_H
#    include <sys/types.h>
#  endif
#endif

}

#include <vector>

namespace otk {

#ifdef HAVE_STDINT_H
typedef uint32_t pixel32;
#else
typedef u_int32_t pixel32;
#endif

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

public:
  TrueRenderControl(int screen);
  virtual ~TrueRenderControl();

  virtual void drawBackground(Surface& sf, const RenderTexture &texture) const;
  virtual void drawGradientBackground(Surface &sf,
                                      const RenderTexture &texture) const;

  inline void highlight(pixel32 *x, pixel32 *y, bool raised) const;
};

}

#endif // __truerendercontrol_hh
