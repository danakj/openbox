#ifndef __image_h
#define __image_h

#include "render.h"
#include "../kernel/geom.h"

void image_draw(pixel32 *target, TextureRGBA *rgba, Rect *position,
                Rect *surarea);

#endif
