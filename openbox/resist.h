#ifndef ob__resist_h
#define ob__resist_h

struct _ObClient;

#include <glib.h>

void resist_move(struct _ObClient *c, gint *x, gint *y);
void resist_size(struct _ObClient *c, gint *w, gint *h, ObCorner corn);

#endif
