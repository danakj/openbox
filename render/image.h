#ifndef __image_h
#define __image_h

#include "render.h"
#include "geom.h"

void RrImageDraw(RrPixel32 *target, RrTextureRGBA *rgba,
                 gint target_w, gint target_h,
                 RrRect *area);

#endif
