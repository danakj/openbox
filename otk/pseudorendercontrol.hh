// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __pseudorendercontrol_hh
#define __pseudorendercontrol_hh

#include "rendercontrol.hh"

namespace otk {

class PseudoRenderControl : public RenderControl {
private:
  int _bpc;        // number of bits per color
  int _ncolors;    // number of allocated colors, size of the XColor array
  XColor *_colors; // the valid allocated colors
  
  virtual void reduceDepth(Surface &sf, XImage *im) const;

  const XColor *pickColor(int r, int g, int b) const;
  
public:
  PseudoRenderControl(int screen);
  virtual ~PseudoRenderControl();

  virtual void allocateColor(XColor *color) const;
};

}

#endif // __pseudorendercontrol_hh
