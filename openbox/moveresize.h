#ifndef __moveresize_h
#define __moveresize_h

#include <glib.h>

struct _ObClient;

extern gboolean moveresize_in_progress;
extern struct _ObClient *moveresize_client;

void moveresize_startup();
void moveresize_shutdown();

void moveresize_start(struct _ObClient *c,
                      int x, int y, guint button, guint32 corner);
void moveresize_end(gboolean cancel);

void moveresize_event(XEvent *e);

#endif
