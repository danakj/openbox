#ifndef __mask_h
#define __mask_h

#include "render.h"
#include "kernel/geom.h"

pixmap_mask *pixmap_mask_new(int w, int h, char *data);
pixmap_mask *pixmap_mask_copy(pixmap_mask *src);
void pixmap_mask_free(pixmap_mask *m);
void mask_draw(Pixmap p, TextureMask *m, Rect *position);

#endif
