// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#include "config.h"

#include "pseudorendercontrol.hh"
#include "display.hh"
#include "screeninfo.hh"
#include "surface.hh"
#include "rendertexture.hh"

extern "C" {
#ifdef    HAVE_STDLIB_H
#  include <stdlib.h>
#endif // HAVE_STDLIB_H

#include "../src/gettext.h"
#define _(str) gettext(str)
}

namespace otk {

PseudoRenderControl::PseudoRenderControl(int screen)
  : RenderControl(screen)
{
  printf("Initializing PseudoColor RenderControl\n");

  const ScreenInfo *info = display->screenInfo(_screen);
}

PseudoRenderControl::~PseudoRenderControl()
{
  printf("Destroying PseudoColor RenderControl\n");
}

void PseudoRenderControl::reduceDepth(Surface &sf, XImage *im) const
{
}

}
