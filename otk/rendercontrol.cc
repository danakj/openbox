// -*- mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; -*-

#ifdef    HAVE_CONFIG_H
#  include "../config.h"
#endif // HAVE_CONFIG_H

#include "rendercontrol.hh"
#include "truerendercontrol.hh"
#include "rendertexture.hh"
#include "display.hh"
#include "screeninfo.hh"

extern "C" {
#ifdef    HAVE_STDLIB_H
#  include <stdlib.h>
#endif // HAVE_STDLIB_H

#include "gettext.h"
#define _(str) gettext(str)
}

namespace otk {

RenderControl *RenderControl::getRenderControl(int screen)
{
  const ScreenInfo *info = display->screenInfo(screen);

  // get the visual on the screen and return the correct type of RenderControl
  int vclass = info->visual()->c_class;
  switch (vclass) {
  case TrueColor:
    return new TrueRenderControl(info);
  case PseudoColor:
  case StaticColor:
//    return new PseudoRenderControl(info);
  case GrayScale:
  case StaticGray:
//    return new GrayRenderControl(info);
  default:
    printf(_("RenderControl: Unsupported visual %d specified. Aborting.\n"),
	   vclass);
    ::exit(1);
  }
}

RenderControl::RenderControl(const ScreenInfo *screen)
  : _screen(screen)
{
  printf("Initializing RenderControl\n");

  
}

RenderControl::~RenderControl()
{
  printf("Destroying RenderControl\n");


}

}
