// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

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
  const ScreenInfo *info = display->screenInfo(_screen);

  printf("Initializing PseudoColor RenderControl\n");

}

PseudoRenderControl::~PseudoRenderControl()
{
  printf("Destroying PseudoColor RenderControl\n");


}

void PseudoRenderControl::drawGradientBackground(
     Surface &sf, const RenderTexture &texture) const
{
}

void PseudoRenderControl::drawBackground(Surface& sf,
				       const RenderTexture &texture) const
{
  assert(_screen == sf._screen);
  assert(_screen == texture.color().screen());

  if (texture.gradient() == RenderTexture::Solid) {
    drawSolidBackground(sf, texture);
  } else {
    drawGradientBackground(sf, texture);
  }
}

}
