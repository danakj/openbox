#ifndef __mask_h
#define __mask_h

#include <X11/Xlib.h>
#include <glib.h>

typedef struct {
  Pixmap mask;
  guint w, h;
} pixmap_mask;

#endif
