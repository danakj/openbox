#ifndef __moveresize_h
#define __moveresize_h

#include "client.h"

#include <glib.h>

extern gboolean moveresize_in_progress;
extern Client *moveresize_client;

void moveresize_startup();

void moveresize_start(Client *c, int x, int y, guint button, guint32 corner);
void moveresize_end(gboolean cancel);

void moveresize_event(XEvent *e);

#endif
