#ifndef __mask_h
#define __mask_h

#include <X11/Xlib.h>
#include <glib.h>

typedef struct {
  Pixmap mask;
  guint w, h;
} pixmap_mask;

pixmap_mask *pixmap_mask_new(int w, int h, char *data);
void pixmap_mask_free(pixmap_mask *m);

#endif
