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
typedef uint16_t pixel16;
#else
typedef u_int32_t pixel32;
typedef u_int16_t pixel16;
#endif /* HAVE_STDINT_H */

#ifdef WORDS_BIGENDIAN
const int default_red_shift=0;
const int default_green_shift=8;
const int default_blue_shift=16;
const int endian=MSBFirst;
#else
const int default_red_shift=16;
const int default_green_shift=8;
const int default_blue_shift=0;
const int endian=LSBFirst;
#endif /* WORDS_BIGENDIAN */

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

  inline void highlight(pixel32 *x, pixel32 *y, bool raised) const;
  void reduceDepth(XImage *im, pixel32 *data) const;
  void verticalGradient(Surface &sf, const RenderTexture &texture,
                        pixel32 *data) const;
  void diagonalGradient(Surface &sf, const RenderTexture &texture,
                        pixel32 *data) const;
  void crossDiagonalGradient(Surface &sf, const RenderTexture &texture,
                        pixel32 *data) const;
  virtual void drawGradientBackground(Surface &sf,
                                      const RenderTexture &texture) const;
  
public:
  TrueRenderControl(int screen);
  virtual ~TrueRenderControl();

  virtual void drawBackground(Surface& sf, const RenderTexture &texture) const;
};

}

#endif // __truerendercontrol_hh
