#ifndef __mask_h
#define __mask_h

#include "render.h"

pixmap_mask *pixmap_mask_new(int w, int h, char *data);
void pixmap_mask_free(pixmap_mask *m);
void mask_draw(Pixmap p, TextureMask *m, int width, int height);

#endif
