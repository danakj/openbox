// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-
#ifndef __pseudorendercontrol_hh
#define __pseudorendercontrol_hh

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

class PseudoRenderControl : public RenderControl {
private:

public:
  PseudoRenderControl(int screen);
  virtual ~PseudoRenderControl();

  virtual void drawBackground(Surface& sf, const RenderTexture &texture) const;
};

}

#endif // __pseudorendercontrol_hh
