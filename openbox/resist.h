#ifndef ob__resist_h
#define ob__resist_h

struct _ObClient;

#include <glib.h>

void resist_move_windows(struct _ObClient *c, gint *x, gint *y);
void resist_move_monitors(struct _ObClient *c, gint *x, gint *y);
void resist_size_windows(struct _ObClient *c, gint *w, gint *h, ObCorner corn);
void resist_size_monitors(struct _ObClient *c, gint *w, gint *h,ObCorner corn);

#endif
