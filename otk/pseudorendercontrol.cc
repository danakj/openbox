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
  int depth = info->depth();

  _cpc = 4; // XXX THIS SHOULD BE A USER OPTION
  _ncolors = _cpc * _cpc * _cpc;

  if (_cpc < 2 || _ncolors > 1 << depth) {
    fprintf(stderr,
            _("PseudoRenderControl: Invalid colormap size. Using maximum size
available.\n"));
    _cpc = 1 << (depth/3);
    _ncolors = 1 << depth; // _cpc * _cpc * _cpc
  }

  if (!(_colors = new XColor[_ncolors])) {
    fprintf(stderr,
            _("PseudoRenderControl: error allocating colormap\n"));
    ::exit(1);
  }

  
}

PseudoRenderControl::~PseudoRenderControl()
{
  printf("Destroying PseudoColor RenderControl\n");

  delete _colors;
}

void PseudoRenderControl::reduceDepth(Surface &sf, XImage *im) const
{
}

}
