#ifndef __image_h
#define __image_h

#include "render.h"
#include "../kernel/geom.h"

void image_draw(RrPixel32 *target, RrTextureRGBA *rgba, Rect *area);

#endif
