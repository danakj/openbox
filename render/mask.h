#ifndef __mask_h
#define __mask_h

#include "render.h"
#include "kernel/geom.h"

RrPixmapMask *RrPixmapMaskNew(const RrInstance *inst,
                              gint w, gint h, const gchar *data);
void RrPixmapMaskFree(RrPixmapMask *m);
RrPixmapMask *RrPixmapMaskCopy(const RrPixmapMask *src);
void RrPixmapMaskDraw(Pixmap p, const RrTextureMask *m, const Rect *area);

#endif
