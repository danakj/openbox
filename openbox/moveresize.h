#ifndef __moveresize_h
#define __moveresize_h

#include "client.h"

#include <glib.h>

extern gboolean moveresize_in_progress;
extern ObClient *moveresize_client;

void moveresize_startup();
void moveresize_shutdown();

void moveresize_start(ObClient *c, int x, int y, guint button, guint32 corner);
void moveresize_end(gboolean cancel);

void moveresize_event(XEvent *e);

#endif
