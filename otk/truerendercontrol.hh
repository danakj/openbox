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

public:
  TrueRenderControl(const ScreenInfo *screen);
  virtual ~TrueRenderControl();

  virtual void drawBackground(Surface *sf, const RenderTexture &texture) const;
};

}

#endif // __truerendercontrol_hh
