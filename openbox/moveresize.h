#ifndef __moveresize_h
#define __moveresize_h

#include "client.h"

#include <glib.h>

extern gboolean moveresize_in_progress;

void moveresize_startup();

void moveresize_start(Client *c, int x, int y, guint b, guint32 corner);

void moveresize_event(XEvent *e);

#endif
